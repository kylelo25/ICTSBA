#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <math.h>
#include <algorithm>
#include <random>

#include "Player.h"
#include "Match.h"

using namespace std;

class Mapping
{
public:
    static vector<int> getMapping(int matchCount){
        if (matchCount == 4) return {0,2,3,1};                    
        if (matchCount == 8) return {0,4,6,2,3,7,5,1}; 
        if (matchCount == 16) return {0,8,12,4,6,14,10,2,3,11,15,7,5,13,9,1};
        else return {}; 
    }
};

class PlayerLoader
{
public:
    static void loadPlayers(vector<Player> &players, ifstream &file)
    {
        string line;

        // assign unique id to each player
        int playerCount = 1;
        while (getline(file, line))
        {
            vector<string> playerData;

            stringstream ss(line);
            while (ss.good())
            {
                string substr;
                getline(ss, substr, ',');
                playerData.push_back(substr);
            }

            bool seed = (playerData[1] == "true");

            Player player(playerData[0], seed, playerData[2]);
            player.id = playerCount++;
            players.push_back(player);
        }
        cout<<players.size()<<" players successfully loaded!"<<endl;
        cout<<endl;
    }

    static bool isValidFile(ifstream &file){
        if (!file.is_open())
        {
            cout << "Cannot open file, please re-enter" << endl;
            return false;
        }
        return true;
    }
};

class Tournament
{
public:
    vector<Player> players;

    stringstream generatePlayerReport(int id){
        stringstream playerReport;

        string playerName = players[id - 1].name;
        playerReport<<"Player searching for: "<<playerName<<endl;
        playerReport<<"--------------------"<<endl;

        int count = 0;
        for (auto &round : rounds){

            for (auto &match : round){
                // cout<<match.p1.name<<": "<<match.p1.id<<endl;
                if (match.p1.id == id || match.p2.id == id){
                    count++;
                    playerReport<<getRoundName(round.size())<<endl;
                    playerReport<<match.getMatchResult();
                    playerReport<<"--------------------"<<endl;
                } 
            }

        }
        playerReport<<"Total matches: "<<count<<endl;
        playerReport<<endl;

        return playerReport;
    }

    stringstream generateTournamentReport(){
        stringstream report;
        Player champion = rounds[rounds.size() - 1][0].getWinner();

        for(auto &round : rounds){
            report<<"=================";
            report << getRoundName(round.size());
            report<<"================="<<endl;
            for (Match &match : round)
            {
                report<<match.getMatchResult();
                report<<endl;
            }
            report<<endl;
        }
        report<<"Final champion: "<<champion.name;
        report<<(champion.seed ? "*" : "");
        report<<" ("<<champion.school<<")"<<endl;
        return report;
    }

    stringstream generatePlayerList(){
        stringstream list;

        list << "| " << left << setw(5) << "ID";
        list << "| " << left << setw(15) << "NAME";
        list << "| " << left << setw(5) << "SEED";
        list << "| " << left << setw(30) << "SCHOOL";
        list<<endl;
        list<<"------------------------------------------------------------------------"<<endl;
        for (auto &player : players){

            list << "| " << left << setw(5) << player.id;
            list << "| " << left << setw(15) << player.name;
            list << "| " << left << setw(5) << (player.seed ? "seed" : "");
            list << "| " << left << setw(30) << player.school;
            list << endl;
        }
        list<<"------------------------------------------------------------------------"<<endl;
        
        return list;
    }

    void printTo(stringstream text, ostream& os){
        os << text.str();
    }

    void simulateTournament()
    {
        createInitialMatches();

        int numberOfRounds = log2(getNumberOfMatches(players.size()) * 2);
        for (int i = 0; i < numberOfRounds; i++)
        {
            proceedNextRound(rounds[i]);
        }
    }

private:
    vector<Player> byePlayers;
    vector<vector<Match>> rounds;  

    void createByePlayers(int numberOfByePlayers)
    {
        for (int i = 0; i < numberOfByePlayers; i++)
        {
            byePlayers.push_back(Player());
        }
    }

    int getNumberOfMatches(int playerListSize)
    {
        //special cases
        if (playerListSize == 0)
            return 0;
        if (playerListSize == 1)
            return 1;


        return pow(2, ceil(log(playerListSize) / log(2))) / 2;
    };

    void createInitialMatches()
    {
        vector<reference_wrapper<Player>> seededPlayers = groupSeededPlayers();
        vector<reference_wrapper<Player>> unseededPlayers = groupUnseededPlayers();

        vector<reference_wrapper<Player>> arrangedPlayers = mergeAndFillPlayers(seededPlayers, unseededPlayers);

        // create first round matches
        vector<Match> firstRoundMatches;
        int playerCount = getNumberOfMatches(players.size()) * 2 - 1;


        for (int i = 0; i < arrangedPlayers.size() / 2; i++)
        {
            if (arrangedPlayers[i].get().school == arrangedPlayers[playerCount - i].get().school){

                if (i >= playerCount / 2){
                    // swap with back if there is no player in front of it to swap with
                    auto temp = arrangedPlayers[playerCount - i];
                    arrangedPlayers[playerCount - i] = arrangedPlayers[playerCount - i + 1];
                    arrangedPlayers[playerCount - i - 1] = temp;
                    
                }else{
                    //swap with front
                    auto temp = arrangedPlayers[playerCount - i];
                    arrangedPlayers[playerCount - i] = arrangedPlayers[playerCount - i - 1];
                    arrangedPlayers[playerCount - i - 1] = temp;
                }

            }
        }


        for (int i = 0; i < arrangedPlayers.size() / 2; i++)
        {
            firstRoundMatches.push_back(Match(arrangedPlayers[i], arrangedPlayers[playerCount - i]));
        }

        // rearrange the matches orders
        rounds.push_back(rearrangeMatches(firstRoundMatches));

        return;
    }

    vector<Match> rearrangeMatches(vector<Match> &matches)
    {
        const int size = matches.size();

        vector<int> matchesOrderMapping = Mapping::getMapping(matches.size());
        if (matchesOrderMapping.size() == 0) return matches;

        vector<Match> rearrangedMatches;

        for (size_t i = 0; i < matches.size(); i++)
        {
            rearrangedMatches.push_back(matches[matchesOrderMapping[i]]);
        }

        return rearrangedMatches;

    }

    void proceedNextRound(vector<Match> &matches)
    {
        vector<reference_wrapper<Player>> winners;
        vector<Match> progressedMatches;

        for (Match &match : matches)
        {
            winners.push_back(match.simulateMatch());   
        }

        if (winners.size() == 1)
        {
            return;
        }

        for (int i = 0; i < winners.size(); i += 2)
        {
            progressedMatches.push_back(Match(winners[i], winners[i + 1]));
        }

        rounds.push_back(progressedMatches);
    }

    string getRoundName(int numberOfPlayers)
    {
        if (numberOfPlayers > 4)
            return "Round of " + to_string(numberOfPlayers);
        if (numberOfPlayers == 4)
            return "Quarterfinals";
        if (numberOfPlayers == 2)
            return "Semifinals";
        if (numberOfPlayers == 1)
            return "Final";
        return "";
    }

    vector<reference_wrapper<Player>> groupSeededPlayers()
    {
        vector<reference_wrapper<Player>> seededPlayers;
        for (int i = 0; i < players.size(); i++)
        {
            if (players[i].seed)
            {
                seededPlayers.push_back(players[i]);
            }
        }
        return seededPlayers;
    }
    vector<reference_wrapper<Player>> groupUnseededPlayers()
    {
        vector<reference_wrapper<Player>> unseededPlayers;
        for (int i = 0; i < players.size(); i++)
        {
            if (!players[i].seed)
            {
                unseededPlayers.push_back(players[i]);
            }
        }
        return unseededPlayers;
    }

    vector<reference_wrapper<Player>> mergeAndFillPlayers(vector<reference_wrapper<Player>> &seededPlayers, vector<reference_wrapper<Player>> &unseededPlayers)
    {
        random_device rd;
        mt19937 gen(rd());

        vector<reference_wrapper<Player>> mergedPlayers;

        shuffle(seededPlayers.begin(), seededPlayers.end(), gen);

        shuffle(unseededPlayers.begin(), unseededPlayers.end(), gen);

        // merge the 2 vectors back to 1
        for (int i = 0; i < seededPlayers.size(); i++)
        {
            mergedPlayers.push_back(seededPlayers[i]);
        }
        for (int i = 0; i < unseededPlayers.size(); i++)
        {
            mergedPlayers.push_back(unseededPlayers[i]);
        }

        int numberOfByePlayers = getNumberOfMatches(players.size()) * 2 - players.size();
        createByePlayers(numberOfByePlayers);
        for (int i = 0; i < numberOfByePlayers; i++)
        {
            mergedPlayers.push_back(byePlayers[i]);
        }
        return mergedPlayers;
    }
};




int main()
{
    // generate random seed
    srand(time(nullptr));   

    // create output files
    ofstream tourReportFile("tournamentReport.txt");
    ofstream playerListFile("playerList.txt");



    // create tournament object and load in players
    Tournament tournament;


    string playerLoadFileName;
    ifstream playerLoadFile;

    // validate player file
    do {
        cout<<"File name of players to load in: ";
        cin>>playerLoadFileName;
        playerLoadFile.open(playerLoadFileName);
    }while(!PlayerLoader::isValidFile(playerLoadFile));

        
    PlayerLoader::loadPlayers(tournament.players, playerLoadFile);

    tournament.simulateTournament();

    tournament.printTo(tournament.generatePlayerList(), cout);
    tournament.printTo(tournament.generatePlayerList(), playerListFile);

    tournament.printTo(tournament.generateTournamentReport(), cout);
    tournament.printTo(tournament.generateTournamentReport(), tourReportFile);


    tourReportFile.close();
    playerListFile.close();

    
    // check player records
    string userInput;
    while(true) {
        cout<<"Enter N or n to exit"<<endl; 
        cout<<"Enter player ID to see their records: ";
        cin>>userInput;
        cout<<endl;

        if(userInput == "N" || userInput == "n") break;

        int id;
        try{
            id = stoi(userInput);
        }catch(const std::invalid_argument & e){
            cout<<"Invalid input: not a number"<<endl;
            continue;
        }catch(const std::out_of_range & e){
            cout<<"Invalid input: number too large / too small"<<endl;
            continue;
        }

        // validate inputs
        if (id > tournament.players.size() || id <= 0) {
            cout<<"Invalid input: out of range"<<endl;
            continue;
        }

        tournament.printTo(tournament.generatePlayerReport(id), cout);
    };
    cout<< "Thank you for using this program!"<<endl;


    return 0;
}