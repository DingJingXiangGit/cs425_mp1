#include "Dealer.h"
#include<exception>
#include <errno.h>

Dealer::Dealer(Peer* selfInfo, PeerTable* peerTable, int totalSnapshot){
    _selfInfo = selfInfo;
    _peerTable = peerTable;
    _state = new State(DEFAULT_MONEY, DEFAULT_WIDGETS);
    _state->_timeVector = std::vector<unsigned>(_peerTable->size() + 1);
    
    _inSemName = std::string("in semaphore");
    _inSemName += _selfInfo->_id;
    _outSemName = std::string("out semaphore");
    _outSemName += _selfInfo->_id;
    _totalSnapShot = totalSnapshot;
    
    sem_unlink(_inSemName.c_str());
    sem_unlink(_outSemName.c_str());
    _inMessageSem = sem_open(_inSemName.c_str(), O_CREAT, 0600, 0);
    _outMessageSem = sem_open(_outSemName.c_str(), O_CREAT, 0600, 0);
    if(_inMessageSem == SEM_FAILED || _outMessageSem == SEM_FAILED) {
        perror("parent sem_open");
        exit(-1);
    }
    srand(RANDOM_SEED);
};

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
        std::cout << "start connecting to " << peer->_ip << ":" << peer->_port << std::endl;
        while(true){
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
            sleep(1);
        }
        //SendThread* thread = new SendThread(this, peer->_id, sockfd);
        //_outThreads.push_back(thread);
        //thread->start();
        _socketTable[peer->_id] = sockfd;
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
    connectPeers();
    //int ret = pthread_create(&_listenThread, NULL, Dealer::startConnectThread, (void*) this);
    //return ret;
    return 0;
}

void Dealer::join(){
    pthread_join(_listenThread, NULL);
    pthread_join(_connectThread, NULL);
    pthread_join(_inProcessThread, NULL);
   // pthread_join(_outProcessThread, NULL);
    
    for (InThreadList::iterator it = _inThreads.begin(); it != _inThreads.end(); ++it){
        pthread_join((*it)->getThread(), NULL);
    }
    
}


void Dealer::reportReady(int pid, int sockfd){
    _socketTable[pid] = sockfd;
}

void Dealer::startProcess(){
    pthread_create(&_inProcessThread, NULL, Dealer::startInCommingMessageThread, (void*) this);
    //pthread_create(&_outProcessThread, NULL, Dealer::startOutGoingMessageThread, (void*) this);
    processOutGoingMessage();
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

void Dealer::queueOutGoingMessage(AbstractMessage* msg){
    pthread_mutex_lock(&_outMutex);
    {
        _outMessageQueue.push_back(msg);
    }
    pthread_mutex_unlock(&_outMutex);
    sem_post(_outMessageSem);
}

void Dealer::queueInCommingMessage(AbstractMessage* msg){
    pthread_mutex_lock(&_inMutex);
    {
        _inMessageQueue.push_back(msg);
    }
    pthread_mutex_unlock(&_inMutex);
    sem_post(_inMessageSem);
}

void Dealer::processInCommingMessage(){
    while(true){
        sem_wait(_inMessageSem);
        
        AbstractMessage* msg;
        pthread_mutex_lock(&_inMutex);
        {
            msg = _inMessageQueue.front();
            _inMessageQueue.pop_front();
        }
        pthread_mutex_unlock(&_inMutex);
        
        pthread_mutex_lock(&_updateMutex);
        {
            if(msg->_action == AbstractMessage::DELIVERY_ACTION){
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
            }
           
            pthread_mutex_lock(&_printMutex);
            {
                std::cout<<"receive action: "<<msg->_action<<std::endl;
                
                if(msg->_action == AbstractMessage::DELIVERY_ACTION){
                    std::cout<<"receive event: delivery money="<<_state->_money<<" widgets="<<_state->_widgets<<" time="<<_state->_time <<", time vector = "<<timeVectorToStr(_state->_timeVector)<<std::endl;
                    printf("receive delivery message from %d:<%d, %d, %d, %s>\n",msg->_pid, ((Message*)msg)->_money, ((Message*)msg)->_widgets, msg->_time, timeVectorToStr(msg->_timeVector).c_str());
                }else{
                    std::cout<<"receive event: marker money="<<_state->_money<<" widgets="<<_state->_widgets<<" time="<<_state->_time <<", time vector = "<<timeVectorToStr(_state->_timeVector)<<std::endl;
                    printf("receive marker message from %d:initiator %d, snapshot %d, logical %d, time vector = %s\n",msg->_pid, ((MarkerMessage*)msg)->_initiator, ((MarkerMessage*)msg)->_snapshotId, msg->_time, timeVectorToStr(msg->_timeVector).c_str());
                }
            }
            pthread_mutex_unlock(&_printMutex);
            
            if(msg->_action == AbstractMessage::DELIVERY_ACTION){
                _state->_money += ((Message*)msg)->_money;
                _state->_widgets += ((Message*)msg)->_widgets;
                std::vector<SnapShot*>& snapshots = recordingTable[msg->_pid];
                for (int i = 0; i < snapshots.size(); ++i) {
                    snapshots[i]->recordChannelState((Message*)msg);
                }
            }else if(msg->_action == AbstractMessage::MARKER_ACTION){
                MarkerMessage* marker = (MarkerMessage*)msg;
                unsigned initiator = marker->_initiator;
                unsigned snapshotId = marker->_snapshotId;
                SnapShot* snapshot = _snapshotTable[initiator][snapshotId];
                if (snapshot == NULL) {
                    unsigned num = static_cast<unsigned>(_peerTable->size());
                    snapshot = new SnapShot(initiator, snapshotId, *_state, num);
                    _snapshotTable[initiator][snapshotId] = snapshot;

                    MarkerMessage broadcast;
                    char* buffer;
                    broadcast._snapshotId = snapshotId;
                    broadcast._initiator = initiator;
                    broadcast._time = _state->_time;
                    broadcast._timeVector = _state->_timeVector;
                    buffer = broadcast.toCharArray();
                    for (SocketTable::iterator it = _socketTable.begin(); it != _socketTable.end(); ++it) {
                        tcpWrite(it->second, buffer, (int)strlen(buffer));
                        if(it->first != marker->_pid){
                            recordingTable[it->first].push_back(snapshot);
                        }
                    }
                    delete buffer;
                }else{
                    std::vector<SnapShot*>& snapshots = recordingTable[marker->_pid];
                    for (int i = 0; i < snapshots.size(); ++i) {
                        snapshot = snapshots[i];
                        if(snapshot->ownMarker(marker)){
                            snapshots.erase(snapshots.begin()+i);
                            break;
                        }
                    }
                }
                snapshot->save(marker->_pid);
                if(snapshot->isDone()){
                    _snapshotCounter[initiator] += 1;
                    snapshot->report();
                    std::cout<<"snapshot finished\n"<<std::endl;
                }
                if(isFinished()){
                    std::cout<<"finished\n"<<std::endl;
                    exit(-1);
                }
            }
        }
        pthread_mutex_unlock(&_updateMutex);
        
        if (msg->_action == AbstractMessage::DELIVERY_ACTION) {
            delete ((Message*)msg);
        }else if(msg->_action == AbstractMessage::MARKER_ACTION){
            delete ((MarkerMessage*)msg);
        }
        
    }
}

bool Dealer::isFinished(){
    if(_snapshotCounter.size() < _peerTable->size()){
        return false;
    }
    bool result = true;
    for (std::map<unsigned, unsigned>::iterator it = _snapshotCounter.begin(); it != _snapshotCounter.end(); ++it) {
        if(it->second != _totalSnapShot){
            result = false;
        }
    }
    return result;
}

void Dealer::processOutGoingMessage(){
    for (SocketTable::iterator it = _socketTable.begin(); it!= _socketTable.end(); ++it) {
        int _socket = it->second;
        char* message = InitMessage::toCharArray(_selfInfo->_id, _selfInfo->_ip.c_str(), _selfInfo->_port);
        tcpWrite(_socket, message, (int)strlen(message));
        delete message;
    }
    sleep(1);
    int counter = 0;
    while(counter < _totalSnapShot){
        for (SocketTable::iterator it = _socketTable.begin(); it!= _socketTable.end(); ++it) {
            int _socket = it->second;
            unsigned decision = (rand() % 100);
            if(decision){
                Message* msg = new Message();
                unsigned money = (rand() % 10)*10;
                unsigned widget = money / 10 ;
                msg->_money = money;
                msg->_widgets = widget;
                msg->_pid = it->first;
                pthread_mutex_lock(&_updateMutex);
                    if(money >= _state->_money || widget >= _state->_widgets){
                        pthread_mutex_unlock(&_updateMutex);
                        break;
                    }
                    _state->_time += 1;
                    _state->_timeVector[_selfInfo->_id] += 1;
                    if(msg->_action == AbstractMessage::DELIVERY_ACTION){
                        _state->_money -= ((Message*)msg)->_money;
                        _state->_widgets -= ((Message*)msg)->_widgets;
                    }
                    msg->_time = _state->_time;
                    msg->_timeVector = _state->_timeVector;
                pthread_mutex_unlock(&_updateMutex);

                pthread_mutex_lock(&_printMutex);
                {
                    if(msg->_action == AbstractMessage::DELIVERY_ACTION){
                        std::cout<<"send event: delivery money="<<_state->_money<<" widgets="<<_state->_widgets<<" time="<<_state->_time <<", time vector = "<<timeVectorToStr(_state->_timeVector)<<std::endl;
                        printf("send delivery message to %d:<%d, %d, %d, %s>\n",
                               msg->_pid,
                               ((Message*)msg)->_money,
                               ((Message*)msg)->_widgets,
                               msg->_time,
                               timeVectorToStr(msg->_timeVector).c_str());
                    }
                    std::cout<<"socket status => "<<_socket<<std::endl;//_socketTable[msg->_pid]<<std::endl;
                }
                pthread_mutex_unlock(&_printMutex);
            
                char* buffer = msg->toCharArray();
                tcpWrite(_socket, buffer, (int)strlen(buffer));
                delete buffer;
            }else if(counter <= _totalSnapShot){
                unsigned initiator = _selfInfo->_id;
                unsigned snapshotId = static_cast<unsigned>(_snapshotTable[initiator].size()) + 1;
                unsigned num = static_cast<unsigned>(_peerTable->size());
                MarkerMessage marker;
                char* buffer;
                SnapShot* snapshot;
                snapshot = new SnapShot(initiator, snapshotId, *_state, num);
                _snapshotTable[initiator][snapshotId] = snapshot;
                marker._snapshotId = snapshotId;
                marker._initiator = initiator;
                pthread_mutex_lock(&_updateMutex);
                {
                    marker._time = _state->_time;
                    marker._timeVector = _state->_timeVector;
                }
                pthread_mutex_unlock(&_updateMutex);
                buffer = marker.toCharArray();
                for (SocketTable::iterator it = _socketTable.begin(); it != _socketTable.end(); ++it) {
                    tcpWrite(it->second, buffer, (int)strlen(buffer));
                    recordingTable[it->first].push_back(snapshot);
                }
                delete buffer;
                ++counter;
            }
        }
    }
}

void Dealer::registerThread(int pid, const char* ip, int port){
    
}

Peer* Dealer::getSelfInfo(){
    return _selfInfo;
}

Dealer::~Dealer(){
}