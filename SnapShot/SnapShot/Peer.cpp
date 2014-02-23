//
//  Peer.cpp
//  SnapShot
//
//  Created by KamCheung Ting on 2/15/14.
//  Copyright (c) 2014 KamCheung Ting. All rights reserved.
//

#include "Peer.h"
using namespace std;
Peer::Peer(int pid, std::string ip, int port){
    _id = pid;
    _ip = ip;
    _port = port;
    _inState = new State(0, 0);
    _outState = new State(0, 0);
};

string Peer::toString(){
    std::ostringstream stringStream;
    stringStream << "{peer id: ";
    stringStream << _id;
    stringStream << ", ";
    stringStream << "ip: ";
    stringStream << _ip;
    stringStream << ", ";
    stringStream << "port: ";
    stringStream << _port;
    stringStream << ", ";
    stringStream << "out state: ";
    stringStream << _outState->toString();
    stringStream << ", ";
    stringStream << "in state: ";
    stringStream << _inState->toString();
    stringStream << "}";
    std::string copyOfStr = stringStream.str();
    return copyOfStr;

};

map<int, Peer*>* PeerParser::parsePeers(const char* file, int selfId){
    ifstream infile;
    infile.open (file);
    char line[256];
    int peerId;
    int peerPort;
    char ip[16];
    map<int, Peer*>* peerTable = new map<int, Peer*>();
    
    while(infile.eof() == false){
        infile.getline(line, 256);
        sscanf(line, "id:%d, ip:%15[^,], port:%d", &peerId, ip, &peerPort);
        (*peerTable)[peerId] = new Peer(peerId, std::string(ip), peerPort);
    }
    infile.close();
    return peerTable;
};