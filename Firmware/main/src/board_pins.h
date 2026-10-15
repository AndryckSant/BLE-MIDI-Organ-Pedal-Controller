#ifndef BOARD_PINS_H
#define BOARD_PINS_H

#include <stdint.h>

namespace BoardPins
{
constexpr uint8_t kConfigButton = 15U;

constexpr uint8_t kMidiButtons[] = {
    13U, 12U, 14U, 27U, 26U, 25U, 33U, 32U};

constexpr uint8_t kMidiButtonCount =
    sizeof(kMidiButtons) / sizeof(kMidiButtons[0]);

inline bool isMidiButton(uint8_t pin)
{
    for (uint8_t index = 0U;
         index < kMidiButtonCount;
         ++index)
    {
        if (kMidiButtons[index] == pin)
        {
            return true;
        }
    }

    return false;
}
}

#endif // BOARD_PINS_H
