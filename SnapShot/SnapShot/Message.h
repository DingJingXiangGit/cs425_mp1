//
//  Message.h
//  SnapShot
//
//  Created by KamCheung Ting on 2/20/14.
//  Copyright (c) 2014 KamCheung Ting. All rights reserved.
//

#ifndef __SnapShot__Message__
#define __SnapShot__Message__

#include <iostream>
#include <vector>

class AbstractMessage{
public:
    static const unsigned ACTION_HEADER_SIZE = 32;
    static const char* ACK_MSG_FORMAT;
    static const char* ACK_MSG_PARSE;
    static const char* ACTION_HEADER_FORMAT;
    static const char* ACTION_HEADER_PARSE;
    static const char* ACTION_MSG_FORMAT;
    static const char* ACTION_MSG_PARSE;
    static const char* ACTION_MARKER_PARSE;
    static const char* ACTION_MARKER;
    
    //static const int PURCHASE_ACTION = 1;
    static const int DELIVERY_ACTION = 1;
    static const int MARKER_ACTION = 2;
    static const int TIME_VECTOR_LENGTH = 256;
    AbstractMessage();
    int _action;
    unsigned _time;
    std::vector<unsigned> _timeVector;
    int _pid;
    
    virtual std::string toString() = 0;
    virtual char* toCharArray() = 0;
};

class Message : public AbstractMessage{
    public:
        unsigned _money;
        unsigned _widgets;
        Message(unsigned action, int pid, char* msg);
        Message(const Message& other );
        Message();
        virtual std::string toString();
        virtual char* toCharArray();
};

class MarkerMessage : public AbstractMessage{
public:
    unsigned _snapshotId;
    unsigned _initiator;
    MarkerMessage();
    MarkerMessage(unsigned action, int pid, char* msg);
    MarkerMessage(const MarkerMessage& other );
    virtual std::string toString();
    virtual char* toCharArray();
};

class InitMessage{
public:
    static const char* INIT_MESSAGE_FORMAT;
    static const char* INIT_HEADER_FORMAT;
    static const char* INIT_MESSAGE_PARSE;
    static const int INIT_ACTION = 0;
    static const unsigned INIT_MESSAGE_SIZE = 72;
    static char* toCharArray(int pid, const char* ip, int port);
};

#endif /* defined(__SnapShot__Message__) */
