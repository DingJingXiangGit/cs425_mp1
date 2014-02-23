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

typedef std::map<int, ChannelState> ChannelStates;
typedef std::map<int, Peer*> PeerTable;
typedef std::map<int, int> SocketTable;
class Dealer:TCPUtility{
    private:
        const unsigned MAX_QUEUE_SIZE = 100;
        const unsigned DEFAULT_MONEY = 10000;
        const unsigned DEFAULT_WIDGETS = 10000;
        pthread_mutex_t _readMutex = PTHREAD_MUTEX_INITIALIZER;
        pthread_mutex_t _writeMutex = PTHREAD_MUTEX_INITIALIZER;
        pthread_mutex_t _printMutex = PTHREAD_MUTEX_INITIALIZER;

        int _id;
        int _port;
        unsigned _time;
        std::vector<unsigned> _timeVector;
        State* _state;
        Peer* _selfInfo;
        PeerTable* _peerTable;
    
        pthread_t _listenThread;
        pthread_t _connectThread;
    
        SocketTable _socketTable;

        InThreadList _inThreads;
        OutThreadList _outThreads;
    
        void waitForPeers();
        void connectPeers();
        static void* startListenThread(void* ptr);
        static void* startConnectThread(void* ptr);
    
    public:
        Dealer(Peer* _selfInfo, PeerTable*  peers);
        int startListen();
        int startConnect();
        void registerThread(int pid, const char* ip, int port);
        void sendMessage(int pid, Message& msg);
        void execute(int pid, Message& msg);
        void reportState();
        void saveState();
        void join();
        Peer* getSelfInfo();
        ~Dealer();
};

#endif /* defined(__SnapShot__Dealer__) */