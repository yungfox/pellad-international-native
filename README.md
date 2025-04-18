# pellad-international-native

Graphics concept using my band's alternate logo with latin, cyrillic and japanese script. I used it as an excuse to learn about how to handle with Unicode strings at a lower level, and how to render text with OpenGL by leveraging FreeType 2.<br>
Runs on macOS, Linux and Windows thanks to [RGFW](https://github.com/ColleagueRiley/RGFW) and a custom OpenGL extension loader.<br>
It's a native port of [pellad-international-web](https://github.com/yungfox/pellad-international-web), although arabic script is not yet supported.

## Dependencies

- OpenGL 3.3
- FreeType 2.11+

## Quick start

```console
# posix
$ ./build.sh
$ ./pellad-international

# windows
# note: building on windows has not been tested yet
$ build_msvc.bat
$ pellad-international.exe
```
