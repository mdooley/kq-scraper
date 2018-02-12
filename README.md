# kq-scraper

A program to scrape match results from a Killer Queen VOD

# Building

kq-scraper requires the following libraries:
* [FFmpeg](https://github.com/FFmpeg/FFmpeg)
* [ImageMagick](https://github.com/ImageMagick/ImageMagick)
* [tesseract](https://github.com/tesseract-ocr/tesseract)
* [leptonica](https://github.com/DanBloomberg/leptonica)

```
mkdir build && cd build
cmake ..
make
```

# Running

```
kq-scraper [file]
```

Produces match results of the following format:

```
Map,Win Condition,Winning Color(GOLD/BLUE),Gold Team Name,Blue Team Name
```

Screen positions for the play area/team names/etc default to KQNYC's FvFF stream locations, but can be overridden in an INI file and passed along using the --config option.

A --prefix option is also available to prepend a string to the beginning of each result (useful for dates).



