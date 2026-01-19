#pragma once
#include <stdlib.h>
#include <string>
#include <iostream>
#include <map>
#include "ConnectionHandler.h"
#include "../include/event.h"
#include <vector>
using std::cerr;
using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::map;
// TODO: implement the STOMP protocol
class StompProtocol
{
private:
    
    string command;
    string CountinueMsg;
   map<int ,string> ChannelsSubscHashMap;
    int Subscribtion_ID;
    int Subscribtion_Recipt;
    string CommandToServer;
    string UserName;
    int Subid;
    int Unsubid;
    int Disid;
    string Channel;


public:
map<string ,map<string,std::vector<string>>> MapForSummary;
StompProtocol(string JustForNotDefault);
virtual ~StompProtocol();
string CONNECT();
string SUBSCRIBE();
string UNSUBSCRIBE();
string SEND();
string DISCONNECT();
string SolveCommand(string Command,string Msg);
string GetMessage();
string printHashMaps(map<string,string> hash);
std::vector<string> ReportEvents(std::vector<Event> events);
void SummaryPrepare(string Respond);
string checkIfSubscribe(string channel);
int getSubid();
int getUnsubid();
int getDisid();
string getChannel();
void setChannel(string str);
};