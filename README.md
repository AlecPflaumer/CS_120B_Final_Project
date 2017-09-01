# CS_120B_Final_Project

=======================================================

GitHub:       https://github.com/AlecPflaumer/CS_120B_Final_Project

YouTube Demo: https://youtu.be/oDh5_MYNC7A

=======================================================

High-Level Explanation:

(Relevant) Global Variables:
pattern[9], pIndex, numWins, and numLosses (all unsigned chars)

Synch SMs:

NewGame:
1. Performs its duties at the start of every new game, i.e. upon powering the system on and directly after winning or losing a round and restarting.
2. Sets up pattern to hold 9 random numbers from 1-4 corresponding to the four different LEDs that can light up.
3. Sets pIndex, which keeps track of how far into the pattern the user has guessed correctly so far, to 0.
4. If it’s the first game (upon powering on), then the system will prompt the user to press a button to start. If it’s any subsequent game, the user has already been prompted to press a button to start a new game from the EndGame SM.
5. Once the user presses and releases a button, the system displays the number of wins and losses on the LCD screen and goes back to its Idle state and tells OutputSeq to perform its duties.

OutputSeq:
1. Lights up the LEDs and sounds buzzer according to pattern and up until pIndex, ignoring input along the way.
2. Once it’s done it returns to its Idle state and tells InputSeq to begin.

InputSeq
1. Waits for the user to press a button.
2. While the user is pressing a button it lights up the corresponding LED and sounds buzzer.
3. Steps a-c repeat until the user has correctly imitated pattern up until pIndex or the user has entered the wrong button and lost.
4. If the user has won (which increments numWins) or lost (which increments numLosses) the game the SM returns to its Idle state and tells EndGame to start its processes.
5. If the user has correctly entered up until pIndex the SM returns to Idle state, increments pIndex, and tells OutputSeq to start again.

EndGame
1. If the user has won the game then the LEDs light up in a celebratory pattern and tell the user they have won and to press a button to play again.
2. If the user has lost the game then LEDs flash and tell the user they have lost and to press a button to play again.
3. Once the user has a pressed a button EndGame returns to its Idle state and tells NewGame to begin.

=======================================================
