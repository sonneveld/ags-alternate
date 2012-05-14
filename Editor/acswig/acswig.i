


%module acswig
%{
#include "agsnative.h"
%}


%include "carrays.i"

// ============================================================================

//%include "arrays_csharp.i"

// from http://www.nickdarnell.com/2011/05/swig-and-a-miss/

// void* magic
%typemap(ctype)  void * "void *"
%typemap(imtype) void * "IntPtr"
%typemap(cstype) void * "IntPtr"
%typemap(csin)   void * "$csinput"
%typemap(in)     void * %{ $1 = $input; %}
%typemap(out)    void * %{ $result = $1; %}
%typemap(csout)  void * { return $imcall; }


// char** magic

/*
%typemap(ctype) char** "char**"
%typemap(imtype) char** "string[]"
%typemap(cstype) char** "string[]"
 
%typemap(csin) char** "$csinput"
%typemap(csout, excode=SWIGEXCODE) char**, const char**& {
    int ret = $imcall;$excode
    return ret;
  }
%typemap(csvarin, excode=SWIGEXCODE2) char** %{
    set {
      $imcall;$excode
    } %}
%typemap(csvarout, excode=SWIGEXCODE2) char** %{
    get {
      int ret = $imcall;$excode
      return ret;
    } %}
 
%typemap(in) char** %{ $1 = $input; %}
%typemap(out) char** %{ $result = $1; %}
*/


// ============================================================================

typedef int int32;

// ============================================================================

#define COLORCONV_KEEP_TRANS        0x4000000

// allegro
typedef struct BITMAP            /* a bitmap structure */
{
   int w, h;                     /* width and height in pixels */
   int clip;                     /* flag if clipping is turned on */
   int cl, cr, ct, cb;           /* clip left, right, top and bottom values */
   GFX_VTABLE *vtable;           /* drawing functions */
   unsigned long id;             /* for identifying sub-bitmaps */
   int x_ofs;                    /* horizontal offset (for sub-bitmaps) */
   int y_ofs;                    /* vertical offset (for sub-bitmaps) */
   int seg;                      /* bitmap segment */
   unsigned char * line[0];
} BITMAP;


%extend BITMAP {
 void *GetLine(int index) { return $self->line[index]; }
};

typedef struct RGB
{
   unsigned char r, g, b;
   unsigned char filler;
} RGB;

#define PAL_SIZE     256

typedef RGB PALETTE[PAL_SIZE];


typedef BITMAP *block;





%rename(allegro_set_palette) set_palette;
void set_palette(RGB pal[]);

void destroy_bitmap(BITMAP* bmp);
BITMAP *create_bitmap_ex(int bpp, int width, int height);
extern BITMAP *abuf;


void blit (struct BITMAP *source, struct BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height);



%array_class(RGB , RgbArray);
void select_palette(RGB pal[]);

void unselect_palette(void);

void set_color_conversion (int mode);
int get_color_conversion (void);


int bitmap_color_depth(BITMAP *bmp);




// ============================================================================

#define SIMP_INDEX0  0
#define SIMP_TOPLEFT 1
#define SIMP_BOTLEFT 2
#define SIMP_TOPRIGHT 3
#define SIMP_BOTRIGHT 4
#define SIMP_LEAVEALONE 5
#define SIMP_NONE     6

#define MAX_INIT_SPR  40
#define MAX_OBJ       16  // max walk-behinds
#define NUM_MISC      20
#define MAXMESS       100
#define NUMOTCON      7                 // number of conditions before standing on
#define NUM_CONDIT    (120 + NUMOTCON)
#define MAX_HOTSPOTS  50   // v2.62 increased from 20 to 30; v2.8 to 50
#define MAX_REGIONS   16

#define MAX_SCRIPT_NAME_LEN 20

#define MAX_WALK_AREAS 15
#define MAXPOINTS 30

#define MAX_SPRITES         30000
#define MAX_CURSOR          20
#define PAL_GAMEWIDE        0
#define PAL_LOCKED          1
#define PAL_BACKGROUND      2
#define MAXGLOBALMES        500
#define MAXLANGUAGE         5
#define MAX_FONTS           15
#define OPT_DEBUGMODE       0
#define OPT_SCORESOUND      1
#define OPT_WALKONLOOK      2
#define OPT_DIALOGIFACE     3
#define OPT_ANTIGLIDE       4
#define OPT_TWCUSTOM        5
#define OPT_DIALOGGAP       6
#define OPT_NOSKIPTEXT      7
#define OPT_DISABLEOFF      8
#define OPT_ALWAYSSPCH      9
#define OPT_SPEECHTYPE      10
#define OPT_PIXPERFECT      11
#define OPT_NOWALKMODE      12
#define OPT_LETTERBOX       13
#define OPT_FIXEDINVCURSOR  14
#define OPT_NOLOSEINV       15
#define OPT_NOSCALEFNT      16
#define OPT_SPLITRESOURCES  17
#define OPT_ROTATECHARS     18
#define OPT_FADETYPE        19
#define OPT_HANDLEINVCLICKS 20
#define OPT_MOUSEWHEEL      21
#define OPT_DIALOGNUMBERED  22
#define OPT_DIALOGUPWARDS   23
#define OPT_CROSSFADEMUSIC  24
#define OPT_ANTIALIASFONTS  25
#define OPT_THOUGHTGUI      26
#define OPT_TURNTOFACELOC   27
#define OPT_RIGHTLEFTWRITE  28  // right-to-left text writing
#define OPT_DUPLICATEINV    29  // if they have 2 of the item, draw it twice
#define OPT_SAVESCREENSHOT  30
#define OPT_PORTRAITSIDE    31
#define OPT_STRICTSCRIPTING 32  // don't allow MoveCharacter-style commands
#define OPT_LEFTTORIGHTEVAL 33  // left-to-right operator evaluation
#define OPT_COMPRESSSPRITES 34
#define OPT_STRICTSTRINGS   35  // don't allow old-style strings
#define OPT_NEWGUIALPHA     36
#define OPT_RUNGAMEDLGOPTS  37
#define OPT_NATIVECOORDINATES 38
#define OPT_OLDTALKANIMSPD  39
#define OPT_HIGHESTOPTION   39
#define OPT_NOMODMUSIC      98
#define OPT_LIPSYNCTEXT     99
#define PORTRAIT_LEFT       0
#define PORTRAIT_RIGHT      1
#define PORTRAIT_ALTERNATE  2
#define PORTRAIT_XPOSITION  3
#define FADE_NORMAL         0
#define FADE_INSTANT        1
#define FADE_DISSOLVE       2
#define FADE_BOXOUT         3
#define FADE_CROSSFADE      4
#define FADE_LAST           4   // this should equal the last one
#define SPF_640x400         1
#define SPF_HICOLOR         2
#define SPF_DYNAMICALLOC    4
#define SPF_TRUECOLOR       8
#define SPF_ALPHACHANNEL 0x10
#define SPF_HADALPHACHANNEL 0x80  // the saved sprite on disk has one
//#define FFLG_NOSCALE        1
#define FFLG_SIZEMASK 0x003f
#define FONT_OUTLINE_AUTO -10
#define MAX_FONT_SIZE 63

// This struct is written directly to the disk file
// The GameSetupStruct subclass parts are written individually
struct GameSetupStructBase {
  char              gamename[50];
  int32             options[100];
  unsigned char     paluses[256];
  color             defpal[256];
  int32             numviews;
  int32             numcharacters;
  int32             playercharacter;
  int32             totalscore;
  short             numinvitems;
  int32             numdialog, numdlgmessage;
  int32             numfonts;
  int32             color_depth;          // in bytes per pixel (ie. 1 or 2)
  int32             target_win;
  int32             dialog_bullet;        // 0 for none, otherwise slot num of bullet point
  unsigned short    hotdot, hotdotouter;  // inv cursor hotspot dot
  int32             uniqueid;    // random key identifying the game
  int32             numgui;
  int32             numcursors;
  int32             default_resolution; // 0=undefined, 1=320x200, 2=320x240, 3=640x400 etc
  int32             default_lipsync_frame; // used for unknown chars
  int32             invhotdotsprite;
  int32             reserved[17];
  char             *messages[MAXGLOBALMES];
  WordsDictionary  *dict;
  char             *globalscript;
  CharacterInfo    *chars;
  ccScript         *compiled_script;

};

#define MAXVIEWNAMELENGTH 15
#define MAXLIPSYNCFRAMES  20
#define MAX_GUID_LENGTH   40
#define MAX_SG_EXT_LENGTH 20
#define MAX_SG_FOLDER_LEN 50
struct GameSetupStruct: public GameSetupStructBase {
  unsigned char     fontflags[MAX_FONTS];
  char              fontoutline[MAX_FONTS];
  unsigned char     spriteflags[MAX_SPRITES];
  InventoryItemInfo invinfo[MAX_INV];
  MouseCursor       mcurs[MAX_CURSOR];
  NewInteraction   **intrChar;
  NewInteraction   *intrInv[MAX_INV];
  InteractionScripts **charScripts;
  InteractionScripts **invScripts;
  int               filever;  // just used by editor
  char              lipSyncFrameLetters[MAXLIPSYNCFRAMES][50];
  CustomPropertySchema propSchema;
  CustomProperties  *charProps, invProps[MAX_INV];
  char              **viewNames;
  char              invScriptNames[MAX_INV][MAX_SCRIPT_NAME_LEN];
  char              dialogScriptNames[MAX_DIALOG][MAX_SCRIPT_NAME_LEN];
  char              guid[MAX_GUID_LENGTH];
  char              saveGameFileExtension[MAX_SG_EXT_LENGTH];
  char              saveGameFolderName[MAX_SG_FOLDER_LEN];
  int               roomCount;
  int              *roomNumbers;
  char            **roomNames;
  int               audioClipCount;
  ScriptAudioClip  *audioClips;
  int               audioClipTypeCount;
  AudioClipType    *audioClipTypes;
};


%extend GameSetupStruct {

#define GETSETTER(T, N, F) T Get ## F(int index) { return $self-> N [index];} void Set ## F(int index, T value) { $self-> N [index] = value; }

	GETSETTER(bool, options, Option)
	GETSETTER(unsigned char, spriteflags, SpriteFlag)
	GETSETTER(unsigned char, paluses, PalUse)
	GETSETTER(unsigned char, fontflags, FontFlag)

};

#define color RGB

%inline %{
color get_palette_entry(int index) { return palette[index]; }
void set_palette_entry(int index, color value) { palette[index] = value; }
%}

struct roomstruct {
	block         ebscene[MAX_BSCENE];
};

%extend roomstruct {
	block GetEbScene(int index) { return $self->ebscene[index];}
};

%include "agsnative.h"



%inline %{ 

roomstruct *cast_to_roomstruct(void *voidptr) {
	roomstruct *roomptr = (roomstruct*)voidptr;
	return roomptr;
}

block get_bitmap_for_mask_from_void(void *voidptr, RoomAreaMask maskType) {
	return get_bitmap_for_mask(cast_to_roomstruct(voidptr), maskType);
}


%}