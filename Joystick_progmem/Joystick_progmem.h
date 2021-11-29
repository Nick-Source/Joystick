/*  
    Joystick - A library capable of handling receiving digital and analog inputs from microcontrollers.
    Copyright (C) 2021  Nicholas Daniel

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once
#include <Arduino.h>
#include <limits.h>

#define JOYSTICK static_cast<T*>(this)

//#define BIT_RESOLUTION_MAX_VALUE 1023 //10-bit
#define BIT_RESOLUTION_MAX_VALUE 4095 //12-bit

namespace detail
{
    struct print_values
    {
        static constexpr char colon[] PROGMEM = ":";
        static constexpr char invalid[] PROGMEM = "Invalid input.";
    };

    constexpr char print_values::colon[];
    constexpr char print_values::invalid[];

    // Primary template.
    /// Define a member typedef @c type only if a boolean constant is true.
    template<bool, typename _Tp = void>
    struct enable_if 
    { };

    // Partial specialization for true.
    template<typename _Tp>
    struct enable_if<true, _Tp>
    { typedef _Tp type; };

    constexpr size_t round_to_fast16(const size_t bits)
    {
        return bits % (sizeof(uint_fast16_t) * CHAR_BIT) ? ((bits/(sizeof(uint_fast16_t) * CHAR_BIT)) + 1) : (bits/(sizeof(uint_fast16_t) * CHAR_BIT));
    }

    template <class T, size_t BUTTONS_SIZE, size_t POTS_SIZE, bool IS_PULL_UP>
    class base_Joystick_BUTTONS
    {
    protected:
        base_Joystick_BUTTONS() {}
        uint_fast16_t data_buttons[detail::round_to_fast16(BUTTONS_SIZE)] = {0};
        
        template<bool value = IS_PULL_UP, typename detail::enable_if<value>::type* = nullptr>
        void read_buttons() //PULL_UP
        {
            for (size_t i = 0; i < BUTTONS_SIZE;)
            {
                if (!digitalRead(pgm_read_byte(JOYSTICK->BUTTONS + i)))
                {
                    ++i;
                    data_buttons[detail::round_to_fast16(i) - 1] |= (1 << (((i % (sizeof(uint_fast16_t) * CHAR_BIT)) ? (i % (sizeof(uint_fast16_t) * CHAR_BIT)) : (sizeof(uint_fast16_t) * CHAR_BIT)) - 1));
                    continue;
                }

                ++i;
            }
        }

        template<bool value = IS_PULL_UP, typename detail::enable_if<!value>::type* = nullptr>
        void read_buttons() //PULL_DOWN
        {
            for (size_t i = 0; i < BUTTONS_SIZE;)
            {
                if (digitalRead(pgm_read_byte(JOYSTICK->BUTTONS + i)))
                {
                    ++i;
                    data_buttons[detail::round_to_fast16(i) - 1] |= (1 << (((i % (sizeof(uint_fast16_t) * CHAR_BIT)) ? (i % (sizeof(uint_fast16_t) * CHAR_BIT)) : (sizeof(uint_fast16_t) * CHAR_BIT)) - 1));
                    continue;
                }

                ++i;
            }
        }

        template<size_t value_b = BUTTONS_SIZE, size_t value_p = POTS_SIZE, typename detail::enable_if<(value_b >= value_p)>::type* = nullptr>
        void print_buttons()
        {
            JOYSTICK->printComma = &T::printComma_;

            for (size_t i = 0; i < BUTTONS_SIZE; ++i)
            {
                (JOYSTICK->*(JOYSTICK->printComma))();

                Serial.print(pgm_read_byte(JOYSTICK->BUTTONS + i));
                Serial.print(reinterpret_cast<const __FlashStringHelper*>(print_values::colon));
                Serial.print(bitRead(data_buttons[i / (sizeof(uint_fast16_t) * CHAR_BIT)], i % (sizeof(uint_fast16_t) * CHAR_BIT)));
            }

            Serial.println();
        }

        template<size_t value_b = BUTTONS_SIZE, size_t value_p = POTS_SIZE, typename detail::enable_if<(value_p > value_b)>::type* = nullptr>
        void print_buttons()
        {
            JOYSTICK->printComma = &T::printComma_;

            for (size_t i = 0; i < BUTTONS_SIZE; ++i)
            {
                (JOYSTICK->*(JOYSTICK->printComma))();
                
                Serial.print(pgm_read_byte(JOYSTICK->BUTTONS + i));
                Serial.print(reinterpret_cast<const __FlashStringHelper*>(print_values::colon));
                Serial.print(bitRead(this->data_buttons[i / (sizeof(uint_fast16_t) * CHAR_BIT)], i % (sizeof(uint_fast16_t) * CHAR_BIT)));
            }

            for (size_t i = 0; i < POTS_SIZE - BUTTONS_SIZE; ++i)
            {
                (JOYSTICK->*(JOYSTICK->printComma))();
                Serial.print(F("N/A:0"));
            }
            
            Serial.println();
        }

        template<bool value = IS_PULL_UP, typename detail::enable_if<value>::type* = nullptr>
        void setup_buttons() //PULL_UP
        {
        #ifdef INPUT_PULLUP
            for (size_t i = 0; i < BUTTONS_SIZE; ++i)
                pinMode(pgm_read_byte(JOYSTICK->BUTTONS + i), INPUT_PULLUP);
        #else
            #warning INPUT_PULLUP is not defined, falling back to INPUT if necessary
            for (size_t i = 0; i < BUTTONS_SIZE; ++i)
                pinMode(pgm_read_byte(JOYSTICK->BUTTONS + i), INPUT);
        #endif
        }

        template<bool value = IS_PULL_UP, typename detail::enable_if<!value>::type* = nullptr>
        void setup_buttons() //PULL_DOWN
        {
        #ifdef INPUT_PULLDOWN
            for (size_t i = 0; i < BUTTONS_SIZE; ++i)
                pinMode(pgm_read_byte(JOYSTICK->BUTTONS + i), INPUT_PULLDOWN);
        #else
            #warning INPUT_PULLDOWN is not defined, falling back to INPUT if necessary
            for (size_t i = 0; i < BUTTONS_SIZE; ++i)
                pinMode(pgm_read_byte(JOYSTICK->BUTTONS + i), INPUT);
        #endif
        }
    };

    template <class T, size_t POTS_SIZE, bool IS_PULL_UP>
    class base_Joystick_BUTTONS<T, 0, POTS_SIZE, IS_PULL_UP> {};

    template <class T, size_t BUTTONS_SIZE, size_t POTS_SIZE>
    class base_Joystick_POTS
    {
    private:
        void calibrate()
        {
            for (size_t i = 0; i < (POTS_SIZE * 2); ++i)
            {
                JOYSTICK->CALIBRATION[i] = -1; //min - standard enforces 2s complement for unsinged variables
                JOYSTICK->CALIBRATION[++i] = 0; //max
            }

            Serial.println(F("Adjust the potentiometers to reach their maximum and minimum values."));
            Serial.println(F("Once finished input any character into the Serial Monitor."));
            Serial.println();

            while (Serial.available())
                Serial.read();

            while (!Serial.available())
            {
                for (size_t i = 0; i < (POTS_SIZE * 2); ++i)
                {
                    uint_fast16_t value = analogRead(pgm_read_byte(JOYSTICK->POTS + (i / 2)));

                    if (value < JOYSTICK->CALIBRATION[i])
                    {
                        JOYSTICK->CALIBRATION[i] = value;
                        ++i;
                        continue;
                    }

                    if (value > JOYSTICK->CALIBRATION[++i])
                    {
                        JOYSTICK->CALIBRATION[i] = value;
                    }
                }
            }

            for (size_t i = 0; i < (POTS_SIZE * 2); ++i)
            {
                if (JOYSTICK->CALIBRATION[i] == JOYSTICK->CALIBRATION[i + 1])
                {
                    Serial.print(F("Potentiometer PIN_"));
                    Serial.print(pgm_read_byte(JOYSTICK->POTS + (i / 2)));
                    Serial.println(F(" has same MIN and MAX."));
                    Serial.println(F("Defaulting to min and max values"));
                    Serial.println();

                    JOYSTICK->CALIBRATION[i] = 0;
                    JOYSTICK->CALIBRATION[++i] = -1; //standard enforces 2s complement for unsinged variables
                    continue;
                }

                ++i;
            }

            Serial.print(F("Calibration complete, the values are: "));

            JOYSTICK->printComma = &T::printComma_;

            for (const auto itr : JOYSTICK->CALIBRATION)
            {
                (JOYSTICK->*(JOYSTICK->printComma))();
                Serial.print(itr);
            }

            Serial.println();
            Serial.println();

            Serial.println(F("Use these values for the CALIBRATION variable to prevent this procedure."));
            Serial.println();
        }

    protected:
        base_Joystick_POTS() {}
        uint_fast16_t data_pots[POTS_SIZE] = {0};

        void read_pots() //improper calibration (negative value) will overflow to max value (65,535 --> BIT_RESOLUTION_MAX_VALUE)
        {
            for (size_t i = 0, count = 0; i < POTS_SIZE; ++i, count += 2)
            {
                uint_fast16_t value = map(analogRead(pgm_read_byte(JOYSTICK->POTS + i)), JOYSTICK->CALIBRATION[count], JOYSTICK->CALIBRATION[count + 1], 0, BIT_RESOLUTION_MAX_VALUE);
                value = (value > BIT_RESOLUTION_MAX_VALUE) ? BIT_RESOLUTION_MAX_VALUE : value;

                data_pots[i] = value;
            }
        }

        template<size_t value_b = BUTTONS_SIZE, size_t value_p = POTS_SIZE, typename detail::enable_if<(value_p >= value_b)>::type* = nullptr>
        void print_pots()
        {
            JOYSTICK->printComma = &T::printComma_;

            for (size_t i = 0; i < POTS_SIZE; ++i)
            {
                (JOYSTICK->*(JOYSTICK->printComma))();

                Serial.print(pgm_read_byte(JOYSTICK->POTS + i));
                Serial.print(reinterpret_cast<const __FlashStringHelper*>(print_values::colon));
                Serial.print(data_pots[i]);
            }
            
            Serial.println();
        }

        template<size_t value_b = BUTTONS_SIZE, size_t value_p = POTS_SIZE, typename detail::enable_if<(value_b > value_p)>::type* = nullptr>
        void print_pots()
        {
            JOYSTICK->printComma = &T::printComma_;

            for (size_t i = 0; i < POTS_SIZE; ++i)
            {
                (JOYSTICK->*(JOYSTICK->printComma))();

                Serial.print(pgm_read_byte(JOYSTICK->POTS + i));
                Serial.print(reinterpret_cast<const __FlashStringHelper*>(print_values::colon));
                Serial.print(data_pots[i]);
            }

            for (size_t i = 0; i < BUTTONS_SIZE - POTS_SIZE; ++i)
            {
                (JOYSTICK->*(JOYSTICK->printComma))();
                Serial.print(F("N/A:0"));
            }

            Serial.println();
        }

        void setup_pots()
        {
            for (size_t i = 0; i < POTS_SIZE; ++i)
                pinMode(pgm_read_byte(JOYSTICK->POTS + i), INPUT);

            for (const auto itr : JOYSTICK->CALIBRATION)
            {
                if (itr != 0)
                    return;
            }

            calibrate();
        }
    };

    #undef BIT_RESOLUTION_MAX_VALUE

    template <class T, size_t BUTTONS_SIZE>
    class base_Joystick_POTS<T, BUTTONS_SIZE, 0> {};

    template <class T, size_t BUTTONS_SIZE, size_t POTS_SIZE, bool IS_PULL_UP>
    class base_Joystick : protected base_Joystick_BUTTONS<T, BUTTONS_SIZE, POTS_SIZE, IS_PULL_UP>, protected base_Joystick_POTS<T, BUTTONS_SIZE, POTS_SIZE>
    {
    protected:
        base_Joystick() {}

        void (base_Joystick::*printComma)();
        void printComma_() { printComma = &base_Joystick::printComma__; }
        void printComma__() { Serial.print(F(", ")); }

        void (base_Joystick::*print_)() = &base_Joystick::print__;

        void print__()
        { 
            while (Serial.available())
                Serial.read();

            Serial.println(F("Enter 0 to print buttons or 1 to print potentiometers."));
            Serial.println(F("You are able to switch between the two, just enter the value into Serial later."));
            Serial.println();

            while (!Serial.available()) {}
            JOYSTICK->determine_print();
        }

        template<size_t value_b = BUTTONS_SIZE, size_t value_p = POTS_SIZE, typename detail::enable_if<(value_b > 0 && value_p > 0)>::type* = nullptr>
        void send_data()
        {
            Serial.write(4 + (detail::round_to_fast16(BUTTONS_SIZE) * sizeof(uint_fast16_t)) + (POTS_SIZE * sizeof(uint_fast16_t)));
            Serial.write(0x40);

            uint_fast16_t checksum = 0xffff - (4 + (detail::round_to_fast16(BUTTONS_SIZE) * sizeof(uint_fast16_t)) + (POTS_SIZE * sizeof(uint_fast16_t))) - 0x40;

            for (size_t i = 0; i < detail::round_to_fast16(BUTTONS_SIZE); ++i)
            {
                for (size_t z = 0; z < sizeof(uint_fast16_t); ++z)
                {
                    Serial.write((uint8_t)this->data_buttons[i]);
                    checksum -= (uint8_t)this->data_buttons[i];
                    this->data_buttons[i] = (this->data_buttons[i] >> CHAR_BIT);
                }
            }

            for (size_t i = 0; i < POTS_SIZE; ++i)
            {
                for (size_t z = 0; z < sizeof(uint_fast16_t); ++z)
                {
                    Serial.write((uint8_t)this->data_pots[i]);
                    checksum -= (uint8_t)this->data_pots[i];
                    this->data_pots[i] = (this->data_pots[i] >> CHAR_BIT);
                }
            }

            for (size_t i = 0; i < sizeof(checksum); ++i)
            {
                Serial.write((uint8_t)checksum);
                checksum = checksum >> CHAR_BIT;
            }
        }

        template<size_t value_b = BUTTONS_SIZE, size_t value_p = POTS_SIZE, typename detail::enable_if<(value_b > 0 && value_p == 0)>::type* = nullptr>
        void send_data()
        {
            Serial.write(4 + (detail::round_to_fast16(BUTTONS_SIZE) * sizeof(uint_fast16_t)));
            Serial.write(0x40);

            uint_fast16_t checksum = 0xffff - (4 + (detail::round_to_fast16(BUTTONS_SIZE) * sizeof(uint_fast16_t))) - 0x40;

            for (size_t i = 0; i < detail::round_to_fast16(BUTTONS_SIZE); ++i)
            {
                for (size_t z = 0; z < sizeof(uint_fast16_t); ++z)
                {
                    Serial.write((uint8_t)this->data_buttons[i]);
                    checksum -= (uint8_t)this->data_buttons[i];
                    this->data_buttons[i] = (this->data_buttons[i] >> CHAR_BIT);
                }
            }

            for (size_t i = 0; i < sizeof(checksum); ++i)
            {
                Serial.write((uint8_t)checksum);
                checksum = checksum >> CHAR_BIT;
            }
        }

        template<size_t value_b = BUTTONS_SIZE, size_t value_p = POTS_SIZE, typename detail::enable_if<(value_b == 0 && value_p > 0)>::type* = nullptr>
        void send_data()
        {
            Serial.write(4 + (POTS_SIZE * sizeof(uint_fast16_t)));
            Serial.write(0x40);

            uint_fast16_t checksum = 0xffff - (4 + (POTS_SIZE * sizeof(uint_fast16_t))) - 0x40;

            for (size_t i = 0; i < POTS_SIZE; ++i)
            {
                for (size_t z = 0; z < sizeof(uint_fast16_t); ++z)
                {
                    Serial.write((uint8_t)this->data_pots[i]);
                    checksum -= (uint8_t)this->data_pots[i];
                    this->data_pots[i] = (this->data_pots[i] >> CHAR_BIT);
                }
            }

            for (size_t i = 0; i < sizeof(checksum); ++i)
            {
                Serial.write((uint8_t)checksum);
                checksum = checksum >> CHAR_BIT;
            }
        }
    };
}

#undef JOYSTICK

template <size_t BUTTONS_SIZE, size_t POTS_SIZE, bool IS_PULL_UP = true>
class Joystick : private detail::base_Joystick<Joystick<BUTTONS_SIZE, POTS_SIZE, IS_PULL_UP>, BUTTONS_SIZE, POTS_SIZE, IS_PULL_UP>
{
private:
    friend class detail::base_Joystick<Joystick, BUTTONS_SIZE, POTS_SIZE, IS_PULL_UP>;
    friend class detail::base_Joystick_BUTTONS<Joystick, BUTTONS_SIZE, POTS_SIZE, IS_PULL_UP>;
    friend class detail::base_Joystick_POTS<Joystick, BUTTONS_SIZE, POTS_SIZE>;

    const uint_fast8_t (&BUTTONS)[BUTTONS_SIZE];
    const uint_fast8_t (&POTS)[POTS_SIZE];
    uint_fast16_t (&CALIBRATION)[POTS_SIZE * 2];

    void determine_print()
    {
        switch (Serial.read())
        {
            case 48: //0
                this->print_ = static_cast<void (detail::base_Joystick<Joystick, BUTTONS_SIZE, POTS_SIZE, IS_PULL_UP>::*)()>(&Joystick::template print_buttons<BUTTONS_SIZE, POTS_SIZE>);
                break;
            case 49: //1
                this->print_ = static_cast<void (detail::base_Joystick<Joystick, BUTTONS_SIZE, POTS_SIZE, IS_PULL_UP>::*)()>(&Joystick::template print_pots<BUTTONS_SIZE, POTS_SIZE>);
                break;
            default:
                Serial.println(reinterpret_cast<const __FlashStringHelper*>(detail::print_values::invalid));
                Serial.println();
                break;
        }
    }

public:
    Joystick(const uint_fast8_t (&BUTTONS)[BUTTONS_SIZE], const uint_fast8_t (&POTS)[POTS_SIZE], uint_fast16_t (&CALIBRATION)[POTS_SIZE * 2])
        : BUTTONS(BUTTONS), POTS(POTS), CALIBRATION(CALIBRATION)
    {}

    void setup()
    {
        this->setup_buttons();
        this->setup_pots();
    }

    //Read button and potentiometer values
    void read()
    {
        memset(this->data_buttons, 0, sizeof(this->data_buttons));
        memset(this->data_pots, 0, sizeof(this->data_pots));

        this->read_buttons();
        this->read_pots();
    }

    //Designed for Serial Plotter
    void print()
    {
        (this->*(this->print_))();

        while (Serial.available())
        {
            switch (Serial.read())
            {
                case 48: //0
                    this->print_ = static_cast<void (detail::base_Joystick<Joystick, BUTTONS_SIZE, POTS_SIZE, IS_PULL_UP>::*)()>(&Joystick::template print_buttons<BUTTONS_SIZE, POTS_SIZE>);
                    break;
                case 49: //1
                    this->print_ = static_cast<void (detail::base_Joystick<Joystick, BUTTONS_SIZE, POTS_SIZE, IS_PULL_UP>::*)()>(&Joystick::template print_pots<BUTTONS_SIZE, POTS_SIZE>);
                    break;
            }
        }
    }

    void send()
    {
        this->send_data();
    }
};

template <size_t BUTTONS_SIZE, bool IS_PULL_UP>
class Joystick<BUTTONS_SIZE, 0, IS_PULL_UP> : private detail::base_Joystick<Joystick<BUTTONS_SIZE, 0, IS_PULL_UP>, BUTTONS_SIZE, 0, IS_PULL_UP>
{
private:
    friend class detail::base_Joystick<Joystick, BUTTONS_SIZE, 0, IS_PULL_UP>;
    friend class detail::base_Joystick_BUTTONS<Joystick, BUTTONS_SIZE, 0, IS_PULL_UP>;
    friend class detail::base_Joystick_POTS<Joystick, BUTTONS_SIZE, 0>;

    const uint_fast8_t (&BUTTONS)[BUTTONS_SIZE];

    void determine_print()
    {
        switch (Serial.read())
        {
            case 48: //0
                this->print_ = static_cast<void (detail::base_Joystick<Joystick, BUTTONS_SIZE, 0, IS_PULL_UP>::*)()>(&Joystick::template print_buttons<BUTTONS_SIZE, 0>);
                break;
            case 49: //1
                Serial.println(F("There are no potentiometers!"));
                Serial.println(reinterpret_cast<const __FlashStringHelper*>(detail::print_values::invalid));
                Serial.println();
                break;
            default:
                Serial.println(reinterpret_cast<const __FlashStringHelper*>(detail::print_values::invalid));
                Serial.println();
                break;
        }
    }

public:
    Joystick(const uint_fast8_t (&BUTTONS)[BUTTONS_SIZE], decltype(nullptr), decltype(nullptr))
        : BUTTONS(BUTTONS)
    {}

    void setup()
    {
        this->setup_buttons();
    }

    //Read button values
    void read()
    {
        memset(this->data_buttons, 0, sizeof(this->data_buttons));
        this->read_buttons();
    }

    //Designed for Serial Plotter
    void print()
    {
        (this->*(this->print_))();
    }

    void send()
    {
        this->send_data();
    }
};

template <size_t POTS_SIZE, bool IS_PULL_UP>
class Joystick<0, POTS_SIZE, IS_PULL_UP> : private detail::base_Joystick<Joystick<0, POTS_SIZE, IS_PULL_UP>, 0, POTS_SIZE, IS_PULL_UP>
{
private:
    friend class detail::base_Joystick<Joystick, 0, POTS_SIZE, IS_PULL_UP>;
    friend class detail::base_Joystick_BUTTONS<Joystick, 0, POTS_SIZE, IS_PULL_UP>;
    friend class detail::base_Joystick_POTS<Joystick, 0, POTS_SIZE>;

    const uint_fast8_t (&POTS)[POTS_SIZE];
    uint_fast16_t (&CALIBRATION)[POTS_SIZE * 2];

    void determine_print()
    {
        switch (Serial.read())
        {
            case 48: //0
                Serial.println(F("There are no buttons!"));
                Serial.println(reinterpret_cast<const __FlashStringHelper*>(detail::print_values::invalid));
                Serial.println();
                break;
            case 49: //1
                this->print_ = static_cast<void (detail::base_Joystick<Joystick, 0, POTS_SIZE, IS_PULL_UP>::*)()>(&Joystick::template print_pots<0, POTS_SIZE>);
                break;
            default:
                Serial.println(reinterpret_cast<const __FlashStringHelper*>(detail::print_values::invalid));
                Serial.println();
                break;
        }
    }

public:
    Joystick(decltype(nullptr), const uint_fast8_t (&POTS)[POTS_SIZE], uint_fast16_t (&CALIBRATION)[POTS_SIZE * 2])
        : POTS(POTS), CALIBRATION(CALIBRATION)
    {}

    void setup()
    {
        this->setup_pots();
    }

    //Read potentiometer values
    void read()
    {
        memset(this->data_pots, 0, sizeof(this->data_pots));
        this->read_pots();
    }
    
    //Designed for Serial Plotter
    void print()
    {
        (this->*(this->print_))();
    }

    void send()
    {
        this->send_data();
    }
};