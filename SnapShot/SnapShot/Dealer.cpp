#include "Dealer.h"
#include<thread>

Dealer::Dealer(Peer* selfInfo, PeerTable* peerTable){
    _selfInfo = selfInfo;
    _peerTable = peerTable;
    _state = new ProcessState(DEFAULT_MONEY, DEFAULT_WIDGETS, _selfInfo->_id);
    _time = 0;
    _timeVector = std::vector<unsigned>(_peerTable->size() + 1);
    srand(RANDOM_SEED);
}

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
    serverAddr.sin_addr.s_addr = inet_addr(_selfInfo->_ip.c_str());//htonl(INADDR_ANY);
    serverAddr.sin_port = htons(_selfInfo->_port);
    std::cout<<"start binding"<<std::endl;
    ret = bind(serverSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));
    if( ret < 0){
        std::cout << "bind() failed" << std::endl;
        exit(-1);
    }
    std::cout<<"end binding"<<std::endl;
    std::cout<<"start listening"<<std::endl;
    if(listen(serverSocket, MAX_QUEUE_SIZE) < 0){
        pthread_mutex_lock(&_printMutex);
        std::cout << "listen() failed" << std::endl;
        pthread_mutex_unlock(&_printMutex);
        exit(-1);
    }
    std::cout<<"end listening"<<std::endl;
    while(true){
        len = sizeof(struct sockaddr_in);
        std::cout<<__FILE__<<"@"<<__LINE__<<"[DEBUG]: "<<"start accepting :" << serverSocket <<std::endl;
        if((clientSocket = accept(serverSocket, (struct sockaddr *) &clientAddr, &len)) < 0){
            std::cout<<__FILE__<<"@"<<__LINE__<<"[DEBUG]: " << "accept() failed" << std::endl;
        }
        std::cout<<__FILE__<<"@"<<__LINE__<<"[DEBUG]: "<<"end accepting"<<std::endl;
        
        ReceiveThread* thread = new ReceiveThread(this, clientSocket);
        _inThreads.push_back(thread);
        thread->start();
    }
}

void Dealer::connectPeers(){
    int sockfd, portno;
    struct sockaddr_in sockerAddr;
    int res;
    for (PeerTable::iterator it = _peerTable->begin(); it != _peerTable->end(); ++it){
        Peer* peer = it->second;
        portno = peer->_port;
        sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (-1 == sockfd){
            perror("cannot create socket");
            exit(EXIT_FAILURE);
        }
        
        memset(&sockerAddr, 0, sizeof(sockerAddr));
        sockerAddr.sin_family = AF_INET;
        sockerAddr.sin_port = htons(portno);
        res = inet_pton(AF_INET, peer->_ip.c_str(), &sockerAddr.sin_addr);
        if (0 > res){
            perror("error: first parameter is not a valid address family");
            close(sockfd);
            exit(EXIT_FAILURE);
        }else if (0 == res){
            perror("char string (second parameter does not contain valid ipaddress)");
            close(sockfd);
            exit(EXIT_FAILURE);
        }
        pthread_mutex_lock(&_printMutex);
        std::cout<<__FILE__<<"@"<<__LINE__<<"[DEBUG]: "<<"start connecting to "<<peer->_ip<<":"<<peer->_port<<std::endl;
        int i = 1;
        while(connect(sockfd, (struct sockaddr *)&sockerAddr, sizeof(sockerAddr)) == -1 && i <= 30){
            sleep(1000*i);
            i += 1;
        }
        if (-1 == sockfd){
            perror("Dealer");
            close(sockfd);
        }else{
            _socketTable[peer->_id] = sockfd;

            SendThread* thread = new SendThread(this, sockfd);
            _outThreads.push_back(thread);
            thread->start();
            std::cout<<__FILE__<<"@"<<__LINE__<<"[DEBUG]: " << peer->_id << " connection setup completed." << std::endl;
        }
         pthread_mutex_unlock(&_printMutex);
    }
    std::cout<<__FILE__<<"@"<<__LINE__<<"[DEBUG]: " << "Outgoing connection setup completed." << std::endl;
}

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
    for (InThreadList::iterator it = _inThreads.begin(); it != _inThreads.end(); ++it){
        pthread_join((*it)->getThread(), NULL);
    }
    
    for (OutThreadList::iterator it = _outThreads.begin(); it != _outThreads.end(); ++it){
        pthread_join((*it)->getThread(), NULL);
    }
}




void Dealer::sendMessage(int pid, Message& msg){
    pthread_mutex_lock(&_writeMutex);
    {
        if(msg._action == Message::PURCHASE_ACTION){
            _state->_money -= msg._money;
        }else if(msg._action == Message::DELIVERY_ACTION){
            _state->_widgets -= msg._widgets;
        }
        /* start update time and vector timestamp on sending message*/
        _time += 1;
        _timeVector[_selfInfo->_id] += 1;
        /* start update time and vector timestamp on sending message*/
        msg._time = _time;
        msg._timeVector = _timeVector;
        char* buffer = msg.toCharArray();
        std::cout<<__FILE__<<"@"<<__LINE__<<"[DEBUG]: " << "send message "<<buffer<<std::endl;
        tcpWrite(_socketTable[pid], buffer, (int)strlen(buffer));
        delete buffer;
    }
    pthread_mutex_unlock(&_writeMutex);
}

void Dealer::execute(int pid, Message& msg){
    pthread_mutex_lock(&_readMutex);
    /* start update time and vector timestamp on receiving message*/
    if(msg._time > _time){
        _time = msg._time;
    }
    for(int i = 0; i < _timeVector.size(); ++i){
        if(i!=_id){
            _timeVector[i] = std::max(_timeVector[i], msg._timeVector[i]);
        }
    }
    _time += 1;
    _timeVector[_id] += 1;
    /* end update time and vector timestamp on receiving message */
    
    if (msg._action == Message::PURCHASE_ACTION) {
        _state->_money += msg._money;
        Message response;
        response._action = Message::DELIVERY_ACTION;
        response._money = 0;
        response._widgets = msg._widgets;
        sendMessage(pid, response);
    }else if(msg._action == Message::DELIVERY_ACTION){
        _state->_widgets += msg._widgets;
    }else if(msg._action == Message::MARKER_ACTION){
        saveState();
        std::cout<<__FILE__<<"@"<<__LINE__<<"[DEBUG]: "<<"record marker\n";
    }
    pthread_mutex_unlock(&_readMutex);
}


void Dealer::registerThread(int pid, const char* ip, int port){
    
}

Peer* Dealer::getSelfInfo(){
    return _selfInfo;
}

void Dealer::reportState(){
}

void Dealer::saveState(){
    std::cout<<__FILE__<<"@"<<__LINE__<<"[DEBUG]: "<<"save current state"<<std::endl;
}

Dealer::~Dealer(){
}