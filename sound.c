#include "sound.h"

#include <stdbool.h>
#include "raylib.h"

static const char* filenames[6] = { "audio/do.wav", "audio/re.wav", "audio/mi.wav", "audio/fa.wav", "audio/so.wav", "audio/la.wav" };

void PlayPianoNote(PianoNote note)
{
    static Sound sounds[6];
    static bool loaded[6] = { 0 };

    if (!IsAudioDeviceReady()) return;
    if (!loaded[note])
    {
       sounds[note] = LoadSound(filenames[note]);
       loaded[note] = true;
    }

    PlaySound(sounds[note]);
}
