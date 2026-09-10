#ifndef __POS_LUT__
#define __POS_LUT__

extern const char* positions[21];

extern const char* fullPositions[21];

// Indices into positions[], matches pos_id in players.csv/players_data
enum PosId{
    POS_NA   = 0,
    POS_C    = 1,
    POS_CB   = 2,
    POS_DT   = 3,
    POS_RB   = 4,
    POS_S    = 5,
    POS_K    = 6,
    POS_LE   = 7,
    POS_LG   = 8,
    POS_LOLB = 9,
    POS_LS   = 10,
    POS_LT   = 11,
    POS_MLB  = 12,
    POS_P    = 13,
    POS_QB   = 14,
    POS_RE   = 15,
    POS_RG   = 16,
    POS_ROLB = 17,
    POS_RT   = 18,
    POS_TE   = 19,
    POS_WR   = 20
};

#endif