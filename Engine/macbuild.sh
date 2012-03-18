#!/bin/bash

echo
echo
echo

# check out define ENABLE_THIS_LATER for code to fix later.
# rename to FUTURE_TODO ? or FUTURE_CODE ?

# not needed, just a reminder for when i have to run it.
export DYLD_LIBRARY_PATH=/Users/sonneveld/allegro5/lib

export PKG_CONFIG_PATH=/Users/sonneveld/allegro5/lib/pkgconfig

CFLAG="-DMAC_VERSION -DNO_MP3_PLAYER -DTHIS_IS_THE_ENGINE -D_DEBUG -DDEBUGMODE \
-g3 -O0 \
-w \
-m32 \
-ferror-limit=100 \
-I . \
-I ../Common \
-I ../External/hq2x/include \
-I ../External/a4_aux \
`pkg-config allegro-5.0 --cflags`"


LDFLAG="-DMAC_VERSION -DNO_MP3_PLAYER -DTHIS_IS_THE_ENGINE -D_DEBUG -DDEBUGMODE \
-g3 -O0 \
-w \
-m32 \
-ferror-limit=16 \
"

LIBS="`pkg-config allegro-5.0 --libs` \
`pkg-config allegro_audio-5.0 --libs` \
`pkg-config allegro_color-5.0 --libs` \
`pkg-config allegro_main-5.0 --libs` "

clang++ $CFLAG -c stub.cpp -o stub.o
clang++ $CFLAG -c ../external/hq2x//src/hq2x3x.cpp -o ../external/hq2x//src/hq2x3x.o
clang++ $CFLAG -c ../common/lzw.cpp -o ../common/lzw.o
clang++ $CFLAG -c ../common/mousew32.cpp -o ../common/mousew32.o
clang++ $CFLAG -c ../common/compress.cpp -o ../common/compress.o
clang++ $CFLAG -c ../common/cscommon.cpp -o ../common/cscommon.o
clang++ $CFLAG -c ../common/clib32.cpp -o ../common/clib32.o
clang++ $CFLAG -c ../common/sprcache.cpp -o ../common/sprcache.o
clang++ $CFLAG -c ../common/csrun.cpp -o ../common/csrun.o
clang++ $CFLAG -c macport.cpp -o macport.o
clang++ $CFLAG -c acsound_stub.cpp -o acsound_stub.o
clang++ $CFLAG -c scrptrt.cpp -o scrptrt.o
clang++ $CFLAG -c acplmac.cpp -o acplmac.o
clang++ $CFLAG -c bigend.cpp -o bigend.o
clang++ $CFLAG -c misc.cpp -o misc.o
clang++ $CFLAG -c acfonts_allegro5.cpp -o acfonts_allegro5.o
clang++ $CFLAG -c acgfx.cpp -o acgfx.o
clang++ $CFLAG -c ali3d_stub.cpp -o ali3d_stub.o
clang++ $CFLAG -c acaudio.cpp -o acaudio.o
clang++ $CFLAG -c routefnd.cpp -o routefnd.o
clang++ $CFLAG -c acplatfm.cpp -o acplatfm.o
clang++ $CFLAG -c acdialog.cpp -o acdialog.o
clang++ $CFLAG -c acgui.cpp -o acgui.o
clang++ $CFLAG -c acchars.cpp -o acchars.o
clang++ $CFLAG -c ac.cpp -o ac.o

clang++ $LDFLAG -o ags.binary stub.o ../external/hq2x//src/hq2x3x.o ../common/lzw.o ../common/mousew32.o ../common/compress.o ../common/cscommon.o ../common/clib32.o ../common/sprcache.o ../common/csrun.o macport.o acsound_stub.o scrptrt.o acplmac.o bigend.o misc.o acfonts_allegro5.o acgfx.o ali3d_stub.o acaudio.o routefnd.o acplatfm.o acdialog.o acgui.o acchars.o ac.o $LIBS