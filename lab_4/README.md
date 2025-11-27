# HW4: Color sequence door unlock system

## Goal
The goal is to design and implement a color-sequence-based door unlocking system using Arduino, APDS-9960 RGB color sensor, servo motor, button and indicator LEDs.

## Build Steps
1. Upload code to Arduino board.
2. Wire circuit as described below.
3. On startup, the system reads the door state from EEPROM and positions the servo accordingly.
4. Press the button to start a new color sequence.
5. Observe the LED sequence and show the same colors in order to the APDS-9960 sensor.
6. Correct input opens servo-controlled doors.
7. Incorrect input triggers LED blinking as feedback.

## Timer Config
* **Timer2** in CTC mode
* **Prescaler**: 1024
* **OCR2A**: 155 -> interrupt every 10ms

## ISR Roles
* **External interrupt**: detect button press and generate new color sequence.
* **Timer interrupt**: set a 10ms tick flag used for LED blinking during incorrect sequence input.

## EEPROM Layout
|Address|Value|Description|
|-------|--|---|
| 0|42|Magic number (validation)|
|1|0/1|Door state (0 = closed, 1 = open)|


## Wiring
|Component|Arduino Uno Pin|
|---------|---------------|
|Red LED | 5 |
|Green LED | 6 |
|Blue LED | 10 |
|Button | 2  |
|Servo | 9  |
|APDS-9960 SDA | A4  |
|APDS-9960 SCL | A5  |
|APDS-9960 GND | GND  |
|APDS-9960 VCC | 3.3V  |

* LEDs, servo motor should be powered from **5V**.
* Each LED should have a 220 Ω resistor in series.
* APDS-9960 sensor should be powered from **3.3V**.
* Common GND should be connected between Arduino, LEDs, servo, button and sensor.


