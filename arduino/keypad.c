/****************************************************************************
 * Copyright (C) 2021 by Lukas Brüggemann, Markus Reinhold, Sebastian Pötter
 ****************************************************************************/

/**
 * @file keypad.c
 * @author Sebastian Pötter
 * @date 1 Jan 2021
 * @brief This file contains all functions for the key detection
 *
 * This file controlles the keyinput and return of the pressed key
 */
 
#include <stdbool.h>
#include <avr/io.h>
#include <util/delay.h>
#include "led.h"

///Variables used in for loops
int h=0;	
///Variables used in for loops
int v=0;    

///Number of rows of keypad
const int rows=4;       
///Number of columnss of keypad      
const int columns=4;            
///Array of pins used as output for rows of keypad
const char Output[4]={PORTD2,PORTD3,PORTD4,PORTD5};
///Array of pins used as input for columnss of keypad
const char Input[4]={PORTD6,PORTD7,PORTB0,PORTB1}; 
///Array representing the values on the Keypad 
const char keys[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};



/**
* @brief Initiates the kepad for use
* 
* @details Set poin modes for the kepad, rows as Output, colomns as Input and all set to High
*
*/
void setup_keypad(void)
{
    /// Set Port 2,3,4,5 as Outpt
    DDRD |= _BV(DDD2);
    DDRD |= _BV(DDD3);     
    DDRD |= _BV(DDD4);     
    DDRD |= _BV(DDD5);          

    /// Set Port 6,7,8,9 as Input and High 
    DDRD &= ~_BV(DDD6);
    DDRD &= ~_BV(DDD7);
    DDRB &= ~_BV(DDB0);
    DDRB &= ~_BV(DDB1);
       
    PORTD |= _BV(PORTD6);
    PORTD |= _BV(PORTD7);
    PORTB |= _BV(PORTB0);
    PORTB |= _BV(PORTB1);
}


/**
* @brief reads the pressed key and returns pressed value
* 
* @details test if a button is pressed then test which button it is. If colomn Pin changed to Low button is pressed. 
*		   By setting one row after another to High check if the Low Pin changes to High.
*
* @return Char of the pressed button
*/

char keypad(void) // function used to detect which button is used 
{

    static bool no_press_flag=0;  // static flag used to ensure no button is pressed 

	for(int x=0;x<columns;x++)   // for loop used to read all inputs of keypad to ensure no button is pressed 
    { 
		if (x<2)
		{
			if (PORTD & Input[x]){   //read every input if high continue else break;
				setLED(true, false, false);
				_delay_ms(1000);	
				break;	
			} 
		}else{
		
			if (PORTB & Input[x]){   //read every input if high continue else break; 
				setLED(true, false, false);  
				_delay_ms(1000);
				break;
			}
		}
		
		if(x==(columns-1))   //if no button is pressed 
		{
			no_press_flag=1;
			h=0;
			v=0;
			setLED(true, true, true);	 
			_delay_ms(1000);	
		}
    }


    if(no_press_flag==1) // if no button is pressed 
    {	
		for(int r=0;r<rows;r++) // for loop used to make all output as low
            PORTD &= ~_BV(Output[r]);		

        for(h=0;h<columns;h++)  // for loop to check if one of inputs is low
        {
	    	if(h<2)
	    	{
            	if(PORTD & Input[h]) // if specific input is remain high (no press on it) continue
            	    continue;
           		else    // if one of inputs is low
           	 	{
                	for (v=0;v<rows;v++)   // for loop used to specify the number of row
                	{
                    
                    	PORTB |= _BV(Output[v]); // make specified output as HIGH

						// if the input that selected from first sor loop is change to high
                    	if(PORTD & Input[h])
                    	{
                        	no_press_flag=0;    //reset the no press flag;
                        	for(int w=0;w<rows;w++)
                        		PORTD &= ~_BV(Output[w]);
								// set all Output as Low

                        	return keys[v][h];  //return number of button 
                        	// h and v needed to convert to char -> for loop
                    	}
                	}
            	}
	    	}else{
	    	
	    		if(PORTB & Input[h]) // if specific input is remain high (no press on it) continue
              	  	continue;
          		else    // if one of inputs is low
          		{
                	for (v=0;v<rows;v++)   // for loop used to specify the number of row
                	{
                    
                    	PORTB |= _BV(Output[v]); // make specified output as HIGH

                    	if(PORTB & Input[h])  // if the input that selected from first sor loop is change to high
                    	{
                        	no_press_flag=0;    // reset the no press flag;
                        	for(int w=0;w<rows;w++)
                        		PORTD &= ~_BV(Output[w]); // set all Output as Low

                        	return keys[v][h];  // return number of button 
                        	// h and v needed to convert to char -> for loop
                    	}
                	}
            	}
   	    	}
        }
    }
    return 'x';
}
