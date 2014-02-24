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

#endif /* defined(__SnapShot__State__) */
