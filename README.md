# Adventure Game Studio fork

This is a fork of Chris Jones' AGS open source release. 

http://www.adventuregamestudio.co.uk/

## Current changes

 * moved library code into External/
 * started looking into creating an allegro4/5 wrapper (dunno if will still go down this road), see stub.cpp/stub.h
 * started work on removing all the externs and adding header files
 
## Todo

 * split ac.cpp.
 * add opengl support
 * clean up code
 * port to mac (lion at least!). This may require porting allegro 4 to lion.
 * port to allegro5/sdl/???
 * replace some of the libraries.
 
## Building

Open Engine/acwin.vcproj in Visual Studio and compile.  There's a macbuild.sh but that's a quick hack for now.

## Libraries

The source for most libraries is included.  Most been modified slightly to add features or prevent symbol conflicts.  I've tried to hunt down version numbers and download locations.

 * directx Apr 2007 SDK
 * allegro 4.2.2 - http://sourceforge.net/projects/alleg/files/allegro/4.2.2/
 * alfont 1.9.1 - http://www.helixsoft.nl/project_page.php?file_name=alfont.proj original: http://web.archive.org/web/*/http://nekros.freeshell.org/delirium/alfont.php
 * aastr 0.1.1
 * hq2x v????
 * libogg 1.1.3 - http://downloads.xiph.org/releases/ogg/
 * libtheora 1.0 - http://downloads.xiph.org/releases/theora/
 * libvorbis 1.2.0 - http://downloads.xiph.org/releases/vorbis/
 * dumb 0.9.2 - http://dumb.sourceforge.net/index.php?page=downloads
 * libcda 0.4 - http://tjaden.strangesoft.net/libcda/libcda04.zip
 * apeg 1.2.1 - http://kcat.strangesoft.net/apeg.html
 * almp3 2.0.5 - http://web.archive.org/web/*/http://nekros.freeshell.org/delirium/almp3.php via http://www.allegro.cc/forums/thread/600266
 * alogg 1.0.0 or 1.0.3 with wrong header - http://web.archive.org/web/*/http://nekros.freeshell.org/delirium/alogg.php

The forum post http://www.allegro.cc/forums/thread/600266 hinted where to find the homepage of alogg, almp3, alfont.

## Links


AGS Source Release
http://www.bigbluecup.com/yabb/index.php?topic=43383.0

AGS Linux port
http://www.bigbluecup.com/yabb/index.php?topic=37968.0

AGS Mac port
http://www.bigbluecup.com/yabb/index.php?topic=43590.0
(no source)

svn 
https://svn.bigbluecup.com:7743/svn/ags/trunk
u: guest
p:

AGS forks
https://gitorious.org/ags
https://gitorious.org/~jjs/ags/ags-for-psp

re AGS for psp
http://www.bigbluecup.com/yabb/index.php?topic=44768.msg610523

AG source refactoring
http://www.bigbluecup.com/yabb/index.php?topic=45372.0
https://github.com/AlanDrake/Adventure-Game-Studio

AG future  stalemate
http://www.bigbluecup.com/yabb/index.php?topic=44047.0

scummvm ags port
https://github.com/fuzzie/scummvm/commits/ags
http://logs.scummvm.org/log.php?log=scummvm.log.12Feb2012&format=html
https://github.com/fuzzie/scummvm/tree/3211375fb8caae03c3b23e7bf738ea6a08d7c723/engines/ags


