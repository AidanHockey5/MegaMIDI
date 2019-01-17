#include <stdint.h>

//Great SN76489 driver example by https://github.com/allenhuffman/MusicSequencerTest/blob/master/SN76489.ino
#define LOWEST_NOTE   20  // B2
#define HIGHEST_NOTE  88  // C8 It is really NC9, beyond piano.
#define DATA_CMD      0b00000000
#define LATCH_CMD     0b10000000
#define TYPE_TONE     0b00000000
#define TYPE_VOL      0b00010000
#define VOL_OFF     0b00001111  // 15
#define VOL_LOW     0b00001010  // 10
#define VOL_MED     0b00000101  // 5
#define VOL_MAX     0b00000000  // 0
static const uint16_t PROGMEM G_notes[88] = 
{
  //DEC     HEX     #   NOTE  MIDI  Comments
  4545,  // 0x11C1  0   A0    21    Lowest piano key. (MIDI note 21)
  4290,  // 0x10C2  1   A0#   22
  4050,  // 0xFD2   2   B0    23

  3822,  // 0xEEE   3   C1    24
  3608,  // 0xE18   4   C1#   25
  3405,  // 0xD4D   5   D1    26
  3214,  // 0xC8E   6   D1#   27
  3034,  // 0xBDA   7   E1    28
  2863,  // 0xB2F   8   F1    29
  2703,  // 0xA8F   9   F1#   30
  2551,  // 0x9F7   10  G1    31
  2408,  // 0x968   11  G1#   32
  2273,  // 0x8E1   12  A1    33
  2145,  // 0x861   13  A1#   34
  2025,  // 0x7E9   14  B1    35
 
  1911,  // 0x777   15  C2    36
  1804,  // 0x70C   16  C2#   37
  1703,  // 0x6A7   17  D2    38
  1607,  // 0x647   18  D2#   39
  1517,  // 0x5ED   19  E2    40
  1432,  // 0x598   20  F2    41
  1351,  // 0x547   21  F2#   42
  1276,  // 0x4FC   22  G2    43
  1204,  // 0x4B4   23  G2#   44
  1136,  // 0x470   24  A2    45
  1073,  // 0x431   25  A2#   46
  1012,  // 0x3F4   26  B2    47    LOWEST NOTE AT 4Mhz (?)

  956,   // 0x3BC   27  C3    48
  902,   // 0x386   28  C3#   49
  851,   // 0x353   29  D3    50
  804,   // 0x324   20  D3#   51
  758,   // 0x2F6   31  E3    52
  716,   // 0x2CC   32  F3    53
  676,   // 0x2A4   33  F3#   54
  638,   // 0x27E   34  G3    55
  602,   // 0x25A   35  G3#   56
  568,   // 0x238   36  A3    57
  536,   // 0x218   37  A3#   58
  506,   // 0x1FA   38  B3    59

  478,   // 0x1DE   39  C4    60    MIDDLE C
  451,   // 0x1C3   40  C4#   61
  426,   // 0x1AA   41  D4    62
  402,   // 0x192   42  D4#   63
  379,   // 0x17B   43  E4    64
  358,   // 0x166   44  F4    65
  338,   // 0x152   45  F4#   66
  319,   // 0x13F   46  G4    67
  301,   // 0x12D   47  G4#   68
  284,   // 0x11C   48  A4    69    440hz (standard tuning)
  268,   // 0x10C   49  A4#   70
  253,   // 0xFD    50  B4    71

  239,   // 0xEF    51  C5    72
  225,   // 0xE1    52  C5#   73
  213,   // 0xD5    53  D5    74
  201,   // 0xC9    54  D5#   75
  190,   // 0xBE    55  E5    76
  179,   // 0xB3    56  F5    77
  169,   // 0xA9    57  F5#   78
  159,   // 0x9F    58  G5    79
  150,   // 0x96    59  G5#   80
  142,   // 0x8E    60  A5    81
  134,   // 0x86    61  A5#   82
  127,   // 0x7F    62  B5    83

  119,   // 0x77    63  C6    84
  113,   // 0x71    64  C6#   85
  106,   // 0x6A    65  D6    86
  100,   // 0x64    66  D6#   87
  95,    // 0x5F    67  E6    88
  89,    // 0x59    68  F6    89
  84,    // 0x54    69  F6#   90
  80,    // 0x50    70  G6    91
  75,    // 0x4B    71  G6#   92
  71,    // 0x47    72  A6    93
  67,    // 0x43    73  A6#   94
  63,    // 0x3F    74  B6    95

  60,    // 0x3C    75  C7    96
  56,    // 0x38    76  C7#   97
  53,    // 0x35    77  D7    98 
  50,    // 0x32    78  D7#   99
  47,    // 0x2F    79  E7    100
  45,    // 0x2D    80  F7    101
  42,    // 0x2A    81  F7#   102
  40,    // 0x28    82  G7    103
  38,    // 0x26    83  G7#   104
  36,    // 0x24    84  A7    105
  34,    // 0x22    85  A7#   106
  32,    // 0x20    86  B7    107

  30,    // 0x1E    87  C8    108   Highest piano key.
/*
 * These are higher notes that are not on a piano.
 * 
  28,    // 0x1C    88  C8#   109 
  27,    // 0x1B    89  D8    110
  25,    // 0x19    90  D8#   111
  24,    // 0x18    91  E8    112
  22,    // 0x16    92  F8    113
  21,    // 0x15    93  G8    114
  20,    // 0x14    94  G8#   115
  19,    // 0x13    95  A8    116
  18,    // 0x12    96  A8#   117
  17,    // 0x11    97  B8    118
 
  16,    // 0x10    98  C9    119   HIGHEST NOTE at 4MHz (?)
  15,    // 0xF     99  C8#   120
  14,    // 0xE     100 D8    121
  13,    // 0xD     101 D8#   122
  13,    // 0xD     102 E8    123
  12,    // 0xC     103 F8    124
  11,    // 0xB     104 F8#   125
  11,    // 0xB     105 G8    126
  10,    // 0xA     106 G8#   127   HIGHEST MIDI NOTE
*/
};