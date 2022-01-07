#include <msp430.h>
#include "dht22.h"
#include <stdio.h>

//#define DEBUG

// constants to determine the current board action
#define BOARD_ACTION_MEASURE 0
#define BOARD_ACTION_LED_ON 1
#define BOARD_ACTION_LED_BLINK_ON 2
#define BOARD_ACTION_LED_OFF 3
#define BOARD_ACTION_MEASURING 4

// timeout constants 1 tick ~ 4ms
#define TIMEOUT_MULTIPLIER 20 // use this multiplier to generate a higher measure timeout than 16 seconds (calculation: TIMEOUT_BETWEEN_MEASURES*TIMEOUT_MULTIPLIER)
#define BLINK_COUNT_BETWEEN_MEASURES 5 // how many times should the led flash between one measurement cycle?
const uint16_t TIMEOUT_BETWEEN_MEASURES = 65535; // ~16s
const uint16_t TIMEOUT_PWM_BLINK_ON = 100; // how long should the led be on?
const uint16_t TIMEOUT_PWM_BLINK_OFF = TIMEOUT_BETWEEN_MEASURES/BLINK_COUNT_BETWEEN_MEASURES-TIMEOUT_PWM_BLINK_ON; //automatically calculate how long the led should be off


// constants for predefined comfort levels
#define COMFORT_LEVEL_UNCOMFORTABLE_DRY 0
#define COMFORT_LEVEL_UNCOMFORTABLE_WET 1
#define COMFORT_LEVEL_COMFORTABLE 2
#define COMFORT_LEVEL_STILL_COMFORTABLE_WET 3
#define COMFORT_LEVEL_STILL_COMFORTABLE_DRY 4

const int HUM_RANGE_COMFORTABLE[] =  //humidity range
{
    63,75, // 18
    37,73,
    36,71,
    35,68,
    35,66, // 22
    34,55,
    33,41
};

const int HUM_RANGE_STILL_COMFORTABLE[] =
{
    75,75, // 16
    36,87, // 17
    30,85,
    25,83,
    19,82, // 20
    19,81,
    18,76,
    18,71,
    17,66,
    17,63, // 25
    22,50,
    32,32
};

void measureDHT(void);
int getDHTComfortState(int, int);
static void setCPUThrottled(int);
static void setupPWM(int);
static void stopPWM();
static void setTimerPeriod(uint16_t);

int currentBlinkCount = 0;
int currentBoardState = BOARD_ACTION_MEASURE;
short dhtCheckActive = 0; // is the timer running?
short currentTimeoutCount = 0;
int currentComfortLevel;


int main() {
    WDTCTL = WDTPW | WDTHOLD;
    DCOCTL = 0;

    //for button press interrupt
    P1REN |= 8;
    P1IE |= 0x08; // P1.3 interrupt enabled
    P1IFG &= ~0x08; // P1.3 IFG cleared

    P1DIR = ~BIT3; // Set P1.0 to output direction (3 = 0 for button)
    P1OUT = 0x0;
    #ifdef DEBUG
        P1OUT &= ~(BIT0 + BIT6);
    #endif
    // for pulse timer
    TACCTL0 |= CCIE; // enable interrupt for CCR0.
    setTimerPeriod(0);// initially, Stop the Timer

    //close remaining gpio
    P2DIR = 0xFF;
    P2OUT = BIT0; // sensor

    P3DIR = 0xFF; //set port 3 to output to increase efficiency
    P3OUT = 0x0;

    setCPUThrottled(1);


    while(1){
        __bis_SR_register(LPM3_bits + GIE); // status register: Low Power Mode and general interrupt enabled
        measureDHT(); //call this so every time the CPU awakes, the data gets measured
    }
}

//this function is used to dynamically change the cpu clock speed (as temp/humidity measurement does not work with 1MHz clock)
void setCPUThrottled(int throttled){
    if (throttled){
        BCSCTL1 = CALBC1_1MHZ;
        DCOCTL = CALDCO_1MHZ;
    } else {
        BCSCTL1 = CALBC1_8MHZ;
        DCOCTL = CALDCO_8MHZ;
    }
}

// this function is used to call the DHT peripheral and convert the returned floating point values to ints
void measureDHT(){
    volatile int temperature;
    volatile int humidity;
    #ifdef DEBUG
        P1OUT |= BIT6; // toggle red led every time a measurement is called from the timer
    #endif

    setCPUThrottled(0); //speed up CPU to enable measuring

    dht_start_read();
    temperature = (int)(dht_get_temp()+0.5); // round temp to next int
    humidity = (int)(dht_get_rh()+0.5); // round humidity to next int

    setCPUThrottled(1);

    currentComfortLevel = getDHTComfortState(temperature, humidity); // gets saved globally to be accessible from interrupts
    currentBoardState = BOARD_ACTION_LED_OFF;
    #ifdef DEBUG
        P1OUT &= ~BIT6; // toggle red led every time a measurement is called from the timer
    #endif
}

// this function is used to convert temperature and humidity to predefined comfort levels using humidity ranges in lookup tables
int getDHTComfortState(int temperature, int humidity) {

    return COMFORT_LEVEL_UNCOMFORTABLE_WET;
    //return COMFORT_LEVEL_STILL_COMFORTABLE_WET;
    //first, check if our values are inside the approximation of the two comfort zones
    int check_comfortable = ((temperature >= 18 && temperature <= 24)
            && (humidity >= 33 && humidity <= 75));

    int check_still_comfortable = ((temperature >= 16 && temperature <= 27)
            && (humidity >= 17 && humidity <= 87));

    // check inner polygon (comfortable) first
    if (check_comfortable) {
        int low = HUM_RANGE_COMFORTABLE[2*(temperature-18)];
        int high = HUM_RANGE_COMFORTABLE[2*(temperature-18) + 1];
        if (humidity >= low && humidity <= high)
            return COMFORT_LEVEL_COMFORTABLE;
    }

    //after that, check for second polygon (still comfortable)
    if (check_still_comfortable) {
        int low = HUM_RANGE_STILL_COMFORTABLE[2*(temperature-16)];
        int high = HUM_RANGE_STILL_COMFORTABLE[2*(temperature-16) + 1];

        if (humidity >= low && humidity <= high) {
            //bonus, check if the humidity is too dry or too wet
            if (humidity < 50) {
                return COMFORT_LEVEL_STILL_COMFORTABLE_DRY;
            }
            return COMFORT_LEVEL_STILL_COMFORTABLE_WET;
        }
    }

    //last, decide if conditions are too dry or wet
    if (humidity < 50) {
        return COMFORT_LEVEL_UNCOMFORTABLE_DRY;
    }
    return COMFORT_LEVEL_UNCOMFORTABLE_WET;
}



// button press ISR, switches from active to passive mode by pressing on the physical button
#pragma vector=PORT1_VECTOR
__interrupt void Port_1_ISR(void)
{
    //inline CPU throttling, used to reset the clock speed in case the user cancelled measuring in the middle of an active measurement
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;
    dhtCheckActive ^= 1; // toggle active state

    if (dhtCheckActive) {
        setTimerPeriod(10); // start timer
    } else {
        setTimerPeriod(0); // disable timer
        currentBoardState = BOARD_ACTION_MEASURE;
        stopPWM(); // deactivate status led
        #ifdef DEBUG
            P1OUT &= ~(BIT0 + BIT6); //deactivate (debug) leds
        #endif
    }

    P1IFG &= ~0x08; // P1.3 IFG cleared
}

static inline void setTimerPeriod(uint16_t period){
    TACTL = TACLR; // clear this to be able to use the up/down mode
    TACTL = TASSEL_1 | ID_3 | MC_1; //MC1 up MC3 up/down
    TACCR0 = period;
}


// timer ISR, this controls the DHT measurement pulse and the status led
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A_CCR0_ISR(void)
{
    currentTimeoutCount++;
    if (currentTimeoutCount < TIMEOUT_MULTIPLIER) {
        return;
    }
    currentTimeoutCount = 0;
    #ifdef DEBUG
    P1OUT ^= BIT0; // flicker green led to indicate if the timer is running
    #endif
    switch(currentBoardState){
        case BOARD_ACTION_MEASURE:
            LPM3_EXIT; //status register: disable low power mode
            setTimerPeriod(10);
            currentBoardState = BOARD_ACTION_MEASURING;
            break;
        case BOARD_ACTION_MEASURING:
            setTimerPeriod(10);
            break;
        case BOARD_ACTION_LED_OFF: //enable leds and check if the timer is already set off
            if (currentBlinkCount >= BLINK_COUNT_BETWEEN_MEASURES) { //is the display interval already over? set it back to measurement
                currentBoardState = BOARD_ACTION_MEASURE;
                setTimerPeriod(10); //recall this function
                currentBlinkCount = 0;
            } else {
                switch(currentComfortLevel) {
                    case COMFORT_LEVEL_UNCOMFORTABLE_WET:
                        setupPWM(BIT5); //Set Led to blue
                        currentBoardState = BOARD_ACTION_LED_ON;
                        setTimerPeriod(TIMEOUT_BETWEEN_MEASURES); // set timer to full interval -> next call of this method will be with next measurement
                        break;
                    case COMFORT_LEVEL_UNCOMFORTABLE_DRY:
                        setupPWM(BIT1); //Set Led to red
                        currentBoardState = BOARD_ACTION_LED_ON;
                        setTimerPeriod(TIMEOUT_BETWEEN_MEASURES); // set timer to full interval -> next call of this method will be with next measurement
                        break;
                    case COMFORT_LEVEL_STILL_COMFORTABLE_WET:
                        setupPWM(BIT5); //Set Led to blue
                        currentBoardState = BOARD_ACTION_LED_BLINK_ON;
                        currentBlinkCount ++;
                        setTimerPeriod(TIMEOUT_PWM_BLINK_ON); // set timer to blink interval -> next call of this method will be when the led switches off
                        break;
                    case COMFORT_LEVEL_STILL_COMFORTABLE_DRY:
                        setupPWM(BIT1); //Set Led to red
                        currentBoardState = BOARD_ACTION_LED_BLINK_ON;
                        currentBlinkCount ++;
                        setTimerPeriod(TIMEOUT_PWM_BLINK_ON); // set timer to blink interval -> next call of this method will be when the led switches off
                        break;
                    case COMFORT_LEVEL_COMFORTABLE:
                        setupPWM(BIT3); // set Led to green
                        currentBoardState = BOARD_ACTION_LED_BLINK_ON;
                        currentBlinkCount ++;
                        setTimerPeriod(TIMEOUT_PWM_BLINK_ON); // set timer to blink interval -> next call of this method will be when the led switches off
                        break;
                }
            }
            break;
        case BOARD_ACTION_LED_BLINK_ON: // disable leds
            setTimerPeriod(TIMEOUT_PWM_BLINK_OFF);
            currentBoardState = BOARD_ACTION_LED_OFF;
            stopPWM();
            break;
        case BOARD_ACTION_LED_ON: // measure interval in non blinking mode is over
            currentBoardState = BOARD_ACTION_MEASURE;
            setTimerPeriod(1); //recall this function
            stopPWM();
            break;
    }
}





static inline void setupPWM(int colorBit){
    //Set Base Value for PWM (GREEN)
   TA1CCTL0 &= ~CCIE;
   switch(colorBit){
   case BIT1:
       TA1CCTL1 = OUTMOD_4; //toggle mode
       break;
   case BIT3:
       TA1CCTL0 = OUTMOD_4; //toggle mode
       break;
   case BIT5:
       TA1CCTL2 = OUTMOD_4; //toggle mode
       break;
   }

   TA1CCR0 = 300; //switch after 300 ticks

   P2DIR |= colorBit; //switch on color
   P2SEL |= colorBit;

   TA1CTL = TACLR;
   TA1CTL = TASSEL_1 | ID_0 | MC_1;//Start in UpMode
}

static inline void stopPWM() {
    P2DIR &= ~(BIT1 + BIT3 + BIT5); //switch off rgb led
    P2SEL &= ~(BIT1 + BIT3 + BIT5);
    TA1CTL = 0; //stop timer
}

