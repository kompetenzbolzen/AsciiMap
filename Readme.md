# AsciiMap

CMDline tool to Convert 24bit Windows BitMap files to ASCII with optional ANSI-colors

### Options

* `-c` use ANSI 256 color-mode

* `-x <n>` average n x 2n pixels to one char

* `-s <n>` print n characters wide to be able to fit in terminal window, independent from picture size.

* `-i` use STDIN as file input

* `-w` print whitespaces with background color instead of ASCII chars. Only useful with -c

* `-d` activate dynamic brightness range

* `-m <string>` use a custom ASCII character palette. From darkest to brightest

To use with other image types, use imagemagick: `magick convert <image> -depth 24 bmp:- | asciimap -i`

License: MIT

![alt text](screenshot.png "Screenshot")
