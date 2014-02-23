//
//  main.cpp
//  SnapShot
//
//  Created by KamCheung Ting on 2/15/14.
//  Copyright (c) 2014 KamCheung Ting. All rights reserved.
//

#include <iostream>
#include <map>
#include "Peer.h"
#include "Dealer.h"
using namespace std;
int main(int argc, const char* argv[])
{

    // insert code here...
    //"/Users/Virtue/Courses/CS425/cs425_mp1/peer_list"

    int pid = atoi(argv[1]);
    const char* peerFile = argv[2];
    std::cout << "Process ID is " << pid << endl;
    std::cout << "Peer Information File is " << peerFile << endl;
    map<int, Peer*>* peers = PeerParser::parsePeers(peerFile, 1);
    for(map<int, Peer*>::iterator iterator = peers->begin(); iterator != peers->end(); iterator++) {
        cout<<iterator->second->toString()<<endl;
    }
    Peer* self = (*peers)[pid];
    peers->erase(pid);
    Dealer* dealer = new Dealer(self, peers);
    dealer->startListen();
    dealer->startConnect();
    dealer->join();
    delete peers;
    return 0;
}

