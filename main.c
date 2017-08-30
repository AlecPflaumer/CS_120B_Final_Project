/////////////////////////////////////////////

/* Alec Pflaumer apfla001@ucr.edu
 * Lab Section 21
 * Custom Lab
 * Simon Game
 */

/////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>

#include <avr/io.h>
#include "io.c"

/////////////////////////////////////////////

#include <avr/sleep.h>
#include <avr/interrupt.h>

volatile unsigned char TimerFlag = 0;

unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0;

void TimerOn(){
    TCCR1B = 0x0B;
    OCR1A = 125;
    TIMSK1 = 0x02;
    TCNT1 = 0;
    _avr_timer_cntcurr = _avr_timer_M;
    SREG |= 0x80;
}

void TimerOff(){
    TCCR1B = 0x00;
}

void TimerISR();

ISR(TIMER1_COMPA_vect){
    _avr_timer_cntcurr--;
    if (_avr_timer_cntcurr == 0){
        TimerISR();
        _avr_timer_cntcurr = _avr_timer_M;
    }
}

void TimerSet(unsigned long M){
    _avr_timer_M = M;
    _avr_timer_cntcurr = _avr_timer_M;
}

/////////////////////////////////////////////

void set_PWM(double frequency) {
    static double current_frequency;
    if (frequency != current_frequency) {
        if (!frequency) { TCCR3B &= 0x08; }
        else { TCCR3B |= 0x03; }
        if (frequency < 0.954) { OCR3A = 0xFFFF; }
        else if (frequency > 31250) { OCR3A = 0x0000; }
        else { OCR3A = (short)(8000000 / (128 * frequency)) - 1; }
        TCNT3 = 0;
        current_frequency = frequency;
    }
}

void PWM_on() {
    TCCR3A = (1 << COM3A0);
    TCCR3B = (1 << WGM32) | (1 << CS31) | (1 << CS30);
    set_PWM(0);
}

void PWM_off() {
    TCCR3A = 0x00;
    TCCR3B = 0x00;
}

/////////////////////////////////////////////

unsigned char SetBit(unsigned char x, unsigned char k, unsigned char b){
    return (b ? x | (0x01 << k) : x & ~(0x01 << k));
}

unsigned char GetBit(unsigned char x, unsigned char k){
    return ((x & (0x01 << k)) != 0);
}

/////////////////////////////////////////////

double note_C4 = 261.63;
double note_D4 = 293.66;
double note_E4 = 329.63;
double note_F4 = 349.23;
double note_G4 = 392.00;
double note_A5 = 440.00;
double note_B5 = 493.88;
double note_C5 = 523.25;

/////////////////////////////////////////////

// Task Struct Definition
typedef struct task{
	int state;
	unsigned long period;
	unsigned long elapsedTime;
	int (*TickFct)(int);
} task;

// Program Settings
task tasks[4];
const unsigned char tasksNum = 4;
const unsigned long tasksPeriodGCD = 50;

// Global Variables
unsigned char pattern[9];
unsigned char pIndex;
unsigned char numWins, numLosses;

// Flags
unsigned char firstRound, startNew, playSeq, getInput, endGame, gameWon, gameLost;

// State Settings and Tick Functions

// New Game SM
const unsigned long period_NewGame = 100;
enum NG_States { NG_Init, NG_Idle, NG_Setup, NG_Wait1, NG_Wait2 } NG_State;
int TickFct_NewGame(){
	
	// Transitions
	switch (NG_State){
		case NG_Init :
			NG_State = NG_Setup;
			firstRound = 1;
			startNew   = 1;
			numWins    = 0;
			numLosses  = 0;
 			break;
		case NG_Idle :
			if (startNew) { NG_State = NG_Setup; }
			else		  { NG_State = NG_Idle;  }
			break;
		case NG_Setup :
			if (!firstRound){
				NG_State = NG_Wait2;
			}
			else {
				NG_State = NG_Wait1;
				LCD_DisplayString(1, "Welcome! Press abutton to begin.");
			}
			break;
		case NG_Wait1 :
			if (0x0F & ~PINA){
				NG_State = NG_Wait2;
			}
			else {
				NG_State = NG_Wait1;
			}
			break;
		case NG_Wait2 :
			if (!(0x0F & ~PINA)){
				NG_State = NG_Idle;
				startNew   = 0;
				firstRound = 0;
				playSeq  = 1;
				LCD_ClearScreen();
				LCD_DisplayString(1, "Wins:           Losses: ");
				LCD_Cursor(9);  LCD_WriteData(numWins + '0');
				LCD_Cursor(25); LCD_WriteData(numLosses + '0');
				LCD_Cursor(32);
			}
			else {
				NG_State = NG_Wait2;
			}
			break;
		default :
			NG_State = NG_Init;
			break;
	}
	
	// State Actions
	switch (NG_State){
		case NG_Init  : break;
		case NG_Idle  : break;
		case NG_Setup :
			for (unsigned char i=0; i<9; i++){ pattern[i] = rand() % 4 + 1; }
			pIndex = 0;
			break;
		case NG_Wait1 : break;
		case NG_Wait2 : break;
		default		  : break;
	}
	
	return NG_State;
};

// Output Sequence SM
const unsigned long period_OutputSeq = 300;
enum OS_States { OS_Init, OS_Idle, OS_Play, OS_Quiet } OS_State;
int TickFct_OutputSeq(){

	// Local Variables
	static unsigned char i;
	
	// Transitions
	switch (OS_State){
		case OS_Init :
			playSeq  = 0;
			OS_State = OS_Idle;
			break;
		case OS_Idle :
			if (playSeq){
				OS_State = OS_Quiet;
				i = 0;
			}
			else {
				OS_State = OS_Idle;
			}
			break;
		case OS_Play :
			OS_State = OS_Quiet;
			break;
		case OS_Quiet :
			if (i <= pIndex){
				OS_State = OS_Play;
			}
			else {
				OS_State = OS_Idle;
				playSeq  = 0;
				getInput = 1;
			}
			break;
	}
	
	// State Actions
	switch (OS_State){
		case OS_Init : break;
		case OS_Idle : break;
		case OS_Play :
			switch (pattern[i]){
				case 1: PORTB = 0x01; set_PWM(note_C5); break;
				case 2: PORTB = 0x02; set_PWM(note_G4); break;
				case 3: PORTB = 0x04; set_PWM(note_E4); break;
				case 4: PORTB = 0x08; set_PWM(note_C4); break;
			}
			i++;
			break;
		case OS_Quiet :
			PORTB = 0x00; set_PWM(0);
	//		i++;
			break;
	}
	
	return OS_State;
};

// Input Sequence SM
const unsigned long period_InputSeq = 50;
enum IS_States { IS_Init, IS_Idle, IS_Wait, IS_Display } IS_State;
int TickFct_InputSeq(){
	
	// Local Variables
	static unsigned char i;
	static unsigned char dispCount;
	static unsigned char userInput;
	
	// Transitions
	switch (IS_State){
		case IS_Init:
			IS_State = IS_Idle;
			getInput = 0;
			userInput = 0;
			break;
		case IS_Idle :
			if (getInput){
				IS_State = IS_Wait;
				i = 0;
			}
			else {
				IS_State = IS_Idle;
			}
			break;
		case IS_Wait :
			if (userInput){
				IS_State = IS_Display;
				dispCount = 0;
			}
			else {
				IS_State = IS_Wait;
			}
			break;
		case IS_Display :
			if (0x0F & ~PINA){
				IS_State = IS_Display;
			}
			else if (gameWon || gameLost){
				IS_State = IS_Idle;
				PORTB = 0x00; set_PWM(0);
				getInput = 0;
				endGame = 1;
			}
			else if (i == pIndex){
				IS_State = IS_Idle;
				PORTB = 0x00; set_PWM(0);
				pIndex++;
				getInput = 0;
				playSeq  = 1;
			}
			else {
				IS_State = IS_Wait;
				PORTB = 0x00; set_PWM(0);
				i++;
			}
			break;
	}
	
	// State Actions
	switch (IS_State){
		case IS_Init : break;
		case IS_Idle : break;
		case IS_Wait :
			switch (0x0F & ~PINA){ 
				case 0x01 : userInput = 1; break;
				case 0x02 : userInput = 2; break;
				case 0x04 : userInput = 3; break;
				case 0x08 : userInput = 4; break;
				default   : userInput = 0; break;
			}
			if ((userInput) && (userInput != pattern[i])){
				gameLost = 1;
				numLosses++;
			}
			else if ((i == 8) && (userInput == pattern[i])){
				gameWon = 1;
				numWins++;
			}
			break;
		case IS_Display :
			switch (userInput){
				case 1: PORTB = 0x01; set_PWM(note_C5); break;
				case 2: PORTB = 0x02; set_PWM(note_G4); break;
				case 3: PORTB = 0x04; set_PWM(note_E4); break;
				case 4: PORTB = 0x08; set_PWM(note_C4); break;
			}
			dispCount++;
			break;
	}
	
	return IS_State;
};

// End Game SM
const unsigned long period_EndGame = 200;
enum EG_States { EG_Init, EG_Idle, EG_Win, EG_Lose } EG_State;
int TickFct_EndGame(){
	
	// Local Variables
	static unsigned char i;
	static unsigned char flip;
	
	// Transitions
	switch (EG_State){
		case EG_Init :
			EG_State = EG_Idle;
			endGame  = 0;
			gameWon  = 0;
			gameLost = 0;
			break;
		case EG_Idle :
			if (endGame){
				if (gameWon && !gameLost){
					EG_State = EG_Win;
					i=1;
					LCD_DisplayString(1, "You Win! Press  to play again.");
				}
				else if (gameLost && !gameWon){
					EG_State = EG_Lose;
					i=1;
					flip=0;
					LCD_DisplayString(1, "Bummer! Press to play again.");
				}
			}
			else {
				EG_State = EG_Idle;
			}
			break;
		case EG_Win :
			if (0x0F & ~PINA){
				PORTB = 0x00; set_PWM(0);
				EG_State = EG_Idle;
				gameWon = 0;
				endGame = 0;
				startNew = 1;
			}
			else {
				EG_State = EG_Win;
			}
			break;
		case EG_Lose :
			if (0x0F & ~PINA){
				PORTB = 0x00; set_PWM(0);
				EG_State = EG_Idle;
				gameLost = 0;
				endGame  = 0;
				startNew = 1;
			}
			else {
				EG_State = EG_Lose;
			}
			break;
	}
	
	// State Actions
	switch (EG_State){
		case EG_Init : break;
		case EG_Idle : break;
		case EG_Win  :
			switch (i){
				case 1: PORTB = 0x01; set_PWM(note_C5); i++; break;
				case 2: PORTB = 0x02; set_PWM(note_G4); i++; break;
				case 3: PORTB = 0x04; set_PWM(note_E4); i++; break;
				case 4: PORTB = 0x08; set_PWM(note_C4); i=1; break;
			}
			break;
		case EG_Lose :
			PORTB = flip ? 0x0F : 0x00;
			set_PWM(flip ? note_C4 : 0);
			if (i==2){ i=0; flip=!flip; }
			else { i++; }
			break;
	}
		
	return EG_State;
};

// Timer ISR
void TimerISR(){
	for (unsigned char i=0; i < tasksNum; i++){
		if (tasks[i].elapsedTime >= tasks[i].period){
			tasks[i].state = tasks[i].TickFct(tasks[i].state);
			tasks[i].elapsedTime = 0;
		}
		tasks[i].elapsedTime += tasksPeriodGCD;
	}
}

// Main
int main(){
	
	// PORT Initialization
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;
	
	// Tasks Initialization
	unsigned char i=0;
	tasks[i].state		 = NG_Init;
	tasks[i].period		 = period_NewGame;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct	 = &TickFct_NewGame;
	i++;
	tasks[i].state		 = OS_Init;
	tasks[i].period		 = period_OutputSeq;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct	 = &TickFct_OutputSeq;
	i++;
	tasks[i].state		 = IS_Init;
	tasks[i].period		 = period_InputSeq;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct	 = &TickFct_InputSeq;
	i++;
	tasks[i].state		 = EG_Init;
	tasks[i].period		 = period_EndGame;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct	 = &TickFct_EndGame;
	
	srand(0);
	PWM_on();
	LCD_init();
	TimerSet(tasksPeriodGCD);
	TimerOn();

	set_sleep_mode(SLEEP_MODE_IDLE);
	while(1){ sleep_mode(); }

	return 0;
}