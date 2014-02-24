//
//  State.h
//  SnapShot
//
//  Created by KamCheung Ting on 2/15/14.
//  Copyright (c) 2014 KamCheung Ting. All rights reserved.
//

#ifndef __SnapShot__State__
#define __SnapShot__State__

#include <iostream>
#include <string>
#include <vector>
class State{
public:
    int _widgets;
    int _money;
    unsigned _time;
    std::vector<unsigned> _timeVector;
    State(int money, int wdt);
    State(const State& state);
    virtual std::string toString();
    virtual ~State();
};

class ChannelState: public State{
public:
    unsigned _source;
    unsigned _destination;
    ChannelState(int money, int wdt, unsigned time, std::vector<unsigned>& timeVector, unsigned src, unsigned dest);
    virtual ~ChannelState();
};

#endif /* defined(__SnapShot__State__) */
