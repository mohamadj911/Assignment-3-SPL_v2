#include "../include/Summary.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <set>

using std::string;

Summary::Summary(string gameName, string userName, map<string, map<string, vector<string>>> &data)
    : game(gameName), user(userName), summaryData(data) {}

static inline string trim(const string &s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    size_t b = s.find_last_not_of(" \t\r\n");
    if (a == string::npos) return "";
    return s.substr(a, b - a + 1);
}

string Summary::PrintSummary() {
    if (summaryData.find(user) == summaryData.end()) {
        return "ERR0R\n";
    }
    if (summaryData[user].find(game) == summaryData[user].end()) {
        return "ERR0R\n";
    }

    vector<string> &messages = summaryData[user][game];
    if (messages.empty()) return "ERR0R\n";

    struct EventRec {
        int time;
        string name;
        string desc;
        EventRec() : time(0), name(), desc() {}
    };

    string teamA, teamB;
    std::map<string, string> generalStats;
    std::map<string, string> teamAStats;
    std::map<string, string> teamBStats;
    std::vector<EventRec> events;
    std::set<string> seen; // key: time|name to deduplicate

    for (const auto &msg : messages) {
        std::istringstream in(msg);
        string line;
        string curName;
        int curTime = 0;
        string curDesc;
        enum Sect { NONE, GEN, AUPD, BUPD, DESC } sect = NONE;
        while (std::getline(in, line)) {
            if (line.rfind("team a:", 0) == 0) teamA = trim(line.substr(7));
            else if (line.rfind("team b:", 0) == 0) teamB = trim(line.substr(7));
            else if (line.rfind("event name:", 0) == 0) curName = trim(line.substr(11));
            else if (line.rfind("time:", 0) == 0) {
                try { curTime = std::stoi(trim(line.substr(5))); } catch (...) { curTime = 0; }
            }
            else if (line == "general game updates:") sect = GEN;
            else if (line == "team a updates:") sect = AUPD;
            else if (line == "team b updates:") sect = BUPD;
            else if (line == "description:") { sect = DESC; curDesc.clear(); }
            else {
                if (sect == DESC) {
                    if (!curDesc.empty()) curDesc += "\n";
                    curDesc += line;
                } else if (sect == GEN || sect == AUPD || sect == BUPD) {
                    auto pos = line.find(":");
                    if (pos != string::npos) {
                        string k = trim(line.substr(0, pos));
                        string v = trim(line.substr(pos + 1));
                        if (!k.empty()) {
                            if (sect == GEN) generalStats[k] = v;
                            else if (sect == AUPD) teamAStats[k] = v;
                            else if (sect == BUPD) teamBStats[k] = v;
                        }
                    }
                }
            }
        }
        string key = std::to_string(curTime) + "|" + curName;
        if (!curName.empty() && seen.insert(key).second) {
            EventRec e; e.time = curTime; e.name = curName; e.desc = trim(curDesc);
            events.push_back(e);
        }
    }

    std::sort(events.begin(), events.end(), [](const EventRec &a, const EventRec &b){
        if (a.time != b.time) return a.time < b.time;
        return a.name < b.name;
    });

    std::ostringstream out;
    if (teamA.empty() || teamB.empty()) {
        // Fallback to game name split e.g. Germany_Japan
        auto pos = game.find('_');
        if (pos != string::npos) {
            teamA = game.substr(0, pos);
            teamB = game.substr(pos + 1);
        } else {
            teamA = game;
            teamB = "";
        }
    }

    out << teamA << " vs " << teamB << "\n";
    out << "Game stats:\n";
    out << "General stats:\n";
    // Print general stats sorted by key for deterministic output
    for (const auto &p : generalStats) {
        out << p.first << ": " << p.second << "\n";
    }
    out << teamA << " stats:\n";
    for (const auto &p : teamAStats) {
        out << p.first << ": " << p.second << "\n";
    }
    out << teamB << " stats:\n";
    for (const auto &p : teamBStats) {
        out << p.first << ": " << p.second << "\n";
    }
    out << "Game event reports:\n";
    for (const auto &e : events) {
        out << e.time << " - " << e.name << ":\n\n";
        out << e.desc << "\n\n\n";
    }

    return out.str();
}
