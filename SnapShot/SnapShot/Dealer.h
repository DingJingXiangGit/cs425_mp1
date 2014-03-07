#ifndef __SnapShot__Dealer__
#define __SnapShot__Dealer__
#include "State.h"
#include "Thread.h"
#include "Peer.h"
#include "Message.h"
#include "Utility.h"
#include "SnapShot.h"

#include <map>
#include <list>
#include <string>
#include <algorithm>
#include <queue>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <exception>
#include <errno.h>
class ReceiveThread;
class SendThread;
typedef std::list<ReceiveThread*> InThreadList;
typedef std::map<int, Peer*> PeerTable;
typedef std::map<int, int> SocketTable;
typedef std::map<int, std::vector<AbstractMessage*> > MessageStore;
typedef std::map<unsigned, std::map<unsigned, SnapShot*> > SnapShotTable;
typedef std::map<int, int> SnapshotCount;

//pthread_t _connectThread;

class Dealer:TCPUtility{
    private:
        static const unsigned MAX_QUEUE_SIZE = 100;
        static const unsigned DEFAULT_MONEY = 10000;
        static const unsigned DEFAULT_WIDGETS = 10000;
    
        pthread_mutex_t _outMutex;
        pthread_mutex_t _inMutex;
        pthread_mutex_t _updateMutex;
        pthread_mutex_t _printMutex;
        sem_t* _inMessageSem;
        std::string _inSemName;
    
        State* _state;
        Peer* _selfInfo;
        PeerTable* _peerTable;
        std::queue<AbstractMessage*> _inMessageQueue;
        SnapshotCount _snapshotCount;
        int _totalSnapShot;
    
        pthread_t _listenThread;
        pthread_t _inProcessThread;
    
        SocketTable _socketTable;
        InThreadList _inThreads;
        SnapShotTable _snapshotTable;
        std::map<unsigned, unsigned> _snapshotCounter;
        std::map<unsigned, std::vector<SnapShot*> > recordingTable;
        SnapShotGroup* _snapshotGroup;
        std::string _logFileName;
    
        void waitForPeers();
        void connectPeers();
        static void* startListenThread(void* ptr);
        static void* startConnectThread(void* ptr);
        static void* startInCommingMessageThread(void* ptr);
        bool isFinished();
    public:

        Dealer(Peer* _selfInfo, PeerTable*  peers, int totalSnapshot);
        int startListen();
        int startConnect();
        void queueInCommingMessage(AbstractMessage* msg);

        void processInCommingMessage();
        void processOutGoingMessage();
        void startProcess();
        void join();
        ~Dealer();
};

#endif /* defined(__SnapShot__Dealer__) */