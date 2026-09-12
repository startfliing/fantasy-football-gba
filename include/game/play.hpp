#ifndef __PLAY__
#define __PLAY__

#include "tonc.h"

struct IngamePlayer{
    int playerInd;          //players_data
    int dailyPerformance;   //how is the player today, negative or positive
};

struct OffenseRoster{
    IngamePlayer qb;
    IngamePlayer wr1;
    IngamePlayer wr2;
    IngamePlayer rb1;
    IngamePlayer rb2;
    IngamePlayer lot;
    IngamePlayer log;
    IngamePlayer c;
    IngamePlayer rog;
    IngamePlayer rot;
    IngamePlayer te;
};

struct DefenseRoster{
    IngamePlayer le;
    IngamePlayer re;
    IngamePlayer dt1;
    IngamePlayer dt2;
    IngamePlayer lolb;
    IngamePlayer mlb;
    IngamePlayer rolb;
    IngamePlayer cb1;
    IngamePlayer cb2;
    IngamePlayer s1;
    IngamePlayer s2;
};

struct SpecialTeams{
    IngamePlayer kicker;
    IngamePlayer punter;
};

struct IngameTeam{
    int teamInd;
    int score;
    OffenseRoster offense;
    DefenseRoster defense;
    SpecialTeams special;
};

struct Game{
    IngameTeam team1;
    IngameTeam team2;
};

enum PlayType{
    PLAY_RUN,
    PLAY_PASS,
    PLAY_PUNT,
    PLAY_FIELD_GOAL
};

// down/distance/field position/clock. yardLine is 0-100, measured from the
// possessing team's own goal line (100 = opponent's goal line / touchdown)
struct GameSituation{
    int quarter;    //Quarter
    int clock;      //Time left in current Quarter
    int down;       //down
    int distance;   //distance for first down
    int yardLine;   //where team is
    int possession; // 0 = team1 has the ball, 1 = team2 has the ball
};

//if the team 1 is 3rd and 12 at the 45 yard line in the 4th with 15 seconds left
//you could write it as {possession} {down} and {distance} at the {yardLine} in {quarter} with {clock}

struct PlayResult{
    PlayType type;
    int yards;
    bool touchdown;
    bool turnover;
    bool incomplete;
    bool scoringKick;
    int offenseCredit; //playerInd of the player who made the play (rusher/receiver/passer/kicker)
    int defenseCredit; //playerInd of the defender involved (tackle/int), or -1
};

void buildTeams(Game* game, int team1, int team2);

void initGameSituation(GameSituation* situation);

PlayResult simulatePlay(IngameTeam* offenseTeam, IngameTeam* defenseTeam, GameSituation* situation);

void applyPlayResult(GameSituation* situation, IngameTeam* offenseTeam, IngameTeam* defenseTeam, const PlayResult& result);

// advances the game by one play, returns false once the game has ended (after Q4)
// outResult, if non-null, is filled in with the play that was just resolved
bool stepPlay(Game* game, GameSituation* situation, PlayResult* outResult = nullptr);

#endif