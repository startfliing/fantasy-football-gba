#ifndef __SEASON__
#define __SEASON__

#define SEASON_WEEKS 18
#define SEASON_TEAMS 32
#define SEASON_MAX_GAMES_PER_WEEK 16

// team1/team2 are 0-based team indices, matching buildTeams()'s parameters
struct WeekMatchup{
    int team1;
    int team2;
    int team1Score;
    int team2Score;
};

struct TeamSeasonRecord{
    int wins;
    int losses;
    int pointsFor;
    int pointsAgainst;
};

struct WeekSchedule{
    int gameCount;
    WeekMatchup games[SEASON_MAX_GAMES_PER_WEEK];
    int byeTeam1; // -1 if every team plays this week
    int byeTeam2;
};

struct Season{
    int currentWeek; // 0-based, 0..SEASON_WEEKS-1
    WeekSchedule weeks[SEASON_WEEKS];
    TeamSeasonRecord teamRecords[SEASON_TEAMS];
};

// builds a fresh 18-week schedule: every team plays 17 games and gets exactly
// one week off, with matchups shuffled (via qran()) for variety between seasons
void generateSeasonSchedule(Season* season);
void recordWeekResult(Season* season, int weekIndex, int gameIndex, int team1Score, int team2Score);
void getSortedStandings(const Season* season, int sortedTeams[SEASON_TEAMS]);

extern Season currSeason;

#endif
