// set roster

//exit states: ingame, title?
#include "pregame.hpp"
#include "terminal.hpp"
#include "ingame.hpp"
#include "save.hpp"

#include "team_lut.hpp"
#include "play.hpp"
#include "season.hpp"

// Pregame state implementation
// Builds a fresh 18-week season schedule, then hands off to ingameState,
// which plays every week's games and iterates through the whole season
GameState pregameState(){

    saveData* sd = getSaveData();
    sqran(sd->currSeed);
    Terminal::reset();

    Terminal::log("Building season schedule...");
    generateSeasonSchedule(&currSeason);

    Terminal::reset();
    Terminal::log("18-week season ready!");
    Terminal::log("Press Start to begin!");
    while(!key_hit(KEY_START)){
        key_poll();
        VBlankIntrWait();
    }
    key_poll();
    sd->currSeed = qran();
    save();

    return (GameState)&ingameState;
}