#include <LedControl.h>
#include <EEPROM.h>

#define EEPROM_MAGIC_ADDR 0
#define EEPROM_NUM_ADDR 1
#define MAGIC_VALUE 42

int DIN = 12;
int CS = 11;
int CLK = 10;
LedControl lc(DIN, CLK, CS, 1);

int btnRight = 2;
int btnLeft = 3; 

int player = 4;

volatile unsigned long lastBtnInterruptTime = 0;
volatile bool btnRightPressed = false;
volatile bool btnLeftPressed = false;


int columns[] = {0,0,0,0,0,0,0,0};

volatile bool dropAppleFlag = false;
int appleSpawnCounter = 0;
int score = 0;
int lives = 3;
bool gameOverFlag = false;
int bestScore = 0;

byte digits[10][8]{
  {B00000000,B00000000,B00000000,B01110000,B01010000,B01010000,B01010000,B01110000},
  {B00000000,B00000000,B00000000,B00010000,B00110000,B01010000,B00010000,B00010000},
  {B00000000,B00000000,B00000000,B01110000,B00010000,B01110000,B01000000,B01110000},
  {B00000000,B00000000,B00000000,B01110000,B00010000,B00110000,B00010000,B01110000},
  {B00000000,B00000000,B00000000,B01010000,B01010000,B01110000,B00010000,B00010000},
  {B00000000,B00000000,B00000000,B01110000,B01000000,B01110000,B00010000,B01110000},
  {B00000000,B00000000,B00000000,B01110000,B01000000,B01110000,B01010000,B01110000},
  {B00000000,B00000000,B00000000,B01110000,B00010000,B00010000,B00010000,B00010000},
  {B00000000,B00000000,B00000000,B01110000,B01010000,B01110000,B01010000,B01110000},
  {B00000000,B00000000,B00000000,B01110000,B01010000,B01110000,B00010000,B01110000}
};

void setup() {

  pinMode(btnRight, INPUT_PULLUP);
  pinMode(btnLeft, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(btnRight), moveRight, FALLING);
  attachInterrupt(digitalPinToInterrupt(btnLeft), moveLeft, FALLING);

  lc.shutdown(0,false);
  lc.setIntensity(0,8);
  lc.clearDisplay(0);
  lc.setLed(0, 7, player, true);
  randomSeed(analogRead(0));

  byte magic = EEPROM.read(EEPROM_MAGIC_ADDR);
  if(magic == MAGIC_VALUE){
    bestScore = EEPROM.read(EEPROM_NUM_ADDR);
  } else{
    bestScore = 0;
    EEPROM.write(EEPROM_MAGIC_ADDR, MAGIC_VALUE);
    EEPROM.write(EEPROM_NUM_ADDR, bestScore);
  }

  // --- Timer1 setup ---
  noInterrupts();        
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;
  OCR1A = 37500;           
  TCCR1B |= (1 << WGM12);
  TCCR1B |= (1 << CS12);
  TIMSK1 |= (1 << OCIE1A);
  interrupts();

  startScreen();
}

void loop() {

  if(gameOverFlag){
    drawScore(score);
    return;
  }

  lc.clearDisplay(0);
  lc.setLed(0, 7, player, true);

  if(dropAppleFlag){
    moveObstaclesDown();
    
    if(appleSpawnCounter % 2 == 0){
      int r;
      do{
        r = random(0,8);
      } while(columns[r] != 0);
      columns[r] = 1;
    }
    
    appleSpawnCounter++;
    dropAppleFlag = false;
  }
  drawObstacle();
  
  if(btnRightPressed){
    if(player != 7){
      player++;
    }
    else{
      player = 0;
    }
    lc.clearDisplay(0);
    btnRightPressed = false;
  }

  if(btnLeftPressed){
    if(player != 0){
      player--;
    }
    else{
      player = 7;
    }
    lc.clearDisplay(0);
    btnLeftPressed = false;
  }

  if(columns[player] == 8){
    score++;
    columns[player] = 0;
  }

  for(int i = 0; i < 8; i++){
    if(columns[i] == 8 && player != i){
      lives--;
      columns[i]=0;
    }
  }

  if(lives <= 0){
    gameOverFlag = true;
    
    if(score > bestScore){
      bestScore = score;
      EEPROM.write(EEPROM_NUM_ADDR, bestScore);
    }
  }
}

// External interrupt
void moveRight(){
  unsigned long t = millis();
  if(t - lastBtnInterruptTime > 150){  // debounce
    btnRightPressed = true;
  }
  lastBtnInterruptTime = t;
}

// External interrupt
void moveLeft(){
  unsigned long t = millis();
  if (t - lastBtnInterruptTime > 150){  // debounce
    btnLeftPressed = true;
  }
  lastBtnInterruptTime = t;
}

void moveObstaclesDown(){
  for(int i = 0; i < 8; i++){
    if(columns[i] > 0){
      columns[i]++;
    } 
    if(columns[i] > 8){
      columns[i] = 0;
    } 
  }
}

void drawObstacle(){
  for(int i = 0; i < 8; i++){
    if(columns[i] > 0){
      lc.setLed(0, columns[i]-1, i, true);
    }
  }
}

void drawScore(int num){
  lc.clearDisplay(0);

  if(num >= 100){
    lc.setRow(0, 3, B11111111);
  }
  else{
    int tens = num / 10;
    int ones = num % 10;

    // draw tens digit
    for(int row = 0; row < 8; row++) {
      byte rowData = digits[tens][row];
      
      for(int col = 0; col < 4; col++) {
        bool bit = rowData & (1 << (7-col));
        lc.setLed(0, row, col, bit);
      }
    }

    // draw ones digit
    for(int row = 0; row < 8; row++){
      byte rowData = digits[ones][row] >> 4;

      for(int col = 0; col < 4; col++){
        bool bit = rowData & (1 << (3-col));
        lc.setLed(0, row, col+4, bit);
      }
    }
  }
}

void startScreen(){
  byte digit3[8]{B00000000,B00111100,B00000100,B00000100,B00011100,B00000100,B00000100,B00111100};
  byte digit2[8]{B00000000,B00111100,B00000100,B00000100,B00111100,B00100000,B00100000,B00111100};
  byte digit1[8]{B00000000,B00001000,B00011000,B00101000,B00001000,B00001000,B00001000,B00001000};
  byte digitGo[8]{B00000000,B00000000,B11100111,B10000101,B10110101,B10010101,B11110111,B00000000};

  drawScore(bestScore);
  delay(5000);
  drawDigit(digit3);
  delay(1000);
  drawDigit(digit2);
  delay(1000);
  drawDigit(digit1);
  delay(1000);
  drawDigit(digitGo);
  delay(1000);
}

void drawDigit(byte digit[8]){
  for(int row = 0; row < 8; row++){
    lc.setRow(0, row, digit[row]);
  }
}

// Timer interrupt
ISR(TIMER1_COMPA_vect){
  dropAppleFlag = true;
}


