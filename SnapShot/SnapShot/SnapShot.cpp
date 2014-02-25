//
//  SnapShot.cpp
//  SnapShot
//
//  Created by KamCheung Ting on 2/23/14.
//  Copyright (c) 2014 KamCheung Ting. All rights reserved.
//

#include "SnapShot.h"
#include <stdio.h>
SnapShot::SnapShot(unsigned initiator, unsigned sid, State& state, int num){
    _initiator = initiator;
    _snapshotId = sid;
    _localState = new State(state);
    _total = num;
    _channelStates = std::map<unsigned, std::list<ChannelState*> >();
    _counter = 0;
};

bool SnapShot::isDone(){
    std::cout<<"total is "<<_total<<std::endl;
    std::cout<<"counter is "<<_counter<<std::endl;
    return (_total == _counter);
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
    std::cout<<"logical "<<time<<" record channel state: "<<money <<", "<<widget << " from "<<source<<" to "<<dest<<std::endl;
};

void SnapShot::save(unsigned pid){
    _counter += 1;
};

void SnapShot::report(){
    if(_counter == _total){
        std::stringstream ss;
        ss<<"id "<<_initiator;
        ss<<" : snapshot "<<_snapshotId;
        ss<<" : logical "<<_localState->_time;
        ss<<" : vector ";
        ss<<"[";
        for (int i = 0; i < _localState->_timeVector.size(); ++i) {
            ss<<_localState->_timeVector[i]<<", ";
        }
        ss<<"]";
        ss<<" : money "<<_localState->_money;
        ss<<" : widgets "<<_localState->_widgets<<"\n";
        
        for (std::map<unsigned, std::list<ChannelState*> >::iterator i = _channelStates.begin(); i != _channelStates.end(); ++i)
        {
            std::list<ChannelState*>& csl = (i->second);
            for (std::list<ChannelState*>::iterator item = csl.begin(); item != csl.end(); ++item) {
                ChannelState& cs = *(*item);
                ss << "message from "<<cs._source <<" to "<<cs._destination << "money "<< cs._money<<" widget "<<cs._widgets<<"\n";
            }
        }
        std::cout<<ss.str()<<std::endl;
    }
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