cmake_minimum_required(VERSION 3.5.1)
project(kq-scraper)

set(CMAKE_CXX_STANDARD 99)

add_definitions(-DMAGICKCORE_QUANTUM_DEPTH=16 -DMAGICKCORE_HDRI_ENABLE)

include_directories(BEFORE /usr/local/include)
include_directories(inih)
include_directories(/usr/local/include/ImageMagick-7)

add_executable(kq-scraper
        inih/ini.c
        inih/ini.h
        src/blue.h
        src/economy.h
        src/frameextractor.c
        src/frameextractor.h
        src/frameprocessor.c
        src/frameprocessor.h
        src/frameprocessorconfig.c
        src/frameprocessorconfig.h
        src/frameprocessorresults.c
        src/frameprocessorresults.h
        src/gold.h
        src/main.c
        src/military.h
        src/snail.h)

target_link_libraries(kq-scraper MagickWand-7.Q16HDRI tesseract avformat avcodec swscale avutil lept m)
