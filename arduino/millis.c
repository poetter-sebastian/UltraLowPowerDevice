/****************************************************************************
 * Copyright (C) 2021 by Lukas Brüggemann, Markus Reinhold, Sebastian Pötter
 ****************************************************************************/

/**
 * @file millis.c
 * @author Markus Reinhold
 * @date 1 Jan 2021
 * @brief Functions for the delay for the key press detection
 *
 */



#include <avr/io.h>
#include <util/atomic.h>
#include <avr/interrupt.h>



volatile unsigned long timer1_millis; 
//NOTE: A unsigned long holds values from 0 to 4,294,967,295 (2^32 - 1). It will roll over to 0 after reaching its maximum value.

ISR(TIMER1_COMPA_vect)
{
  timer1_millis++;  
}

/**
* @brief Initiates the millis for use
* 
* @details Sets Timer Counter Register and Output Compare Register
*/

void init_millis(unsigned long f_cpu)
{
  unsigned long ctc_match_overflow;
  
  ctc_match_overflow = ((f_cpu / 1000) / 8); 
    
  TCCR1B |= (1 << WGM12) | (1 << CS11);
  
  OCR1AH = (ctc_match_overflow >> 8);
  OCR1AL = ctc_match_overflow;
 
  TIMSK1 |= (1 << OCIE1A);
}


/**
* @brief Used to get millis time
* 
* @details returns the milliseconds elapsed since the program was started
*
* @return millis time 
*/

unsigned long millis ()
{
  unsigned long millis_return;
 
  // Ensure this cannot be disrupted
  ATOMIC_BLOCK(ATOMIC_FORCEON) {
    millis_return = timer1_millis;
  }
  return millis_return;
} 
