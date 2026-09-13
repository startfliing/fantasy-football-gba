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
    for(int t = 0; t < SEASON_TEAMS; t++){
        season->teamRecords[t] = {0, 0, 0, 0, 0};
    }

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
            WeekMatchup& matchup = week.games[week.gameCount++];
            matchup = bestGames[g];
            matchup.team1Score = -1;
            matchup.team2Score = -1;
        }
    }
}

void recordWeekResult(Season* season, int weekIndex, int gameIndex, int team1Score, int team2Score){
    WeekMatchup& matchup = season->weeks[weekIndex].games[gameIndex];
    if(matchup.team1Score >= 0) return;

    matchup.team1Score = team1Score;
    matchup.team2Score = team2Score;

    TeamSeasonRecord& team1Record = season->teamRecords[matchup.team1];
    TeamSeasonRecord& team2Record = season->teamRecords[matchup.team2];
    team1Record.pointsFor += team1Score;
    team1Record.pointsAgainst += team2Score;
    team2Record.pointsFor += team2Score;
    team2Record.pointsAgainst += team1Score;

    if(team1Score > team2Score){
        team1Record.wins++;
        team2Record.losses++;
    }else if(team2Score > team1Score){
        team2Record.wins++;
        team1Record.losses++;
    }else{
        team1Record.ties++;
        team2Record.ties++;
    }
}

static bool compareTeamStandings(const Season* season, int a, int b){
    const TeamSeasonRecord& recA = season->teamRecords[a];
    const TeamSeasonRecord& recB = season->teamRecords[b];

    int totalA = recA.wins + recA.losses + recA.ties;
    int totalB = recB.wins + recB.losses + recB.ties;

    // Tier 1: Record (winning percentage: (Wins + 0.5 * Ties) / TotalGames)
    // Scaled by 2 to keep integer precision: (2 * Wins + Ties) / (2 * TotalGames)
    int ptsA = (recA.wins * 2) + recA.ties;
    int ptsB = (recB.wins * 2) + recB.ties;

    int winRateDiff = 0;
    if(totalA > 0 && totalB > 0){
        winRateDiff = (ptsA * totalB) - (ptsB * totalA);
    }else if(totalA > 0){
        winRateDiff = (ptsA > 0) ? 1 : -1;
    }else if(totalB > 0){
        winRateDiff = (ptsB > 0) ? -1 : 1;
    }

    if(winRateDiff != 0){
        return winRateDiff > 0;
    }

    // Tier 2: Points scored so far (highest to lowest)
    if(recA.pointsFor != recB.pointsFor){
        return recA.pointsFor > recB.pointsFor;
    }

    // Tier 3: Points scored against (lowest to highest)
    if(recA.pointsAgainst != recB.pointsAgainst){
        return recA.pointsAgainst < recB.pointsAgainst;
    }

    // Tie-breaker: team index
    return a < b;
}

void getSortedStandings(const Season* season, int sortedTeams[SEASON_TEAMS]){
    for(int i = 0; i < SEASON_TEAMS; i++){
        sortedTeams[i] = i;
    }

    for(int i = 1; i < SEASON_TEAMS; i++){
        int key = sortedTeams[i];
        int j = i - 1;
        while(j >= 0 && compareTeamStandings(season, key, sortedTeams[j])){
            sortedTeams[j + 1] = sortedTeams[j];
            j--;
        }
        sortedTeams[j + 1] = key;
    }
}
