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
    _channelStates = std::map<unsigned, std::list<ChannelState*> >();
};

bool SnapShot::isDone(){
    return (_total) == (_channelStates.size());
};

bool SnapShot::ownMarker(MarkerMessage* marker){
    return (marker->_snapshotId == _snapshotId && marker->_initiator == _initiator);
};

void SnapShot::recordChannelState(Message* message){
    unsigned money = message->_money;
    unsigned widget = message->_widgets;
    unsigned time = message->_time;
    unsigned source = message->_pid;
    unsigned dest = _initiator;
    ChannelState* channelState = new ChannelState(money,widget,time, message->_timeVector,source, dest);
    _channelStates[source].push_back(channelState);
};

void SnapShot::save(){
    
};


SnapShot::~SnapShot(){
    /*
    for (std::map<unsigned, std::list<ChannelState*> >::iterator i = _channelStates.begin(); i != _channelStates.end(); ++i)
    {
        std::list<ChannelState*>& csl = (i->second);
        for (std::list<ChannelState*>::iterator item = csl.begin(); item != csl.end(); ++item) {
            delete *item;
        }
        _channelStates.erase (i);
    }*/
    _channelStates.clear();
    delete _localState;
};