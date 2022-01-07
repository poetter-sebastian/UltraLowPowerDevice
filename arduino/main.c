/****************************************************************************
 * Copyright (C) 2021 by Lukas Brüggemann, Markus Reinhold, Sebastian Pötter
 ****************************************************************************/

/**
 * @file main.c
 * @author Markus Reinhold
 * @date 1 Jan 2021
 * @brief This is the main file of the arduino project
 *
 * This file set up the keypad-pin and led-pin and controll the state machine
 */

/*! \mainpage Lock controller
 * This is the documentation page for a lock controller with a pin and puk.
 * The whole project is tested on a arduino uno rev3 with an ATmega328P.
 */ 

#include "keypad.h"
#include "led.h"
#include "states.h"
#include "millis.h"
#include <stdio.h>
#include <avr/io.h>
#include <util/atomic.h>
#include <avr/interrupt.h>

#include <util/delay.h>
#define MS_DELAY 3000

/// used to make non-blocking delay
const unsigned long period = 50; 
/// variable used in non-blocking delay 
unsigned long kdelay=0;        

/**
 * @brief Main function
 *
 * Here is the programme entry point where the setup and the state machine stats
 */
int main(void)
{
    setup_LED();
    setup_keypad();
	init_millis(16000000UL); //frequency the atmega328p is running at
	sei();
	
    while(1)
    {


        if(millis()-kdelay>period) //used to make non-blocking delay
        {

            kdelay=millis();  //capture time from millis function

            stateM();
           
        }
    }
    return 0;
}


