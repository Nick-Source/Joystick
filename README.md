# Joystick Library

A library capable of handling receiving **digital** and **analog** inputs from microcontrollers. This data can then be directed towards a **Windows PC**, through this library, to be interpreted as a controller; however, for those interested, the method of sending data can easily be changed or simply removed.

## Board Support
|Boards|Notes|
|----------------|-------------------------------|
|*AVR Boards*|Any microcontroller supporting the Arduino library|
|*ESP32*|Specialized non-volatile storage method|
|*ESP8266*|Uses Arduino EEPROM library|

## Requirements
- [vJoySerialFeeder](https://github.com/Cleric-K/vJoySerialFeeder)
	- 
	- This program is only required for the current method of sending data. This method can be simply changed though; the function is titled `send_data` inside the class `base_Joystick`. For those interested in changing this function, data is stored as `data_buttons` and `data_pots`. To access them you must use either the `this` pointer or the `JOYSTICK` preprocessor; so, a complete syntax would consist of `this->data_buttons` or `JOYSTICK->data_buttons`. The templated functions are already setup, so it's as simple as rewriting the code inside the functions. The templates differentiate the functions depending on whether there any pots or buttons present.

## Options
|Name|Explanation|
|----------------|-------------------------------|
|*Joystick*|Normal library with no data stored to flash or non-volatile memory|
|*Joystick_no_calibration*|`Joystick` but no calibration for potentiometers|
|*Joystick_progmem*|Library with progmem (flash) values. `BUTTONS` **and** `POTS` must be progmem|
|*Joystick_progmem_no_calibration*|`Joystick_Progmem` but no calibration for potentiometers|
|*Joystick_eeprom*|Library with progmem (flash) values with calibration values stored in non-volatile memory|

## How to use the library?
Instructions and explanations are given in each individual folder.
