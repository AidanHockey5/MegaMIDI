#ifndef ADJUSTMNETS_H_
#define ADJUSTMNETS_H_

#define SEMITONE_ADJ_YM 3 //Adjust this to add or subtract semitones to the final note on the YM2612.
#define TUNE -0.065     //Use this constant to tune your instrument!
#define SEMITONE_ADJ_PSG 0 //Adjust this to add or subtract semitones to the final note on the PSG.
#define MAX_OCTAVE_SHIFT 5

static unsigned char octaveShift = 0;
static unsigned short pitchBendYM = 0;
static unsigned char pitchBendYMRange = 2; //How many semitones would you like the pitch-bender to range? Standard = 2

#endif