#ifndef __SnapShot__Dealer__
#define __SnapShot__Dealer__
#include "State.h"
#include "Thread.h"
#include "Peer.h"
#include "Message.h"
#include "Utility.h"

#include <map>
#include <list>
#include <string>
#include <algorithm>
#include <deque>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

class ReceiveThread;
class SendThread;
typedef std::list<ReceiveThread*> InThreadList;
typedef std::list<SendThread*> OutThreadList;

typedef std::map<int, Peer*> PeerTable;
typedef std::map<int, int> SocketTable;
typedef std::map<int, std::vector<Message*> > MessageStore;
class Dealer:TCPUtility{
    private:
        const unsigned MAX_QUEUE_SIZE = 100;
        const unsigned DEFAULT_MONEY = 10000;
        const unsigned DEFAULT_WIDGETS = 10000;

        pthread_mutex_t _inMutex = PTHREAD_MUTEX_INITIALIZER;
        pthread_mutex_t _outMutex = PTHREAD_MUTEX_INITIALIZER;
        pthread_mutex_t _checkMutex = PTHREAD_MUTEX_INITIALIZER;
        pthread_mutex_t _printMutex = PTHREAD_MUTEX_INITIALIZER;

        sem_t* _inMessageSem;
        sem_t* _outMessageSem;
        std::string _inSemName;
        std::string _outSemName;
        int _port;
        unsigned _time;
        std::vector<unsigned> _timeVector;
        State* _state;
        Peer* _selfInfo;
        PeerTable* _peerTable;
        std::deque<Message*> _inMessageQueue;
        std::deque<Message*> _outMessageQueue;
    
        pthread_t _listenThread;
        pthread_t _connectThread;
        pthread_t _inProcessThread;
        pthread_t _outProcessThread;
    
        SocketTable _socketTable;

        InThreadList _inThreads;
        OutThreadList _outThreads;
        MessageStore _messageStore;

        void waitForPeers();
        void connectPeers();
        static void* startListenThread(void* ptr);
        static void* startConnectThread(void* ptr);
        static void* startInCommingMessageThread(void* ptr);
        static void* startOutGoingMessageThread(void* ptr);
    public:
        Dealer(Peer* _selfInfo, PeerTable*  peers);
        int startListen();
        int startConnect();
        void registerThread(int pid, const char* ip, int port);
        void clearUpCachedMessage(int pid);
    /*
        void sendMessage(int pid, Message& msg);
        void execute(int pid, Message& msg);
        void clearUpCachedMessage(int pid);
     */
        void reportReady(int pid, int sockfd);
        void queueInCommingMessage(Message* msg);
        void queueOutGoingMessage(Message* msg);
        void processInCommingMessage();
        void processOutGoingMessage();
        void startProcess();
        void saveState();
        void join();
        Peer* getSelfInfo();
        ~Dealer();
};

#endif /* defined(__SnapShot__Dealer__) */