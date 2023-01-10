//
// Copyright (C) 2020 Marcel Marek
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "HpccFlavour.h"

#include <algorithm> // min,max

#include "inet/transportlayer/tcp/Tcp.h"

namespace inet {
namespace tcp {

#define MIN_REXMIT_TIMEOUT     1.0   // 1s
#define MAX_REXMIT_TIMEOUT     240   // 2 * MSL (RFC 1122)

Register_Class(HpccFlavour);

simsignal_t HpccFlavour::txRateSignal = cComponent::registerSignal("txRate");
simsignal_t HpccFlavour::tauSignal = cComponent::registerSignal("tau");
simsignal_t HpccFlavour::uSignal = cComponent::registerSignal("u");
simsignal_t HpccFlavour::USignal = cComponent::registerSignal("U");

HpccFlavour::HpccFlavour() : TcpReno(),
    state((HpccStateVariables *&)TcpAlgorithm::state)
{
}

void HpccFlavour::initialize()
{
    TcpReno::initialize();
    state->B = conn->getTcpMain()->par("bandwidth");
    state->T = conn->getTcpMain()->par("basePropagationRTT");
    state->u = 0;
    //TODO add Par for number of N. Currently is 10 meaning 10 flows. Look at paper for wAI
    state->additiveIncrease = ((state->B * state->T.dbl())*(1-state->eta))/10;
    std::cout << "\n additiveIncrease factor: " << state->additiveIncrease << endl;
    //state->prevWnd = state->B * state->T.dbl();
    state->prevWnd = 10000;
}

void HpccFlavour::established(bool active)
{
    //state->snd_cwnd = state->B * state->T.dbl();
    state->snd_cwnd = 10000;
    initPackets = true;
    dynamic_cast<HpccConnection*>(conn)->changeIntersendingTime(state->T.dbl()/(double) state->snd_cwnd);
    EV_DETAIL << "HPCC initial CWND is set to " << state->snd_cwnd << "\n";
    if (active) {
        // finish connection setup with ACK (possibly piggybacked on data)
        EV_INFO << "Completing connection setup by sending ACK (possibly piggybacked on data)\n";
        if (!sendData(false)) // FIXME - This condition is never true because the buffer is empty (at this time) therefore the first ACK is never piggyback on data
            conn->sendAck();
    }
}

void HpccFlavour::receiveSeqChanged(IntDataVec intData)
{
    // If we send a data segment already (with the updated seqNo) there is no need to send an additional ACK
    if (state->full_sized_segment_counter == 0 && !state->ack_now && state->last_ack_sent == state->rcv_nxt && !delayedAckTimer->isScheduled()) { // ackSent?
//        tcpEV << "ACK has already been sent (possibly piggybacked on data)\n";
    }
    else {
        // RFC 2581, page 6:
        // "3.2 Fast Retransmit/Fast Recovery
        // (...)
        // In addition, a TCP receiver SHOULD send an immediate ACK
        // when the incoming segment fills in all or part of a gap in the
        // sequence space."
        if (state->lossRecovery)
            state->ack_now = true; // although not mentioned in [Stevens, W.R.: TCP/IP Illustrated, Volume 2, page 861] seems like we have to set ack_now

        if (!state->delayed_acks_enabled) { // delayed ACK disabled
            EV_INFO << "rcv_nxt changed to " << state->rcv_nxt << ", (delayed ACK disabled) sending ACK now\n";
            dynamic_cast<HpccConnection*>(conn)->sendIntAck(intData);
        }
        else { // delayed ACK enabled
            if (state->ack_now) {
                EV_INFO << "rcv_nxt changed to " << state->rcv_nxt << ", (delayed ACK enabled, but ack_now is set) sending ACK now\n";
                dynamic_cast<HpccConnection*>(conn)->sendIntAck(intData);
            }
            // RFC 1122, page 96: "in a stream of full-sized segments there SHOULD be an ACK for at least every second segment."
            else if (state->full_sized_segment_counter >= 2) {
                EV_INFO << "rcv_nxt changed to " << state->rcv_nxt << ", (delayed ACK enabled, but full_sized_segment_counter=" << state->full_sized_segment_counter << ") sending ACK now\n";
                dynamic_cast<HpccConnection*>(conn)->sendIntAck(intData);
            }
            else {
                EV_INFO << "rcv_nxt changed to " << state->rcv_nxt << ", (delayed ACK enabled and full_sized_segment_counter=" << state->full_sized_segment_counter << ") scheduling ACK\n";
                if (!delayedAckTimer->isScheduled()) // schedule delayed ACK timer if not already running
                    conn->scheduleAfter(0.2, delayedAckTimer); //TODO 0.2 s is default delayed ack timout, potentially increase for higher BDP
            }
        }
    }
}

void HpccFlavour::rttMeasurementComplete(simtime_t tSent, simtime_t tAcked)
{
    //
    // Jacobson's algorithm for estimating RTT and adaptively setting RTO.
    //
    // Note: this implementation calculates in doubles. An impl. which uses
    // 500ms ticks is available from old tcpmodule.cc:calcRetransTimer().
    //

    // update smoothed RTT estimate (srtt) and variance (rttvar)
    const double g = 0.125; // 1 / 8; (1 - alpha) where alpha == 7 / 8;
    simtime_t newRTT = tAcked - tSent;

    simtime_t& srtt = state->srtt;
    simtime_t& rttvar = state->rttvar;

    simtime_t err = newRTT - srtt;

    srtt += g * err;
    rttvar += g * (fabs(err) - rttvar);

    // assign RTO (here: rexmit_timeout) a new value
    simtime_t rto = srtt + 4 * rttvar;

    if (rto > MAX_REXMIT_TIMEOUT)
        rto = MAX_REXMIT_TIMEOUT;
    else if (rto < MIN_REXMIT_TIMEOUT)
        rto = MIN_REXMIT_TIMEOUT;

    state->rexmit_timeout = rto;

    // record statistics
    EV_DETAIL << "Measured RTT=" << (newRTT * 1000) << "ms, updated SRTT=" << (srtt * 1000)
              << "ms, new RTO=" << (rto * 1000) << "ms\n";

    rtt = newRTT;
    conn->emit(rttSignal, newRTT);
    conn->emit(srttSignal, srtt);
    conn->emit(rttvarSignal, rttvar);
    conn->emit(rtoSignal, rto);
}

void HpccFlavour::receivedDataAckInt(uint32_t firstSeqAcked, IntDataVec intData)
{
    EV_INFO << "\nHPCCInfo ___________________________________________" << endl;
    EV_INFO << "\nHPCCInfo - Received Data Ack" << endl;
    TcpTahoeRenoFamily::receivedDataAck(firstSeqAcked);

//    std::cout << "\nInt Data size: " << intData.size() << endl;
//    for(int i = 0; i < intData.size(); i++){
//        IntMetaData* intDataEntry = intData.front();
//        std::cout << "\nData Ack has INT data!" << endl;
//        std::cout << "Queue Length: " << intDataEntry->getQLen() << endl;
//        std::cout << "Timestamp: " << intDataEntry->getTs() << endl;
//    }
//    std::cout << "\nMeasuring inflight u: " << measureInflight(intData) << endl;

    if (state->dupacks >= state->dupthresh) {
        //
        // Perform Fast Recovery: set cwnd to ssthresh (deflating the window).
        //
        EV_INFO << "Fast Recovery: setting cwnd to ssthresh=" << state->ssthresh << "\n";
        state->snd_cwnd = state->ssthresh;
        conn->emit(cwndSignal, state->snd_cwnd);
    }
    else {
        if(firstSeqAcked > state->lastUpdateSeq) {
            //std::cout << "\n firstSeqAcked: " << firstSeqAcked << endl;
            //std::cout << "\n state->lastUpdateSeq: " << state->lastUpdateSeq << endl;
            double uVal = measureInflight(intData);
            if(uVal > 0){
                state->snd_cwnd = computeWnd(uVal, true);
                state->ssthresh = state->snd_cwnd / 2;
            }
            conn->emit(cwndSignal, state->snd_cwnd);
            state->lastUpdateSeq = state->snd_nxt;
        }
        else {
            double uVal = measureInflight(intData);
            if(uVal > 0){
                state->snd_cwnd = computeWnd(uVal, false);
                state->ssthresh = state->snd_cwnd / 2;
            }
            conn->emit(cwndSignal, state->snd_cwnd);
        }
    }

    state->L = intData;
//    std::cout << "\n Setting Intersending Time..." << endl;
//    std::cout << "\n state->T" << state->T.dbl() << endl;
//    std::cout << "\n state->snd_cwnd" << state->snd_cwnd;
//    std::cout << "\n state->u" << state->u;
    dynamic_cast<HpccConnection*>(conn)->changeIntersendingTime(state->T.dbl()/((double) state->snd_cwnd/1460));

    if (state->sack_enabled && state->lossRecovery) {
            // RFC 3517, page 7: "Once a TCP is in the loss recovery phase the following procedure MUST
            // be used for each arriving ACK:
            //
            // (A) An incoming cumulative ACK for a sequence number greater than
            // RecoveryPoint signals the end of loss recovery and the loss
            // recovery phase MUST be terminated.  Any information contained in
            // the scoreboard for sequence numbers greater than the new value of
            // HighACK SHOULD NOT be cleared when leaving the loss recovery
            // phase."
            if (seqGE(state->snd_una, state->recoveryPoint)) {
                EV_INFO << "Loss Recovery terminated.\n";
                state->lossRecovery = false;
            }
            // RFC 3517, page 7: "(B) Upon receipt of an ACK that does not cover RecoveryPoint the
            // following actions MUST be taken:
            // (B.1) Use Update () to record the new SACK information conveyed
            // by the incoming ACK.
            //
            // (B.2) Use SetPipe () to re-calculate the number of octets still
            // in the network."
            else {
                // update of scoreboard (B.1) has already be done in readHeaderOptions()
                conn->setPipe();

                // RFC 3517, page 7: "(C) If cwnd - pipe >= 1 SMSS the sender SHOULD transmit one or more
                // segments as follows:"
                if (((int)state->snd_cwnd - (int)state->pipe) >= (int)state->snd_mss) // Note: Typecast needed to avoid prohibited transmissions
                    conn->sendDataDuringLossRecoveryPhase(state->snd_cwnd);
            }
        }
        // RFC 3517, pages 7 and 8: "5.1 Retransmission Timeouts
        // (...)
        // If there are segments missing from the receiver's buffer following
        // processing of the retransmitted segment, the corresponding ACK will
        // contain SACK information.  In this case, a TCP sender SHOULD use this
        // SACK information when determining what data should be sent in each
        // segment of the slow start.  The exact algorithm for this selection is
        // not specified in this document (specifically NextSeg () is
        // inappropriate during slow start after an RTO).  A relatively
        // straightforward approach to "filling in" the sequence space reported
        // as missing should be a reasonable approach."
        sendData(false);
}

double HpccFlavour::measureInflight(IntDataVec intData)
{
    double u = 0;
    double tau;
    for(int i = 0; i < intData.size(); i++){ //Start at front of queue. First item is first hop etc.
        double uPrime = 0;
        IntMetaData* intDataEntry = intData.at(i);

        if(state->L.size() == intData.size()){ //TODO replace with check to ensure the hops are the same, maybe hopID? Look at paper/rfc
            std::cout << "\n average RTT: " << intDataEntry->getAverageRtt() << endl;
            if(intDataEntry->getAverageRtt() > 0) {
                initPackets = false;
            }
            else{
                return 0;
            }

            if(!initPackets){
                state->txRate = (intDataEntry->getTxBytes() - state->L.at(i)->getTxBytes())/(intDataEntry->getTs().dbl() - state->L.at(i)->getTs().dbl());
                //std::cout << "\n state->txRate: " << state->txRate << endl;
                //std::cout << "\n intDataEntry->getB(): " << intDataEntry->getB() << endl;
                uPrime = ((std::min(intDataEntry->getQLen(), state->L.at(i)->getQLen()))/(intDataEntry->getB()*state->T.dbl()))+(state->txRate/intDataEntry->getB());
    //            std::cout << "\n Part 1: " << ((std::min(intDataEntry->getQLen(), state->L.at(i)->getQLen()))/(intDataEntry->getB()*state->T.dbl())) << endl;
    //            std::cout << "\n state->L.at(i)->getQLen()" << state->L.at(i)->getQLen() << endl;
    //            std::cout << "\n state->T.dbl()" << state->T.dbl() << endl;
    //            std::cout << "\n Part 2: " << (state->txRate/intDataEntry->getB()) << endl;
    //            std::cout << "\n Hop: " << i << endl;
    //            std::cout << "\n Hop Name: " << intDataEntry->getHopName() << endl;
    //            std::cout << "\n u Part 2: " << (state->txRate/intDataEntry->getB()) << endl;
    //            std::cout << "\n intDataEntry->getB(): " << intDataEntry->getB() << endl;
                if(uPrime > u) {
                    u = uPrime;
                    tau = intDataEntry->getTs().dbl() - state->L.at(i)->getTs().dbl();
                    //std::cout << "\n intDataEntry->getTs(): " << intDataEntry->getTs() << endl;
                    //std::cout << "\n state->L.at(i)->getTs(): " << state->L.at(i)->getTs() << endl;
                }
            }
        }
        else{
            if(intDataEntry->getAverageRtt() > 0) {
                initPackets = false;
            }
            else{
                return 0;
            }

            state->txRate = intDataEntry->getTxBytes()/intDataEntry->getTs().dbl();
            uPrime = (intDataEntry->getQLen()/(intDataEntry->getB()*state->T.dbl()))+(state->txRate/intDataEntry->getB());
           // std::cout << "\n intDataEntry->getQLen(): " << intDataEntry->getQLen() << endl;
            if(uPrime > u) {
                u = uPrime;
                tau = intDataEntry->getTs().dbl();
            }
        }
    }
    conn->emit(txRateSignal, state->txRate);
    conn->emit(uSignal, u);
    //std::cout << "\n initial u val: " << u << endl;
    tau = std::min(tau, state->T.dbl());
    conn->emit(tauSignal, tau);
//    std::cout << "\n Tau: " << tau << endl;
//    std::cout << "\n state->T: " << state->T.dbl() << endl;

    state->u = (1-(tau/state->T.dbl()))*state->u+(tau/state->T.dbl())*u;
    conn->emit(USignal, state->u);
    return state->u;
}

uint32_t HpccFlavour::computeWnd(double u, bool updateWc)
{
    uint32_t w;
    //std::cout << "\n\n computeWnd..." << endl;
    //std::cout << "\n u value: " << u << endl;
    if(u >= state->eta || state->incStage >= state->maxStage) {
        w = (state->prevWnd/(u/state->eta))+state->additiveIncrease;
        if(updateWc) {
            state->incStage = 0;
            state->prevWnd = w;
        }
    }
    else {
        w = state->prevWnd + state->additiveIncrease;
        //std::cout << "\n Updating with just additive increase " << state->additiveIncrease << " to " << w << " at sim time: " << simTime() << endl;
        if(updateWc) {
            state->incStage++;
            state->prevWnd = w;
        }
    }
    //std::cout << "\n state->txRate: " << state->txRate << endl;
    //std::cout << "\n state->u: " << state->u << endl;
    //std::cout << "\n state->eta: " << state->eta << endl;
   // std::cout << "\n additiveIncrease val: " << state->additiveIncrease << endl;
    //std::cout << "\n computeWnd result: " << w << endl;
    return w;
}

simtime_t HpccFlavour::getRtt()
{
    return rtt;
}

} // namespace tcp
} // namespace inet

