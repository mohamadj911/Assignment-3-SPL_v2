#include "../include/StompProtocol.h"
#include "../include/event.h"
#include <stdlib.h>
#include <string>
#include <iostream>
#include <map>
#include <vector>
using std::cerr;
using std::cin;
using std::cout;
using std::endl;
using std::map;
using std::string;

StompProtocol::StompProtocol(string JustForNotDefault) : command(""), CountinueMsg(""), ChannelsSubscHashMap(), Subscribtion_ID(0), Subscribtion_Recipt(0), CommandToServer(""), UserName(""), Subid(-1), Unsubid(-1), Disid(-1), Channel(""), MapForSummary()
{
}
StompProtocol::~StompProtocol()
{
}
string StompProtocol::CONNECT()
{
	string ans = CountinueMsg.substr(1);
	string extract = "";
	bool a = false;
	for (unsigned int i = 0; i < ans.size() && !a; i++)
	{
		if (ans[i] == ' ')
		{
			a = true;
		}
		if (!a)
		{
			extract += ans[i];
		}
	}
	string user = CountinueMsg.substr(extract.length() + 2);
	bool c = false;
	for (unsigned int i = 0; i < user.length() && !c; i++)
	{
		if (user[i] == ' ')
		{
			c = true;
		}
		if (!c)
		{
			UserName = UserName + user[i];
		}
	}
	string host = "";
	int k = extract.find(":");
	bool b = true;
	string portt = extract.substr(k + 1);
	ans = ans.substr(extract.size() + 1);
	CommandToServer = "CONNECT\naccept-version:1.2\nhost:stomp.cs.bgu.ac.il\nlogin:";
	for (unsigned int i = 0; i < ans.size(); i++)
	{
		if (i < extract.size() && b && extract[i] != ':')
		{
			host = host + extract[i];
		}
		else if (extract[i] == ':')
			b = false;
		if (ans[i] == ' ')
		{
			CommandToServer = CommandToServer + "\npasscode:";
		}
		else
		{
			CommandToServer = CommandToServer + ans[i];
		}
	}
	CommandToServer = CommandToServer + '\n' + '\n';
	return CommandToServer;
}
string StompProtocol::SUBSCRIBE()
{
	string ans = CountinueMsg.substr(1);
	CommandToServer = "SUBSCRIBE\ndestination:/" + ans + "\nid:" + std::to_string(Subscribtion_ID) + "\nreceipt:" + std::to_string(Subscribtion_Recipt) + '\n' + '\n';
	Subid=Subscribtion_Recipt;
	Channel=ans;
	ChannelsSubscHashMap.insert({Subscribtion_ID, ans});
	Subscribtion_ID = Subscribtion_ID + 1;
	Subscribtion_Recipt = Subscribtion_Recipt + 1;	
	return CommandToServer;
}

string StompProtocol::UNSUBSCRIBE()
{
	string destination = "";
	for (unsigned int i = 1; i < CountinueMsg.length(); i++)
	{
		if (CountinueMsg[i] != ' ')
		{
			destination += CountinueMsg[i];
		}
		else
		{
			break;
		}
	}
	int Exit_Id = -1;
	map<int, string>::iterator it;
	for (it = ChannelsSubscHashMap.begin(); it != ChannelsSubscHashMap.end(); it++)
	{
		if (it->second == destination)
		{
			Exit_Id = it->first;
			ChannelsSubscHashMap.erase(Exit_Id);
			break;
		}
	}
	CommandToServer = "UNSUBSCRIBE\nid:" + std::to_string(Exit_Id) + "\nreceipt:" + std::to_string(Subscribtion_Recipt) + '\n' + '\n';
	Unsubid=Subscribtion_Recipt;
	//Channel
	Subscribtion_Recipt = Subscribtion_Recipt + 1;
	return CommandToServer;
}
string StompProtocol::SEND()
{
	string file = CountinueMsg.substr(1);
	if(file.substr(file.length()-5,5)!=".json"){
		return "ERR0R\n";
	}
	names_and_events ReportMsg = parseEventsFile(file);
	std::vector<string> EventsToSend = ReportEvents(ReportMsg.events);
	if(EventsToSend.at(0)=="ERR0R\n"){
		return "ERR0R\n";
	}
	CommandToServer = "";
	for (unsigned int i = 0; i < EventsToSend.size(); i++)
	{
		CommandToServer = CommandToServer + EventsToSend[i] + '$';
	}

	return CommandToServer;
}

string StompProtocol::DISCONNECT()
{
	CommandToServer = "DISCONNECT\nreceipt:" + std::to_string(Subscribtion_Recipt) + '\n' + '\n';
	Disid=Subscribtion_Recipt;
	Subscribtion_Recipt = Subscribtion_Recipt + 1;
	return CommandToServer;
}

string StompProtocol::SolveCommand(string Command, string Msg)
{
	command = Command;
	CountinueMsg = Msg;
	if (command == "login") // 3 left information
	{
		return CONNECT();
	}
	else if (command == "join")
	{ // 1 left informtion

		return SUBSCRIBE();
	}
	else if (command == "exit")
	{ // 1 left information
		return UNSUBSCRIBE();
	}
	else if (command == "report")
	{ // 1 left information
		return SEND();
	}
	else if (command == "summary")
	{ // 3 left information
		return "summary";
	}
	else if (command == "logout")
	{ // 1 left information
		return DISCONNECT();
	}
	return "0There is no command";
}
string StompProtocol::GetMessage()
{
	return CommandToServer;
}
string StompProtocol::printHashMaps(map<string, string> hash)
{
	map<string, string>::iterator itr;
	string ans = "";
	for (itr = hash.begin(); itr != hash.end(); itr++)
	{
		ans = ans + "   " + itr->first + ": " + itr->second + '\n';
	}
	return ans;
}
std::vector<string> StompProtocol::ReportEvents(std::vector<Event> events)
{
	std::vector<string> FramesVec;
	for (unsigned int i = 0; i < events.size(); i++)
	{
		Event event = events.at(i);
		string Channel = event.get_team_a_name() + '_' + event.get_team_b_name();
		if(checkIfSubscribe(Channel)=="NOT_OK"){
			FramesVec.push_back("ERR0R\n");
			break;
		}
		string ans = "SEND\ndestination:/" + Channel + '\n' + '\n' + "user:" + UserName + '\n' + "team a:" + event.get_team_a_name() + '\n' + "team b:" + event.get_team_b_name() + '\n' + "event name:" + event.get_name() + '\n' + "time:" + std::to_string(event.get_time()) + '\n';
		ans = ans + "general game updates:" + '\n';
		ans = ans + printHashMaps(event.get_game_updates());
		ans = ans + "team a updates:" + '\n';
		ans = ans + printHashMaps(event.get_team_a_updates());
		ans = ans + "team b updates:" + '\n';
		ans = ans + printHashMaps(event.get_team_b_updates());
		ans = ans + "description:" + '\n';
		ans = ans + event.get_discription() + '\n' + '\n';
		FramesVec.push_back(ans);
		// MapForSummary[UserName][Channel].push_back(ans);
	}

	return FramesVec;
}
void StompProtocol::SummaryPrepare(string Respond)
{
	string message = "";
	int counter = 0;
	for (unsigned int i = 0; i < Respond.length(); i++)
	{
		if (counter == 3)
		{
			message = Respond.substr(i);
			break;
		}
		else if (Respond.at(i) == '\n')
		{
			counter++;
		}
	}
	int a = message.find("/");
	int to = message.find('\n');
	string des = message.substr(a + 1, to - (a + 1));
	message = message.substr(to + 2);
	int aa = message.find(":");
	int too = message.find('\n');
	string user = message.substr(aa + 1, too - (aa + 1));
	message = message.substr(too + 1);
	message = "SEND\ndestination:/" + des + "\n\n" + "user:" + user + '\n' + message;
	MapForSummary[user][des].push_back(message);
}
string StompProtocol::checkIfSubscribe(string channel)
{
	for (unsigned int i = 0; i < ChannelsSubscHashMap.size(); i++)
	{
		if (ChannelsSubscHashMap.at(i) == channel)
		{ // the "Germany_Japan" should be updated and changes according to json file
			return "OK";
		}
	}
	return "NOT_OK";
}




int StompProtocol::getSubid(){
	return Subid;
}
int StompProtocol::getUnsubid(){
	return Unsubid;
}
int StompProtocol::getDisid(){
	return Disid;
}
string StompProtocol::getChannel(){
	return Channel;
}
void StompProtocol::setChannel(string str){
Channel=str;

}

