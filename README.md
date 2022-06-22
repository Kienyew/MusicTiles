# Music Tiles
A simple Piano Tiles like desktop game created using [raylib](https://github.com/raysan5/raylib/). With 200 crappy musics supported (see in `sheet` folder).

![screenshot](https://user-images.githubusercontent.com/31496021/100824993-13622200-3492-11eb-897c-993f9f267194.gif)

# Build
1. Navigate to this project directory.
2. Build with cmake (it needs to download raylib source from web so it might take some time).
```sh
mkdir build
cd build
cmake ..
make
```
The result will be built to `build/release` folder.

# Run
Run the executable, the arguments passed to it will be the music sheets to repeat.
`./music_tiles [music_sheet_files...]`

example:
`./music_tiles sheet/baby-shark-pinkfong.txt sheet/let-it-go-frozen-disney.txt`

Make sure to stick the executable file in the same directory with the `audio` directory :)

# Credit
Thanks for [noobnotes.net](https://noobnotes.net/) for providing the music sheets.
