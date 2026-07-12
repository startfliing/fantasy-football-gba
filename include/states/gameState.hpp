#ifndef __GAME_STATE__
#define __GAME_STATE__

// GameState is a function pointer that takes no arguments and returns
// a pointer to the next GameState function
typedef fnptr* (*GameState)(void);

#endif
