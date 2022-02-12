typedef unsigned char letter_t;
typedef unsigned char *word_t;

// First element denotes array length
letter_t VIJF   [] = {4, 7, 8, 9, 10                  };
letter_t IS     [] = {2, 4, 5                         };
letter_t HET    [] = {3, 0, 1, 2                      };
letter_t TIEN   [] = {4, 21, 20, 19, 18               };
letter_t VOOR   [] = {4, 14, 13, 12, 11               };
letter_t KWART  [] = {5, 28, 29, 30, 31, 32           };
letter_t OVER   [] = {4, 22, 23, 24, 25               };
letter_t HALF   [] = {4, 43, 42, 41, 40               };
letter_t OVER_2 [] = {4, 36, 35, 34, 33               };
letter_t EEN    [] = {3, 51, 52, 53                   };
letter_t VOOR_2 [] = {4, 44, 45, 46, 47               };
letter_t DRIE   [] = {4, 58, 57, 56, 55               };
letter_t TWEE   [] = {4, 65, 64, 63, 62               };
letter_t VIER   [] = {4, 66, 67, 68, 69               };
letter_t VIJF_2 [] = {4, 70, 71, 72, 73               };
letter_t ZES    [] = {3, 74, 75, 76                   };
letter_t ZEVEN  [] = {5, 87, 86, 85, 84, 83           };
letter_t NEGEN  [] = {5, 81, 80, 79, 78, 77           };
letter_t ACHT   [] = {4, 88, 89, 90, 91               };
letter_t TIEN_2 [] = {4, 91, 92, 93, 94               };
letter_t ELF    [] = {3, 96, 97, 98                   };
letter_t TWAALF [] = {6, 109, 108, 107, 106, 105, 104 };
letter_t UUR    [] = {3, 101, 100, 99                 };
letter_t ROBIN  [] = {5, 17, 26, 39, 48, 61           };
letter_t HALLO  [] = {5, 16, 27, 38, 49, 60           };

word_t HOURS[] = {
    EEN, TWEE, DRIE, VIER, VIJF_2, ZES, ZEVEN, ACHT, NEGEN, TIEN_2, ELF, TWAALF
};

short letterByRowCol(short row, short col) {
    short base = row * 11;
    return row % 2 == 0 ? base + col : base - col;
}

void letterToRowCol(short letter, short *row, short *col) {
    *row = letter / 11;
    *col = *row % 2 == 0 ? letter % 11 : 10 - (letter % 11);
}
