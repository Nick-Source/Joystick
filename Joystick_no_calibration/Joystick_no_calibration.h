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

namespace detail
{
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
            for (size_t i = 0; i <= BUTTONS_SIZE;)
            {
                if (!digitalRead(JOYSTICK->BUTTONS[i]))
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
            for (size_t i = 0; i <= BUTTONS_SIZE;)
            {
                if (digitalRead(JOYSTICK->BUTTONS[i]))
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
                
                Serial.print(JOYSTICK->BUTTONS[i]);
                Serial.print(":");
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
                
                Serial.print(JOYSTICK->BUTTONS[i]);
                Serial.print(":");
                Serial.print(bitRead(data_buttons[i / (sizeof(uint_fast16_t) * CHAR_BIT)], i % (sizeof(uint_fast16_t) * CHAR_BIT)));
            }

            for (size_t i = 0; i < POTS_SIZE - BUTTONS_SIZE; ++i)
            {
                (JOYSTICK->*(JOYSTICK->printComma))();
                Serial.print("N/A:0");
            }
            
            Serial.println();
        }

        template<bool value = IS_PULL_UP, typename detail::enable_if<value>::type* = nullptr>
        void setup_buttons() //PULL_UP
        {
        #ifdef INPUT_PULLUP
            for (const auto itr : JOYSTICK->BUTTONS)
                pinMode(itr, INPUT_PULLUP);
        #else
            #warning INPUT_PULLUP is not defined, falling back to INPUT
            for (const auto itr : JOYSTICK->BUTTONS)
                pinMode(itr, INPUT);
        #endif
        }

        template<bool value = IS_PULL_UP, typename detail::enable_if<!value>::type* = nullptr>
        void setup_buttons() //PULL_DOWN
        {
        #ifdef INPUT_PULLDOWN
            for (const auto itr : JOYSTICK->BUTTONS)
                pinMode(itr, INPUT_PULLDOWN);
        #else
            #warning INPUT_PULLDOWN is not defined, falling back to INPUT
            for (const auto itr : JOYSTICK->BUTTONS)
                pinMode(itr, INPUT);
        #endif
        }
    };

    template <class T, size_t POTS_SIZE, bool IS_PULL_UP>
    class base_Joystick_BUTTONS<T, 0, POTS_SIZE, IS_PULL_UP> {};

    template <class T, size_t BUTTONS_SIZE, size_t POTS_SIZE>
    class base_Joystick_POTS
    {
    protected:
        base_Joystick_POTS() {}
        uint_fast16_t data_pots[POTS_SIZE] = {0};

        void read_pots()
        {
            for (size_t i = 0; i < POTS_SIZE; ++i)
            {
                data_pots[i] = analogRead(JOYSTICK->POTS[i]);
            }
        }

        template<size_t value_b = BUTTONS_SIZE, size_t value_p = POTS_SIZE, typename detail::enable_if<(value_p >= value_b)>::type* = nullptr>
        void print_pots()
        {
            JOYSTICK->printComma = &T::printComma_;

            for (size_t i = 0; i < POTS_SIZE; ++i)
            {
                (JOYSTICK->*(JOYSTICK->printComma))();

                Serial.print(JOYSTICK->POTS[i]);
                Serial.print(":");
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

                Serial.print(JOYSTICK->POTS[i]);
                Serial.print(":");
                Serial.print(data_pots[i]);
            }

            for (size_t i = 0; i < BUTTONS_SIZE - POTS_SIZE; ++i)
            {
                (JOYSTICK->*(JOYSTICK->printComma))();
                Serial.print("N/A:0");
            }

            Serial.println();
        }

        void setup_pots()
        {
            for (const auto itr : JOYSTICK->POTS)
                pinMode(itr, INPUT);
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
        void printComma__() { Serial.print(", "); }

        void (base_Joystick::*print_)() = &base_Joystick::print__;

        void print__()
        { 
            while (Serial.available())
                Serial.read();

            Serial.println("Enter 0 to print buttons or 1 to print potentiometers.");
            Serial.println("You are able to switch between the two, just enter the value into Serial later.");
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
                Serial.println("Invalid input.");
                Serial.println();
                break;
        }
    }

public:
    Joystick(const uint_fast8_t (&BUTTONS)[BUTTONS_SIZE], const uint_fast8_t (&POTS)[POTS_SIZE])
        : BUTTONS(BUTTONS), POTS(POTS)
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
                Serial.println("There are no potentiometers!");
                Serial.println("Invalid input.");
                Serial.println();
                break;
            default:
                Serial.println("Invalid input.");
                Serial.println();
                break;
        }
    }

public:
    Joystick(const uint_fast8_t (&BUTTONS)[BUTTONS_SIZE], decltype(nullptr))
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

    void determine_print()
    {
        switch (Serial.read())
        {
            case 48: //0
                Serial.println("There are no buttons!");
                Serial.println("Invalid input.");
                Serial.println();
                break;
            case 49: //1
                this->print_ = static_cast<void (detail::base_Joystick<Joystick, 0, POTS_SIZE, IS_PULL_UP>::*)()>(&Joystick::template print_pots<0, POTS_SIZE>);
                break;
            default:
                Serial.println("Invalid input.");
                Serial.println();
                break;
        }
    }

public:
    Joystick(decltype(nullptr), const uint_fast8_t (&POTS)[POTS_SIZE])
        : POTS(POTS)
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