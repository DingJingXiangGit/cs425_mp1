//
//  State.cpp
//  SnapShot
//
//  Created by KamCheung Ting on 2/15/14.
//  Copyright (c) 2014 KamCheung Ting. All rights reserved.
//

#include "State.h"
#include <iostream>
#include <sstream>

State::State(int money, int wdt){
    _widgets = wdt;
    _money = money;
};

std::string State::toString(){
    std::ostringstream stringStream;
    stringStream << "{money: ";
    stringStream << _money;
    stringStream << ", ";
    stringStream << "widgets: ";
    stringStream << _widgets;
    stringStream << "}";
    std::string copyOfStr = stringStream.str();
    return copyOfStr;
};

State::~State(){
};
