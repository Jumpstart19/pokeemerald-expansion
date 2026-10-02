#ifndef GUARD_CONSTANTS_TEXT_BLIPS_H
#define GUARD_CONSTANTS_TEXT_BLIPS_H

#define WEIGHT_DEFAULT  1

#define TEMPO_DEFAULT  0
#define TEMPO_PHENOMES 2000

#define VOLUME_DEFAULT             0
#define VOLUME_VARIANCE_NONE       0  // Recommended for songs with master volume at or near max volume of 127
#define VOLUME_VARIANCE_DEFAULT    10

#define PITCH_SHIFT_DEFAULT              0
#define PITCH_SHIFT_QUARTER_OCTAVE_UP    3 // Recommended smallest pitch shift up that sounds distinct
#define PITCH_SHIFT_QUARTER_OCTAVE_DOWN -3 // Recommended smallest pitch shift down that sounds distinct
#define PITCH_VARIANCE_NONE              0
#define PITCH_VARIANCE_DEFAULT           1

#define VOICE_DEFAULT   -1

#define FREQUENCY_EVERY_CHARACTER  0
#define FREQUENCY_DEFAULT          1

#define SPOKEN_CHARACTERS              0
#define SPOKEN_CHARACTERS_NO_NUMBERS   1
#define PRINTED_CHARACTERS             2

#define MAX_BYTES_TO_CHECK  50 // Chosen somewhat arbitrarily (large enough to sift through all data that could possibly preceed the VOICE keyword in a track's data)

#define MAX_VARIANCE_SEGMENTS 10 // Ensures full variance in volume/pitch can be heard when MAKE_TEXT_BLIPS_PREDICTABLE is TRUE

// The IDs of the audio settings for each text blip in sTextBlipAudioValues.
// Also add to charmap.txt.
#define PHENOME_DEFAULT 0
#define PHENOME_HIGH_1  1
#define PHENOME_HIGH_2  2
#define PHENOME_HIGH_3  3
#define PHENOME_HIGH_4  4
#define PHENOME_HIGH_5  5
#define PHENOME_LOW_1   6
#define PHENOME_LOW_2   7
#define PHENOME_LOW_3   8
#define PHENOME_LOW_4   9
#define TEXT_PRINTER_1  10
#define TEXT_PRINTER_2  11
#define TEXT_PRINTER_3  12
#define TEXT_PRINTER_4  13
#define TEXT_PRINTER_5  14
#define TEXT_PRINTER_6  15
#define SPOOKY          16
#define ROBO_1          17
#define ROBO_2          18
#define ROBO_3          19
#define ROBO_4          20
#define VOICE_1         21
#define VOICE_2         22
#define VOICE_3         23
#define VOICE_4         24
#define VOICE_5         25
#define VOICE_6         26

#endif //GUARD_CONSTANTS_TEXT_BLIPS_H
