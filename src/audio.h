#ifndef __H_AUDIO
#define __H_AUDIO

#include "music_sheet.h"
#include <raylib.h>

Sound LoadMusicNoteSound(const char* note);
void PlayMusicNote(const char* note);
void PlayMusicSheet(MusicSheet* sheet);

#endif
