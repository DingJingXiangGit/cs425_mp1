#include "Dealer.h"
#include <exception>



Dealer::Dealer(Peer* selfInfo, PeerTable* peerTable, int totalSnapshot){
    _selfInfo = selfInfo;
    _peerTable = peerTable;
    _state = new State(DEFAULT_MONEY, DEFAULT_WIDGETS);
    _state->_timeVector = std::vector<unsigned>(_peerTable->size() + 1);
    _snapshotGroup = new SnapShotGroup();
    pthread_mutex_init(&_outMutex, NULL);
    pthread_mutex_init(&_inMutex, NULL);
    pthread_mutex_init(&_updateMutex, NULL);
    pthread_mutex_init(&_printMutex, NULL);
    
    std::stringstream ss;
    ss<<"in semaphore-"<< _selfInfo->_id;
    _inSemName = ss.str();
    
    ss.str("");
    ss<<"snapshot."<<_selfInfo->_id;
    _logFileName = ss.str();
   
    _totalSnapShot = totalSnapshot;
    sem_unlink(_inSemName.c_str());
    _inMessageSem = sem_open(_inSemName.c_str(), O_CREAT, 0600, 0);
    if(_inMessageSem == SEM_FAILED){// || _outMessageSem == SEM_FAILED) {
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
    return 0;
}

void Dealer::join(){
    pthread_join(_inProcessThread, NULL);
    for (InThreadList::iterator it = _inThreads.begin(); it != _inThreads.end(); ++it){
        pthread_join((*it)->getThread(), NULL);
    }
}

void Dealer::startProcess(){
    pthread_create(&_inProcessThread, NULL, Dealer::startInCommingMessageThread, (void*) this);
    processOutGoingMessage();
}

void* Dealer::startInCommingMessageThread(void *ptr){
    Dealer* thisPtr = (Dealer*)ptr;
    thisPtr->processInCommingMessage();
    return 0;
}

void Dealer::queueInCommingMessage(AbstractMessage* msg){
    pthread_mutex_lock(&_inMutex);
    {
        _inMessageQueue.push(msg);
        sem_post(_inMessageSem);
    }
    pthread_mutex_unlock(&_inMutex);
}

void Dealer::processInCommingMessage(){
    AbstractMessage* msg;
    while(true){
        sem_wait(_inMessageSem);
        pthread_mutex_lock(&_inMutex);
        {
            msg = _inMessageQueue.front();
            _inMessageQueue.pop();
            if(msg == NULL){
                std::cout<<"null message"<<_inMessageQueue.size()<<std::endl;
                exit(-1);
            }
        }
        pthread_mutex_unlock(&_inMutex);
        
        pthread_mutex_lock(&_updateMutex);
        {
            pthread_mutex_lock(&_outMutex);
            
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
                if(msg->_action == AbstractMessage::DELIVERY_ACTION){
                    printf("receive delivery message from %d:<%d, %d, %d, %s>\n",msg->_pid, ((Message*)msg)->_money, ((Message*)msg)->_widgets, msg->_time, timeVectorToStr(msg->_timeVector).c_str());
                    std::cout<<"receive delivery event:  money="<<_state->_money<<" widgets="<<_state->_widgets<<" time="<<_state->_time <<", time vector = "<<timeVectorToStr(_state->_timeVector)<<std::endl;

                }else{
                    printf("receive marker message from %d:initiator %d, snapshot %d, logical %d, time vector = %s\n",msg->_pid, ((MarkerMessage*)msg)->_initiator, ((MarkerMessage*)msg)->_snapshotId, msg->_time, timeVectorToStr(msg->_timeVector).c_str());
                    std::cout<<"receive marker event: marker money="<<_state->_money<<" widgets="<<_state->_widgets<<" time="<<_state->_time <<", time vector = "<<timeVectorToStr(_state->_timeVector)<<std::endl;
                }
            }
            pthread_mutex_unlock(&_printMutex);
            if(msg->_action == AbstractMessage::DELIVERY_ACTION){

                _state->_money += ((Message*)msg)->_money;

                _state->_widgets += ((Message*)msg)->_widgets;

                if(recordingTable.find(msg->_pid) != recordingTable.end()){
                    std::vector<SnapShot*>& snapshots = recordingTable[msg->_pid];

                    for (int i = 0; i < snapshots.size(); ++i){
                        if ( snapshots[i] != NULL) {
                            snapshots[i]->recordChannelState(_selfInfo->_id, (Message*)msg);
                        }
                    }
                }
            }else if(msg->_action == AbstractMessage::MARKER_ACTION){
                MarkerMessage* marker = (MarkerMessage*)msg;
                unsigned initiator = marker->_initiator;
                unsigned snapshotId = marker->_snapshotId;
                SnapShot* snapshot = _snapshotTable[initiator][snapshotId];
                if (snapshot == NULL) {
                    unsigned num = static_cast<unsigned>(_peerTable->size());
                    snapshot = new SnapShot(initiator, _selfInfo->_id, snapshotId, *_state, num);
                    _snapshotTable[initiator][snapshotId] = snapshot;
                    MarkerMessage broadcast;
                    char* buffer;
                    broadcast._snapshotId = snapshotId;
                    broadcast._initiator = initiator;
                    broadcast._time = _state->_time;
                    broadcast._timeVector = _state->_timeVector;
                    broadcast._pid = _selfInfo->_id;
                    buffer = broadcast.toCharArray();
                    for (SocketTable::iterator it = _socketTable.begin(); it != _socketTable.end(); ++it){
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
                    _snapshotGroup->add(snapshot);
                }
                if(isFinished()){
                    _snapshotGroup->save(_logFileName);
                    for (SocketTable::iterator it = _socketTable.begin(); it != _socketTable.end(); ++it) {
                        char dummy[10];
                        shutdown(it->second, 2);
                        while(read(it->second,dummy, 10)){
                        }
                        close(2);
                    }
                    sleep(3);
                    return;
                }
            }

            pthread_mutex_unlock(&_outMutex);
        }
        pthread_mutex_unlock(&_updateMutex);
        delete msg;
    }
}

bool Dealer::isFinished(){
    std::cout<<"counter => "<<_snapshotCounter.size()<<"  peertable= "<<_peerTable->size()<<std::endl;
    if(_snapshotCounter.size() < _peerTable->size()){
        return false;
    }
    bool result = true;
    for (std::map<unsigned, unsigned>::iterator it = _snapshotCounter.begin(); it != _snapshotCounter.end(); ++it) {
        std::cout<<"first => "<<it->second<<"  second= "<<_totalSnapShot<<std::endl;

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
        pthread_mutex_lock(&_outMutex);
        tcpWrite(_socket, message, (int)strlen(message));
        pthread_mutex_unlock(&_outMutex);
        delete message;
    }
    sleep(SLEEP_TIME);
    int counter = 0;
    while(counter < _totalSnapShot){
        usleep(10 * 1000);
        for (SocketTable::iterator it = _socketTable.begin(); it!= _socketTable.end(); ++it) {
            int _socket = it->second;
            unsigned decision = (rand() % 100);
            if(decision<90){
                pthread_mutex_lock(&_updateMutex);
                pthread_mutex_lock(&_outMutex);
                Message* msg = new Message();
                unsigned money = (rand() % 10);
                unsigned widget = money * 10 ;
                msg->_money = money;
                msg->_widgets = widget;
                msg->_pid = it->first;

                    if(money <= 0 || money >= _state->_money || widget >= _state->_widgets){
                        pthread_mutex_unlock(&_outMutex);
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

                pthread_mutex_lock(&_printMutex);
                {
                    if(msg->_action == AbstractMessage::DELIVERY_ACTION){
                        std::cout<<"send event: delivery money="<<_state->_money<<" widgets="<<_state->_widgets<<" time="<<_state->_time <<", time vector = "<<timeVectorToStr(_state->_timeVector)<<std::endl;
                    }
                }
                pthread_mutex_unlock(&_printMutex);
                char* buffer = msg->toCharArray();
                tcpWrite(_socket, buffer, (int)strlen(buffer));
                pthread_mutex_unlock(&_outMutex);
                pthread_mutex_unlock(&_updateMutex);
                delete buffer;
            }else if(counter < _totalSnapShot){
                pthread_mutex_lock(&_updateMutex);
                pthread_mutex_lock(&_outMutex);
                {
                    unsigned initiator = _selfInfo->_id;
                    unsigned snapshotId = static_cast<unsigned>(_snapshotTable[initiator].size()) + 1;
                    unsigned num = static_cast<unsigned>(_peerTable->size());
                    MarkerMessage marker;
                    char* buffer;
                    SnapShot* snapshot;
                    snapshot = new SnapShot(initiator, _selfInfo->_id, snapshotId, *_state, num);
                    _snapshotTable[initiator][snapshotId] = snapshot;
                    marker._snapshotId = snapshotId;
                    marker._initiator = initiator;

                        marker._time = _state->_time;
                        marker._timeVector = _state->_timeVector;

                    buffer = marker.toCharArray();

                    for (SocketTable::iterator it = _socketTable.begin(); it != _socketTable.end(); ++it) {
                        tcpWrite(it->second, buffer, (int)strlen(buffer));
                        recordingTable[it->first].push_back(snapshot);
                    }
                    delete buffer;
                }
                pthread_mutex_unlock(&_outMutex);
                pthread_mutex_unlock(&_updateMutex);
                ++counter;
            }
        }
    }
}
Dealer::~Dealer(){
}