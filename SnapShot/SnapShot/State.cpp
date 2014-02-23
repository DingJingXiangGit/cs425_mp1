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

ProcessState::ProcessState(int money, int wdt, int pid): State(money, wdt){
    _widgets = wdt;
    _money = money;
    _processId = pid;
};

std::string ProcessState::toString(){
    std::ostringstream stringStream;
    stringStream << "{money: ";
    stringStream << _money;
    stringStream << ", ";
    stringStream << "widgets: ";
    stringStream << _widgets;
    stringStream << ", ";
    stringStream << "process id: ";
    stringStream << _processId;
    stringStream << "}";
    std::string copyOfStr = stringStream.str();
    return copyOfStr;
};

ProcessState::~ProcessState(){
    
};



ChannelState::ChannelState(int money, int wdt, int src, int tgt):State(money, wdt){
    _widgets = wdt;
    _money = money;
    _source = src;
    _target = tgt;
};

std::string ChannelState::toString(){
    
    std::ostringstream stringStream;
    stringStream << "{money: ";
    stringStream << _money;
    stringStream << ", ";
    stringStream << "widgets: ";
    stringStream << _widgets;
    stringStream << ", ";
    stringStream << "source: ";
    stringStream << _source;
    stringStream << ", ";
    stringStream << "target: ";
    stringStream << _target;
    stringStream << "}";
    std::string copyOfStr = stringStream.str();
    return copyOfStr;
};

ChannelState::~ChannelState(){
    
};