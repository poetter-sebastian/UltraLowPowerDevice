/****************************************************************************
 * Copyright (C) 2021 by Lukas Brüggemann, Markus Reinhold, Sebastian Pötter
 ****************************************************************************/

/**
 * @file states.c
 * @author Lukas Brüggemann
 * @date 1 Jan 2021
 * @brief Controlls the state machine of the lock
 *
 * This file contains the state machine of the lock
 */

#include <stdio.h>
#include <stdbool.h>
#include <util/delay.h>
#include <avr/io.h>
#include "states.h"
#include "led.h"
#include "keypad.h"

#define INIT 0
#define KEY 1
#define OPEN 2
#define CHANGEKEY 3
#define NEWKEY 4
#define LOCKED 5


/// after 4 wrong keys => locked
int error = 0; 
int state = INIT;
int stateOld = INIT;
///current position of key
int pos = 0; 
/// init pin
char pin[4] = "****"; 
/// init puk
char puck[6] = "#12345"; 
int timeout = 0;
/// time out for open state
int openTimeout = 250; 


/*
INIT = Blue --
KEY = White
OPEN = Pink
CHANGEKEY = Yellow
NEWKEY = Red
Input = Green
LOCKED = RED
INPUT correct = Green
INOUT incorrect = White

Input correct = Green 
Timeout = light Blue
*/

/**
* @brief States of lock funktionality
* 
* @details Reads pressed button and depending on current state and pressed button to handle locks. 
* Depending on state of the lock the indicator LED is set.
*/

void stateM(void)
{   
	_delay_ms(1000);
    char readed = keypad();
    stateOld = state;
    if(error == 4)
    {
        pos = 0;
        state = LOCKED;
        error = 0;
    }
	
    switch (state) 
    {
		//start state 
        case INIT:
            setLED(false, false, true);
            
            //first key is right
            if(readed != 'x' && readed == pin[pos])
            {
                pos++;
                state = KEY;
                setLED(false, true, false);
                _delay_ms(200);
            }
            //wrong key
            else if(readed != 'x')
            {
                state = INIT;
                error++;
                pos = 0;
            }
            break;
		//pressed key state
        case KEY:
            setLED(true, true, true);
            //the pin is right
            if(pos == 4)
            {
                pos = 0;
                state = OPEN;
                setLED(false, true, false);
                _delay_ms(200);
                break;
            }
            //key is right
            if(readed != 'x' && readed == pin[pos])
            {
                pos++;
                setLED(false, true, false);
                _delay_ms(200);
            }
            //wrong key
            else if(readed != 'x')
            {
                state = INIT;
                error++;
                pos = 0;
            }
            break;
		//opend lock state
        case OPEN:
            setLED(true, false, true);
            //timeout
            if(timeout == openTimeout)
            {
                error = 0;
                pos = 0;
                state = INIT;
                setLED(false, false, false);
                _delay_ms(1000);
                setLED(false, true, true);
                _delay_ms(1000);        
            }
            //new key ?
            if(readed != 'x' && readed == '#')
            {
                state = CHANGEKEY;
                setLED(true, true, false);
                _delay_ms(200);
            }
            if(readed != 'x')
            {
                timeout = 0;
            }
            else
            {
                timeout++;
            }
            break;
		//started pin change state
        case CHANGEKEY:
        	//change key
            if(readed != 'x' && readed == 'a')
            {
                state = NEWKEY;
                setLED(true, true, false);
                _delay_ms(200);
                pos = 0;
            }
            else if(readed != 'x')
            {
                state = OPEN;
            }
            break;
		//new pin state
        case NEWKEY:
            setLED(true, false, true);
            //new key
            if(readed != 'x')
            {
                state = NEWKEY;
                pin[pos] = readed;
                pos++;
                setLED(true, true, false);
                _delay_ms(200);
            }
            //set new key
            if(pos == 4)
            {
                state = INIT;
                pos = 0;
                error = 0;
                setLED(false, false, false);
                _delay_ms(1000);
                setLED(false, true, false);
                _delay_ms(1000);
            }
            break;
		//locked state
        case LOCKED:
            setLED(true, false, false);
            //the puck is right
            if(pos == 6)
            {
                pos = 0;
                error = 0;
                state = INIT;
                setLED(false, false, false);
                _delay_ms(200);
                setLED(false, true, false);
                _delay_ms(200);
                break;
            }
            //key is right
            if(readed != 'x' && readed == puck[pos])
            {
                pos++;
                setLED(true, true, false);
                _delay_ms(200);
            }
            //locked,  key is wrong
            else if(readed != 'x')
            {
                setLED(false, true, true);
                _delay_ms(200);
                pos = 0;
            }
            break;
		//error state
        default:
            break;
    }
}
