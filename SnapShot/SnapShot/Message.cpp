//
//  Message.cpp
//  SnapShot
//
//  Created by KamCheung Ting on 2/20/14.
//  Copyright (c) 2014 KamCheung Ting. All rights reserved.
//

#include "Message.h"
#include <sstream>
#include <iomanip>
#include <unistd.h>
using namespace std;

const char* AbstractMessage::ACK_MSG_FORMAT = "{action:ack, id:%09d, ip:%s, port:%05d}";
const char* AbstractMessage::ACK_MSG_PARSE = "{action:%10[^,], id:%d, ip:%15[^,], port:%d}";
const char* AbstractMessage::ACTION_HEADER_FORMAT = "action:%04d\r\nlength:%010d\r\n";
const char* AbstractMessage::ACTION_HEADER_PARSE = "action:%d\r\nlength:%d\r\n";
const char* AbstractMessage::ACTION_MSG_FORMAT = "{money:%05d, widgets:%05, time:%012d, vector:%s}";
const char* AbstractMessage::ACTION_MSG_PARSE = "{money:%d, widgets:%d, time:%d}";
const char* AbstractMessage::ACTION_MARKER_PARSE = "{snapshotId:%d, initiator:%d, time:%d}";
const char* AbstractMessage::ACTION_MARKER = "{snapshotId:%04d, initiator:%04d, time:%012d, vector:%s}";

AbstractMessage::AbstractMessage(){
}
AbstractMessage::~AbstractMessage(){
}


Message::Message(){
    _action = DELIVERY_ACTION;
    _widgets = 0;
    _money = 0;
    _time = 0;
    _pid = 0;
    _timeVector = std::vector<unsigned>();
}

Message::Message(const Message& other){
    _action = other._action;
    _money = other._money;
    _time = other._time;
    _widgets = other._widgets;
    _pid = other._pid;
    _timeVector = std::vector<unsigned>(other._timeVector);
}

void parseTimeVector( vector<unsigned>& timeVector, string& input, string delimiter){
    size_t pos = 0;
    int value = 0;
    while((pos = input.find(delimiter)) != string::npos){
        string token = input.substr(0, pos);
        istringstream(token)>>value;
        input.erase(0, pos + delimiter.length());
        timeVector.push_back(value);
    }
}

Message::Message(unsigned action, int pid, char* msg){
    std::string delimiter = ", ";
    _pid = pid;
    std::string timeString(msg);
    sscanf(msg, ACTION_MSG_PARSE, &_money, &_widgets, &_time);
    timeString = timeString.substr(timeString.find("[") + 1, timeString.find("]") - timeString.find("[") - 1);
    _action = action;
    _timeVector = std::vector<unsigned>();
    parseTimeVector(_timeVector, timeString, delimiter);
}

std::string Message::toString(){
    using namespace std;
    char buff[ACTION_HEADER_SIZE+1];
    stringstream ss;
    ss << "{money:"<<setw(5)<<setfill('0')<<_money<<", ";
    ss << "widgets:"<<setw(5)<<setfill('0')<<_widgets<<", ";
    ss << "time:"<<setw(12)<<setfill('0')<<_time<<", ";
    ss << "vector:[";
    for(int i = 0; i < _timeVector.size(); ++i){
        ss<<_timeVector[i]<<", ";
    }
    ss << "]}";
    string content = ss.str();
    sprintf(buff, ACTION_HEADER_FORMAT, _action, content.size());
    ss.str("");
    ss<<buff<<content;
    return ss.str();
};

char* Message::toCharArray(){
    std::string stringFormat = toString();
    size_t size =  stringFormat.size();
    char* result = new char[size + 1];
    strcpy(result, stringFormat.c_str());
    return result;
};

Message::~Message(){
};


MarkerMessage::MarkerMessage(){
    _action = MARKER_ACTION;
};

MarkerMessage::MarkerMessage(unsigned action, int pid, char* msg){
    std::string delimiter = ", ";
    _action = MARKER_ACTION;
    _pid = pid;
    std::string timeString(msg);
    sscanf(msg, ACTION_MARKER_PARSE, &_snapshotId, &_initiator, &_time);
    timeString = timeString.substr(timeString.find("[") + 1, timeString.find("]") - timeString.find("[") - 1);
    _timeVector = std::vector<unsigned>();
    parseTimeVector(_timeVector, timeString, delimiter);
};

MarkerMessage::MarkerMessage(const MarkerMessage& other ){
    _action = other._action;
    _snapshotId = other._snapshotId;
    _time = other._time;
    _initiator = other._initiator;
    _pid = other._pid;
    _timeVector = std::vector<unsigned>(other._timeVector);
};

std::string MarkerMessage::toString(){
    char buff[ACTION_HEADER_SIZE+1];
    stringstream ss;
    ss << "{snapshotId:"<<_snapshotId<<", ";
    ss << "initiator:"<<_initiator<<", ";
    ss << "time:"<<setw(12)<<setfill('0')<<_time<<", ";
    ss << "vector:[";
    for(int i = 0; i < _timeVector.size(); ++i){
        ss<<_timeVector[i]<<", ";
    }
    ss << "]}";
    string content = ss.str();
    sprintf(buff, ACTION_HEADER_FORMAT, _action, content.size());
    ss.str("");
    ss<<buff<<content;
    return ss.str();
}

char* MarkerMessage::toCharArray(){
    std::string stringFormat = toString();
    size_t size =  stringFormat.size();
    char* result = new char[size + 1];
    strcpy(result, stringFormat.c_str());
    return result;
}

MarkerMessage::~MarkerMessage(){
};

const char* InitMessage::INIT_MESSAGE_FORMAT = "{id:%09d, ip:%s, port:%05d}";
const char* InitMessage::INIT_HEADER_FORMAT = "action:0000\r\nlength:%010d\r\n";
const char* InitMessage::INIT_MESSAGE_PARSE = "action:%d\r\nlength:%d\r\n{id:%d, ip:%15[^,], port:%d}";
char* InitMessage::toCharArray(int pid, const char* ip, int port){
    char content[100];
    char header[100];
    char* result;
    std::stringstream ss;
    unsigned size = 0;
    sprintf(content, INIT_MESSAGE_FORMAT, pid, ip, port);
    size = (unsigned)strlen(content);
    sprintf(header, INIT_HEADER_FORMAT, size);
    ss << header << content;
    result = new char[ss.str().length() + 1];
    strcpy(result, ss.str().c_str());
    return result;
};