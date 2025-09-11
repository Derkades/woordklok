#ifndef EFFECTS_H
#define EFFECTS_H

#include <string.h>

enum LedEffect {
    EFFECT_STATIC,
    EFFECT_COLOR_FADE,
    EFFECT_RAINBOW,
    EFFECT_SPARKLE_STATIC,
    EFFECT_SHOWER,
    EFFECT_SHOWER_COLOR_FADE,
    EFFECT_TEST_PATTERN,
};

bool ledEffectFromString(const char *str, LedEffect *effect);

const char *ledEffectToString(const LedEffect &ledEffect);

#endif // EFFECTS_H
