#include "audio.h"
#include "map.h"

#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char* DEFAULT_NOTES[] = { "do", "re", "mi", "fa", "so", "la", "si" };

Sound LoadMusicNoteSound(const char* note)
{
    char file_path[256];
    int i;

    // if note name is default note name, search in that folder
    for (i = 0; i < 7; ++i) {
        if (strcasecmp(note, DEFAULT_NOTES[i]) == 0) {
            sprintf(file_path, "audio/default/%s.wav", note);
            break;
        }
    }

    if (i == 7) {
        sprintf(file_path, "audio/%s.mp3", note);
    }

    Sound sound = LoadSound(file_path);
    return sound;
}

static int keycmp(const char* search_key, const char** map_key_ptr)
{
    return strcasecmp(search_key, *map_key_ptr);
}

// Play a sound of music note
void PlayMusicNote(const char* note)
{
    static Map sounds = {
        .list = { 0, 0, 0 },
        .key_size = sizeof(char*),
        .value_size = sizeof(Sound)
    };

    Sound* sound = (Sound*)map_find(&sounds, note, (int (*)(const void*, const void*))keycmp);
    if (sound != NULL) {
        PlaySound(*sound);
    } else {
        Sound sound = LoadMusicNoteSound(note);
        char* key = strdup(note);
        map_insert(&sounds, &key, &sound);
        PlaySound(sound);
    }
}

// Play the next music note of 'sheet' and advance the progress
void PlayMusicSheet(MusicSheet* sheet)
{

    if (sheet->random) {
        const char* note = DEFAULT_NOTES[random() % 7];
        PlayMusicNote(note);
    } else if (sheet->current_node != NULL) {
        PlayMusicNote((char*)sheet->current_node->data);
        sheet->current_node = sheet->current_node->prev;
    }
}
