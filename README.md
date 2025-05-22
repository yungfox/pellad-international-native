# pellad-international-native

Graphics concept using my band's alternate logo with latin, cyrillic and japanese script. I used it as an excuse to learn about how to handle with Unicode strings at a lower level, and how to render text with OpenGL by leveraging FreeType 2.<br>
Runs on macOS, Linux and Windows thanks to [RGFW](https://github.com/ColleagueRiley/RGFW) and a custom OpenGL extension loader.<br>
It's a native port of [pellad-international-web](https://github.com/yungfox/pellad-international-web), although arabic script is not yet supported.

## Dependencies

- OpenGL 3.3
- FreeType 2.11+
- CMake 3.8+

## Quick start

```console
$ mkdir build
$ cd build
$ cmake ..

# posix
$ make

# windows... open folder with visual studio and build from there i guess...?
```
