#include "Dealer.h"
#include<exception>
#include <errno.h>

Dealer::Dealer(Peer* selfInfo, PeerTable* peerTable){
    _selfInfo = selfInfo;
    _peerTable = peerTable;
    _state = new State(DEFAULT_MONEY, DEFAULT_WIDGETS);
    _state->_timeVector = std::vector<unsigned>(_peerTable->size() + 1);
    
    //_time = 0;
    //_timeVector = std::vector<unsigned>(_peerTable->size() + 1);
    
    _inSemName = std::string("in semaphore");
    _inSemName += _selfInfo->_id;
    _outSemName = std::string("out semaphore");
    _outSemName += _selfInfo->_id;
    _inMessageSem = sem_open(_inSemName.c_str(), O_CREAT, 0600, 0);
    _outMessageSem = sem_open(_outSemName.c_str(), O_CREAT, 0600, 0);
    if(_inMessageSem == SEM_FAILED || _outMessageSem == SEM_FAILED) {
        perror("parent sem_open");
        exit(-1);
    }
    srand(RANDOM_SEED);
}

std::string timeVectorToStr(std::vector<unsigned>& timeVector){
    std::stringstream ss;
    ss<<"[";
    for (int i = 0; i < timeVector.size(); ++i) {
        ss<<timeVector[i]<<", ";
    }
    ss<<"]";
    return ss.str();
};

void Dealer::waitForPeers(){
    int serverSocket;
    int clientSocket;
    struct sockaddr_in serverAddr;
    struct sockaddr_in clientAddr;
    unsigned len;
    int ret;
    
    if((serverSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
        std::cout << "socket() failed" << std::endl;
        exit(-1);
    }
    
    memset(&serverAddr, 0 , sizeof(struct sockaddr_in));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(_selfInfo->_ip.c_str());
    serverAddr.sin_port = htons(_selfInfo->_port);
    ret = bind(serverSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));
    if( ret < 0){
        std::cout << "bind() failed" << std::endl;
        exit(-1);
    }
    if(listen(serverSocket, MAX_QUEUE_SIZE) < 0){
        std::cout << "listen() failed" << std::endl;
        exit(-1);
    }
    while(true){
        len = sizeof(struct sockaddr_in);
        std::cout<<"start accepting new connection." <<std::endl;
        clientSocket = accept(serverSocket, (struct sockaddr *) &clientAddr, &len);
        std::cout<<"end accepting. "<<std::endl;
        ReceiveThread* thread = new ReceiveThread(this, clientSocket);
        _inThreads.push_back(thread);
        thread->start();
    }
}

void Dealer::connectPeers(){
    int sockfd, portno;
    struct sockaddr_in socketAddr;
    int res;
    for (PeerTable::iterator it = _peerTable->begin(); it != _peerTable->end(); ++it){
        Peer* peer = it->second;
        portno = peer->_port;
        std::cout<<"start connecting to "<<peer->_ip<<":"<<peer->_port<<std::endl;
        int i = 1;
        while(i <= 30){
            sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (-1 == sockfd){
                perror("cannot create socket");
                exit(EXIT_FAILURE);
            }
            memset(&socketAddr, 0, sizeof(socketAddr));
            socketAddr.sin_family = AF_INET;
            socketAddr.sin_port = htons(portno);
            socketAddr.sin_addr.s_addr = inet_addr(peer->_ip.c_str());
            res = connect(sockfd, (struct sockaddr *)&socketAddr, sizeof(socketAddr));
            
            if(res != -1){
                break;
            }
            close(sockfd);
            sleep(++i);
        }
        SendThread* thread = new SendThread(this, peer->_id, sockfd);
        thread->start();
        _outThreads.push_back(thread);
        std::cout<<peer->_ip<<":"<<peer->_port<< " connection setup completed." << std::endl;
    }
    std::cout<< "Outgoing connection setup completed." << std::endl;
};

void* Dealer::startListenThread(void* ptr){
    Dealer* thisPtr = (Dealer*)ptr;
    thisPtr->waitForPeers();
    return NULL;
};

void* Dealer::startConnectThread(void* ptr){
    Dealer* thisPtr = (Dealer*)ptr;
    thisPtr->connectPeers();
    return NULL;
}

int Dealer::startListen(){
    int ret = pthread_create(&_listenThread, NULL, Dealer::startListenThread, (void*) this);
    return ret;
}

int  Dealer::startConnect(){
    int ret = pthread_create(&_listenThread, NULL, Dealer::startConnectThread, (void*) this);
    return ret;
}

void Dealer::join(){
    pthread_join(_listenThread, NULL);
    pthread_join(_connectThread, NULL);
    pthread_join(_inProcessThread, NULL);
    pthread_join(_outProcessThread, NULL);
    
    for (InThreadList::iterator it = _inThreads.begin(); it != _inThreads.end(); ++it){
        pthread_join((*it)->getThread(), NULL);
    }
    
    for (OutThreadList::iterator it = _outThreads.begin(); it != _outThreads.end(); ++it){
        pthread_join((*it)->getThread(), NULL);
    }
}
void Dealer::reportReady(int pid, int sockfd){
    pthread_mutex_lock(&_checkMutex);
    _socketTable[pid] = sockfd;
    pthread_mutex_unlock(&_checkMutex);

}

void Dealer::clearUpCachedMessage(int pid){
    pthread_mutex_lock(&_checkMutex);
    std::vector<Message*> messages = _messageStore[pid];
    for(int i = 0; i < messages.size(); ++i){
        queueOutGoingMessage(messages[i]);
    }
    messages.clear();
    pthread_mutex_unlock(&_checkMutex);
}


void Dealer::startProcess(){
    pthread_create(&_inProcessThread, NULL, Dealer::startInCommingMessageThread, (void*) this);
    pthread_create(&_outProcessThread, NULL, Dealer::startOutGoingMessageThread, (void*) this);
}

void* Dealer::startInCommingMessageThread(void *ptr){
    Dealer* thisPtr = (Dealer*)ptr;
    thisPtr->processInCommingMessage();
    return 0;
}

void* Dealer::startOutGoingMessageThread(void *ptr){
    Dealer* thisPtr = (Dealer*)ptr;
    thisPtr->processOutGoingMessage();
    return 0;
}

void Dealer::queueOutGoingMessage(Message* msg){
    pthread_mutex_lock(&_outMutex);
    {
        //std::cout<<"[start] queue out message"<<std::endl;
        //std::cout<<"out queue size: "<<_outMessageQueue.size()<<std::endl;
        _outMessageQueue.push_back(msg);
        //std::cout<<"out queue size: "<<_outMessageQueue.size()<<std::endl;
        //std::cout<<"[end] queue out message"<<std::endl;
    }
    pthread_mutex_unlock(&_outMutex);
    sem_post(_outMessageSem);
}

void Dealer::queueInCommingMessage(Message* msg){
    pthread_mutex_lock(&_inMutex);
    {
        //std::cout<<"[start] queue in message"<<std::endl;
        //std::cout<<"in queue size: "<<_inMessageQueue.size()<< std::endl;
        _inMessageQueue.push_back(msg);
        //std::cout<<"in queue size: "<<_inMessageQueue.size()<< std::endl;
        //std::cout<<"[end] queue in message"<<std::endl;
    }
    pthread_mutex_unlock(&_inMutex);
    sem_post(_inMessageSem);
}

void Dealer::processInCommingMessage(){
    while(true){
        sem_wait(_inMessageSem);
        
        Message* msg;
        pthread_mutex_lock(&_inMutex);
        {
            msg = _inMessageQueue.front();
            _inMessageQueue.pop_front();
        }
        pthread_mutex_unlock(&_inMutex);
        
        pthread_mutex_lock(&_updateMutex);
        {
            if(msg->_time > _state->_time){
                _state->_time = std::max(_state->_time, msg->_time);
            }
            for(int i = 0; i < _state->_timeVector.size(); ++i){
                if(i != _selfInfo->_id){
                    _state->_timeVector[i] = std::max(_state->_timeVector[i], msg->_timeVector[i]);
                }
            }
            _state->_time += 1;
            _state->_timeVector[_selfInfo->_id] += 1;
            if (msg->_action == AbstractMessage::PURCHASE_ACTION) {
                _state->_money += msg->_money;
            }else if(msg->_action == AbstractMessage::DELIVERY_ACTION){
                _state->_widgets += msg->_widgets;
            }
            
        }
        pthread_mutex_unlock(&_updateMutex);

        pthread_mutex_lock(&_printMutex);
        {
            std::cout<<"receive action: "<<msg->_action<<std::endl;
            if(msg->_action == AbstractMessage::PURCHASE_ACTION){
                std::cout<<"receive event: order money="<<_state->_money<<" widgets="<<_state->_widgets<<" time="<<_state->_time <<", time vector = "<<timeVectorToStr(_state->_timeVector)<<std::endl;
                    printf("receive order message from %d:<%d, %d, %d, %s>\n",msg->_pid, msg->_money, msg->_widgets, msg->_time, timeVectorToStr(msg->_timeVector).c_str());
            }else if(msg->_action == AbstractMessage::DELIVERY_ACTION){
                std::cout<<"receive event: delivery money="<<_state->_money<<" widgets="<<_state->_widgets<<" time="<<_state->_time <<", time vector = "<<timeVectorToStr(_state->_timeVector)<<std::endl;
                printf("receive delivery message from %d:<%d, %d, %d, %s>\n",msg->_pid, msg->_money, msg->_widgets, msg->_time, timeVectorToStr(msg->_timeVector).c_str());
            }
        }
        pthread_mutex_unlock(&_printMutex);
        
        if (msg->_action == AbstractMessage::PURCHASE_ACTION) {
            Message* response = new Message();
            response->_action = AbstractMessage::DELIVERY_ACTION;
            response->_money = 0;
            response->_widgets = msg->_widgets;
            response->_pid = msg->_pid;
            queueOutGoingMessage(response);
        }else if(msg->_action == AbstractMessage::MARKER_ACTION){
            saveState();
            std::cout<<__FILE__<<"@"<<__LINE__<<"[DEBUG]: "<<"record marker\n";
        }
        
        delete msg;
    }
}

void Dealer::processOutGoingMessage(){
    while(true){
        Message* msg;
        sem_wait(_outMessageSem);

        pthread_mutex_lock(&_inMutex);
        {
            msg = _outMessageQueue.front();
            _outMessageQueue.pop_front();
        }
        pthread_mutex_unlock(&_inMutex);
        
        //checking unavailable sockets
        pthread_mutex_lock(&_checkMutex);
        if(_socketTable[msg->_pid]==0){
            _messageStore[msg->_pid].push_back(msg);
            pthread_mutex_unlock(&_checkMutex);
            continue;
        }
        pthread_mutex_unlock(&_checkMutex);
        
        pthread_mutex_lock(&_updateMutex);
        _state->_time += 1;
        _state->_timeVector[_selfInfo->_id] += 1;
        if(msg->_action == AbstractMessage::PURCHASE_ACTION){
            _state->_money -= msg->_money;
        }else if(msg->_action == AbstractMessage::DELIVERY_ACTION){
            _state->_widgets -= msg->_widgets;
        }
        msg->_time = _state->_time;
        msg->_timeVector = _state->_timeVector;
        pthread_mutex_unlock(&_updateMutex);

        pthread_mutex_lock(&_printMutex);
        {
            if(msg->_action == AbstractMessage::PURCHASE_ACTION){
                std::cout<<"send event: order money="<<_state->_money<<" widgets="<<_state->_widgets<<" time="<<_state->_time <<", time vector = "<<timeVectorToStr(_state->_timeVector)<<std::endl;
                printf("send order message to %d:<%d, %d, %d, %s>\n",msg->_pid, msg->_money, msg->_widgets, msg->_time, timeVectorToStr(msg->_timeVector).c_str());
            }else if(msg->_action == AbstractMessage::DELIVERY_ACTION){
                std::cout<<"send event: delivery money="<<_state->_money<<" widgets="<<_state->_widgets<<" time="<<_state->_time <<", time vector = "<<timeVectorToStr(_state->_timeVector)<<std::endl;
                printf("send delivery message to %d:<%d, %d, %d, %s>\n",msg->_pid, msg->_money, msg->_widgets, msg->_time, timeVectorToStr(msg->_timeVector).c_str());
            }
            std::cout<<"socket status => "<<_socketTable[msg->_pid]<<std::endl;
        }
        pthread_mutex_unlock(&_printMutex);
        
        char* buffer = msg->toCharArray();
        tcpWrite(_socketTable[msg->_pid], buffer, (int)strlen(buffer));
        delete buffer;
        
        unsigned decision = (rand() % 100);
        if(decision <= 10){
            std::cout << "Start SnapShot" << std::endl;
            unsigned initiator = _selfInfo->_id;
            unsigned snapshotId = static_cast<unsigned>(_snapshotTable[initiator].size()) + 1;
            unsigned num = static_cast<unsigned>(_peerTable->size());
            MarkerMessage marker;
            char* buffer;
            SnapShot* snapshot = new SnapShot(initiator, snapshotId, *_state, num);
            pthread_mutex_lock(&_updateMutex);
            {
                _snapshotTable[initiator][snapshotId] = snapshot;
                marker._snapshotId = snapshotId;
                marker._initiator = initiator;
                
                for (SocketTable::iterator it = _socketTable.begin(); it != _socketTable.end(); ++it) {
                    _state->_time += 1;
                    _state->_timeVector[_selfInfo->_id] += 1;
                    marker._time = _state->_time;
                    marker._timeVector = _state->_timeVector;
                    buffer = marker.toCharArray();
                    tcpWrite(it->second, buffer, (int)strlen(buffer));
                    delete buffer;
                }
            }
            pthread_mutex_unlock(&_updateMutex);
        }
    }
}



void Dealer::registerThread(int pid, const char* ip, int port){
    
}

Peer* Dealer::getSelfInfo(){
    return _selfInfo;
}


void Dealer::saveState(){
    std::cout<<__FILE__<<"@"<<__LINE__<<"[DEBUG]: "<<"save current state"<<std::endl;
}

Dealer::~Dealer(){
}