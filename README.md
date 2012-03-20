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
 
## Building

Open Engine/acwin.vcproj in Visual Studio and compile.  There's a macbuild.sh but that's a quick hack for now.