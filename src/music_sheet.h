// A structure containing the orders of music notes in a music
#ifndef __H_MUSIC_SHEET
#define __H_MUSIC_SHEET

#include "list.h"
#include <stdbool.h>

typedef struct MusicSheet {
    List notes;
    ListNode* current_node;
    bool random; /* play random notes instead of specific sheet if set */
} MusicSheet;

void LoadMusicSheetFromString(MusicSheet* sheet, const char* filename);
void LoadMusicSheetFromFile(MusicSheet* sheet, const char* filename);
void MusicSheetFree(MusicSheet* sheet);
void MusicSheetRewind(MusicSheet* sheet);
bool MusicSheetEnded(MusicSheet* sheet);

void GetDefaultMusicSheet(MusicSheet* sheet);

#endif
