//
//  SnapShot.h
//  SnapShot
//
//  Created by KamCheung Ting on 2/23/14.
//  Copyright (c) 2014 KamCheung Ting. All rights reserved.
//

#ifndef __SnapShot__SnapShot__
#define __SnapShot__SnapShot__

#include <iostream>
#include <sstream>
#include <map>
#include <list>
#include <string>
#include "State.h"
#include "Message.h"



class SnapShot{
private:
    State* _localState;
    std::map<unsigned, std::list<ChannelState*> > _channelStates;
    unsigned _counter;
    unsigned _snapshotId;
    unsigned _initiator;
    unsigned _pid;
    unsigned _total;
    
public:
    SnapShot(unsigned initiator, unsigned pid, unsigned sid, State& state, int num);
    bool isDone();
    bool ownMarker(MarkerMessage* marker);
    void save(unsigned pid);
    void report();
    void recordChannelState(Message* message);
    std::string serialize();
    ~SnapShot();
};

class SnapShotGroup{
private:
    std::list<SnapShot*> _snapshots;
public:
    void add(SnapShot* entry);
    void save(std::string& dest);
};


#endif /* defined(__SnapShot__SnapShot__) */
