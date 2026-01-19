#include "../include/ConnectionHandler.h"
#include <stdlib.h>
#include <string>
#include <iostream>
#include <map>
#include "../include/StompProtocol.h"
#include "../include/Summary.h"
#include <fstream>
#include <thread>
using std::cerr;
using std::cin;
using std::cout;
using std::endl;
using std::map;
using std::string;

// Added by us
int ReadThreadRun(ConnectionHandler &connectionHandler, StompProtocol &ClientProtocol, std::atomic<bool> &close, std::atomic<bool> &waiit)
{
	while (!(close.load()))
	{
		string Respond;
		if (!connectionHandler.getFrameAscii(Respond, '\0'))
		{
			std::cout << "1Disconnected. Exiting...\n"
					  << std::endl;
			break;
		}
		if (Respond.substr(0, 9) == "CONNECTED")
		{
			cout << "Login successful" << endl;
		}
		if (Respond.substr(0, 7) == "RECEIPT")
		{
			int index = Respond.find("receipt-id:");
			string temp = Respond.substr(index);
			std::string temp2 = "";
			for (unsigned int i = 11; i < temp.find('\n'); i++)
			{
				temp2 = temp2 + temp.at(i);
			}
			int a = stoi(temp2);

			if (a == ClientProtocol.getSubid())
			{
				cout << "Joined channel " << ClientProtocol.getChannel() << endl;
			}
			if (a == ClientProtocol.getUnsubid())
			{

				cout << "exited channel " << ClientProtocol.getChannel() << endl;
			}
		}
		if (Respond.substr(0, 5) == "ERROR" || Respond.substr(0, 8) == "\nRECEIPT") //////////////////
		{
			
			if (Respond.substr(0, 5) == "ERROR")
			{
				string ans = "";
				int index = Respond.find("Description:");
				for (unsigned int i = index + 13; i < Respond.size(); i++)
				{
					ans = ans + Respond.at(i);
				}
				cout << '\n'
					 << ans << '\n'
					 << endl;
					 
			}
			
			close.store(true);
		}
		if (Respond.substr(0, 7) == "MESSAGE")
		{
			ClientProtocol.SummaryPrepare(Respond);
		}
		waiit.store(true);
	}
	return 0;
}

int WriteThreadRun(ConnectionHandler &connectionHandler, StompProtocol &ClientProtocol, std::atomic<bool> &close, std::atomic<bool> &waiit)
{
	bool run = true;
	while (run)
	{
		while (waiit)
		{
			if (close.load())
			{
				string a = "CLOSE\n";
				if (!connectionHandler.sendFrameAscii(a, '\0'))
				{
					std::cout << "7Disconnected. Exiting....from the respond..\n"
							  << std::endl;
					break;
				}
				run = false;
				waiit.store(false);
			}
			else
			{
				const short bufsize = 1024;
				string input[bufsize];
				char buf[bufsize];
				cin.getline(buf, bufsize);
				string line(buf);
				if (!cin.good() && line.empty()) {
					// EOF or input closed; terminate gracefully
					close.store(true);
					run = false;
					waiit.store(false);
					break;
				}
				std::istringstream iss(line);
				std::vector<string> res;
				string token;
				while (getline(iss, token, ' '))
				{
					res.push_back(token);
				}
				if (res.empty()) {
					continue;
				}
				if (res.size() == 2)
				{
					ClientProtocol.setChannel(res[1]);
				}
				string TerminalCommand = res[0];
				if (TerminalCommand == "login")
				{
					cout << "The Client is already logged in, log out before try Again" << endl;
					continue;
				}
				string CommandToServer = ClientProtocol.SolveCommand(TerminalCommand, line.substr(TerminalCommand.size()));
				if (CommandToServer.at(0) == '0')
				{
					cout << "Illegal Input Bro --> Try Again" << endl;
					continue;
				}
				else if (TerminalCommand == "report" && CommandToServer != "ERR0R\n")
				{ /// Send frame the server should get them one by one and update the channels
					string Frame = "";
					bool a = false;
					for (unsigned int i = 0; i < CommandToServer.length(); i++)
					{
						Frame = "";
						a = false;
						if (CommandToServer[i] == '$' && CommandToServer.length() > 1)
						{
							Frame = CommandToServer.substr(0, i);
							a = true;
							CommandToServer = CommandToServer.substr(i + 1);
							i = 0;
						}

						if (a && !connectionHandler.sendFrameAscii(Frame, '\0'))
						{
							std::cout << "2Disconnected. Exiting...\n"
									  << std::endl;
							break;
						}
					}
				}
				else if (TerminalCommand == "summary")
				{
					std::istringstream iss(line);
					std::vector<string> tokens;
					string token;
					while (getline(iss, token, ' '))
					{
						tokens.push_back(token);
					}
					Summary SummaryGame(tokens[1], tokens[2], ClientProtocol.MapForSummary);
					string summaryfile=SummaryGame.PrintSummary();
					if ( summaryfile!= "ERR0R\n")
					{
						std::ofstream file(tokens[3]);
						file << summaryfile;
						cout << "\nYour final summary file is Ready ^_^" << endl;
					}
				}
				else
				{
					if (!connectionHandler.sendFrameAscii(CommandToServer, '\0'))
					{
						std::cout << "3Disconnected. Exiting...\n"
								  << std::endl;
						break;
					}
					waiit.store(false);
				}
			}
		}
	}
	return 0;
}
int main()
{
	while (1)
	{
		cout << "Login To The Server Program" << endl;
		const short bufsize = 1024;
		char buf[bufsize];
		cin.getline(buf, bufsize);
		string line(buf);
		std::istringstream iss(line);
		std::vector<string> tokens;
		string token;
		while (getline(iss, token, ' '))
		{
			tokens.push_back(token);
		}
		string arg = tokens[1];
		int from = arg.find(":");
		string host = arg.substr(0, from);
		short port = stoi(arg.substr(from + 1));
		ConnectionHandler connectionHandler(host, port);
		if (!connectionHandler.connect())
		{
			std::cerr << "Could not connect to the server" << std::endl;
		}
		StompProtocol ClientProtocol("");
		line = ClientProtocol.SolveCommand("login", line.substr(6));
		if (!connectionHandler.sendFrameAscii(line, '\0'))
		{
			std::cout << "4Disconnected. Exiting...\n"
					  << std::endl;
		}
		string Respond;
		if (!connectionHandler.getFrameAscii(Respond, '\0'))
		{
			std::cout << "5Disconnected. Exiting...\n"
					  << std::endl;
		}
		if (Respond.substr(0, 9) == "CONNECTED")
		{
			cout << "Login successful" << endl;
		}
		if (Respond.substr(0, 5) == "ERROR")
		{
			string ans = "";
			int index = Respond.find("Description:");
			for (unsigned int i = index + 13; i < Respond.size(); i++)
			{
				ans = ans + Respond.at(i);
			}
			cout << '\n'
				 << ans << '\n'
				 << endl;
		}

		std::atomic<bool> closebool(false);
		std::atomic<bool> &close = closebool;
		std::atomic<bool> waitalter(true);
		std::atomic<bool> &waiit = waitalter;
		if (Respond.substr(0, 5) == "ERROR" || Respond.substr(0, 10) == "DISCONNECT" || Respond.substr(0, 8) == "\nRECEIPT") //////////////////////////////////
		{
			close.store(true);
			waiit.store(true);
		}
		std::thread WriteThread(WriteThreadRun, std::ref(connectionHandler), std::ref(ClientProtocol), std::ref(close), std::ref(waiit));
		std::thread ReadThread(ReadThreadRun, std::ref(connectionHandler), std::ref(ClientProtocol), std::ref(close), std::ref(waiit));
		WriteThread.join();
		ReadThread.join();
	}
	return 0;
}
// Adding end