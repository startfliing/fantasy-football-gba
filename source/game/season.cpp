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
    //(weeks 1..SEASON_MAX_GAMES_PER_WEEK, leaving the first and last week full)
    int byeWeek[SEASON_TEAMS];
    for(int t = 0; t < SEASON_TEAMS; t++) byeWeek[t] = -1;
    for(int k = 0; k < SEASON_MAX_GAMES_PER_WEEK; k++){
        int wk = k + 1;
        byeWeek[order[2*k]] = wk;
        byeWeek[order[2*k + 1]] = wk;
    }

    //tracks which teams have already faced each other this season, so week
    //pairings can avoid repeat matchups
    bool played[SEASON_TEAMS][SEASON_TEAMS];
    for(int a = 0; a < SEASON_TEAMS; a++)
        for(int b = 0; b < SEASON_TEAMS; b++)
            played[a][b] = false;

    season->currentWeek = 0;

    for(int wk = 0; wk < SEASON_WEEKS; wk++){
        WeekSchedule& week = season->weeks[wk];
        week.gameCount = 0;
        week.byeTeam1 = -1;
        week.byeTeam2 = -1;

        int active[SEASON_TEAMS];
        int activeCount = 0;
        for(int t = 0; t < SEASON_TEAMS; t++){
            if(byeWeek[t] == wk){
                if(week.byeTeam1 < 0) week.byeTeam1 = t;
                else week.byeTeam2 = t;
            }else{
                active[activeCount++] = t;
            }
        }

        //try a handful of shuffles and keep whichever pairing has the fewest
        //repeat matchups; this reliably finds a fully repeat-free pairing
        WeekMatchup bestGames[SEASON_MAX_GAMES_PER_WEEK];
        int bestGameCount = 0;
        int bestRepeats = 1 << 30;

        for(int attempt = 0; attempt < 30 && bestRepeats > 0; attempt++){
            int cand[SEASON_TEAMS];
            for(int t = 0; t < activeCount; t++) cand[t] = active[t];
            for(int i = activeCount - 1; i > 0; i--){
                int j = qran() % (i + 1);
                int tmp = cand[i]; cand[i] = cand[j]; cand[j] = tmp;
            }

            bool used[SEASON_TEAMS];
            for(int t = 0; t < SEASON_TEAMS; t++) used[t] = false;

            WeekMatchup games[SEASON_MAX_GAMES_PER_WEEK];
            int gameCount = 0;
            int repeats = 0;

            for(int i = 0; i < activeCount; i++){
                int t1 = cand[i];
                if(used[t1]) continue;

                int t2 = -1;
                for(int j = i + 1; j < activeCount; j++){
                    if(used[cand[j]] || played[t1][cand[j]]) continue;
                    t2 = cand[j];
                    break;
                }
                //fallback: everyone left has already played t1 this season
                if(t2 < 0){
                    for(int j = i + 1; j < activeCount; j++){
                        if(!used[cand[j]]){ t2 = cand[j]; break; }
                    }
                    repeats++;
                }

                used[t1] = true;
                used[t2] = true;
                games[gameCount].team1 = t1;
                games[gameCount].team2 = t2;
                gameCount++;
            }

            if(repeats < bestRepeats){
                bestRepeats = repeats;
                bestGameCount = gameCount;
                for(int g = 0; g < gameCount; g++) bestGames[g] = games[g];
            }
        }

        for(int g = 0; g < bestGameCount; g++){
            int t1 = bestGames[g].team1;
            int t2 = bestGames[g].team2;
            played[t1][t2] = true;
            played[t2][t1] = true;
            week.games[week.gameCount++] = bestGames[g];
        }
    }
}
