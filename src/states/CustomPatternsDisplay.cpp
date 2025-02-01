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
#include <CustomPatterns.h>
#include <EFPrideFlags.h>

#include "FSMState.h"

#define ANIMATE_CUSTOM_NUM_TOTAL 4  //!< Number of available animations

/**
 * @brief Index of all animations, each consisting of a periodically called
 * animation function and an associated tick rate in milliseconds.
 */ 
const struct {
    void (CustomPatternsDisplay::* animate)();
    const unsigned int tickrate;
} animations[ANIMATE_CUSTOM_NUM_TOTAL] = {
//    {.animate = &CustomPatternsDisplay::_staticPattern, .tickrate = 20},
    {.animate = &CustomPatternsDisplay::_rotatingDragonHead, .tickrate = 20},
    {.animate = &CustomPatternsDisplay::_rotatingFull, .tickrate = 20},
    {.animate = &CustomPatternsDisplay::_starlight, .tickrate = 20},
    {.animate = &CustomPatternsDisplay::_randomPattern, .tickrate = 200},
};

const char* CustomPatternsDisplay::getName() {
    return "CustomPattern";
}

bool CustomPatternsDisplay::shouldBeRemembered() {
    return true;
}

const unsigned int CustomPatternsDisplay::getTickRateMs() {
    return animations[this->globals->cstPatternsIdx % ANIMATE_CUSTOM_NUM_TOTAL].tickrate;
    return 20;
}

void CustomPatternsDisplay::entry() {
    this->switchdelay_ms = 5000;
    this->tick = 0;
}

void CustomPatternsDisplay::run() {
    (*this.*(animations[this->globals->cstPatternsIdx % ANIMATE_CUSTOM_NUM_TOTAL].animate))();
    this->tick++;
}

std::unique_ptr<FSMState> CustomPatternsDisplay::touchEventFingerprintShortpress() {
    if (this->isLocked()) {
        return nullptr;
    }

    return std::make_unique<MenuMain>();
}

std::unique_ptr<FSMState> CustomPatternsDisplay::touchEventFingerprintLongpress() {
    return this->touchEventFingerprintShortpress();
}

std::unique_ptr<FSMState> CustomPatternsDisplay::touchEventFingerprintRelease() {
    if (this->isLocked()) {
        return nullptr;
    }

    this->globals->cstPatternsIdx = (this->globals->cstPatternsIdx + 1) %  ANIMATE_CUSTOM_NUM_TOTAL;
    this->is_globals_dirty = true;
    this->tick = 0;
    EFLed.clear();

    LOGF_INFO(
        "(CustomPatterns) Changed animation mode to: %d\r\n",
        this->globals->cstPatternsIdx % ANIMATE_CUSTOM_NUM_TOTAL
    );

    return nullptr;
}

std::unique_ptr<FSMState> CustomPatternsDisplay::touchEventAllLongpress() {
    this->toggleLock();
    return nullptr;
}

/**
 * @brief Static EFBar with rotating dragon head
 */
void CustomPatternsDisplay::_rotatingDragonHead() {
    // Determine colors to show
    const CRGB* customPatternsColor = CustomPatterns::CircularDragonHead;
    
    // Animate dragon: Rotate color palette to cycle through dragon head
    std::vector<CRGB> rotatedflag(customPatternsColor, customPatternsColor + EFLED_EFBAR_NUM);
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

    // Refresh periodically
    if (this->tick % (this->switchdelay_ms / this->getTickRateMs()) == 0) {
        EFLed.setEFBar(customPatternsColor);
    }

    // Prepare next tick
    this->tick++;
}

/*
void CustomPatternsDisplay::_animateCirclePattern() {
    EFLed.setAllSolid(CRGB(customPatternsColor[tick % 17]));
}
*/

/**
 * @brief Rotating all LEDs
 */
void CustomPatternsDisplay::_rotatingFull() {
    // Determine colors to show
    const CRGB* customPatternsColor = CustomPatterns::CircularFull;

    CRGB data[EFLED_TOTAL_NUM];
//    fill_palette_circular(data, EFLED_TOTAL_NUM, (tick % 128) * 2, customPatternsColor,
//        this->globals->ledBrightnessPercent * 255, LINEARBLEND, true);
    EFLed.setAll(data);
}

/**
 * @brief Starlight effect
 */
void CustomPatternsDisplay::_starlight() {
    CRGB data[EFLED_TOTAL_NUM];
    CRGB data_next[EFLED_TOTAL_NUM];
    for( int i = 0; i < EFLED_TOTAL_NUM; ++i) {
        if(random(0, 5) > 1) {
            data[i] = 0x06ffa1; // blue
        }
        else {
            data[i] = 0xff3a1a; // orange
        }
        if(random(0, 5) > 1) {
            data_next[i] = 0x06ffa1; // blue
        }
        else {
            data_next[i] = 0xff3a1a; // orange
        }
    }
    if (this->tick % (this->switchdelay_ms / this->getTickRateMs()) == 0) {
        // Create current and next pattern
        for( int i = 0; i < EFLED_TOTAL_NUM; ++i) {
            if(random(0, 5) > 1) {
                data[i] = 0x06ffa1; // blue
            }
            else {
                data[i] = 0xff3a1a; // orange
            }
            if(random(0, 5) > 1) {
                data_next[i] = 0x06ffa1; // blue
            }
            else {
                data_next[i] = 0xff3a1a; // orange
            }
        }
    }

    // Blend both patterns based on current tick and reduce brightness
    CRGB data_now[EFLED_DRAGON_NUM];
    blend(data, data_next, data_now, EFLED_DRAGON_NUM, ((this->tick % 20) / 20.0) * 255);
    fadeLightBy(data_now, EFLED_DRAGON_NUM, 128);

    // Animate dragon: Finally show it!
    EFLed.setAll(data_now);

    // Prepare next tick
    this->tick++;
}

/**
 * @brief Starlight effect
 */
void CustomPatternsDisplay::_randomPattern() {
    const CRGB Random[] = {
        CHSV(random(0, 255), 255, 255),
        CHSV(random(0, 255), 255, 255),
        CHSV(random(0, 255), 255, 255),
        CHSV(random(0, 255), 255, 255),
        CHSV(random(0, 255), 255, 255),
        CHSV(random(0, 255), 255, 255),
        CHSV(random(0, 255), 255, 255),
        CHSV(random(0, 255), 255, 255),
        CHSV(random(0, 255), 255, 255),
        CHSV(random(0, 255), 255, 255),
        CHSV(random(0, 255), 255, 255),
    };

    const CRGB* prideFlag = Random;

    // Animate dragon: Rotate current flag to cycle through dragon head
    std::vector<CRGB> rotatedflag(prideFlag, prideFlag + EFLED_EFBAR_NUM);
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
        EFLed.setEFBar(prideFlag);
    }

    // Prepare next tick
    this->tick++;
}