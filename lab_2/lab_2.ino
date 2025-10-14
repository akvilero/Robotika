#include <EEPROM.h>

#define EEPROM_MAGIC_ADDR 0 // address to store a "magic" byte
#define EEPROM_NUM_ADDR 1   // address to store last displayed number
#define MAGIC_VALUE 42      // magic number to check if everything is ok?

// 7-segment display pins
int a = 3;
int b = 4;
int c = 5;
int d = 6;
int e = 7;
int f = 8;
int g = 9;

int btnPin = 2; // button pin

// variables
int num = 0;                                    // current displayed number
volatile bool btnPressed = false;               // button press flag (set by interrupt)
volatile unsigned long lastInterruptTime = 0;   // for debouncing
volatile bool blinkFlag = false;                // flag to trigger blinking
unsigned long blinkStart = 0;                   // time when blinking started
bool isBlinking = false;                        // true while number is off during blink

void setup()
{
  // button setup 
  pinMode(btnPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(btnPin), handleButton, FALLING);

  // 7-segment setup
  pinMode(a, OUTPUT);
  pinMode(b, OUTPUT);
  pinMode(c, OUTPUT);
  pinMode(d, OUTPUT);
  pinMode(e, OUTPUT);
  pinMode(f, OUTPUT);
  pinMode(g, OUTPUT);

  // EEPROM initialization
  byte magic = EEPROM.read(EEPROM_MAGIC_ADDR);
  
  if (magic == MAGIC_VALUE) {
    num = EEPROM.read(EEPROM_NUM_ADDR);
  } else {
    num = 0;
    EEPROM.write(EEPROM_MAGIC_ADDR, MAGIC_VALUE);
    EEPROM.write(EEPROM_NUM_ADDR, num);
  }
  showNumber(numToBits(num));

  // Timer1 setup
  noInterrupts();
  TCCR1A = 0;                 // Init Timer1 reg A
  TCCR1B = 0;                 // Init Timer1 reg B
  TCNT1  = 0;                 // reset counter value
  OCR1A = 46874;              // compare value for 3s (3s*16MHz/1024 -1)
  TCCR1B |= (1 << WGM12);     // enable CTC mode (Clear TIme on Compare)
  TCCR1B |= (1 << CS12) | (1 << CS10);  // prescaler 1024
  TIMSK1 |= (1 << OCIE1A);    // enable Timer1 compare A interrupt
  interrupts(); 
}

void loop()
{

  if (blinkFlag) {
    blinkFlag = false;
    showNumber(0b0000000);  // turn off all segments
    blinkStart = millis();
    isBlinking = true;
  }

  // end blinking after 200 ms
  if (isBlinking && millis() - blinkStart >= 200) {
    showNumber(numToBits(num));
    isBlinking = false;
  }

  // handle button press
  if(btnPressed){
    num = (num + 1) % 10;
    showNumber(numToBits(num));
    EEPROM.write(EEPROM_NUM_ADDR, num);
    
    btnPressed = false;
  }
}

byte numToBits(int num){
  switch(num){
    case 0:
    	return 0b1111110;
      break;
    case 1:
		  return 0b0110000;
    	break;
    case 2:
		  return 0b1101101;
    	break;
    case 3:
		  return 0b1111001;
    	break;
    case 4:
		  return 0b0110011;
    	break;
    case 5:
		  return 0b1011011;
    	break;
    case 6:
		  return 0b1011111;
    	break;
    case 7:
		  return 0b1110000;
    	break;
    case 8:
		  return 0b1111111;
    	break;
    case 9:
		  return 0b1111011;
    	break;
    default:
    	return 0b1111110;
    	break;
  }
}

// displays the given bit pattern on 7-segment
void showNumber(byte byteNum){
  int j = 6;
  for(int i = a; i <= g; i++){
    int numBit = bitRead(byteNum, j--);
    digitalWrite(i, numBit? LOW:HIGH);  // LOW turns on a segment
  }
}

//------- ISR section -------

// external interrupt:button press
void handleButton() {
  unsigned long t = millis();
  if (t - lastInterruptTime > 200) {  // debounce time 200ms
    btnPressed = true;
  }
  lastInterruptTime = t;
}

// internal interrupt
ISR(TIMER1_COMPA_vect) {
  blinkFlag = true; // trigger blink in the main loop
}



 
