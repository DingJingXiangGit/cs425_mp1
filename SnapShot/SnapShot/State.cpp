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
    _time = 0;
    _timeVector = std::vector<unsigned>();
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

State::State(const State& state){
    _time = state._time;
    _money = state._money;
    _widgets = state._widgets;
    _timeVector = std::vector<unsigned>(state._timeVector);
};

State::~State(){
};


ChannelState::ChannelState(int money, int wdt, unsigned time, std::vector<unsigned>& timeVector, unsigned src, unsigned dest):State(money, wdt){
    _money = money;
    _widgets = wdt;
    _time = time;
    _timeVector = std::vector<unsigned>(timeVector);
    _source = src;
    _destination = dest;
    
};
ChannelState::~ChannelState(){

};
