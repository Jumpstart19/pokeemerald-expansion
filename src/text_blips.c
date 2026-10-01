#include "global.h"
#include "m4a.h"
#include "main.h"
#include "malloc.h"
#include "random.h"
#include "sound.h"
#include "text.h"
#include "text_blips.h"
#include "constants/characters.h"
#include "constants/songs.h"
#include "constants/text_blips.h"

struct TextBlipAudioClip
{
    u16 songNum;    // Song ideally with a single note on track 1 that is held for 0.1-0.2 seconds
    u8 weight;      // Adjusts how often a clip is selected when using multiple audio clips
    s8 voice;       // Sets voice number in song's current voicegroup to be used for the text blip; only applies to first track
};

struct TextBlipAudioValues
{
    const struct TextBlipAudioClip *clips;
    u8 numClips;
    bool8 equalWeights; // If TRUE, overrides weights in clips and considers all weights equal
    u16 tempoAdjust;    // Scales default tempo by tempoAdjust/256; used to adjust song duration to 0.1-0.2 seconds
    u8 volume;          // Sets base volume value for the text blip (accepts values between 1 and 127)
    u8 volumeRange;     // Percent variance in volume (e.g. if volumeRange = 10, text blips will play with volumes +/- 10% of the base volume)
    s8 pitchShift;      // Shift in semitones from notes' default pitch in song (e.g. if pitchShift = -12, text blips will play with a pitch one octave lower than the notes' default pitch)
    u8 pitchRange;      // Variance in pitch in semitones from base value (e.g. if pitchRange = 2, text blips will play with pitches within one whole step above or below the base pitch)
    u8 frequency;       // Number of characters printed until text blip is played (e.g. if frequency = 2, text blips play every other character)
    u8 validChars;      // Characters that produce a text blip
};

static const EWRAM_DATA struct TextBlipAudioValues *sTextBlipDataPtr = NULL;
static EWRAM_DATA u8 sCharsUntilNextTextBlip = 0;
static EWRAM_DATA u8 sBaseVolume = 0;
static EWRAM_DATA s8 sBasePitchShift = 0;
static EWRAM_DATA u16 sLastClip = 0;
EWRAM_DATA bool8 gTextBlipActive = FALSE;
EWRAM_DATA bool8 gTextBlipSetActive = FALSE;
EWRAM_DATA u8 gTextBlipVolume = 0;
EWRAM_DATA s8 gTextBlipPitchShift = 0;
EWRAM_DATA s16 gTextBlipVoice = 0;
EWRAM_DATA struct MusicPlayerInfo *gTextBlipMusicPlayerPtr = NULL;

static const u8 sTextBlipFrequencyAdjustment[] =
{
    [OPTIONS_TEXT_SPEED_SLOW]    = 1 * TEXT_SPEED_SLOW_MODIFIER,
    [OPTIONS_TEXT_SPEED_MID]     = 1 * TEXT_SPEED_MEDIUM_MODIFIER,
    [OPTIONS_TEXT_SPEED_FAST]    = 3 * TEXT_SPEED_FAST_MODIFIER,
    [OPTIONS_TEXT_SPEED_INSTANT] = 2 * TEXT_SPEED_INSTANT_MODIFIER / 3,
};

static const struct TextBlipAudioClip sPhenomeAudioClips[] = {
    {PH_TRAP_BLEND,     WEIGHT_DEFAULT, VOICE_DEFAULT},
    {PH_TRAP_SOLO,      WEIGHT_DEFAULT, VOICE_DEFAULT},
    {PH_FACE_BLEND,     WEIGHT_DEFAULT, VOICE_DEFAULT},
    {PH_FACE_SOLO,      WEIGHT_DEFAULT, VOICE_DEFAULT},
    {PH_CLOTH_BLEND,    WEIGHT_DEFAULT, VOICE_DEFAULT},
    {PH_CLOTH_SOLO,     WEIGHT_DEFAULT, VOICE_DEFAULT},
    {PH_DRESS_BLEND,    WEIGHT_DEFAULT, VOICE_DEFAULT},
    {PH_DRESS_SOLO,     WEIGHT_DEFAULT, VOICE_DEFAULT},
    {PH_FLEECE_BLEND,   WEIGHT_DEFAULT, VOICE_DEFAULT},
    {PH_FLEECE_SOLO,    WEIGHT_DEFAULT, VOICE_DEFAULT},
    {PH_KIT_BLEND,      WEIGHT_DEFAULT, VOICE_DEFAULT},
    {PH_KIT_SOLO,       WEIGHT_DEFAULT, VOICE_DEFAULT},
    {PH_PRICE_BLEND,    WEIGHT_DEFAULT, VOICE_DEFAULT},
    {PH_PRICE_SOLO,     WEIGHT_DEFAULT, VOICE_DEFAULT},
    {PH_LOT_BLEND,      WEIGHT_DEFAULT, VOICE_DEFAULT},
    {PH_LOT_SOLO,       WEIGHT_DEFAULT, VOICE_DEFAULT},
    {PH_GOAT_BLEND,     WEIGHT_DEFAULT, VOICE_DEFAULT},
    {PH_GOAT_SOLO,      WEIGHT_DEFAULT, VOICE_DEFAULT},
    {PH_THOUGHT_BLEND,  WEIGHT_DEFAULT, VOICE_DEFAULT},
    {PH_THOUGHT_SOLO,   WEIGHT_DEFAULT, VOICE_DEFAULT},
    {PH_CHOICE_BLEND,   WEIGHT_DEFAULT, VOICE_DEFAULT},
    {PH_CHOICE_SOLO,    WEIGHT_DEFAULT, VOICE_DEFAULT},
    {PH_MOUTH_BLEND,    WEIGHT_DEFAULT, VOICE_DEFAULT},
    {PH_MOUTH_SOLO,     WEIGHT_DEFAULT, VOICE_DEFAULT},
    {PH_FOOT_BLEND,     WEIGHT_DEFAULT, VOICE_DEFAULT},
    {PH_FOOT_SOLO,      WEIGHT_DEFAULT, VOICE_DEFAULT},
    {PH_GOOSE_BLEND,    WEIGHT_DEFAULT, VOICE_DEFAULT},
    {PH_GOOSE_SOLO,     WEIGHT_DEFAULT, VOICE_DEFAULT},
    {PH_STRUT_BLEND,    WEIGHT_DEFAULT, VOICE_DEFAULT},
    {PH_STRUT_SOLO,     WEIGHT_DEFAULT, VOICE_DEFAULT},
    {PH_CURE_BLEND,     WEIGHT_DEFAULT, VOICE_DEFAULT},
    {PH_CURE_SOLO,      WEIGHT_DEFAULT, VOICE_DEFAULT},
    {PH_NURSE_BLEND,    WEIGHT_DEFAULT, VOICE_DEFAULT},
    {PH_NURSE_SOLO,     WEIGHT_DEFAULT, VOICE_DEFAULT}
};

static const struct TextBlipAudioClip sTextPrinter1AudioClips[] = {
    {SE_MUD_BALL,  WEIGHT_DEFAULT, VOICE_DEFAULT}
};

static const struct TextBlipAudioClip sTextPrinter2AudioClips[] = {
    {SE_BIKE_HOP,  WEIGHT_DEFAULT, VOICE_DEFAULT}
};

static const struct TextBlipAudioClip sTextPrinter3AudioClips[] = {
    {SE_BALL,  WEIGHT_DEFAULT, VOICE_DEFAULT}
};

static const struct TextBlipAudioClip sTextPrinter4AudioClips[] = {
    {SE_SWITCH,  WEIGHT_DEFAULT, VOICE_DEFAULT}
};

static const struct TextBlipAudioClip sTextPrinter5AudioClips[] = {
    {SE_BALL_TRAY_EXIT,  WEIGHT_DEFAULT, VOICE_DEFAULT}
};

static const struct TextBlipAudioClip sTextPrinter6AudioClips[] = {
    {SE_CONTEST_ICON_CHANGE,  WEIGHT_DEFAULT, VOICE_DEFAULT}
};

static const struct TextBlipAudioClip sSpookyAudioClips[] = {
    {SE_BALL_BOUNCE_2,  WEIGHT_DEFAULT, VOICE_DEFAULT}
};

static const struct TextBlipAudioClip sRobo1Clips[] = {
    {SE_TEXT_BLIP_TEMPLATE,  WEIGHT_DEFAULT, VOICE_DEFAULT}
};

static const struct TextBlipAudioClip sRobo2Clips[] = {
    {SE_TEXT_BLIP_TEMPLATE,  WEIGHT_DEFAULT, 1}
};

static const struct TextBlipAudioClip sRobo3Clips[] = {
    {SE_TEXT_BLIP_TEMPLATE,  WEIGHT_DEFAULT, 2}
};

static const struct TextBlipAudioClip sRobo4Clips[] = {
    {SE_TEXT_BLIP_TEMPLATE,  WEIGHT_DEFAULT, 3}
};

static const struct TextBlipAudioClip sVoice1Clips[] = {
    {PH_THOUGHT_BLEND,  WEIGHT_DEFAULT, VOICE_DEFAULT}
};

static const struct TextBlipAudioClip sVoice2Clips[] = {
    {PH_FLEECE_BLEND,  WEIGHT_DEFAULT, VOICE_DEFAULT}
};

static const struct TextBlipAudioClip sVoice3Clips[] = {
    {PH_TRAP_BLEND,  WEIGHT_DEFAULT, VOICE_DEFAULT}
};

static const struct TextBlipAudioClip sVoice4Clips[] = {
    {PH_PRICE_SOLO,  WEIGHT_DEFAULT, VOICE_DEFAULT}
};

static const struct TextBlipAudioClip sVoice5Clips[] = {
    {PH_MOUTH_BLEND,  WEIGHT_DEFAULT, VOICE_DEFAULT}
};

static const struct TextBlipAudioClip sVoice6Clips[] = {
    {PH_FACE_BLEND,  WEIGHT_DEFAULT, VOICE_DEFAULT}
};

static const struct TextBlipAudioValues sTextBlipAudioValues[] = {
    [PHENOME_HIGH_5] =
    {
        .clips = sPhenomeAudioClips,
        .numClips = ARRAY_COUNT(sPhenomeAudioClips),
        .equalWeights = TRUE,
        .tempoAdjust = TEMPO_PHENOMES,
        .volume = VOLUME_DEFAULT,
        .volumeRange = VOLUME_VARIANCE_NONE,
        .pitchShift = PITCH_SHIFT_QUARTER_OCTAVE_UP * 5,
        .pitchRange = PITCH_VARIANCE_DEFAULT,
        .frequency = FREQUENCY_DEFAULT,
        .validChars = SPOKEN_CHARACTERS,
    },
    [PHENOME_HIGH_4] =
    {
        .clips = sPhenomeAudioClips,
        .numClips = ARRAY_COUNT(sPhenomeAudioClips),
        .equalWeights = TRUE,
        .tempoAdjust = TEMPO_PHENOMES,
        .volume = VOLUME_DEFAULT,
        .volumeRange = VOLUME_VARIANCE_NONE,
        .pitchShift = PITCH_SHIFT_QUARTER_OCTAVE_UP * 4,
        .pitchRange = PITCH_VARIANCE_DEFAULT,
        .frequency = FREQUENCY_DEFAULT,
        .validChars = SPOKEN_CHARACTERS,
    },
    [PHENOME_HIGH_3] =
    {
        .clips = sPhenomeAudioClips,
        .numClips = ARRAY_COUNT(sPhenomeAudioClips),
        .equalWeights = TRUE,
        .tempoAdjust = TEMPO_PHENOMES,
        .volume = VOLUME_DEFAULT,
        .volumeRange = VOLUME_VARIANCE_NONE,
        .pitchShift = PITCH_SHIFT_QUARTER_OCTAVE_UP * 3,
        .pitchRange = PITCH_VARIANCE_DEFAULT,
        .frequency = FREQUENCY_DEFAULT,
        .validChars = SPOKEN_CHARACTERS,
    },
    [PHENOME_HIGH_2] =
    {
        .clips = sPhenomeAudioClips,
        .numClips = ARRAY_COUNT(sPhenomeAudioClips),
        .equalWeights = TRUE,
        .tempoAdjust = TEMPO_PHENOMES,
        .volume = VOLUME_DEFAULT,
        .volumeRange = VOLUME_VARIANCE_NONE,
        .pitchShift = PITCH_SHIFT_QUARTER_OCTAVE_UP * 2,
        .pitchRange = PITCH_VARIANCE_DEFAULT,
        .frequency = FREQUENCY_DEFAULT,
        .validChars = SPOKEN_CHARACTERS,
    },
    [PHENOME_HIGH_1] =
    {
        .clips = sPhenomeAudioClips,
        .numClips = ARRAY_COUNT(sPhenomeAudioClips),
        .equalWeights = TRUE,
        .tempoAdjust = TEMPO_PHENOMES,
        .volume = VOLUME_DEFAULT,
        .volumeRange = VOLUME_VARIANCE_NONE,
        .pitchShift = PITCH_SHIFT_QUARTER_OCTAVE_UP,
        .pitchRange = PITCH_VARIANCE_DEFAULT,
        .frequency = FREQUENCY_DEFAULT,
        .validChars = SPOKEN_CHARACTERS,
    },
    [PHENOME_DEFAULT] =
    {
        .clips = sPhenomeAudioClips,
        .numClips = ARRAY_COUNT(sPhenomeAudioClips),
        .equalWeights = TRUE,
        .tempoAdjust = TEMPO_PHENOMES,
        .volume = VOLUME_DEFAULT,
        .volumeRange = VOLUME_VARIANCE_NONE,
        .pitchShift = PITCH_SHIFT_DEFAULT,
        .pitchRange = PITCH_VARIANCE_DEFAULT,
        .frequency = FREQUENCY_DEFAULT,
        .validChars = SPOKEN_CHARACTERS,
    },
    [PHENOME_LOW_1] =
    {
        .clips = sPhenomeAudioClips,
        .numClips = ARRAY_COUNT(sPhenomeAudioClips),
        .equalWeights = TRUE,
        .tempoAdjust = TEMPO_PHENOMES,
        .volume = VOLUME_DEFAULT,
        .volumeRange = VOLUME_VARIANCE_NONE,
        .pitchShift = PITCH_SHIFT_QUARTER_OCTAVE_DOWN,
        .pitchRange = PITCH_VARIANCE_DEFAULT,
        .frequency = FREQUENCY_DEFAULT,
        .validChars = SPOKEN_CHARACTERS,
    },
    [PHENOME_LOW_2] =
    {
        .clips = sPhenomeAudioClips,
        .numClips = ARRAY_COUNT(sPhenomeAudioClips),
        .equalWeights = TRUE,
        .tempoAdjust = TEMPO_PHENOMES,
        .volume = VOLUME_DEFAULT,
        .volumeRange = VOLUME_VARIANCE_NONE,
        .pitchShift = PITCH_SHIFT_QUARTER_OCTAVE_DOWN * 2,
        .pitchRange = PITCH_VARIANCE_DEFAULT,
        .frequency = FREQUENCY_DEFAULT,
        .validChars = SPOKEN_CHARACTERS,
    },
    [PHENOME_LOW_3] =
    {
        .clips = sPhenomeAudioClips,
        .numClips = ARRAY_COUNT(sPhenomeAudioClips),
        .equalWeights = TRUE,
        .tempoAdjust = TEMPO_PHENOMES,
        .volume = VOLUME_DEFAULT,
        .volumeRange = VOLUME_VARIANCE_NONE,
        .pitchShift = PITCH_SHIFT_QUARTER_OCTAVE_DOWN * 3,
        .pitchRange = PITCH_VARIANCE_DEFAULT,
        .frequency = FREQUENCY_DEFAULT,
        .validChars = SPOKEN_CHARACTERS,
    },
    [PHENOME_LOW_4] =
    {
        .clips = sPhenomeAudioClips,
        .numClips = ARRAY_COUNT(sPhenomeAudioClips),
        .equalWeights = TRUE,
        .tempoAdjust = TEMPO_PHENOMES,
        .volume = VOLUME_DEFAULT,
        .volumeRange = VOLUME_VARIANCE_NONE,
        .pitchShift = PITCH_SHIFT_QUARTER_OCTAVE_DOWN * 4,
        .pitchRange = PITCH_VARIANCE_DEFAULT,
        .frequency = FREQUENCY_DEFAULT,
        .validChars = SPOKEN_CHARACTERS,
    },
    [TEXT_PRINTER_1] =
    {
        .clips = sTextPrinter1AudioClips,
        .numClips = ARRAY_COUNT(sTextPrinter1AudioClips),
        .equalWeights = TRUE,
        .tempoAdjust = TEMPO_DEFAULT,
        .volume = VOLUME_DEFAULT,
        .volumeRange = VOLUME_VARIANCE_DEFAULT,
        .pitchShift = PITCH_SHIFT_DEFAULT,
        .pitchRange = PITCH_VARIANCE_NONE,
        .frequency = FREQUENCY_DEFAULT,
        .validChars = PRINTED_CHARACTERS,
    },
    [TEXT_PRINTER_2] =
    {
        .clips = sTextPrinter2AudioClips,
        .numClips = ARRAY_COUNT(sTextPrinter2AudioClips),
        .equalWeights = TRUE,
        .tempoAdjust = TEMPO_DEFAULT,
        .volume = 70,
        .volumeRange = VOLUME_VARIANCE_DEFAULT,
        .pitchShift = PITCH_SHIFT_DEFAULT,
        .pitchRange = PITCH_VARIANCE_NONE,
        .frequency = FREQUENCY_DEFAULT,
        .validChars = PRINTED_CHARACTERS,
    },
    [TEXT_PRINTER_3] =
    {
        .clips = sTextPrinter3AudioClips,
        .numClips = ARRAY_COUNT(sTextPrinter3AudioClips),
        .equalWeights = TRUE,
        .tempoAdjust = TEMPO_DEFAULT,
        .volume = 50,
        .volumeRange = VOLUME_VARIANCE_DEFAULT,
        .pitchShift = PITCH_SHIFT_DEFAULT,
        .pitchRange = PITCH_VARIANCE_NONE,
        .frequency = FREQUENCY_DEFAULT,
        .validChars = PRINTED_CHARACTERS,
    },
    [TEXT_PRINTER_4] =
    {
        .clips = sTextPrinter4AudioClips,
        .numClips = ARRAY_COUNT(sTextPrinter4AudioClips),
        .equalWeights = TRUE,
        .tempoAdjust = TEMPO_DEFAULT,
        .volume = 80,
        .volumeRange = VOLUME_VARIANCE_DEFAULT,
        .pitchShift = PITCH_SHIFT_DEFAULT,
        .pitchRange = PITCH_VARIANCE_NONE,
        .frequency = FREQUENCY_DEFAULT,
        .validChars = PRINTED_CHARACTERS,
    },
    [TEXT_PRINTER_5] =
    {
        .clips = sTextPrinter5AudioClips,
        .numClips = ARRAY_COUNT(sTextPrinter5AudioClips),
        .equalWeights = TRUE,
        .tempoAdjust = TEMPO_DEFAULT,
        .volume = 127,
        .volumeRange = VOLUME_VARIANCE_DEFAULT,
        .pitchShift = PITCH_SHIFT_DEFAULT,
        .pitchRange = PITCH_VARIANCE_NONE,
        .frequency = FREQUENCY_DEFAULT,
        .validChars = PRINTED_CHARACTERS,
    },
    [TEXT_PRINTER_6] =
    {
        .clips = sTextPrinter6AudioClips,
        .numClips = ARRAY_COUNT(sTextPrinter6AudioClips),
        .equalWeights = TRUE,
        .tempoAdjust = TEMPO_DEFAULT,
        .volume = VOLUME_DEFAULT,
        .volumeRange = VOLUME_VARIANCE_DEFAULT,
        .pitchShift = PITCH_SHIFT_DEFAULT,
        .pitchRange = PITCH_VARIANCE_NONE,
        .frequency = FREQUENCY_DEFAULT,
        .validChars = PRINTED_CHARACTERS,
    },
    [SPOOKY] =
    {
        .clips = sSpookyAudioClips,
        .numClips = ARRAY_COUNT(sSpookyAudioClips),
        .equalWeights = TRUE,
        .tempoAdjust = TEMPO_DEFAULT,
        .volume = VOLUME_DEFAULT,
        .volumeRange = VOLUME_VARIANCE_DEFAULT,
        .pitchShift = PITCH_SHIFT_DEFAULT,
        .pitchRange = PITCH_VARIANCE_DEFAULT,
        .frequency = FREQUENCY_DEFAULT,
        .validChars = PRINTED_CHARACTERS,
    },
    [ROBO_1] =
    {
        .clips = sRobo1Clips,
        .numClips = ARRAY_COUNT(sRobo1Clips),
        .equalWeights = TRUE,
        .tempoAdjust = TEMPO_DEFAULT,
        .volume = VOLUME_DEFAULT,
        .volumeRange = VOLUME_VARIANCE_DEFAULT,
        .pitchShift = PITCH_SHIFT_DEFAULT,
        .pitchRange = PITCH_VARIANCE_DEFAULT,
        .frequency = FREQUENCY_DEFAULT,
        .validChars = PRINTED_CHARACTERS,
    },
    [ROBO_2] =
    {
        .clips = sRobo2Clips,
        .numClips = ARRAY_COUNT(sRobo2Clips),
        .equalWeights = TRUE,
        .tempoAdjust = TEMPO_DEFAULT,
        .volume = VOLUME_DEFAULT,
        .volumeRange = VOLUME_VARIANCE_DEFAULT,
        .pitchShift = PITCH_SHIFT_DEFAULT,
        .pitchRange = PITCH_VARIANCE_NONE,
        .frequency = FREQUENCY_DEFAULT,
        .validChars = PRINTED_CHARACTERS,
    },
    [ROBO_3] =
    {
        .clips = sRobo3Clips,
        .numClips = ARRAY_COUNT(sRobo3Clips),
        .equalWeights = TRUE,
        .tempoAdjust = TEMPO_DEFAULT,
        .volume = VOLUME_DEFAULT,
        .volumeRange = VOLUME_VARIANCE_DEFAULT,
        .pitchShift = PITCH_SHIFT_DEFAULT,
        .pitchRange = PITCH_VARIANCE_NONE,
        .frequency = FREQUENCY_DEFAULT,
        .validChars = PRINTED_CHARACTERS,
    },
    [ROBO_4] =
    {
        .clips = sRobo4Clips,
        .numClips = ARRAY_COUNT(sRobo4Clips),
        .equalWeights = TRUE,
        .tempoAdjust = TEMPO_DEFAULT,
        .volume = VOLUME_DEFAULT,
        .volumeRange = VOLUME_VARIANCE_DEFAULT,
        .pitchShift = PITCH_SHIFT_DEFAULT,
        .pitchRange = PITCH_VARIANCE_NONE,
        .frequency = FREQUENCY_DEFAULT,
        .validChars = PRINTED_CHARACTERS,
    },
    [VOICE_1] =
    {
        .clips = sVoice1Clips,
        .numClips = ARRAY_COUNT(sVoice1Clips),
        .equalWeights = TRUE,
        .tempoAdjust = TEMPO_PHENOMES,
        .volume = VOLUME_DEFAULT,
        .volumeRange = VOLUME_DEFAULT,
        .pitchShift = PITCH_SHIFT_DEFAULT,
        .pitchRange = PITCH_VARIANCE_DEFAULT,
        .frequency = FREQUENCY_DEFAULT,
        .validChars = SPOKEN_CHARACTERS,
    },
    [VOICE_2] =
    {
        .clips = sVoice2Clips,
        .numClips = ARRAY_COUNT(sVoice2Clips),
        .equalWeights = TRUE,
        .tempoAdjust = TEMPO_PHENOMES,
        .volume = VOLUME_DEFAULT,
        .volumeRange = VOLUME_DEFAULT,
        .pitchShift = PITCH_SHIFT_DEFAULT,
        .pitchRange = PITCH_VARIANCE_DEFAULT,
        .frequency = FREQUENCY_DEFAULT,
        .validChars = SPOKEN_CHARACTERS,
    },
    [VOICE_3] =
    {
        .clips = sVoice3Clips,
        .numClips = ARRAY_COUNT(sVoice3Clips),
        .equalWeights = TRUE,
        .tempoAdjust = TEMPO_PHENOMES,
        .volume = VOLUME_DEFAULT,
        .volumeRange = VOLUME_DEFAULT,
        .pitchShift = PITCH_SHIFT_DEFAULT,
        .pitchRange = PITCH_VARIANCE_DEFAULT,
        .frequency = FREQUENCY_DEFAULT,
        .validChars = SPOKEN_CHARACTERS,
    },
    [VOICE_4] =
    {
        .clips = sVoice4Clips,
        .numClips = ARRAY_COUNT(sVoice4Clips),
        .equalWeights = TRUE,
        .tempoAdjust = TEMPO_PHENOMES,
        .volume = VOLUME_DEFAULT,
        .volumeRange = VOLUME_DEFAULT,
        .pitchShift = PITCH_SHIFT_DEFAULT,
        .pitchRange = PITCH_VARIANCE_DEFAULT,
        .frequency = FREQUENCY_DEFAULT,
        .validChars = SPOKEN_CHARACTERS,
    },
    [VOICE_5] =
    {
        .clips = sVoice5Clips,
        .numClips = ARRAY_COUNT(sVoice5Clips),
        .equalWeights = TRUE,
        .tempoAdjust = TEMPO_PHENOMES,
        .volume = VOLUME_DEFAULT,
        .volumeRange = VOLUME_DEFAULT,
        .pitchShift = PITCH_SHIFT_DEFAULT,
        .pitchRange = PITCH_VARIANCE_DEFAULT,
        .frequency = FREQUENCY_DEFAULT,
        .validChars = SPOKEN_CHARACTERS,
    },
    [VOICE_6] =
    {
        .clips = sVoice6Clips,
        .numClips = ARRAY_COUNT(sVoice6Clips),
        .equalWeights = TRUE,
        .tempoAdjust = TEMPO_PHENOMES,
        .volume = VOLUME_DEFAULT,
        .volumeRange = VOLUME_DEFAULT,
        .pitchShift = PITCH_SHIFT_DEFAULT,
        .pitchRange = PITCH_VARIANCE_DEFAULT,
        .frequency = FREQUENCY_DEFAULT,
        .validChars = SPOKEN_CHARACTERS,
    },
};

void ResetDefaultValuesLastClip(void)
{
    const struct MusicPlayer *mplayTable = gMPlayTable;
    const struct Song *songTable = gSongTable;
    const struct Song *song = &songTable[sLastClip];
    const struct MusicPlayer *mplay = &mplayTable[song->ms];

    gTextBlipMusicPlayerPtr = mplay->info;

    if (sBaseVolume != 0)
    {
        gTextBlipMusicPlayerPtr->tracks[0].vol = sBaseVolume;
        sBaseVolume = 0;
    }

    for (u8 i = 0; i < gTextBlipMusicPlayerPtr->trackCount; i++)
        gTextBlipMusicPlayerPtr->tracks[i].keyShift = sBasePitchShift;
    sBasePitchShift = 0;
    
    sLastClip = 0;
    gTextBlipVolume = 0;
    gTextBlipPitchShift = 0;
    gTextBlipVoice = 0;
}

void ResetTextBlips(void)
{
    sCharsUntilNextTextBlip = 1;
    gTextBlipActive = FALSE;
    gTextBlipSetActive = FALSE;
    sTextBlipDataPtr = NULL;
    gTextBlipMusicPlayerPtr = NULL;

    if (sLastClip != 0)
        ResetDefaultValuesLastClip();
    else
    {
        gTextBlipVolume = 0;
        gTextBlipPitchShift = 0;
        gTextBlipVoice = 0;
    }
}

void InitTextBlip(u16 textBlipId)
{
    ResetTextBlips();
    gTextBlipActive = TRUE;
    gTextBlipSetActive = FALSE;
    sTextBlipDataPtr = &sTextBlipAudioValues[textBlipId];
    sCharsUntilNextTextBlip = 1;
}

bool8 IsValidTextBlipChar(u16 currentChar, u8 charSettings)
{
    switch (charSettings)
    {
    case SPOKEN_CHARACTERS:
        if ((currentChar > CHAR_SPACE && currentChar < CHAR_MASCULINE_ORDINAL)  // special vowels
         || (currentChar > CHAR_SUPER_RE && currentChar < CHAR_EXCL_MARK)       // numbers
         || (currentChar > CHAR_SLASH && currentChar < CHAR_BLACK_TRIANGLE)     // letters
         || (currentChar > CHAR_COLON && currentChar < CHAR_DYNAMIC))           // more special vowels
            return TRUE;
        break;
    case SPOKEN_CHARACTERS_NO_NUMBERS:
        if ((currentChar > CHAR_SPACE && currentChar < CHAR_MASCULINE_ORDINAL)
         || (currentChar > CHAR_SLASH && currentChar < CHAR_BLACK_TRIANGLE)
         || (currentChar > CHAR_COLON && currentChar < CHAR_DYNAMIC))
            return TRUE;
        break;
    case PRINTED_CHARACTERS:
        if (currentChar > CHAR_SPACE)
            return TRUE;
        break;
    default:
        break;
    }

    if (ALWAYS_ADVANCE_TEXT_BLIPS && (sCharsUntilNextTextBlip > 1))
        sCharsUntilNextTextBlip--;
    
    return FALSE;
}

u32 GetTextBlipFrequencyAdjustment(void)
{
    return sTextBlipFrequencyAdjustment[GetPlayerTextSpeed()];
}

u16 GetHashcode(u16 currChar)
{
    if (EQUATE_LETTER_CASE && (currChar > CHAR_Z && currChar < CHAR_BLACK_TRIANGLE))
        return currChar - (CHAR_a - CHAR_A);
    else
        return currChar;
}

void TryPlayTextBlip(u16 currChar, bool8 hasPrintBeenSpedUp)
{
    u8 validChars = sTextBlipDataPtr->validChars;
    
    if (IsValidTextBlipChar(currChar, validChars))
    {
        sCharsUntilNextTextBlip--;
        if (sCharsUntilNextTextBlip == 0)
        {   
            u8 i, clipIndex;
            u16 clip;
            s8 voice;
            u8 numClips = sTextBlipDataPtr->numClips;
            u16 tempoAdjust = sTextBlipDataPtr->tempoAdjust;
            u8 volume = sTextBlipDataPtr->volume;
            u8 volumeRange = sTextBlipDataPtr->volumeRange;
            s8 pitchShift = sTextBlipDataPtr->pitchShift;
            u8 pitchRange = sTextBlipDataPtr->pitchRange;
            u8 frequency = sTextBlipDataPtr->frequency;

            if (sLastClip != 0)
                ResetDefaultValuesLastClip();

            if (MAKE_TEXT_BLIPS_PREDICTABLE)
            {
                clipIndex = GetHashcode(currChar) % numClips;
            }
            else
            {
                if (sTextBlipDataPtr->equalWeights)
                    clipIndex = Random() % numClips;
                else
                {
                    u8 weights[numClips];

                    for (i = 0; i < numClips; i++)
                        weights[i] = sTextBlipDataPtr->clips[i].weight;

                    clipIndex = RandomWeightedIndex(weights, numClips);
                }
            }

            clip = sTextBlipDataPtr->clips[clipIndex].songNum;
            sLastClip = clip;
            voice = sTextBlipDataPtr->clips[clipIndex].voice;

            const struct MusicPlayer *mplayTable = gMPlayTable;
            const struct Song *songTable = gSongTable;
            const struct Song *song = &songTable[clip];
            const struct MusicPlayer *mplay = &mplayTable[song->ms];

            gTextBlipMusicPlayerPtr = mplay->info;

            // Preset volume, pitchShift, and voice before first note plays.
            // These values replace the default song values in m4a_1.s.
            if (volume != VOLUME_DEFAULT || volumeRange != VOLUME_VARIANCE_NONE)
            {
                sBaseVolume = gTextBlipMusicPlayerPtr->tracks[0].vol;
                
                if (volume == VOLUME_DEFAULT)
                    volume = sBaseVolume;
                
                s16 maxVol = volume * (100 + volumeRange) / 100;
                s16 minVol = volume * (100 - volumeRange) / 100;

                if (maxVol > 127)
                    maxVol = 127;
                if (minVol < 1)
                    minVol = 1;

                if (MAKE_TEXT_BLIPS_PREDICTABLE)
                {
                    if (MAX_VARIANCE_SEGMENTS < (maxVol - minVol))
                        gTextBlipVolume = minVol + (((GetHashcode(currChar) % (maxVol - minVol + 1)) % MAX_VARIANCE_SEGMENTS) * ((maxVol - minVol)/(MAX_VARIANCE_SEGMENTS - 1)));
                    else
                        gTextBlipVolume = minVol + (GetHashcode(currChar) % (maxVol - minVol + 1));
                }
                else
                    gTextBlipVolume = RandomUniform(RNG_TEXT_BLIP_SETTINGS, minVol, maxVol + 1);
            }

            if (pitchShift != PITCH_SHIFT_DEFAULT || pitchRange != PITCH_VARIANCE_NONE)
            {
                sBasePitchShift = gTextBlipMusicPlayerPtr->tracks[0].keyShift;
                
                if (pitchShift == PITCH_SHIFT_DEFAULT)
                    pitchShift = sBasePitchShift;

                if (MAKE_TEXT_BLIPS_PREDICTABLE)
                {
                    if (MAX_VARIANCE_SEGMENTS < pitchRange)
                        gTextBlipPitchShift = pitchShift + (((GetHashcode(currChar) % ((2 * pitchRange) + 1)) % MAX_VARIANCE_SEGMENTS) * (pitchShift/(MAX_VARIANCE_SEGMENTS - 1))) - pitchRange;
                    else
                        gTextBlipPitchShift = pitchShift + (GetHashcode(currChar) % ((2 * pitchRange) + 1)) - pitchRange;
                }
                else
                    gTextBlipPitchShift = pitchShift + (Random() % ((2 * pitchRange) + 1)) - pitchRange;
            }

            if (voice > VOICE_DEFAULT)
                gTextBlipVoice = (voice * 12) + 1;

            PlaySE(clip);

            // Tempo can be dynamically adjusted while a note is playing.
            if (tempoAdjust != TEMPO_DEFAULT)
            {
                m4aMPlayTempoControl(mplay->info, tempoAdjust);
            }

            // Reset sCharsUntilNextTextBlip
            if (frequency == FREQUENCY_EVERY_CHARACTER)
                sCharsUntilNextTextBlip = 1;
            else if (JOY_HELD(A_BUTTON | B_BUTTON) && hasPrintBeenSpedUp)
                sCharsUntilNextTextBlip = sTextBlipDataPtr->frequency * sTextBlipFrequencyAdjustment[OPTIONS_TEXT_SPEED_INSTANT];
            else
                sCharsUntilNextTextBlip = sTextBlipDataPtr->frequency * GetTextBlipFrequencyAdjustment();
        }
    }
}

void ResetTextBlipChars(void)
{
    sCharsUntilNextTextBlip = 1;
}
