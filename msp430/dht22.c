/*
 * dht22.c
 *
 *  Created on: 18.11.2020
 *      Author: Administrator
 */

#include <msp430.h>
#include "dht22.h"

enum {
  DHT_IDLE = 0,
  DHT_TRIGGERING,
  DHT_WAITING_ACK,
  DHT_ACK_LOW,
  DHT_ACK_HIGH,
  DHT_IN_BIT_LOW,
  DHT_IN_BIT_HIGH,
} dht_current_state;

dht22data dht_data;

uint8_t dht_data_byte, dht_data_bit;

float dht_get_temp() {
  float ret = -200.0;
    //uint16_t temp_temp;
  while (dht_current_state != DHT_IDLE);

  ret = (float)(((dht_data.val.th&0x7f)<<8)+dht_data.val.tl) * 0.1;

  //check sign
  if (dht_data.val.th&0x80)
  {
      ret *= -1.0;
  }

  return ret;

  //temp_temp = (((dht_data.val.th&0x7f)<<8)+dht_data.val.tl);
  //return ((-1)*((dht_data.val.th&0x80)>>7)+temp_temp);
}

float
dht_get_rh() {
  //uint16_t temp_rh;
  float ret = -1.0;
  while (dht_current_state != DHT_IDLE);

  ret = (float)((dht_data.val.hh<<8)+dht_data.val.hl)*0.1;
  return ret;

  //temp_rh = (dht_data.val.hh<<8)+dht_data.val.hl;
  //return temp_rh;
}

void
dht_start_read() {
  // First, low pulse of 1ms
  P2OUT &= ~BIT0;
  P2SEL &= ~BIT0;
  P2DIR |= BIT0;

  TA1CCTL0 &= ~CCIFG;
  TA1CCR0 = 8000u; // IMP this number
  TA1CCTL0 = CCIE;
  TA1CTL = TACLR;
  TA1CTL = TASSEL_2 | ID_0 | MC_1;

  dht_current_state = DHT_TRIGGERING;
}

void __attribute__((interrupt (TIMER1_A0_VECTOR)))
timer1_a0_isr() {
  TA1CCTL0 &= ~CCIFG;
  // This handles only TA1CCR0 interrupts
  switch (dht_current_state) {
  case DHT_IDLE:
    break; // Shouldn't be here
  case DHT_TRIGGERING:
    // 1ms has passed since setting the pin low
    // Let P2.0 go high and set Compare input on T1
    P2DIR &= ~BIT0; // input
    P2SEL |= BIT0; // Timer1_A3.CCI0A input
    TA1CTL = TACLR;
    TA1CTL = TASSEL_2 | ID_0 | MC_2;
    TA1CCTL0 = CM_2 | CCIS_0 | CAP | CCIE; // Capture on falling edge
    dht_current_state = DHT_WAITING_ACK;
    break;
  case DHT_WAITING_ACK:
    // I don't care about timings here...
    P2DIR &= ~BIT0; // input
    TA1CTL = TACLR;
    TA1CTL = TASSEL_2 | ID_0 | MC_2;
    TA1CCTL0 = CM_1 | CCIS_0 | CAP | CCIE; // Capture on rising edge
    dht_current_state = DHT_ACK_LOW;
    break;
  case DHT_ACK_LOW:
    // I don't care about timings here either...
    TA1CTL = TACLR;
    TA1CTL = TASSEL_2 | ID_0 | MC_2;
    TA1CCTL0 = CM_2 | CCIS_0 | CAP | CCIE; // Capture on falling edge
    dht_current_state = DHT_ACK_HIGH;
    dht_data_byte = dht_data_bit = 0;
    break;
  case DHT_ACK_HIGH:
    TA1CTL = TACLR;
    TA1CTL = TASSEL_2 | ID_0 | MC_2;
    TA1CCTL0 = CM_1 | CCIS_0 | CAP | CCIE; // Capture on rising edge
    dht_current_state = DHT_IN_BIT_LOW;
    break;
  case DHT_IN_BIT_LOW:
    TA1CTL = TACLR;
    TA1CTL = TASSEL_2 | ID_0 | MC_2;
    TA1CCTL0 = CM_2 | CCIS_0 | CAP | CCIE; // Capture on falling edge
    dht_current_state = DHT_IN_BIT_HIGH;
    break;
  case DHT_IN_BIT_HIGH:
    // OK now I need to measure the time since last time
    dht_data.bytes[dht_data_byte] <<= 1;
    if (TA1CCR0 > 375) { // IMP this number
      // Long pulse: 1
      dht_data.bytes[dht_data_byte] |= 1;
    }
    if (++dht_data_bit >= 8) {
      dht_data_bit = 0;
      dht_data_byte++;
    }
    if (dht_data_byte >= 5) {
      // I'm done, bye
      // TODO: check CRC
      TA1CTL = TACLR;
      dht_current_state = DHT_IDLE;
    } else {
      TA1CTL = TACLR;
      TA1CTL = TASSEL_2 | ID_0 | MC_2;
      TA1CCTL0 = CM_1 | CCIS_0 | CAP | CCIE; // Capture on rising edge
      dht_current_state = DHT_IN_BIT_LOW;
    }
    break;
  }
}



