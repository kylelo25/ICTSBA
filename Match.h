#ifndef MATCH_H
#define MATCH_H


#include <iostream>
#include <sstream>
#include "Player.h"


class Match
{
private:
    int winnerIndex;

public:
    Player &p1;
    Player &p2;

    // Constructor: takes 2 Players by reference
    Match(Player &_p1, Player &_p2) : p1(_p1), p2(_p2) {};

    Player &simulateMatch()
    {
        int randomNum = rand() % (p1.strengthRating + p2.strengthRating) + 1;
        if (randomNum <= p1.strengthRating){
            winnerIndex = 0;
            return p1;
        }
        else{
            winnerIndex = 1;
            return p2;
        }
    }
    
    string getMatchResult(){
        stringstream ss;
        ss<<p1.name;
        ss<<(p1.seed ? "*" : "");

        ss<<" vs ";
        ss<<p2.name;
        ss<<(p2.seed ? "*" : "");

        ss<<endl;

        ss<<"Match winner: ";
        ss<<getWinner().name;
        ss<<(getWinner().seed ? "*" : "")<<endl;

        return ss.str();
    }
    
    Player &getWinner(){
        return winnerIndex == 0 ? p1 : p2;
    }



};

#endif