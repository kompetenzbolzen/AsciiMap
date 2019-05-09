# AsciiMap

CMDline tool to Convert 24bit Windows BitMap files to ASCII with optional ANSI-colors

### Options

* -c to use ANSI 256 color-mode

* -x <n> to average n x 2n pixels to one char

* -i use STDIN as file input

To use with other image types, use imagemagick: `magick convert <image> -depth 24 bmp:- | asciimap -i`

License: MIT

![alt text](screenshot.png "Screenshot")
