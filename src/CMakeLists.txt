cmake_minimum_required(VERSION 3.11)

add_executable(music_tiles main.c list.c map.c music_sheet.c audio.c)
target_link_libraries(music_tiles raylib)
target_link_libraries(music_tiles m)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/release)
