//
//  SnapShot.cpp
//  SnapShot
//
//  Created by KamCheung Ting on 2/23/14.
//  Copyright (c) 2014 KamCheung Ting. All rights reserved.
//

#include "SnapShot.h"
#include <stdio.h>
#include <iostream>
#include <fstream>

SnapShot::SnapShot(unsigned initiator, unsigned pid, unsigned sid, State& state, int num){
    _initiator = initiator;
    _snapshotId = sid;
    _pid = pid;
    _localState = new State(state);
    _total = num;
    _channelStates = std::map<unsigned, std::list<ChannelState*> >();
    _counter = 0;
};

bool SnapShot::isDone(){
    //std::cout<<"total is "<<_total<<" counter is "<<_counter<<std::endl;
    return (_total == _counter);
};

bool SnapShot::ownMarker(MarkerMessage* marker){
    return (marker->_snapshotId == _snapshotId && marker->_initiator == _initiator);
};

void SnapShot::recordChannelState(unsigned pid, Message* message){
    unsigned money = message->_money;
    unsigned widget = message->_widgets;
    unsigned time = message->_time;
    unsigned source = message->_pid;
    unsigned dest = pid;
    ChannelState* channelState = new ChannelState(money, widget, time, message->_timeVector, source, dest);
    _channelStates[source].push_back(channelState);
};

void SnapShot::save(unsigned pid){
    _counter += 1;
};

void SnapShot::report(){
    if(_counter == _total){
        std::cout<<serialize()<<std::endl;
    }
};
std::string SnapShot::serialize(){
    std::stringstream ss;
    ss<<"initiator "<<_initiator;
    ss<<" : id "<< _pid;
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
    ss<<" : messages "<<_channelStates.size()<<"\n";
    for (std::map<unsigned, std::list<ChannelState*> >::iterator i = _channelStates.begin(); i != _channelStates.end(); ++i)
    {
        std::list<ChannelState*>& csl = (i->second);
        for (std::list<ChannelState*>::iterator item = csl.begin(); item != csl.end(); ++item) {
            ChannelState& cs = *(*item);
            ss <<"id " << _pid <<" : snapshot "<< _snapshotId << " : vector [";
            for (int i = 0; i < cs._timeVector.size(); ++i) {
                ss<< cs._timeVector[i]<<", ";
            }
            ss<<"]";
            ss << " : message " << cs._source <<" to "<<cs._destination << " : money "<< cs._money<<" widget "<<cs._widgets<<"\n";
        }
    }
    return ss.str();
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




void SnapShotGroup::add(SnapShot* entry){
    _snapshots.push_back(entry);
};

void SnapShotGroup::save(std::string& dest){
    std::ofstream outputFile;
    outputFile.open(dest.c_str());
    for (std::list<SnapShot*>::iterator it = _snapshots.begin(); it != _snapshots.end(); it++){
        outputFile<<(*it)->serialize()<<std::endl;
    }
    outputFile.close();
};
