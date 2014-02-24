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
        if(action == InitMessage::INIT_ACTION){
            _parent->registerThread(pid, ip, port);
            _pid = pid;
        }
        std::cout<<"\n[receive] "<<buffer<<std::endl<<std::endl;
    }
    std::cout<<"setup completed."<<std::endl;
};

void ReceiveThread::execute(){
    char header[Message::ACTION_HEADER_SIZE + 1];
    memset(header, 0 , sizeof(header));
    while(true){
        //std::cout << "ReceiveThread execute [start]"<<std::endl;
        try{
        if(tcpRead(_socket, header, Message::ACTION_HEADER_SIZE)){
            unsigned size = 0;
            unsigned action = 0;
            sscanf(header, Message::ACTION_HEADER_PARSE, &action, &size);
            char content[size + 1];
            memset(content, 0 , sizeof(content));
            //std::cout<<"tcp read header finished."<<std::endl;
            if(tcpRead(_socket, content, size)){
                Message* message = new Message(action, _pid, content);
                //std::cout<<"tcp read content finished."<<std::endl;
                _parent->queueInCommingMessage(message);
                //std::cout<<message->toString()<<std::endl;
            }
        }
        }catch(std::exception& e){
            std::cout<<"\n"<<__FILE__<<"@"<<__LINE__<<"[DEBUG]: "<<"!!!!!!!!!!error occurs !!!!!!!!!!!!!!!!!\n";
            std::cout << e.what() << '\n';
            exit(-1);
        }
        //std::cout << "ReceiveThread execute [start]"<<std::endl;
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



SendThread::SendThread(Dealer* parent, int pid, int socket){
    _parent = parent;
    _socket = socket;
    _pid = pid;
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
    _parent->reportReady(_pid, _socket);
    _parent->clearUpCachedMessage(_pid);
    delete message;
    std::cout<<__FILE__<<"@"<<__LINE__<<"[DEBUG]: "<<"socket: "<<_socket<<" send msg: "<<message<<std::endl;
};

void SendThread::execute(){
    while(true){
        //std::cout << "SendThread execute [start]"<<std::endl;
        sleep(SLEEP_TIME);
        try{
        unsigned decision = (rand() % 100);
        if(decision <= 80){
            unsigned money = (rand() % 10)*10;
            unsigned widget = money / 10 ;
            Message* message = new Message();
            message->_action = Message::PURCHASE_ACTION;
            message->_money = money;
            message->_widgets = widget;
            message->_pid = _pid;
            _parent->queueOutGoingMessage(message);
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
        }catch(std::exception& e){
            std::cout<<"\n"<<__FILE__<<"@"<<__LINE__<<"[DEBUG]: "<<"!!!!!!!!!!error occurs !!!!!!!!!!!!!!!!!\n";
            std::cout << e.what() << '\n';
            exit(-1);
        }
        //std::cout << "SendThread execute [end]"<<std::endl;
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