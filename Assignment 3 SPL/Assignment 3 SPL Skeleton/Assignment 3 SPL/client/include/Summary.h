#pragma once
#include <string>
#include <map>
#include <vector>

using std::string;
using std::map;
using std::vector;

class Summary {
private:
    string game;
    string user;
    map<string, map<string, vector<string>>> &summaryData;

public:
    Summary(string gameName, string userName, map<string, map<string, vector<string>>> &data);
    string PrintSummary();
};
