# Features

This feature branch implements text blips, or sounds that play when text is printed. The following features are included:

1) Adds 27 text blip configurations that can be used directly, edited by changing their volume, pitch, and playback speed, or studied as examples for creating custom text blips.

2) Adds a template text blip sound effect that can be easily edited to create multiple custom text blips using any sound or wave samples and minimizes space usage when adding new text blips.

3) Includes configs that allow the user to control whether the same printed text will always produce the same text blip sounds, set a default text blip configuration for text printed in message boxes and/or during battles, and more!

# Usage

## Included text blips

Of the 27 included text blips, 10 use the complete library of Bard phenomes to create text blips : `PHENOME_DEFAULT`, `PHENOME_HIGH_1`, `PHENOME_HIGH_2`, `PHENOME_HIGH_3`, `PHENOME_HIGH_4`, `PHENOME_HIGH_5`, `PHENOME_LOW_1`, `PHENOME_LOW_2`, `PHENOME_LOW_3`, and `PHENOME_LOW_4`. `PHENOME_DEFAULT` plays the phenomes at their default pitch, while the other options play at progressively higher or lower pitches.

There are also 6 text blips that use a single Bard phenome as the base of the text blip sounds: `VOICE_1`, `VOICE_2`, `VOICE_3`, `VOICE_4`, `VOICE_5`, and `VOICE_6`.

Another 6 text blips use sound effects that can serve as a general-purpose text blip or when printing text that is not spoken by an NPC: `TEXT_PRINTER_1`, `TEXT_PRINTER_2`, `TEXT_PRINTER_3`, `TEXT_PRINTER_4`, `TEXT_PRINTER_5`, and `TEXT_PRINTER_6`.

A group of 4 text blips produce beeps using wave samples: `ROBO_1`, `ROBO_2`, `ROBO_3`, and `ROBO_4`.

And the final included text blip uses a sound effect to create a spooky text sound: `SPOOKY`.

## Syntax

To have a string play with a text blip sound, simply add `{SET_TEXT_BLIP TEXT_BLIP_ID}` to the string where you want the text blip to start playing, where `TEXT_BLIP_ID` is one of the text blip IDs listed above in Included Text Blips (or your custom IDs).

For example, you could have text blips play while printing a string using:
```
    "{SET_TEXT_BLIP PHENOME_DEFAULT}Hello world!"
```
Or you could start the text blips at a specific point in the text using:
```
    "Hello {SET_TEXT_BLIP PHENOME_DEFAULT}world!"
```

To stop a text blip mid-string, add `{STOP_TEXT_BLIP}` where desired:
```
    "{SET_TEXT_BLIP PHENOME_DEFAULT}Hello {STOP_TEXT_BLIP}world!"
```

Text blips can be set or stopped at any point in a string. A new text blip can be set in the middle of the string without needing to stop a previous text blip:
```
    "{SET_TEXT_BLIP PHENOME_DEFAULT}He{STOP_TEXT_BLIP}llo {SET_TEXT_BLIP PHENOME_LOW_2}wor{SET_TEXT_BLIP VOICE_1}ld!"
```


If using the config `USE_DEFAULT_TEXT_BLIP` to have a specific default text blip play for all printed text, the text blip can be prevented from playing for a string by either manually stopping the text blip:
```
    "{STOP_TEXT_BLIP}Hello world!"
```
or by overriding the default by setting a different text blip:
```
    "{SET_TEXT_BLIP PHENOME_HIGH_3}Hello world!"
```

# Creating Custom Text Blips

## Defining text blip structures

Two structures must be defined in `src/text_blips.c` to create a custom text blip.

First, the audio clip(s) that will be used must be defined in a `TextBlipAudioClip` struct that has three members:
1) The song that will be used for the text blip (e.g. `SE_SELECT` or `PH_LOT_BLEND`)
2) The relative frequency at which this clip will be selected (only relevant if using more than one audio clip for a text blip). For most purposes, using `WEIGHT_DEFAULT` is the most appropriate option.
3) The voice number in the song's voicegroup that will be used. To use the voice already specified in the song, use `VOICE_DEFAULT`. Generally, you will only use a different value here when using the text blip sound effect template, as described below.

You can create an array of audio clips to have a text blip that randomly chooses from this selection of clips when playing a sound (for an example see [`sPhenomeAudioClips`](https://github.com/Jumpstart19/pokeemerald-expansion/blob/b66d6144b984af33038c33683af970767212f87a/src/text_blips.c#L54)).

Then, adjustments to these audio clips are defined in a `TextBlipAudioValues` struct with 10 members:
1) `clips`: The structure defined above (e.g. `sPhenomeAudioClips`).
2) `numClips`: The number of audio clips defined in `clips` (e.g. `ARRAY_COUNT(sPhenomeAudioClips)`).
3) `equalWeights`: Set to `TRUE` to override any weights set in `clips` and give all audio clips equal weighting.
4) `tempoAdjust`: Used to adjust the tempo of the audio clip(s) so sound is played for only 0.1-0.2 seconds, the ideal duration of a text blip sound. Set as `TEMPO_DEFAULT` to use the unmodified tempo or specify a value to scale the tempo by (1/256) of that value.
5) `volume`: Sets the base volume of the audio clip(s) using an integer between 1 and 127. Set as `VOLUME_DEFAULT` to use the unomidifed volume of the audio clip(s).
6) `volumeRange`: Sets the percent variance in volume from the base value for the audio clip(s) (e.g. using a value of 10 plays the audio clip(s) at volumes within +/- 10% of the base volume). For most applications, use `VOLUME_VARIANCE_DEFAULT` for 10% variance or `VOLUME_VARIANCE_NONE` for no variance.
7) `pitchShift`: Shifts the base pitches in the audio clip(s) by a specified number of semitones (e.g. a value of -12 plays the audio clip(s) with a pitch one octave lower than the notes' default pitches). To use the unmodified pitches of the audio clip(s) use `PITCH_SHIFT_NONE`. Otherwise, it is recommended to use multiples of either `PITCH_SHIFT_QUARTER_OCTAVE_UP` or `PITCH_SHIFT_QUARTER_OCTAVE_DOWN`.
8) `pitchRange`: Sets the variance in pitch in semitones from the base value for the audio clip(s) (e.g. using a value of 2 plays the audio clip(s) at pitches within a whole step above or below the base pitch). For most applications, use `PITCH_VARIANCE_DEFAULT` to play pitches within one semitone of the base pitch or `PITCH_VARIANCE_NONE` for no variance.
9) `frequency`: The base number of characters printed until the next text blip plays. This frequency is automatically scaled by the text speed. For almost all applications, it is recommended to use `FREQUENCY_DEFAULT`.
10) `validChars`: The set of printed characters that can play a text blip. Use `SPOKEN_CHARACTERS` to cover all letters and numbers, `SPOKEN_CHARACTERS_NO_NUMBERS` to only cover letters, or `PRINTED_CHARACTERS` to include all characters other than spaces.

This should be added to [`sTextBlipAudioValues`](https://github.com/Jumpstart19/pokeemerald-expansion/blob/b66d6144b984af33038c33683af970767212f87a/src/text_blips.c#L159) with its index set using a text blip ID as described below, such as in the following example:
```
    [PHENOME_HIGH_2] =
        {
            .clips = sPhenomeAudioClips,
            .numClips = ARRAY_COUNT(sPhenomeAudioClips),
            .equalWeights = TRUE,
            .tempoAdjust = TEMPO_DEFAULT,
            .volume = VOLUME_DEFAULT,
            .volumeRange = VOLUME_VARIANCE_NONE,
            .pitchShift = PITCH_SHIFT_QUARTER_OCTAVE_UP * 2,
            .pitchRange = PITCH_VARIANCE_DEFAULT,
            .frequency = FREQUENCY_DEFAULT,
            .validChars = SPOKEN_CHARACTERS,
        },
```
## Using the text blip sound effect template

The text blip clip in `SE_TEXT_BLIP_TEMPLATE` is set to be the optimal duration for a text blip (between 0.1 and 0.2 seconds), so no tempo adjustments are required. To create new text blips using this template, simply change the voices in the voice group `se_text_blip_template` to those of the voices you want to use in your text blips. Note that voices 0-3 are used for `ROBO_1`, `ROBO_2`, `ROBO_3`, and `ROBO_4`, so it is recommended to only adjust voices 4-127. Then, use these voices in your desired text blip structures as described above.

## Adding text blip IDs

Text blip IDs must be defined in both [`include/constants/text_blips.h`](https://github.com/Jumpstart19/pokeemerald-expansion/blob/f5b2fd48f4f8c57f3e9db74c1f63998f4e5ff473/include/constants/text_blips.h#L36) and [`charmap.txt`](https://github.com/Jumpstart19/pokeemerald-expansion/blob/f5b2fd48f4f8c57f3e9db74c1f63998f4e5ff473/charmap.txt#L471) to label a text blip in `sTextBlipAudioValues` in `src/text_blips.c` and set the text blip in strings using `{SET_TEXT_BLIP TEXT_BLIP_ID}`.

# Known Limitations

- When using multiple text blips and/or multiple audio clips in one string that use changed voices (voices other than `VOICE_DEFAULT`), there is a noticable delay when switching between the text blips. As such, it is recommended to only use multiple text blips or audio clips in a string when they all use the default voice. There is a small delay when switching between audio clips that do not use changed voices, but this is typically only noticeable if you are switching between these clips every character.
- Text blips that use the full set of Bard phenomes produce some audio popping noises.

# Questions/Bug Reports
Please ping @Jumpstart in the Team Aqua's Hideout discord server.
