#ifndef GUARD_TEXT_BLIPS_H
#define GUARD_TEXT_BLIPS_H

extern bool8 gTextBlipActive;
extern bool8 gTextBlipSetActive;
extern u8 gTextBlipVolume;
extern s8 gTextBlipPitchShift;
extern s16 gTextBlipVoice;
extern struct MusicPlayerInfo *gTextBlipMusicPlayerPtr;

void InitTextBlip(u16 textBlipId);
bool8 IsValidTextBlipChar(u16 currentChar, u8 charSettings);
u32 GetTextBlipFrequencyAdjustment(void);
void ResetDefaultValuesLastClip(void);
void TryPlayTextBlip(u16 currChar, bool8 hasPrintBeenSpedUp);
void ResetTextBlipChars(void);
void ResetTextBlips(void);

#endif // GUARD_TEXT_BLIPS_H
