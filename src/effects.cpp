#include <string.h>
#include "effects.h"

const char *led_effect_names[] {
    "Vaste kleur",
    "Testpatroon",
    "Regenboog",
    "Schitter",
    "Regen",
};

bool ledEffectFromString(const char *str, LedEffect *effect) {
    int i = 0;
    for (const char *led_effect_name : led_effect_names) {
        if (strcmp(str, led_effect_name) == 0) {
            *effect = static_cast<LedEffect>(i);
            return true;
        }
        i++;
    }
    return false;
}

const char *ledEffectToString(const LedEffect &ledEffect) {
    return led_effect_names[ledEffect];
}
