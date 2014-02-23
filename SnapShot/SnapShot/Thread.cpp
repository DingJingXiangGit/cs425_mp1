//
//  Thread.cpp
//  SnapShot
//
//  Created by KamCheung Ting on 2/15/14.
//  Copyright (c) 2014 KamCheung Ting. All rights reserved.
//
#include "Thread.h"

ReceiveThread::ReceiveThread(Dealer* parent, int socket){
    _parent = parent;
    _socket = socket;
};

int ReceiveThread::start(){
    int ret = pthread_create(&_thread, NULL, ReceiveThread::entryPoint, (void*) this);
    return ret;
};

void* ReceiveThread::entryPoint(void* ptr){
    ReceiveThread * pthis = (ReceiveThread*)ptr;
    pthis->run();
    return NULL;
};

void ReceiveThread::setup(){
    char buffer[InitMessage::INIT_MESSAGE_SIZE + 1];
    if(tcpRead(_socket, buffer, InitMessage::INIT_MESSAGE_SIZE)){
        char ip[15];
        int action, pid, port, size;;
        sscanf(buffer, InitMessage::INIT_MESSAGE_PARSE, &action, &size, &pid, ip, &port);
        std::cout<<__FILE__<<"@"<<__LINE__<<"[DEBUG]: action: " << action << " size: " << size << " id: " << pid << " ip: "<<ip <<":"<<port<<std::endl;
        if(action == InitMessage::INIT_ACTION){
            _parent->registerThread(pid, ip, port);
            _pid = pid;
        }
    }

};

void ReceiveThread::execute(){
    char header[Message::ACTION_HEADER_SIZE + 1];
    memset(header, 0 , sizeof(header));
    while(true){
        std::cout << "start receiving message"<<std::endl;
        if(tcpRead(_socket, header, Message::ACTION_HEADER_SIZE)){
            unsigned size = 0;
            unsigned action = 0;
            std::cout << header;
            sscanf(header, Message::ACTION_HEADER_PARSE, &action, &size);
            char content[size + 1];
            memset(content, 0 , sizeof(content));
            if(tcpRead(_socket, content, size)){
                Message message(action, content);
                std::cout << content<<std::endl;
                std::cout << message.toString()<<std::endl;
                sleep(2);
                _parent->execute(_pid, message);
                std::cout<<header<<content<<std::endl;
            }
        }
        std::cout << "end receiving message"<<std::endl;
    }
};

int ReceiveThread::run(){
    setup();
    execute();
    return 1;
};

pthread_t ReceiveThread::getThread(){
    return _thread;
};



SendThread::SendThread(Dealer* parent, int socket){
    _parent = parent;
    _socket = socket;
};

int SendThread::start(){
    int ret = pthread_create(&_thread, NULL, SendThread::entryPoint, (void*) this);
    return ret;
};

void* SendThread::entryPoint(void* ptr){
    SendThread * pthis = (SendThread*)ptr;
    pthis->run();
    return NULL;
};

void SendThread::setup(){
    Peer* selfInfo = _parent->getSelfInfo();
    char* message = InitMessage::toCharArray(selfInfo->_id, selfInfo->_ip.c_str(), selfInfo->_port);
    tcpWrite(_socket, message, (int)strlen(message));
    std::cout<<__FILE__<<"@"<<__LINE__<<"[DEBUG]: "<<"socket: "<<_socket<<" send msg: "<<message<<std::endl;
    delete message;
};

void SendThread::execute(){
    while(true){
        std::cout <<"start sleep "<<std::endl;
        sleep(SLEEP_TIME);
        unsigned decision = (rand() % 100);
        std::cout <<"decision "<<decision<<std::endl;
        if(decision <= 80){
            //PURCHASE action
            unsigned money = (rand() % 10)*10;
            unsigned widget = money / 10 ;
            Message message;
            message._action = Message::PURCHASE_ACTION;
            message._money = money;
            message._widgets = widget;
            _parent->sendMessage(_pid, message);
        }else{
            //MARKER
            /*
            Message message;
            message._action = Message::MARKER_ACTION;
            message._money = 0;
            message._widgets = 0;
            _parent->sendMessage(_pid, message);
             */
        }
    }
};

int SendThread::run(){
    setup();
    execute();
    return 1;
};

pthread_t SendThread::getThread(){
    return _thread;
};