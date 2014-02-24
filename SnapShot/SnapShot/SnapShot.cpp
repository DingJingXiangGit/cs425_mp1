//
//  SnapShot.cpp
//  SnapShot
//
//  Created by KamCheung Ting on 2/23/14.
//  Copyright (c) 2014 KamCheung Ting. All rights reserved.
//

#include "SnapShot.h"
SnapShot::SnapShot(unsigned initiator, unsigned sid, State& state, int num){
    _initiator = initiator;
    _snapshotId = sid;
    _localState = new State(state);
    _total = num;
    _channelStates = new std::map<unsigned, ChannelState*>();
};

SnapShot::~SnapShot(){
    
    delete _localState;
    delete _channelStates;
};