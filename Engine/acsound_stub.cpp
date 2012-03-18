/*
  ACSOUND - AGS sound system wrapper

  Adventure Game Studio source code Copyright 1999-2011 Chris Jones.
  All rights reserved.

  The AGS Editor Source Code is provided under the Artistic License 2.0
  http://www.opensource.org/licenses/artistic-license-2.0.php

  You MAY NOT compile your own builds of the engine without making it EXPLICITLY
  CLEAR that the code has been altered from the Standard Version.
*/

#include "acsound.h"

SOUNDCLIP *my_load_wave(const char *filename, int voll, int loop)
{
    printf("stub@%s:%d\n", __FILE__, __LINE__);
    return NULL;
}

SOUNDCLIP *my_load_mp3(const char *filname, int voll)
{
    printf("stub@%s:%d\n", __FILE__, __LINE__);
    return NULL;
}

SOUNDCLIP *my_load_static_mp3(const char *filname, int voll, bool loop)
{
    printf("stub@%s:%d\n", __FILE__, __LINE__);
    return NULL;
}

SOUNDCLIP *my_load_static_ogg(const char *filname, int voll, bool loop)
{
    printf("stub@%s:%d\n", __FILE__, __LINE__);
    return NULL;
}

SOUNDCLIP *my_load_ogg(const char *filname, int voll)
{
    printf("stub@%s:%d\n", __FILE__, __LINE__);
    return NULL;
}

SOUNDCLIP *my_load_midi(const char *filname, int repet)
{
    printf("stub@%s:%d\n", __FILE__, __LINE__);
    return NULL;
}

SOUNDCLIP *my_load_mod(const char *filname, int repet)
{
    printf("stub@%s:%d\n", __FILE__, __LINE__);
    return NULL;
}

int init_mod_player(int numVoices) {
    printf("stub@%s:%d\n", __FILE__, __LINE__);
    return 0;
}

void remove_mod_player() {
    printf("stub@%s:%d\n", __FILE__, __LINE__);
}
