//
// Copyright (C) 2013 Maria Fernandez, Carlos Calafate, Juan-Carlos Cano and Pietro Manzoni
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.transportlayer.tcp.flavours.TcpTahoeRenoFamilyState;

cplusplus{{
    #include "../../../common/IntTag_m.h"
}}

namespace inet::tcp;

///
/// State variables for TcpVegas.
///
struct HpccFamilyStateVariables extends TcpTahoeRenoFamilyStateVariables
{
    @descriptor(readonly);
    
    int B; //bandwidth
    simtime_t T = 0; //base propagation RTT
    double txRate;
    double normalisedInflight;
    double normalisedInflightPrev;
    double u = 1;
    double eta = 0.95;  //0.95
    int incStage = 0;
    int maxStage = 500;
    uint32_t prevWnd;
    uint32_t additiveIncrease;
    
    uint32_t lastUpdateSeq;
    
    double R; //pacing rate
    
    int subFlows = 1;
    int sharingFlows = 2;
};

cplusplus(HpccFamilyStateVariables) {{
  	std::vector<IntMetaData*> L;
  public:
    virtual std::string str() const override;
    virtual std::string detailedInfo() const override;
}}

