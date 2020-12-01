#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"
#include "music_sheet.h"

// Twinkle Star
const char* DEFAULT_MUSIC_SHEET = "c4 c4 g4 g4 a4 a4 a4 a4 g4 f4 f4 e4 e4 d4 d4 c4\n"
                                  "g4 g4 g4 f4 f4 e4 e4 e4 d4 c4 g4 g4 g4 f4 f4 f4\n"
                                  "f4 e4 e4 e4 d4";

void LoadMusicSheetFromString(MusicSheet* sheet, const char* str)
{
    char* buffer = strdup(str);
    char* token = strtok(buffer, " \t\n\r");
    list_init(&sheet->notes);
    sheet->current_node = NULL;

    while (token != NULL) {
        // store the music note as all lower case for good.
        for (char* p = token; *p != '\0'; ++p) {
            *p = tolower(*p);
        }

        list_append_tail(&sheet->notes, token, strlen(token) + 1);
        token = strtok(NULL, " \t\n\r");
    }

    free(buffer);
}

// load the music sheet from file 'filename' to 'sheet'
void LoadMusicSheetFromFile(MusicSheet* sheet, const char* filename)
{
    FILE* file = fopen(filename, "r");
    char* buffer;
    long file_size;

    if (file == NULL) {
        perror("load_music_sheet error");
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    rewind(file);
    buffer = malloc(file_size + 1);
    fread(buffer, sizeof(char), file_size, file);
    buffer[file_size] = '\0';
    fclose(file);

    LoadMusicSheetFromString(sheet, buffer);
}

void MusicSheetClear(MusicSheet* sheet)
{
    list_clear(&sheet->notes, free);
}

bool MusicSheetEnded(MusicSheet* sheet)
{
    return sheet->current_node == NULL;
}

// return to the first music note
void MusicSheetRewind(MusicSheet* sheet)
{
    sheet->current_node = sheet->notes.head;
}

void GetDefaultMusicSheet(MusicSheet* sheet)
{
    LoadMusicSheetFromString(sheet, DEFAULT_MUSIC_SHEET);
}
