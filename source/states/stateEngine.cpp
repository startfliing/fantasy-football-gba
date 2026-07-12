#include "intro.hpp"
#include "terminal.hpp"
#include "save.hpp"


int main(){

    // Initialize Interrupts
    irq_init(nullptr);
	irq_enable(II_VBLANK);

    load();

    // Initialize and run the state engine
    // Start with pregame state, then let each state return the next one
    GameState currentState = (GameState)introState;
    
    while(1){
        // Execute current state and get the next state
        // Each state decides what state to run next and returns it
        currentState = (GameState)currentState();

    }

    return 0;
}