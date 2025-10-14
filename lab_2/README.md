# Asynchronous 7-Segment Display with EEPROM

## Overview
This Arduino project implements an asynchronous, 7-segment display counter with EEPROM. Each button press increments the displayed number (0-9). Every 3 seconds, the displayed number blinks once. The last displayed number is saved in EEPROM and restored after power loss.

## Build Steps
1. Upload code to Arduino Uno.
2. Wire circuit as shown in the schematic.
3. Press the button to increment the number (0-9).
4. Observe periodic blinking every 3 seconds.
5. Power off and back on - the last displayed number will be restored from EEPROM.

## Timer Configuration
- Timer1 in CTC mode
- Prescaler: 1024
- OCR1A : 46874 -> interrupt every 3s
  *(calculated as '3s x 16MHz / 1024 - 1')*

## ISR Roles
- **External interrupt:**
  Detects button press and sets flag btnPressed, which is later handled in the main loop to increment the number and update EEPROM.
- **Timer interrupt:**
  Sets the flag blinkFlag every 3 seconds, which triggers a short display blink in the main loop.

## EEPROM Layout

| Address | Value | Description            |
|:--------:|:------|:----------------------|
| 0        | 42    | Magic number (validation) |
| 1        | 0–9   | Last displayed number    |

## Wiring

| Segment | Arduino Pin |
|:--------:|:------------|
| a        | 3 |
| b        | 4 |
| c        | 5 |
| d        | 6 |
| e        | 7 |
| f        | 8 |
| g        | 9 |
| Button   | 2 *(with internal pull-up)* |

