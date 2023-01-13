#ifndef _EFFECTS_H_
#define	_EFFECTS_H_

enum LedEffect {
    EFFECT_STATIC,
    EFFECT_COLOR_FADE,
    EFFECT_RAINBOW,
    EFFECT_SPARKLE_STATIC,
    EFFECT_SHOWER,
    EFFECT_SHOWER_COLOR_FADE,
    EFFECT_TEST_PATTERN,
};

// extern const char *led_effect_names[];

bool ledEffectFromString(const char *str, LedEffect *effect);

const char *ledEffectToString(const LedEffect &ledEffect);

#endif // _EFFECTS_H_
