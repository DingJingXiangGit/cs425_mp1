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
            //_parent->registerThread(pid, ip, port);
            _pid = pid;
        }
        //std::cout<<"\n[receive] "<<buffer<<std::endl<<std::endl;
    }
    std::cout<<"setup completed."<<std::endl;
};

void ReceiveThread::execute(){
    char header[AbstractMessage::ACTION_HEADER_SIZE + 1];
    memset(header, 0 , sizeof(header));
    while(true){
        try{

            if(tcpRead(_socket, header, AbstractMessage::ACTION_HEADER_SIZE)){
                unsigned size = 0;
                unsigned action = 0;
                sscanf(header, AbstractMessage::ACTION_HEADER_PARSE, &action, &size);
                char content[size + 1];
                memset(content, 0 , sizeof(content));
                if(tcpRead(_socket, content, size)){
                    AbstractMessage* message = NULL;
                    if(action == AbstractMessage::DELIVERY_ACTION){
                        message = new Message(action, _pid, content);
                    }else if(action == AbstractMessage::MARKER_ACTION){
                        message = new MarkerMessage(action, _pid, content);
                    }
                    
                    if(message == NULL){
                        std::cout<<"null content is: "<<content<<std::endl;
                        exit(-1);
                    }
                    _parent->queueInCommingMessage(message);
                }else{
                    shutdown(_socket, 2);
                    close(_socket);
                    return;
                }
                memset(content, 0 , sizeof(content));
            }else{
                shutdown(_socket, 2);
                close(_socket);
                return;
            }
            memset(header, 0 , sizeof(header));
        }catch(std::exception& e){
            std::cout<<"\n"<<__FILE__<<"@"<<__LINE__<<"[DEBUG]: "<<"!!!!!!!!!!error occurs !!!!!!!!!!!!!!!!!\n";
            std::cout << e.what() << '\n';
            exit(-1);
        }
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