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
#include <string.h>


class State{

public:
    int _widgets;
    int _money;
    State(int money, int wdt);
    virtual std::string toString();
    virtual ~State();
};

class ProcessState : public State{
protected:
    int _processId;
public:
    ProcessState(int money, int wdt, int pid);
    
    virtual std::string toString();
    virtual ~ProcessState();
};

class ChannelState : public State{
protected:
    int _source;
    int _target;
public:
    ChannelState(int money, int wdt, int src, int tgt);
    virtual std::string toString();
    virtual ~ChannelState();
};


#endif /* defined(__SnapShot__State__) */
