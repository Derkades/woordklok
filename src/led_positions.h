// number of leds per letter
#define LETTER_LEDS 2

#define NUM_ROWS 10
#define NUM_COLS 11
#define NUM_LETTERS NUM_ROWS*NUM_COLS
#define NUM_LEDS NUM_LETTERS*LETTER_LEDS

typedef uint16_t *word_t;

// First element denotes array length
uint16_t HET    [] = {3, 109, 108, 107                };
uint16_t IS     [] = {2, 105, 104                     };
uint16_t VIJF   [] = {4, 102, 101, 100, 99            };

uint16_t TIEN   [] = {4, 88, 89, 90, 91               };
uint16_t VOOR   [] = {4, 95, 96, 97, 98               };

uint16_t OVER   [] = {4, 87, 86, 85, 84               };
uint16_t KWART  [] = {5, 81, 80, 79, 78, 77           };

uint16_t HALF   [] = {4, 66, 67, 68, 69               };
uint16_t OVER_2 [] = {4, 73, 74, 75, 76               };

uint16_t VOOR_2 [] = {4, 65, 64, 63, 62               };
uint16_t EEN    [] = {3, 58, 57, 56                   };

uint16_t TWEE   [] = {4, 44, 45, 46, 47               };
uint16_t DRIE   [] = {4, 51, 52, 53, 54               };

uint16_t VIER   [] = {4, 43, 42, 41, 40               };
uint16_t VIJF_2 [] = {4, 39, 38, 37, 36               };
uint16_t ZES    [] = {3, 35, 34, 33                   };

uint16_t ZEVEN  [] = {5, 22, 23, 24, 25, 26           };
uint16_t NEGEN  [] = {5, 28, 29, 30, 31, 32           };

uint16_t ACHT   [] = {4, 21, 20, 19, 18               };
uint16_t TIEN_2 [] = {4, 18, 17, 16, 15               };
uint16_t ELF    [] = {3, 13, 12, 11                   };

uint16_t TWAALF [] = {6, 0, 1, 2, 3, 4, 5             };
uint16_t UUR    [] = {3, 8, 9, 10,                    };

uint16_t ROBIN  [] = {5, 92, 83, 70, 61, 48           };
uint16_t HALLO  [] = {5, 93, 82, 71, 60, 49           };

word_t HOURS[] = {
    EEN, TWEE, DRIE, VIER, VIJF_2, ZES, ZEVEN, ACHT, NEGEN, TIEN_2, ELF, TWAALF
};

void letterToRowCol(uint16_t letter, uint8_t *row, uint8_t *col) {
    *row = (NUM_ROWS-1) - letter / NUM_COLS;
    // every other row has swapped direction
    *col = *row % 2 == 0 ? letter % NUM_COLS : (NUM_COLS-1) - (letter % NUM_COLS);
}
