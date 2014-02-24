//
//  Thread.h
//  SnapShot
//
//  Created by KamCheung Ting on 2/15/14.
//  Copyright (c) 2014 KamCheung Ting. All rights reserved.
//

#ifndef __SnapShot__Thread__
#define __SnapShot__Thread__
#include <pthread.h>
#include <iostream>
#include "Dealer.h"
#include "Utility.h"
#include "Message.h"

class Dealer;

class ReceiveThread:TCPUtility{
public:
    ReceiveThread(Dealer* parent, int socket);
    int start();
    pthread_t getThread();
protected:
    int _pid;
    int run();
    static void* entryPoint(void* arg);
    virtual void setup();
    virtual void execute();
    int _socket;
    Dealer* _parent;
    pthread_t _thread;
};

class SendThread : TCPUtility{
public:
    SendThread(Dealer* parent, int pid, int socket);
    int start();
    pthread_t getThread();
protected:
    int _pid;
    int run();
    static void* entryPoint(void* arg);
    virtual void setup();
    virtual void execute();
    int _socket;
    Dealer* _parent;
    pthread_t _thread;
};




#endif /* defined(__SnapShot__Thread__) */
