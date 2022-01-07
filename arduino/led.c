/****************************************************************************
 * Copyright (C) 2021 by Lukas Brüggemann, Markus Reinhold, Sebastian Pötter
 ****************************************************************************/

/**
 * @file led.c
 * @author Markus Reinhold
 * @date 1 Jan 2021
 * @brief This is the file to controll the rgb led
 *
 */

#include <stdbool.h>
#include <avr/io.h>


/**
* @brief Initiates the LED for use
* 
* @details Set pin modes for the red, green and blue in the LED and init with all on
*/
void setup_LED(void)
{

    // set mode to Output
    DDRB |= _BV(DDB4); //red   pin 10
    DDRB |= _BV(DDB3); //green    pin 11
    DDRB |= _BV(DDB2); //blue   pin 12

    PORTB |= _BV(PORTB4); // HIGH
    PORTB |= _BV(PORTB3); // HIGH
    PORTB |= _BV(PORTB2); // HIGH
}


/**
* @brief Aktivates LED colors depending on paramters
* 
* @details Set pin modes for the red, green and blue in the LED and init with all on. Aktivate LED's depending on the Parameters.
*
* @param redValue bool paramter, if true the red LED will be turned on
* @param greenValue bool paramter, if true the green LED will be turned on
* @param blueValue bool paramter, if true the blue LED will be turned on
*/
void setLED(bool redValue, bool greenValue, bool blueValue)
{
    
    // Set all to LOW
    PORTB &= ~_BV(PORTB4);
    PORTB &= ~_BV(PORTB3);
	PORTB &= ~_BV(PORTB2);
	
	// Aktivate LED's depending on the Parameters
    if (redValue)
    {
    	PORTB |= _BV(PORTB2); // HIGH
    }
    if (greenValue)
    {
        PORTB |= _BV(PORTB3); // HIGH
    }
    if (blueValue)
    {
        PORTB |= _BV(PORTB4); // HIGH
    }
}

