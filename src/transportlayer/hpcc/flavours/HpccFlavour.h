//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#ifndef TRANSPORTLAYER_HPCC_FLAVOURS_HPCCFLAVOUR_H_
#define TRANSPORTLAYER_HPCC_FLAVOURS_HPCCFLAVOUR_H_

#include <inet/transportlayer/tcp/flavours/TcpReno.h>
#include "../../../common/IntTag_m.h"
#include "../HpccConnection.h"
#include "HpccFamily.h"

namespace inet {
namespace tcp {

/**
 * State variables for DcTcp.
 */
typedef HpccFamilyStateVariables HpccStateVariables;

/**
 * Implements DCTCP.
 */
class HpccFlavour : public TcpReno
{
  protected:
    HpccStateVariables *& state;

    static simsignal_t txRateSignal; // will record load
    static simsignal_t tauSignal; // will record total number of RTOs
    static simsignal_t uSignal;
    static simsignal_t USignal;

    simtime_t rtt;
    bool initPackets;
    /** Create and return a HpccStateVariables object. */
    virtual TcpStateVariables *createStateVariables() override
    {
        return new HpccStateVariables();
    }

    virtual void initialize() override;

  public:
    /** Constructor */
    HpccFlavour();

    virtual void established(bool active) override;

    virtual void rttMeasurementComplete(simtime_t tSent, simtime_t tAcked) override;

    virtual void receiveSeqChanged(IntDataVec intData);

    virtual void receivedDataAckInt(uint32_t firstSeqAcked, IntDataVec intData);

    virtual uint32_t computeWnd(double u, bool updateWc);

    virtual double measureInflight(IntDataVec intData);

    virtual simtime_t getRtt();


    };

} // namespace tcp
} // namespace inet

#endif

