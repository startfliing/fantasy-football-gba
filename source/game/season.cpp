#include "season.hpp"
#include "tonc.h"

Season currSeason;

void generateSeasonSchedule(Season* season){
    int order[SEASON_TEAMS];
    for(int t = 0; t < SEASON_TEAMS; t++) order[t] = t;

    //Fisher-Yates shuffle so the matchups vary between seasons
    for(int i = SEASON_TEAMS - 1; i > 0; i--){
        int j = qran() % (i + 1);
        int tmp = order[i]; order[i] = order[j]; order[j] = tmp;
    }

    //pair up the shuffled teams and give each pair a distinct week off
    //(weeks 1..SEASON_TEAMS/2, leaving the first and last week full)
    int byeWeek[SEASON_TEAMS];
    for(int t = 0; t < SEASON_TEAMS; t++) byeWeek[t] = -1;
    for(int k = 0; k < SEASON_MAX_GAMES_PER_WEEK; k++){
        int wk = k + 1;
        byeWeek[order[2*k]] = wk;
        byeWeek[order[2*k + 1]] = wk;
    }

    season->currentWeek = 0;

    for(int wk = 0; wk < SEASON_WEEKS; wk++){
        WeekSchedule& week = season->weeks[wk];
        week.gameCount = 0;

        //circle method: keep order[0] fixed, rotate the rest by `wk` steps to
        //get a fresh, non-repeating set of pairings each week
        int rotated[SEASON_TEAMS];
        rotated[0] = order[0];
        for(int i = 1; i < SEASON_TEAMS; i++){
            rotated[i] = order[1 + ((i - 1 + wk) % (SEASON_TEAMS - 1))];
        }

        int orphan[2];
        int orphanCount = 0;
        int byeTeams[2] = {-1, -1};
        int byeCount = 0;

        for(int i = 0; i < SEASON_TEAMS / 2; i++){
            int t1 = rotated[i];
            int t2 = rotated[SEASON_TEAMS - 1 - i];

            bool byeT1 = (byeWeek[t1] == wk);
            bool byeT2 = (byeWeek[t2] == wk);

            //track bye teams by arrival order, not by which pairing slot they
            //landed in, since both can end up as t1 (or both as t2)
            if(byeT1) byeTeams[byeCount++] = t1;
            if(byeT2) byeTeams[byeCount++] = t2;

            if(byeT1 && byeT2){
                //nothing to do, both scheduled teams are resting this week
            }else if(byeT1){
                orphan[orphanCount++] = t2;
            }else if(byeT2){
                orphan[orphanCount++] = t1;
            }else{
                WeekMatchup& matchup = week.games[week.gameCount++];
                matchup.team1 = t1;
                matchup.team2 = t2;
            }
        }

        week.byeTeam1 = byeTeams[0];
        week.byeTeam2 = byeTeams[1];

        //the two teams whose scheduled opponent got a bye instead play each other
        if(orphanCount == 2){
            WeekMatchup& matchup = week.games[week.gameCount++];
            matchup.team1 = orphan[0];
            matchup.team2 = orphan[1];
        }
    }
}
