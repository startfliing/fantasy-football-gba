#include "play.hpp"
#include "team_lut.hpp"
#include "pos_lut.hpp"
#include "players.h"

static const int QUARTER_LENGTH = 900; //abstract "ticks", not real seconds

typedef int Players::*StatPtr;

static int clampInt(int v, int lo, int hi){
    if(v < lo) return lo;
    if(v > hi) return hi;
    return v;
}

//reads a stat off a roster slot, folded together with that player's daily
//performance roll. falls back to a league-average value if the slot is empty
static int getStat(const IngamePlayer& p, StatPtr member, int fallback = 50){
    if(p.playerInd < 0 || p.playerInd >= players_count) return fallback;
    return clampInt(players_data[p.playerInd].*member + p.dailyPerformance, 0, 99);
}

static IngamePlayer makePlayer(int playerInd){
    IngamePlayer p;
    p.playerInd = playerInd;
    //small per-game variance: a good or bad day, roughly +/-10
    p.dailyPerformance = playerInd >= 0 ? ((int)(qran() % 21) - 10) : 0;
    return p;
}

//finds the `count` highest overall_rating players on `teamId` at `posId`.
//outInds must have room for `count` ints; unfilled slots are left as -1
static void findTopPlayers(int teamId, int posId, int count, int* outInds){
    for(int i = 0; i < count; i++) outInds[i] = -1;

    for(int p = 0; p < players_count; p++){
        const Players& player = players_data[p];
        if(player.team_id != teamId || player.pos_id != posId) continue;

        for(int slot = 0; slot < count; slot++){
            if(outInds[slot] == -1 || players_data[outInds[slot]].overall_rating < player.overall_rating){
                for(int shift = count - 1; shift > slot; shift--){
                    outInds[shift] = outInds[shift - 1];
                }
                outInds[slot] = p;
                break;
            }
        }
    }
}

void buildTeams(Game* game, int team1, int team2){
    int csvIds[2] = { team1 + 1, team2 + 1 };
    IngameTeam* teams[2] = { &game->team1, &game->team2 };

    for(int t = 0; t < 2; t++){
        int teamId = csvIds[t];
        IngameTeam* team = teams[t];
        team->teamInd = teamId;
        team->score = 0;

        int inds[2];

        findTopPlayers(teamId, POS_QB, 1, inds); team->offense.qb  = makePlayer(inds[0]);
        findTopPlayers(teamId, POS_WR, 2, inds); team->offense.wr1 = makePlayer(inds[0]); team->offense.wr2 = makePlayer(inds[1]);
        findTopPlayers(teamId, POS_RB, 2, inds); team->offense.rb1 = makePlayer(inds[0]); team->offense.rb2 = makePlayer(inds[1]);
        findTopPlayers(teamId, POS_LT, 1, inds); team->offense.lot = makePlayer(inds[0]);
        findTopPlayers(teamId, POS_LG, 1, inds); team->offense.log = makePlayer(inds[0]);
        findTopPlayers(teamId, POS_C,  1, inds); team->offense.c   = makePlayer(inds[0]);
        findTopPlayers(teamId, POS_RG, 1, inds); team->offense.rog = makePlayer(inds[0]);
        findTopPlayers(teamId, POS_RT, 1, inds); team->offense.rot = makePlayer(inds[0]);
        findTopPlayers(teamId, POS_TE, 1, inds); team->offense.te  = makePlayer(inds[0]);

        findTopPlayers(teamId, POS_LE,   1, inds); team->defense.le   = makePlayer(inds[0]);
        findTopPlayers(teamId, POS_RE,   1, inds); team->defense.re   = makePlayer(inds[0]);
        findTopPlayers(teamId, POS_DT,   2, inds); team->defense.dt1  = makePlayer(inds[0]); team->defense.dt2 = makePlayer(inds[1]);
        findTopPlayers(teamId, POS_LOLB, 1, inds); team->defense.lolb = makePlayer(inds[0]);
        findTopPlayers(teamId, POS_MLB,  1, inds); team->defense.mlb  = makePlayer(inds[0]);
        findTopPlayers(teamId, POS_ROLB, 1, inds); team->defense.rolb = makePlayer(inds[0]);
        findTopPlayers(teamId, POS_CB,   2, inds); team->defense.cb1  = makePlayer(inds[0]); team->defense.cb2 = makePlayer(inds[1]);
        findTopPlayers(teamId, POS_S,    2, inds); team->defense.s1   = makePlayer(inds[0]); team->defense.s2  = makePlayer(inds[1]);

        findTopPlayers(teamId, POS_K, 1, inds); team->special.kicker = makePlayer(inds[0]);
        findTopPlayers(teamId, POS_P, 1, inds); team->special.punter = makePlayer(inds[0]);
    }
}

void initGameSituation(GameSituation* situation){
    situation->quarter = 1;
    situation->clock = QUARTER_LENGTH;
    situation->down = 1;
    situation->distance = 10;
    situation->yardLine = 20;
    situation->possession = 0;
}

static int avgOLineBlock(const IngameTeam* off){
    int total = getStat(off->offense.lot, &Players::block)
              + getStat(off->offense.log, &Players::block)
              + getStat(off->offense.c,   &Players::block)
              + getStat(off->offense.rog, &Players::block)
              + getStat(off->offense.rot, &Players::block);
    return total / 5;
}

static int avgDLineTackle(const IngameTeam* def){
    int total = getStat(def->defense.le,  &Players::tackle)
              + getStat(def->defense.re,  &Players::tackle)
              + getStat(def->defense.dt1, &Players::tackle)
              + getStat(def->defense.dt2, &Players::tackle);
    return total / 4;
}

static int avgLbTackle(const IngameTeam* def){
    int total = getStat(def->defense.lolb, &Players::tackle)
              + getStat(def->defense.mlb,  &Players::tackle)
              + getStat(def->defense.rolb, &Players::tackle);
    return total / 3;
}

static PlayType choosePlayType(const GameSituation* situation){
    if(situation->down == 4){
        int kickDistance = (100 - situation->yardLine) + 17;
        if(kickDistance <= 50) return PLAY_FIELD_GOAL;
        if(situation->distance > 2 || situation->yardLine < 40) return PLAY_PUNT;
        //otherwise it's a 4th-and-short go-for-it; fall through to normal play call
    }

    int passChance = 55;
    if(situation->distance >= 7) passChance = 70;
    if(situation->distance <= 2) passChance = 40;

    return ((int)(qran() % 100) < passChance) ? PLAY_PASS : PLAY_RUN;
}

static PlayResult simulateRun(IngameTeam* off, IngameTeam* def){
    PlayResult r{};
    r.type = PLAY_RUN;
    r.defenseCredit = -1;

    bool useRb2 = off->offense.rb2.playerInd >= 0 && (int)(qran() % 100) < 25;
    IngamePlayer carrier = useRb2 ? off->offense.rb2 : off->offense.rb1;

    int carrying = getStat(carrier, &Players::carrying);
    int strength = getStat(carrier, &Players::strength);
    int juke     = getStat(carrier, &Players::jukeMove);
    int speed    = getStat(carrier, &Players::speed);

    int oLine = avgOLineBlock(off);
    int dFront = avgDLineTackle(def);

    int matchup = clampInt((oLine + strength - dFront), -60, 60);
    int baseYards = 2 + matchup / 8;

    bool breakaway = ((juke + speed) / 2 > 85) && ((int)(qran() % 100) < 10);
    int yards = baseYards + (int)(qran() % 5) - 2;
    if(breakaway) yards += 10 + (int)(qran() % 20);

    int fumbleChance = clampInt(2 + (60 - carrying) / 10, 0, 10);
    if((int)(qran() % 100) < fumbleChance){
        r.turnover = true;
        yards = clampInt(yards, -3, 3); //fumbles happen at/near contact
        r.defenseCredit = def->defense.mlb.playerInd;
    }

    r.yards = yards;
    r.offenseCredit = carrier.playerInd;
    return r;
}

static PlayResult simulatePass(IngameTeam* off, IngameTeam* def){
    PlayResult r{};
    r.type = PLAY_PASS;
    r.defenseCredit = -1;

    IngamePlayer qb = off->offense.qb;
    int throwAcc = getStat(qb, &Players::throwAccuracy);

    //pick a target: WR1 45%, WR2 30%, TE 15%, checkdown to RB1 10%
    int targetRoll = (int)(qran() % 100);
    IngamePlayer target;
    if(targetRoll < 45)      target = off->offense.wr1;
    else if(targetRoll < 75) target = off->offense.wr2;
    else if(targetRoll < 90) target = off->offense.te;
    else                      target = off->offense.rb1;

    int catching  = getStat(target, &Players::catching);
    int recSpeed  = getStat(target, &Players::speed);
    int juke      = getStat(target, &Players::jukeMove);

    //a random defensive back is in coverage
    int covRoll = (int)(qran() % 4);
    IngamePlayer defender = covRoll == 0 ? def->defense.cb1
                          : covRoll == 1 ? def->defense.cb2
                          : covRoll == 2 ? def->defense.s1
                                          : def->defense.s2;
    int coverage = getStat(defender, &Players::coverage);

    int passRush    = avgDLineTackle(def);
    int passProtect = avgOLineBlock(off);
    int pressure = clampInt(passRush - passProtect, -30, 30);

    int sackChance = clampInt(8 + pressure / 3, 2, 30);
    if((int)(qran() % 100) < sackChance){
        r.yards = -(3 + (int)(qran() % 6));
        r.offenseCredit = qb.playerInd;
        r.defenseCredit = def->defense.le.playerInd;
        return r;
    }

    int completionChance = clampInt(50 + (throwAcc - coverage) / 2 + (catching - 50) / 4 - pressure / 4, 10, 95);
    bool completed = (int)(qran() % 100) < completionChance;

    if(!completed){
        r.incomplete = true;
        r.offenseCredit = qb.playerInd;
        r.defenseCredit = defender.playerInd;

        int intChance = clampInt((coverage - throwAcc) / 6, 0, 12);
        if((int)(qran() % 100) < intChance){
            r.turnover = true;
        }
        return r;
    }

    int airYards = 4 + (int)(qran() % 10);
    int yacRoll = clampInt((juke + recSpeed) / 2 - avgLbTackle(def) / 2, 0, 30);
    int yac = (int)(qran() % (yacRoll + 1));

    r.yards = airYards + yac;
    r.offenseCredit = target.playerInd;
    r.defenseCredit = defender.playerInd;
    return r;
}

static PlayResult simulatePunt(IngameTeam* off){
    PlayResult r{};
    r.type = PLAY_PUNT;
    r.defenseCredit = -1;

    IngamePlayer punter = off->special.punter;
    int leg = getStat(punter, &Players::kickAccuracy);
    r.yards = 35 + (leg - 50) / 5 + (int)(qran() % 10) - 5;
    r.offenseCredit = punter.playerInd;
    return r;
}

static PlayResult simulateFieldGoal(IngameTeam* off, const GameSituation* situation){
    PlayResult r{};
    r.type = PLAY_FIELD_GOAL;
    r.defenseCredit = -1;

    IngamePlayer kicker = off->special.kicker;
    int acc = getStat(kicker, &Players::kickAccuracy);
    int distance = (100 - situation->yardLine) + 17;

    int makeChance = clampInt(95 - (distance - 20) + (acc - 50) / 2, 5, 99);
    r.scoringKick = (int)(qran() % 100) < makeChance;
    r.offenseCredit = kicker.playerInd;
    return r;
}

PlayResult simulatePlay(IngameTeam* offenseTeam, IngameTeam* defenseTeam, GameSituation* situation){
    PlayType type = choosePlayType(situation);
    switch(type){
        case PLAY_RUN:         return simulateRun(offenseTeam, defenseTeam);
        case PLAY_PASS:        return simulatePass(offenseTeam, defenseTeam);
        case PLAY_PUNT:        return simulatePunt(offenseTeam);
        case PLAY_FIELD_GOAL:  return simulateFieldGoal(offenseTeam, situation);
    }

    PlayResult r{};
    r.type = type;
    r.defenseCredit = -1;
    return r;
}

static void flipPossession(GameSituation* situation, int newYardLine){
    situation->possession = 1 - situation->possession;
    situation->yardLine = clampInt(newYardLine, 1, 99);
    situation->down = 1;
    situation->distance = 10;
}

static void score(IngameTeam* offenseTeam, int points){
    offenseTeam->score += points;
}

void applyPlayResult(GameSituation* situation, IngameTeam* offenseTeam, IngameTeam* defenseTeam, const PlayResult& result){
    situation->clock -= 15 + (int)(qran() % 25);

    if(result.type == PLAY_FIELD_GOAL){
        if(result.scoringKick) score(offenseTeam, 3);
        flipPossession(situation, 20);
        return;
    }

    if(result.type == PLAY_PUNT){
        flipPossession(situation, 100 - (situation->yardLine + result.yards));
        return;
    }

    if(result.turnover){
        flipPossession(situation, 100 - clampInt(situation->yardLine + result.yards, 0, 100));
        return;
    }

    situation->yardLine = clampInt(situation->yardLine + result.yards, 0, 100);

    if(situation->yardLine >= 100){
        score(offenseTeam, 7); //TD + automatic PAT for MVP
        flipPossession(situation, 20);
        return;
    }

    if(result.yards >= situation->distance){
        situation->down = 1;
        situation->distance = 10;
    }else{
        situation->distance -= result.yards;
        situation->down++;
        if(situation->down > 4){
            //turnover on downs
            flipPossession(situation, 100 - situation->yardLine);
        }
    }
}

bool stepPlay(Game* game, GameSituation* situation, PlayResult* outResult){
    IngameTeam* off = (situation->possession == 0) ? &game->team1 : &game->team2;
    IngameTeam* def = (situation->possession == 0) ? &game->team2 : &game->team1;

    PlayResult result = simulatePlay(off, def, situation);
    applyPlayResult(situation, off, def, result);

    if(outResult) *outResult = result;

    if(situation->clock <= 0){
        situation->quarter++;
        situation->clock = QUARTER_LENGTH;
        if(situation->quarter > 4){
            return false;
        }
    }

    return true;
}