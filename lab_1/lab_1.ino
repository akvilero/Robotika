#include <Servo.h>
#include <Keypad.h>

#define passwordLength  4	//3 digits + '\0'

// pins
int pirPin = 2;		// PIR motion sensor
int ledPin = 4;		// light for entrance
int ldrPin = A0;	// LDR light sensor
int servoPin = 5;	// servo motor (door lock)
int buzzPin = 3;	// buzzer
int led1Pin = A1;	// alarm LED 1
int led2Pin = A2;	// alarm LED 2
int led3Pin = A3;	// alarm LED 3

// settings
int darkTreshold = 500;	// if photoresistors reading > 500 -> considered dark
int failedAttempts = 0;
const int maxFails = 3;

// password
char password[passwordLength] = "123";
char inputPassword[passwordLength];
byte keyCount = 0;

// keypad setup
const byte ROWS = 4;
const byte COLUMNS = 4;

char keys[ROWS][COLUMNS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {13,12,11,10};
byte colPins[ROWS] = {9,8,7,6};

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLUMNS);
Servo servo;

void setup()
{
  pinMode(pirPin, INPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(ldrPin, INPUT);
  pinMode(buzzPin, OUTPUT);
  
  pinMode(led1Pin, OUTPUT);
  pinMode(led2Pin, OUTPUT);
  pinMode(led3Pin, OUTPUT);
  
  servo.attach(servoPin);
  servo.write(0);
  
  Serial.begin(9600); 
}

void loop()
{
  
  // motion + light check
  int pirState = digitalRead(pirPin);
  int lightVal = analogRead(A0);
  //Serial.println(lightVal);
  
  if(pirState == HIGH && lightVal > darkTreshold){
    digitalWrite(ledPin, HIGH);
    delay(3000);
    digitalWrite(ledPin, LOW);
  }
  
  // password input
  char key = keypad.getKey();
  
  if(key){
    inputPassword[keyCount] = key;
    Serial.print(inputPassword[keyCount]);
    keyCount++;
    
    if(keyCount == passwordLength-1){	// check input password length
      
      if(!strcmp(inputPassword, password)){
        openDoors();
        failedAttempts = 0;
      }else{
        failedAttempts++;     
        
        if(failedAttempts == maxFails){
          alarmSystem();
          failedAttempts = 0;
        }else{
          Serial.println("");
          Serial.println("Incorrect password. Try again.");
        }
        
        deleteKeyInput();
        delay(500);
      }
    }
  }
}


void deleteKeyInput(){
  while(keyCount != 0){
    inputPassword[keyCount--] = 0;
  }
}

void openDoors(){
  Serial.println("");
  Serial.println("Access granted. Door unlocked!");
  
  servo.write(90);
  delay(4000);
  
  servo.write(0);
  Serial.println("Door locked again.");
  deleteKeyInput();
}

void alarmSystem(){
  Serial.println("");
  Serial.println("!!! ALARM TRIGGERED !!!");
  tone(buzzPin, 500);
          
  for (int i = 0; i < 6; i++) {
    digitalWrite(led1Pin, HIGH);
    delay(200);
    digitalWrite(led1Pin, LOW);

    digitalWrite(led2Pin, HIGH);
    delay(200);
    digitalWrite(led2Pin, LOW);

    digitalWrite(led3Pin, HIGH);
    delay(200);
    digitalWrite(led3Pin, LOW);
  }
  noTone(buzzPin);
  Serial.println("Alarm stopped.");
}