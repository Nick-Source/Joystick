# Joystick Library

This version of the library **does not** require or consist of `PROGMEM` (flash) values and **does not** utilize non-volatile memory usage. Everything is stored in memory granted no compiler optimizations. This version **does include** calibration for analog values.

Analog Bit Resolution
- 
- This library supports a max analog resolution of **16 bits**; however, depending on implementation, uint_fast16_t may be a bigger type allowing for more bits
- For any analog bit resolution above 16 bits, in which `analogRead` returns values with more than 16 bits, the `send` function must be changed as `vJoySerialFeeder` only supports 16 bit values
	- This limitation does not affect buttons as they are bit based (0 or 1) not consisting of bytes of data
- Defaulted to 12-bit resolution values for calibration purposes, there are defines at the top of the header
	- `#define BIT_RESOLUTION_MAX_VALUE 1023 //10-bit` *(This one is commented out)*
	- `#define BIT_RESOLUTION_MAX_VALUE 4095 //12-bit`
	- These can be changed to suit whatever bit resolution your microcontroller uses

Maximum Amounts of Inputs
- 
- Maximum digital inputs of `size_t` max value
- Maximum analog inputs of of `size_t / 2` value 

**HOWEVER**, when using the current `send` function, the IBUS protocol limits you to one byte of data for initializing the size and is sent the size by using the following code:
```
Serial.write(4 + (detail::round_to_fast16(BUTTONS_SIZE) * sizeof(uint_fast16_t)) + (POTS_SIZE * sizeof(uint_fast16_t)));
```

Essentially this limits you to: 
```
4 + (detail::round_to_fast16(BUTTONS_SIZE) * sizeof(uint_fast16_t)) + (POTS_SIZE * sizeof(uint_fast16_t))
```

The function `round_to_fast16` finds an amount of `uint_fast16_t`s to fit a certain amount of bits

Other notes
- 
- If it is absolutely necessary for saving memory, you can remove the value `invalid` from `print_values` in the namespace `detail`. However, you must also remove all of the implementation using this value.
- Library utilizes empty base optimizations in cases of no analog inputs or no digital inputs
- Library utilizes curiously recurring template pattern
- The library class structure consists of:
```
			 ↗ base_Joystick_BUTTONS
Joystick -> base_Joystick
			 ↘ base_Joystick_POTS
```
- base_Joystick_BUTTONS and base_Joystick_POTS are templated classes which may default to empty classes if their counterpart of buttons or pots is 0

# How to use the library?

Instantiating the class
-
To begin, the templated portion of the class is defined as:
```
template <size_t BUTTONS_SIZE, size_t POTS_SIZE, bool IS_PULL_UP = true>
```

and the constructor is defined as:
```
Joystick(const uint_fast8_t (&BUTTONS)[BUTTONS_SIZE], const uint_fast8_t (&POTS)[POTS_SIZE], uint_fast16_t (&CALIBRATION)[POTS_SIZE * 2])
```

Knowing this, completing the syntax, in **C++11**, looks like the following:
```
Joystick<BUTTONS_SIZE, POTS_SIZE> NAME(BUTTONS, POTS, CALIBRATION);
```

or
```
Joystick<BUTTONS_SIZE, POTS_SIZE, true> NAME(BUTTONS, POTS, CALIBRATION);
```

or, in **C++17**, thanks to CTAD,
```
Joystick NAME(BUTTONS, POTS, CALIBRATION);
```

|Variable Name|Explanation|
|----------------|-------------------------------|
|*NAME*|Name for the new instance of the class|
|*BUTTONS_SIZE*|Amount of buttons (digital inputs)|
|*POTS_SIZE*|Amount of potentiometers (analog inputs)|
|*IS_PULL_UP*|Defines whether digital input pins are pull-up. Digital pins will also be set as `INPUT_PULLUP` or `INPUT_PULLDOWN` depending on this variable.|
|*BUTTONS*|Array consisting of digital input pins|
|*POTS*|Array consisting of analog input pins|
|*CALIBRATION*|Array consisting of calibration values. If uncalibrated, set all values to 0. Must hold the size: `[POTS_SIZE * 2]`|

Library Functions
-
The library also consists of 4 public functions that can be used.

|Function|Explanation|
|----------------|-------------------------------|
|*setup*|Setup the pins to input and begin calibration if necessary|
|*read*|Read the pins and store the input data|
|*print*|Print the input data; designed for Serial Plotter but Serial Output must be used first to choose what to print|
|*send*|Send the input data to PC through `vJoySerialFeeder`|

Calibration
-
Running the `setup` function initiates calibration if `CALIBRATION` consists of all zeros. Calibration will describe the procedure in the terminal (Serial Output). Sending any value to Serial will proceed to the next step. At the end, values are given of which should be used to initialize the `CALIBRATION` array in the future.

vJoySerialFeeder
-
- When connecting to the COM port through `vJoySerialFeeder`, the board is restarted so `CALIBRATION` must be initialized with calibrated values
- You will have padding depending on implementation for `uint_fast16_t`
	- So your channel count will be off

| Frame Length | Data | Checksum |
|----------------|-------------------------------|----------------|
|[`Length`]| [`0x40`][`Channel_1L`][`Channel_1H`][`Channel_2L`][`Channel_2H`]... |[`Checksum_L`][`Checksum_H`]|

As shown above, data is sent in bytes following Little Endian format. `L` and `H` postfixes represent `low` and `high` respectively. So as you could imagine, sending data that is, for example, 32 bits long would result in padding of zeros as long as `analogRead` outputs 16 bit values. As for digital inputs, this isn't an issue as everything is bit based and is not dependent on combining bytes together.

### How to figure out what channels to use for digital inputs:
1. Determine digital input size (`BUTTONS_SIZE`) and modulo it by `(sizeof(uint_fast16_t) * CHAR_BIT)`
	-  `BUTTONS_SIZE % (sizeof(uint_fast16_t) * CHAR_BIT)`
#### If the value is 1 or higher
2. Divide `BUTTONS_SIZE` by `(sizeof(uint_fast16_t) * CHAR_BIT)`
3. Truncate the result
4. Add 1
5. Multiply by `sizeof(uint_fast16_t)`
6. Divide by `sizeof(uint16_t)`
	- This value will be referenced as `DIGITAL_INPUT_OFFSET`
#### If the value is 0
2. Divide `BUTTONS_SIZE` by `(sizeof(uint_fast16_t) * CHAR_BIT)`
3. Multiply by `sizeof(uint_fast16_t)`
4. Divide by `sizeof(uint16_t)`
	- This value will be referenced as `DIGITAL_INPUT_OFFSET`

All channels in between 1 and this result (including the result) will consist of digital bit-mapped buttons

### How to figure out what channels to use for analog inputs:
1. Determine analog inputs size (`POTS_SIZE`) and multiply by `sizeof(uint_fast16_t)`
2. Divide by `sizeof(uint16_t)`
3. Divide by `POTS_SIZE`
	- This value represents the amount of channels allocated for each analog input
	- This value will be referenced as `CH_ALLOCATION_SIZE` standing for "Channel Allocation Size"
4. Subtract by 1
	- This value represents the amount of padding channels in between each real channel
	- This value will be referenced as `CH_PAD_CNT` standing for "Channel Padding Count"

The data section will consist of the real Channel low and high followed by `(CH_PAD_CNT * sizeof(uint16_t))` padding bytes. For every `CH_ALLOCATION_SIZE` channels, the first one will be the real one followed by padded channels. It will happen `POTS_SIZE` times. Remember, analog inputs will be offset by digital input channels as they are sent first.

To figure out Channel numbers mathematically, simply take `DIGITAL_INPUT_OFFSET + 1`, the first real analog input channel, and add `CH_ALLOCATION_SIZE`. Add `CH_ALLOCATION_SIZE` a total of `POTS_SIZE - 1` times. Every number you get is a real analog input channel.

I mean you can either do all this or just keep guessing the channels until you get it right lol

Serial Port
-
Do not forget to open the Serial Port:
`Serial.begin(115200);`

Baud rate can be changed; however, you must redefine it under `Port Setup` in `vJoySerialFeeder`.

## Example Use
```
#include <Arduino.h>
#include "Joystick.h"

enum BUTTONS : uint_fast8_t
{
	R1,
	L1,
	DU,
	DR,
	DL,
	DD,
	BUTTONS_SIZE = 6
};

enum POTS : uint_fast8_t
{
	RT = 34,
	LT = 36
	POTS_SIZE = 2
};

const uint_fast8_t ALL_BUTTONS[BUTTONS_SIZE] { R1, L1, DU, DR, DL, DD };
const uint_fast8_t ALL_POTS[POTS_SIZE] { RT, LT };
uint_fast16_t CALIBRATION[POTS_SIZE * 2] {0};

Joystick<BUTTONS_SIZE, POTS_SIZE> Controller(ALL_BUTTONS, ALL_POTS, CALIBRATION);

void setup() 
{
	Serial.begin(115200);
	Controller.setup();
}

void loop()
{
	Controller.read();
	Controller.send();
}
```