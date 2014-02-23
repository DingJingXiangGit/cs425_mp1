//
//  Utility.h
//  SnapShot
//
//  Created by KamCheung Ting on 2/17/14.
//  Copyright (c) 2014 KamCheung Ting. All rights reserved.
//

#ifndef SnapShot_Utility_h
#define SnapShot_Utility_h
#include <unistd.h>
class TCPUtility{
protected:
    /*
     * initial message format: "{action:ack, id:xxxxxxxxx, ip:xxx.xxx.xxx.xxx, port:xxxxx}".
     * the size is fixed to be 58 bytes.
     */
    //const unsigned INIT_MSG_SIZE = 58;
    /*
     * action message format: "{action:delivery, money:xxxxx, widgets:xxxxx}".
     * the size is fixed to be 58 bytes.
     */
    /*
    const unsigned ACTION_MSG_SIZE = 45;
    const char* ACK_MSG_FORMAT = "{action:ack, id:%09d, ip:%s, port:%05d}";
    const char* ACK_MSG_PARSE = "{action:%10[^,], id:%d, ip:%15[^,], port:%d}";
    const char* BUY_MSG_FORMAT = "{action:purchase, money:%05d, widgets:%05}";
    const char* BUY_MSG_PARSE = "{action:%10[^,], money:%d, widgets:%d}";
    const char* DELIVERY_MSG_FORMAT = "{action:delivery, money:%05d, widgets:%05}";
    const char* DELIVERY_MSG_PARSE = "{action:%10[^,], money:%d, widgets:%d}";
    */
    
    static const unsigned SLEEP_TIME = 5;
    static const unsigned RANDOM_SEED = 100;
    ssize_t tcpRead(int socket, char* buffer, unsigned total){
        char* pBuff = buffer;
        int byteLeft = total;
        while(byteLeft > 0){
            ssize_t n = read(socket, pBuff, byteLeft);
            if (n < 0){
                std::cout<<__FILE__<<"@"<<__LINE__<<"[DEBUG]: " <<"ERROR: reading from socket."<<std::endl;
                return n;
            }
            if(n == 0){
                std::cout<<__FILE__<<"@"<<__LINE__<<"[DEBUG]: " <<"Peer shutdown."<<std::endl;
                return n;
            }
            pBuff += n;
            byteLeft -= n;
        }
        return total;
    };

    ssize_t tcpWrite(int socket, char* buffer, int total){
        char* pBuff = buffer;
        int byteLeft = total;
        while(byteLeft > 0){
            ssize_t n = write(socket, pBuff, byteLeft);
            if (n < 0){
                std::cout <<"ERROR: reading from socket."<<std::endl;
                return n;
            }
        
            if(n == 0){
                std::cout <<"Peer shutdown."<<std::endl;
                return n;
            }
            pBuff += n;
            byteLeft -= n;
        }
        return total;
    };
};
#endif
