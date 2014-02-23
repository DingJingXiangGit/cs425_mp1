//
//  Peer.h
//  SnapShot
//
//  Created by KamCheung Ting on 2/15/14.
//  Copyright (c) 2014 KamCheung Ting. All rights reserved.
//

#ifndef __SnapShot__Peer__
#define __SnapShot__Peer__
#include "State.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>


class Peer{
public:
    int _id;
    std::string _ip;
    int _port;
    State* _inState;
    State* _outState;
    Peer(int pid, std::string ip, int port);
    std::string toString();
};

class PeerParser{
public:
    static std::map<int, Peer*>* parsePeers(const char* file, int selfId);
};
#endif /* defined(__SnapShot__Peer__) */
