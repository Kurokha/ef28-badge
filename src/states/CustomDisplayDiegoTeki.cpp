// MIT License
//
// Copyright 2024 Eurofurence e.V. 
// 
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the “Software”),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

/**
 * @author Kuro
 */

#include <EFLed.h>
#include <EFLogging.h>
#include <CustomDiegoTeki.h>

#include "FSMState.h"

#define ANIMATE_RAINBOW_NUM_TOTAL 4  //!< Number of available animations

/**
 * @brief Index of all animations, each consisting of a periodically called
 * animation function and an associated tick rate in milliseconds.
 
const struct {
    void (AnimateRainbow::* animate)();
    const unsigned int tickrate;
} animations[ANIMATE_RAINBOW_NUM_TOTAL] = {
    {.animate = &AnimateRainbow::_staticDiegoTeki, .tickrate = 20},
    {.animate = &AnimateRainbow::_animateDiegoTekiCircle, .tickrate = 20},
    {.animate = &AnimateRainbow::_animateDiegoTeki, .tickrate = 100},
    {.animate = &AnimateRainbow::_animateDiegoTeki, .tickrate = 20},

};
*/

const char* CustomDisplayDiegoTeki::getName() {
    return "CustomDiegoTeki";
}

bool CustomDisplayDiegoTeki::shouldBeRemembered() {
    return true;
}

const unsigned int CustomDisplayDiegoTeki::getTickRateMs() {
    return 20;
}

void CustomDisplayDiegoTeki::entry() {
    this->switchdelay_ms = 5000;
    this->tick = 0;
}

void CustomDisplayDiegoTeki::run() {
    // Check if we need to switch the flag (Mode: 0)
    if (this->tick % (this->switchdelay_ms / this->getTickRateMs()) == 0) {
        if (this->globals->customIdx == 0) {
            // Cycle through all flags
            //LOGF_DEBUG("(CustomDiegoTeki) Switched mode to: %d\r\n", customIdx);
            //customIdx = (customIdx + 1) % 12;
        }
    }

    // Determine pride flag to show
    const CRGB* diegoTekiColor = CustomDiegoTeki::DiegoTeki;
    
    // Animate dragon: Rotate current flag to cycle through dragon head
    std::vector<CRGB> rotatedflag(diegoTekiColor, diegoTekiColor + EFLED_EFBAR_NUM);
    std::rotate(rotatedflag.begin(), rotatedflag.begin() + (this->tick % (EFLED_EFBAR_NUM*20)) / 20, rotatedflag.end());

    // Animate dragon: Create current and next dragon head patterns
    CRGB dragon[EFLED_DRAGON_NUM];
    CRGB dragon_next[EFLED_DRAGON_NUM];
    std::copy(rotatedflag.begin(), rotatedflag.begin() + EFLED_DRAGON_NUM, dragon);
    std::copy(rotatedflag.begin() + 1, rotatedflag.begin() + 1 + EFLED_DRAGON_NUM, dragon_next);

    // Animate dragon: Blend both patterns based on current tick and reduce brightness
    CRGB dragon_now[EFLED_DRAGON_NUM];
    blend(dragon, dragon_next, dragon_now, EFLED_DRAGON_NUM, ((this->tick % 20) / 20.0) * 255);
    fadeLightBy(dragon_now, EFLED_DRAGON_NUM, 128);

    // Animate dragon: Finally show it!
    EFLed.setDragon(dragon_now);

    // Refresh flag periodically
    if (this->tick % (this->switchdelay_ms / this->getTickRateMs()) == 0) {
        EFLed.setEFBar(diegoTekiColor);
    }

    // Prepare next tick
    this->tick++;
}

std::unique_ptr<FSMState> CustomDisplayDiegoTeki::touchEventFingerprintShortpress() {
    if (this->isLocked()) {
        return nullptr;
    }

    return std::make_unique<MenuMain>();
}

std::unique_ptr<FSMState> CustomDisplayDiegoTeki::touchEventFingerprintLongpress() {
    return this->touchEventFingerprintShortpress();
}

std::unique_ptr<FSMState> CustomDisplayDiegoTeki::touchEventFingerprintRelease() {
    if (this->isLocked()) {
        return nullptr;
    }

    this->globals->customIdx = (this->globals->customIdx + 1) % 13;
    this->is_globals_dirty = true;
    this->tick = 0;

    return nullptr;
}

std::unique_ptr<FSMState> CustomDisplayDiegoTeki::touchEventAllLongpress() {
    this->toggleLock();
    return nullptr;
}
