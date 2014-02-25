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
 
    
    static const unsigned SLEEP_TIME = 1;
    static const unsigned RANDOM_SEED = 100;
    ssize_t tcpRead(int socket, char* buffer, unsigned total){
        char* pBuff = buffer;
        int byteLeft = total;
        while(byteLeft > 0){
            ssize_t n = read(socket, pBuff, byteLeft);
            if (n < 0){
                std::cout<<__FILE__<<"@"<<__LINE__<<"[DEBUG]: " <<"ERROR: reading from socket."<<std::endl;
                exit(-1);
                return -1;
            }
            if(n == 0){
                std::cout<<__FILE__<<"@"<<__LINE__<<"[DEBUG]: " <<"Peer shutdown."<<std::endl;
                exit(-1);
                return -1;
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
                exit(-1);
                return -1;
            }
        
            if(n == 0){
                std::cout <<"Peer shutdown."<<std::endl;
                exit(-1);
                return -1;
            }
            pBuff += n;
            byteLeft -= n;
        }
        return total;
    };
};
#endif
