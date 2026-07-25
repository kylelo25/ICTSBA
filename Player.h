#ifndef PLAYER_H
#define PLAYER_H

#include <string>

using namespace std;

class Player
{
public:
    int id;
    string name;
    bool seed;
    string school;

    int strengthRating;

    //Default constructor - represents a "Bye" (Empty slot)
    Player()
    {
        name = "Bye";
        seed = false;
        school = "";

        strengthRating = 0;
    }

    //constructor - real competitor
    Player(string _name, bool _seed, string _school)
    {
        name = _name;
        seed = _seed;
        school = _school;
        if (seed)
            strengthRating = 70;
        if (!seed)
            strengthRating = 30;
    }
};

#endif