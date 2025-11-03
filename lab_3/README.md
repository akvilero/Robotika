# HW3: 8x8 LED Matrix Apple Catch Game

## Goal
Design a non-blocking, interrupt-driven 8x8 LED matrix game where the player moves right or left to catch falling apples.

## Build Steps
1. Upload code to Arduino board
2. Wire circuit as described below
3. On startup, the best score is displayed
4. Use buttons to move the player and catch falling apples to increase your score
5. The game ends when all lives (3) are lost, best score is saved to EEPROM

## Timer Config
* **Timer1** in CTC mode
* **Prescaler**: 256
* **OCR1A**: 37500 -> interrupt every 0.9s

## ISR Roles
* **External interrupt**: detect button presses (left/right) and update player position.
* **Timer interrupt**: trigger apple drops periodically.

## EEPROM Layout
|Address|Value|Description|
|-------|--|---|
| 0|42|Magic number (validation)|
|1|Best score|Stores the highest score|


## Wiring
|Component|Arduino Uno Pin|
|---------|---------------|
|LED Matrix DIN | 12 |
|LED Matrix CS  | 11 |
|LED Matrix CLK | 10 |
|Button Right   | 2  |
|Button Left    | 3  |


