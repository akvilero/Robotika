#include <Wire.h>
#include <SparkFun_APDS9960.h>
#include <Servo.h>
#include <EEPROM.h>

#define EEPROM_MAGIC_ADDR 0
#define EEPROM_NUM_ADDR 1
#define MAGIC_VALUE 42

// LEDs
int redLED = 5;
int greenLED = 6;
int blueLED = 10;

// servo
int servoPin = 9;
Servo myservo;

int servoOpen = 30;
int servoClosed = 180;
int doorsOpen = 0; 

// button
int btnPin = 2;
volatile unsigned long lastBtnInterruptTime = 0;
volatile bool btnPressed = false;

// for blinking
volatile unsigned long blinkTimer = 0;
volatile bool blinkState = false;

int threshold = 40; // for color detection

volatile bool gameOver = false;

volatile bool tick10ms = false;
bool do10ms = true;

bool ledBlinkOn = false;
unsigned long gameOverStart = 0;
const unsigned long gameOverDuration = 3000; // 3s blink duration

// APDS-9960 sensor
SparkFun_APDS9960 apds = SparkFun_APDS9960();

// color sequence
const int sequenceLength = 3;
int sequence[sequenceLength];


void generateNewSequence(){
  for(int i = 0; i < sequenceLength; i++){
    sequence[i] = random(0, 3);
  }
}

int readColor(){
  uint16_t r, g, b;
  if(apds.readRedLight(r) && apds.readGreenLight(g) && apds.readBlueLight(b)){
    if(r > g + threshold && r > b + threshold) return 0; // red
    else if(g > r + threshold && g > b + threshold) return 1; // green
    else if(b > r + threshold && b > g + threshold) return 2; // blue
  }
  return -1; // unclear color
}

void showLED(int color){
  digitalWrite(redLED, LOW);
  digitalWrite(greenLED, LOW);
  digitalWrite(blueLED, LOW);

  switch(color){
    case 0: digitalWrite(redLED,HIGH); break;
    case 1: digitalWrite(greenLED,HIGH); break;
    case 2: digitalWrite(blueLED,HIGH); break;
  }

  delay(500);

  digitalWrite(redLED, LOW);
  digitalWrite(greenLED, LOW);
  digitalWrite(blueLED, LOW);
  delay(500);
}


void setup() {
  
  // Timer2 setup
  cli();                   // disable global interrupts
  TCCR2A = 0;
  TCCR2B = 0;
  TCCR2A |= (1 << WGM21);  // CTC mode
  TCCR2B |= (1 << CS22) | (1 << CS21) | (1 << CS20); // prescaler 1024 
  OCR2A = 155;    // 10 ms
  TIMSK2 |= (1 << OCIE2A); // enable Timer2 compare interrupt
  sei();                   // enable global interrupts


  pinMode(btnPin, INPUT_PULLUP);
  pinMode(redLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  pinMode(blueLED, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(btnPin), btnInterrupt, FALLING);
  myservo.attach(servoPin);

  Serial.begin(9600);

  // EEPROM
  byte magic = EEPROM.read(EEPROM_MAGIC_ADDR);
  if(magic == MAGIC_VALUE){
    doorsOpen = EEPROM.read(EEPROM_NUM_ADDR);
    if(doorsOpen == 1){
      myservo.write(servoOpen);

    }else{
      myservo.write(servoClosed);
    }
  }else{
    doorsOpen = 0;
    EEPROM.write(EEPROM_MAGIC_ADDR, MAGIC_VALUE);
    EEPROM.write(EEPROM_NUM_ADDR, doorsOpen);
  }

 
  // Initialize interrupt service routine
  if (!apds.init()) {
    Serial.println("APDS init failed");
    while(1);
  }

  // Initialize APDS-9960 (configure I2C and initial values)
  if ( apds.init() ) {
    Serial.println("APDS-9960 initialization complete");
  } else {
    Serial.println("Something went wrong during APDS-9960 init!");
  }

  // Enable light sensor for RGB
  if(apds.enableLightSensor(false)){
    Serial.println(F("RGB sensor enabled"));
  } else {
    Serial.println(F("RGB sensor init failed!"));
  }

  randomSeed(analogRead(0));
  generateNewSequence();

}

void loop() {

  static uint8_t blinkDiv = 0;
  bool do10ms = false;

  noInterrupts();

  if(tick10ms){
    tick10ms=false;
    do10ms= true;
  }
  interrupts();

  // LED blinking
  if(do10ms){
    if(gameOver){
      if(millis() - gameOverStart >= gameOverDuration){
        gameOver = false;
        ledBlinkOn = false;
        digitalWrite(redLED, LOW);
        digitalWrite(greenLED, LOW);
        digitalWrite(blueLED, LOW);

       
        btnPressed = false;   
        Serial.println("Incorrect sequence. Press button to try again.");
      }

      if(++blinkDiv >= 20){ //blink every 200ms
      
        blinkDiv = 0;
        ledBlinkOn =! ledBlinkOn;
        digitalWrite(redLED,ledBlinkOn);
        digitalWrite(greenLED,ledBlinkOn);
        digitalWrite(blueLED,ledBlinkOn);
      } 
    }else{
        blinkDiv = 0;
    }
  }

 
  if(!btnPressed){
    return;
  }

  btnPressed = false;
  Serial.print("Sequence: ");

  for(int i = 0; i < sequenceLength; i++){
    Serial.print(sequence[i]);
    Serial.print(" ");
    showLED(sequence[i]);
  }

  Serial.println();
  delay(1000);

  // read user input
  for(int i = 0; i < sequenceLength; i++){
    int playerColor = -1;

    while(playerColor == -1){
      playerColor = readColor();
    }
  
    Serial.print("Player input color: ");
    Serial.println(playerColor);
    
    if(playerColor != sequence[i]){
      Serial.println("Wrong color!");
      
      gameOver = true;
      gameOverStart = millis();
      generateNewSequence();
      return;
    }

    showLED(playerColor);
    delay(200);
  }

  if(doorsOpen == 0){
    moveServo(servoClosed, servoOpen);
    doorsOpen = 1;
    EEPROM.write(EEPROM_NUM_ADDR, doorsOpen);
    delay(3000);
    moveServo(servoOpen, servoClosed);
    doorsOpen = 0;
    EEPROM.write(EEPROM_NUM_ADDR, doorsOpen);
  }

  if(doorsOpen == 1){
    moveServo(servoOpen, servoClosed);
    doorsOpen = 0;
    EEPROM.write(EEPROM_NUM_ADDR, doorsOpen);
  }

  generateNewSequence();
}

// ISR
void btnInterrupt(){
  unsigned long t = millis();
  if (t - lastBtnInterruptTime > 60){  // debounce
    btnPressed = true;
  }
  lastBtnInterruptTime = t;
}

void moveServo(int startAngle, int endAngle){
  if (startAngle<endAngle){
    for(int pos = startAngle; pos <= endAngle; pos++){
      myservo.write(pos);
      delay(20);
    }
  } else{
    for(int pos = startAngle; pos >= endAngle; pos--){
      myservo.write(pos);
      delay(20);
    }
  }
}

ISR(TIMER2_COMPA_vect){
  tick10ms = true;
}








