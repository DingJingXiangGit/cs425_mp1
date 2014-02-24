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
#include <map>
#include <string>
#include "State.h"



class SnapShot{
private:
    State* _localState;
    std::map<unsigned, ChannelState*>* _channelStates;
    unsigned _snapshotId;
    unsigned _initiator;
    unsigned _total;
public:
    SnapShot(unsigned initiator, unsigned sid, State& state, int num);
    ~SnapShot();
};


#endif /* defined(__SnapShot__SnapShot__) */
