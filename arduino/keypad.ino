/*-------------------------------------------------------------------------------------------

  KEYPAD

-------------------------------------------------------------------------------------------*/
char readed;
byte h=0,v=0;    //variables used in for loops
const unsigned long period=50;  //little period used to prevent error
unsigned long kdelay=0;        // variable used in non-blocking delay 
const byte rows=4;             //number of rows of keypad
const byte columns=4;            //number of columnss of keypad
const byte Output[rows]={2,3,4,5}; //array of pins used as output for rows of keypad
const byte Input[columns]={6,7,8,9}; //array of pins used as input for columnss of keypad
const char keys[rows][columns] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

char keypad() // function used to detect which button is used 
{
 static bool no_press_flag=0;  //static flag used to ensure no button is pressed 
  for(byte x=0;x<columns;x++)   // for loop used to read all inputs of keypad to ensure no button is pressed 
  {
     if (digitalRead(Input[x]) == HIGH);   //read evry input if high continue else break;
     else
      break;
     if(x==(columns-1))   //if no button is pressed 
     {
      no_press_flag=1;
      h=0;
      v=0;
     }
  }
  if(no_press_flag==1) //if no button is pressed 
  {
    for(byte r=0;r<rows;r++) //for loop used to make all output as low
      digitalWrite(Output[r],LOW);
    for(h=0;h<columns;h++)  // for loop to check if one of inputs is low
    {
      if(digitalRead(Input[h])==HIGH) //if specific input is remain high (no press on it) continue
        continue;
      else    //if one of inputs is low
      {
        for (v=0;v<rows;v++)   //for loop used to specify the number of row
        {
          digitalWrite(Output[v],HIGH);   //make specified output as HIGH
          if(digitalRead(Input[h])==HIGH)  //if the input that selected from first sor loop is change to high
          {
            no_press_flag=0;                //reset the no press flag;
            for(byte w=0;w<rows;w++) // make all outputs as low
            digitalWrite(Output[w],LOW);
            return keys[v][h]; //return number of button 
          }
        }
      }
    }
  }
 return 50;
}

/*-------------------------------------------------------------------------------------------

  LED

-------------------------------------------------------------------------------------------*/
const int redPin= 10;
const int greedPin = 11;
const int bluePin = 12;

void setLED(int redValue, int greenValue, int blueValue)
{
  analogWrite(redPin, redValue);
  analogWrite(greedPin, greenValue);
  analogWrite(bluePin, blueValue);
}


/*-------------------------------------------------------------------------------------------

  STATES

-------------------------------------------------------------------------------------------*/
#define INIT 0
#define KEY 1
#define OPEN 2
#define CHANGEKEY 3
#define NEWKEY 4
#define LOCKED 5

int error = 0; //after x wrong keys => locked
int state = INIT;
int stateOld = INIT;
int pos = 0; //current position of key
char pin[4] = "a123";
char puck[6] = "#12345";
int timeout = 0;
int openTimeout = 250;

void stateM()
{
  readed = keypad();
  stateOld = state;
  if(error == 4)
  {
    pos = 0;
    state = LOCKED;
    error = 0;
  }
  switch (state) 
  {
    case INIT:
      setLED(0, 0, 255);
      if(readed != 'x' and readed == pin[pos])
      {
        Serial.println("first key is right");
        pos++;
        state = KEY;
        setLED(0, 255, 0);
        delay(200);
      }
      else if(readed != 'x')
      {
        Serial.println("wrong key!");
        state = INIT;
        error++;
        pos = 0;
      }
      break;
    case KEY:
      setLED(255, 255, 255);
      if(pos == 4)
      {
        Serial.println("the pin is right");
        pos = 0;
        state = OPEN;
        setLED(0, 255, 0);
        delay(200);
        break;
      }
      if(readed != 'x' and readed == pin[pos])
      {
        Serial.println("key is right");
        pos++;
        setLED(0, 255, 0);
        delay(200);
      }
      else if(readed != 'x')
      {
        Serial.println("wrong key!");
        state = INIT;
        error++;
        pos = 0;
      }
      break;
    case OPEN:
      setLED(200, 52, 150);
      if(timeout == openTimeout)
      {
        Serial.println("timeout!");
        error = 0;
        pos = 0;
        state = INIT;
        setLED(0, 0, 0);
        delay(1000);
        setLED(0, 255, 255);
        delay(1000);        
      }
      if(readed != 'x' and readed == '#')
      {
        Serial.println("new key ?");
        state = CHANGEKEY;
        setLED(255, 255, 0);
        delay(200);
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
    case CHANGEKEY:
      if(readed != 'x' and readed == 'a')
      {
        Serial.println("change key!");
        state = NEWKEY;
        setLED(255, 255, 0);
        delay(200);
        pos = 0;
      }
      else if(readed != 'x')
      {
        state = OPEN;
      }
      break;
    case NEWKEY:
      setLED(255, 192, 203);
      if(readed != 'x')
      {
        Serial.println("new key");
        state = NEWKEY;
        pin[pos] = readed;
        pos++;
        setLED(255, 255, 0);
        delay(200);
      }
      if(pos == 4)
      {
        Serial.println("set new key");
        state = INIT;
        pos = 0;
        error = 0;
        setLED(0, 0, 0);
        delay(1000);
        setLED(0, 255, 0);
        delay(1000);
      }
      break;
    case LOCKED:
      setLED(255, 0, 0);
      if(pos == 6)
      {
        Serial.println("the puck is right");
        pos = 0;
        error = 0;
        state = INIT;
        setLED(0, 0, 0);
        delay(200);
        setLED(0, 255, 0);
        delay(200);
        break;
      }
      if(readed != 'x' and readed == puck[pos])
      {
        Serial.println("key is right");
        pos++;
        setLED(255, 255, 0);
        delay(200);
      }
      else if(readed != 'x')
      {
        Serial.println("locked");
        setLED(255, 255, 255);
        delay(200);
        Serial.println("key is wrong");
        pos = 0;
      }
      break;
    default:
      //error state!
      break;
  }
}

void setup() 
{
  for(byte i=0;i<rows;i++)  //init pins for output
  {
    pinMode(Output[i],OUTPUT);
  }
  for(byte s=0;s<columns;s++)  //init pins for input
  {
    pinMode(Input[s],INPUT_PULLUP);
  }
  pinMode(redPin, OUTPUT); //init led pin red
  pinMode(greedPin, OUTPUT); //init led pin green
  pinMode(bluePin, OUTPUT);//init led pin blue
  Serial.begin(9600);
}

void loop()
{
  if(millis()-kdelay>period) //used to make non-blocking delay
  {
    kdelay=millis();  //capture time from millis function
    stateM();
  }
}
