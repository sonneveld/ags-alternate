#define WINDOWS_VERSION
#define USE_CLIB
#define SWAP_RB_HICOL  // win32 uses BGR not RGB
#include <stdio.h>
void serialize_room_interactions(FILE *);
void ThrowManagedException(const char *message);
#pragma unmanaged
#pragma warning (disable: 4996 4312)  // disable deprecation warnings
extern "C" {
  extern FILE *clibfopen(char *filnamm, char *fmt);
  extern int csetlib(char *fileName, char *password);
  extern int clibGetNumFiles();
  extern const char *clibGetFileName(int);
  extern const char *clibgetoriginalfilename();
  extern int cfopenpriority;
}
extern bool Scintilla_RegisterClasses(void *hInstance);
extern int Scintilla_LinkLexers();

int antiAliasFonts = 0;
#define SAVEBUFFERSIZE 5120
bool ShouldAntiAliasText() { return (antiAliasFonts != 0); }

int mousex, mousey;
#include "misc.h"
#include "wgt2allg.h"
#include "sprcache.h"
#include "acroom.h"
#include "acgui.h"

extern void Cstretch_blit(BITMAP *src, BITMAP *dst, int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh);
extern void Cstretch_sprite(BITMAP *dst, BITMAP *src, int x, int y, int w, int h);


int sxmult = 1, symult = 1;
int dsc_want_hires = 0;
bool enable_greyed_out_masks = true;
bool outlineGuiObjects;
color*palette;
GameSetupStruct thisgame;
SpriteCache spriteset(MAX_SPRITES + 2);
GUIMain tempgui;
const char*sprsetname = "acsprset.spr";
const char*clibendsig = "CLIB\x1\x2\x3\x4SIGE";
const char *old_editor_data_file = "editor.dat";
const char *new_editor_data_file = "game.agf";
const char *old_editor_main_game_file = "ac2game.dta";
const char *TEMPLATE_LOCK_FILE = "template.dta";
const char *ROOM_TEMPLATE_ID_FILE = "rtemplate.dat";
const int ROOM_TEMPLATE_ID_FILE_SIGNATURE = 0x74673812;
bool spritesModified = false;
roomstruct thisroom;
bool roomModified = false;
block drawBuffer = NULL;
block undoBuffer = NULL;
int loaded_room_number = -1;

// stuff for importing old games
int numScriptModules;
ScriptModule* scModules = NULL;
DialogTopic *dialog;
char*dlgscript[MAX_DIALOG];
GUIMain *guis;
ViewStruct272 *oldViews;
ViewStruct *newViews;
int numNewViews = 0;


bool reload_font(int curFont);
void drawBlockScaledAt(int hdc, block todraw ,int x, int y, int scaleFactor);
// this is to shut up the linker, it's used by CSRUN.CPP
void write_log(char *) { }

void GUIInv::Draw() {
  wsetcolor(15);
  wrectangle(x,y,x+wid,y+hit);
}



int multiply_up_coordinate(int coord)
{
	return coord * sxmult;
}

int get_fixed_pixel_size(int coord)
{
	return coord * sxmult;
}


// jibbles the sprite around to fix hi-color problems, by swapping
// the red and blue elements
#define fix_sprite(num) fix_block(spriteset[num])
void fix_block (block todraw) {
  int a,b,pixval;
  if (todraw == NULL)
    return;
  if (bitmap_color_depth(todraw) == 16) {
    for (a = 0; a < todraw->w; a++) {
      for (b = 0; b < todraw->h; b++) {
        pixval = _getpixel16 (todraw, a, b);
        _putpixel16 (todraw, a, b, makecol16 (getb16(pixval),getg16(pixval),getr16(pixval)));
      }
    }
  }
  else if (bitmap_color_depth(todraw) == 32) {
    for (a = 0; a < todraw->w; a++) {
      for (b = 0; b < todraw->h; b++) {
        pixval = _getpixel32 (todraw, a, b);
        _putpixel32 (todraw, a, b, makeacol32 (getb32(pixval),getg32(pixval),getr32(pixval), geta32(pixval)));
      }
    }
  }
}

void initialize_sprite(int spnum) {
  fix_sprite(spnum);
}

void pre_save_sprite(int spnum) {
  fix_sprite(spnum);
}

block get_sprite (int spnr) {
  if (spnr < 0)
    return NULL;
  if (spriteset[spnr] == NULL) {
    spnr = 0;
  }
  return spriteset[spnr];
}

void SetNewSprite(int slot, block sprit) {
  if (spriteset[slot] != NULL)
    wfreeblock(spriteset[slot]);

  spriteset.setNonDiscardable(slot, sprit);
  spritesModified = true;
}

void deleteSprite (int sprslot) {
  spriteset.removeSprite(sprslot, true);
  
  spritesModified = true;
}

void SetNewSpriteFromHBitmap(int slot, int hBmp) {
  block tempsprite = convert_hbitmap_to_bitmap((HBITMAP)hBmp);
  SetNewSprite(slot, tempsprite);
}

int GetSpriteAsHBitmap(int slot) {
  return (int)convert_bitmap_to_hbitmap(get_sprite(slot));
}

bool DoesSpriteExist(int slot) {
	return (spriteset[slot] != NULL);
}

int GetMaxSprites() {
	return MAX_SPRITES;
}

int GetSpriteWidth(int slot) {
	return get_sprite(slot)->w;
}

int GetSpriteHeight(int slot) {
	return get_sprite(slot)->h;
}

int GetRelativeSpriteWidth(int slot) {
	return GetSpriteWidth(slot) / ((thisgame.spriteflags[slot] & SPF_640x400) ? 2 : 1);
}

int GetRelativeSpriteHeight(int slot) {
	return GetSpriteHeight(slot) / ((thisgame.spriteflags[slot] & SPF_640x400) ? 2 : 1);
}

int GetSpriteResolutionMultiplier(int slot)
{
	return ((thisgame.spriteflags[slot] & SPF_640x400) ? 1 : 2);
}

unsigned char* GetRawSpriteData(int spriteSlot) {
  return &get_sprite(spriteSlot)->line[0][0];
}

int GetSpriteColorDepth(int slot) {
  return bitmap_color_depth(get_sprite(slot));
}

int GetPaletteAsHPalette() {
  return (int)convert_palette_to_hpalette(palette);
}

void transform_string(char *text) {
	encrypt_text(text);
}

int find_free_sprite_slot() {
  int rr = spriteset.findFreeSlot();
  if (rr < 0) {
    return -1;
  }
  spriteset.images[rr] = NULL;
  spriteset.offsets[rr] = 0;
  spriteset.sizes[rr] = 0;
  return rr;
}

void update_sprite_resolution(int spriteNum, bool isHighRes)
{
	thisgame.spriteflags[spriteNum] &= ~SPF_640x400;
	if (isHighRes)
	{
		thisgame.spriteflags[spriteNum] |= SPF_640x400;
	}
}

void change_sprite_number(int oldNumber, int newNumber) {

  spriteset.setNonDiscardable(newNumber, spriteset[oldNumber]);
  spriteset.removeSprite(oldNumber, false);

  thisgame.spriteflags[newNumber] = thisgame.spriteflags[oldNumber];
  thisgame.spriteflags[oldNumber] = 0;

  spritesModified = true;
}

int crop_sprite_edges(int numSprites, int *sprites, bool symmetric) {
  // this function has passed in a list of sprites, all the
  // same size, to crop to the size of the smallest
  int aa, xx, yy;
  int width = spriteset[sprites[0]]->w;
  int height = spriteset[sprites[0]]->h;
  int left = width, right = 0;
  int top = height, bottom = 0;

  for (aa = 0; aa < numSprites; aa++) {
    block sprit = get_sprite(sprites[aa]);
    int maskcol = bitmap_mask_color(sprit);

    // find the left hand side
    for (xx = 0; xx < width; xx++) {
      for (yy = 0; yy < height; yy++) {
        if (getpixel(sprit, xx, yy) != maskcol) {
          if (xx < left)
            left = xx;
          xx = width + 10;
          break;
        }
      }
    }
    // find the right hand side
    for (xx = width - 1; xx >= 0; xx--) {
      for (yy = 0; yy < height; yy++) {
        if (getpixel(sprit, xx, yy) != maskcol) {
          if (xx > right)
            right = xx;
          xx = -10;
          break;
        }
      }
    }
    // find the top side
    for (yy = 0; yy < height; yy++) {
      for (xx = 0; xx < width; xx++) {
        if (getpixel(sprit, xx, yy) != maskcol) {
          if (yy < top)
            top = yy;
          yy = height + 10;
          break;
        }
      }
    }
    // find the bottom side
    for (yy = height - 1; yy >= 0; yy--) {
      for (xx = 0; xx < width; xx++) {
        if (getpixel(sprit, xx, yy) != maskcol) {
          if (yy > bottom)
            bottom = yy;
          yy = -10;
          break;
        }
      }
    }
  }

  // Now, we have been through all the sprites and found the outer
  // edges that we need to keep. So, now crop everything down
  // to the smaller sizes
  if (symmetric) {
    // symmetric -- make sure that the left and right edge chopping
    // are equal
    int rightDist = (width - right) - 1;
    if (rightDist < left)
      left = rightDist;
    if (left < rightDist)
      right = (width - left) - 1;
  }
  int newWidth = (right - left) + 1;
  int newHeight = (bottom - top) + 1;

  if ((newWidth == width) && (newHeight == height)) {
    // no change in size
    return 0;
  }

  if ((newWidth < 1) || (newHeight < 1))
  {
	  // completely transparent sprite, don't attempt to crop
	  return 0;
  }

  for (aa = 0; aa < numSprites; aa++) {
    block sprit = get_sprite(sprites[aa]);
    // create a new, smaller sprite and copy across
    block newsprit = create_bitmap_ex(bitmap_color_depth(sprit), newWidth, newHeight);
    blit(sprit, newsprit, left, top, 0, 0, newWidth, newHeight);
    destroy_bitmap(sprit);

    spriteset.setNonDiscardable(sprites[aa], newsprit);
  }

  spritesModified = true;

  return 1;
}

int extract_room_template_files(const char *templateFileName, int newRoomNumber) 
{
  if (csetlib((char*)templateFileName, "")) 
  {
    return 0;
  }
  if (cliboffset((char*)ROOM_TEMPLATE_ID_FILE) < 1)
  {
    csetlib(NULL, "");
    return 0;
  }

  int numFile = clibGetNumFiles();

  for (int a = 0; a < numFile; a++) {
    const char *thisFile = clibGetFileName(a);
    if (thisFile == NULL) {
      csetlib(NULL, "");
      return 0;
    }

    // don't extract the template metadata file
    if (stricmp(thisFile, ROOM_TEMPLATE_ID_FILE) == 0)
      continue;

    FILE *readin = clibfopen ((char*)thisFile, "rb");
    char outputName[MAX_PATH];
    const char *extension = strchr(thisFile, '.');
    sprintf(outputName, "room%d%s", newRoomNumber, extension);
    FILE *wrout = fopen(outputName, "wb");
    if ((readin == NULL) || (wrout == NULL)) 
    {
      if (wrout != NULL) fclose(wrout);
      if (readin != NULL) fclose(readin);
      csetlib(NULL, "");
      return 0;
    }
    long size = clibfilesize((char*)thisFile);
    char *membuff = (char*)malloc (size);
    fread (membuff, 1, size, readin);
    fwrite (membuff, 1, size, wrout);
    fclose (readin);
    fclose (wrout);
    free (membuff);
  }

  csetlib(NULL, "");
  return 1;
}

int extract_template_files(const char *templateFileName) 
{
  if (csetlib((char*)templateFileName, "")) 
  {
    return 0;
  }
  
  if ((cliboffset((char*)old_editor_data_file) < 1) && (cliboffset((char*)new_editor_data_file) < 1))
  {
    csetlib(NULL, "");
    return 0;
  }

  int numFile = clibGetNumFiles();

  for (int a = 0; a < numFile; a++) {
    const char *thisFile = clibGetFileName (a);
    if (thisFile == NULL) {
      csetlib(NULL, "");
      return 0;
    }

    // don't extract the dummy template lock file
    if (stricmp(thisFile, TEMPLATE_LOCK_FILE) == 0)
      continue;

    FILE *readin = clibfopen ((char*)thisFile, "rb");
    FILE *wrout = fopen (thisFile, "wb");
    if ((wrout == NULL) && (strchr(thisFile, '\\') != NULL))
    {
      // an old template with Music/Sound folder, create the folder
      char folderName[MAX_PATH];
      strcpy(folderName, thisFile);
      *strchr(folderName, '\\') = 0;
      mkdir(folderName);
      wrout = fopen(thisFile, "wb");
    }
    if ((readin == NULL) || (wrout == NULL)) 
    {
      csetlib(NULL, "");
      return 0;
    }
    long size = clibfilesize((char*)thisFile);
    char *membuff = (char*)malloc (size);
    fread (membuff, 1, size, readin);
    fwrite (membuff, 1, size, wrout);
    fclose (readin);
    fclose (wrout);
    free (membuff);
  }

  csetlib(NULL, "");
  return 1;
}

void extract_icon_from_template(char *iconName, char **iconDataBuffer, long *bufferSize)
{
  // make sure we get the icon from the file
  cfopenpriority = 1;
  long sizey = clibfilesize(iconName);
  FILE* inpu = clibfopen (iconName, "rb");
  if ((inpu != NULL) && (sizey > 0))
  {
    char *iconbuffer = (char*)malloc(sizey);
    fread (iconbuffer, 1, sizey, inpu);
    fclose (inpu);
    *iconDataBuffer = iconbuffer;
    *bufferSize = sizey;
  }
  else
  {
    *iconDataBuffer = NULL;
    *bufferSize = 0;
  }
  // restore to normal setting after NewGameChooser changes it
  cfopenpriority = 2;
}

int load_template_file(const char *fileName, char **iconDataBuffer, long *iconDataSize, bool isRoomTemplate)
{
  if (csetlib((char*)fileName, "") == 0)
  {
    if (isRoomTemplate)
    {
      if (cliboffset((char*)ROOM_TEMPLATE_ID_FILE) > 0)
      {
        FILE *inpu = clibfopen((char*)ROOM_TEMPLATE_ID_FILE, "rb");
        if (getw(inpu) != ROOM_TEMPLATE_ID_FILE_SIGNATURE)
        {
          fclose(inpu);
		  csetlib(NULL, "");
          return 0;
        }
        int roomNumber = getw(inpu);
        fclose(inpu);
        char iconName[MAX_PATH];
        sprintf(iconName, "room%d.ico", roomNumber);
        if (cliboffset(iconName) > 0) 
        {
          extract_icon_from_template(iconName, iconDataBuffer, iconDataSize);
        }
		    csetlib(NULL, "");
        return 1;
      }
	  csetlib(NULL, "");
      return 0;
    }
	  else if ((cliboffset((char*)old_editor_data_file) > 0) || (cliboffset((char*)new_editor_data_file) > 0))
	  {
      const char *oriname = clibgetoriginalfilename();
      if ((strstr(oriname, ".exe") != NULL) ||
          (strstr(oriname, ".dat") != NULL) ||
          (strstr(oriname, ".ags") != NULL)) 
      {
        // wasn't originally meant as a template
		  csetlib(NULL, "");
	      return 0;
      }

	    FILE *inpu = clibfopen((char*)old_editor_main_game_file, "rb");
	    if (inpu != NULL) 
	    {
		    fseek(inpu, 30, SEEK_CUR);
		    int gameVersion = _getw(inpu);
		    fclose(inpu);
		    if (gameVersion != 32)
		    {
			    // older than 2.72 template
				csetlib(NULL, "");
			    return 0;
		    }
	    }

      int useIcon = 0;
      char *iconName = "template.ico";
      if (cliboffset (iconName) < 1)
        iconName = "user.ico";
      // the file is a CLIB file, so let's extract the icon to display
      if (cliboffset (iconName) > 0) 
      {
        extract_icon_from_template(iconName, iconDataBuffer, iconDataSize);
      }
	    csetlib(NULL, "");
      return 1;
    }
  }
  return 0;
}

const char* save_sprites(bool compressSprites) 
{
  const char *errorMsg = NULL;
  char backupname[100];
  sprintf(backupname, "backup_%s", sprsetname);

  if ((spritesModified) || (compressSprites != spriteset.spritesAreCompressed))
  {
    spriteset.detachFile();
    if (exists(backupname) && (unlink(backupname) != 0)) {
      errorMsg = "Unable to overwrite the old backup file. Make sure the backup sprite file is not read-only";
    }
    else if (rename(sprsetname, backupname)) {
      errorMsg = "Unable to create the backup sprite file. Make sure the backup sprite file is not read-only";
    }
    else if (spriteset.attachFile(backupname)) {
      errorMsg = "An error occurred attaching to the backup sprite file. Check write permissions on your game folder";
    }
    else if (spriteset.saveToFile(sprsetname, MAX_SPRITES, compressSprites)) {
      errorMsg = "Unable to save the sprites. An error occurred writing the sprite file.";
    }

    // reset the sprite cache
    spriteset.reset();
    if (spriteset.initFile(sprsetname))
    {
      if (errorMsg == NULL)
        errorMsg = "Unable to re-initialize sprite file after save.";
    }

    if (errorMsg == NULL)
      spritesModified = false;
  }
  return errorMsg;
}

void drawBlockDoubleAt (int hdc, block todraw ,int x, int y) {
  drawBlockScaledAt (hdc, todraw, x, y, 2);
}

void wputblock_stretch(int xpt,int ypt,block tblock,int nsx,int nsy) {
  if (bmp_bpp(tblock) != thisgame.color_depth) {
    block tempst=create_bitmap_ex(thisgame.color_depth*8,tblock->w,tblock->h);
    blit(tblock,tempst,0,0,0,0,tblock->w,tblock->h);
    int ww,vv;
    for (ww=0;ww<tblock->w;ww++) {
      for (vv=0;vv<tblock->h;vv++) {
        if (getpixel(tblock,ww,vv)==bitmap_mask_color(tblock))
          putpixel(tempst,ww,vv,bitmap_mask_color(tempst));
      }
    }
    stretch_sprite(abuf,tempst,xpt,ypt,nsx,nsy);
    wfreeblock(tempst);
  }
  else stretch_sprite(abuf,tblock,xpt,ypt,nsx,nsy);
}

void draw_sprite_compensate(int sprnum, int atxp, int atyp, int seethru) {
  block blptr = get_sprite(sprnum);
  block towrite=blptr;
  int needtofree=0, main_color_depth = thisgame.color_depth * 8;

  if ((bmp_bpp(blptr) > 1) & (main_color_depth==8)) {

    towrite=create_bitmap_ex(8,blptr->w,blptr->h);
    needtofree=1;
    clear_to_color(towrite,bitmap_mask_color(towrite));
    int xxp,yyp,tmv;
    for (xxp=0;xxp<blptr->w;xxp++) {
      for (yyp=0;yyp<blptr->h;yyp++) {
        tmv=getpixel(blptr,xxp,yyp);
        if (tmv != bitmap_mask_color(blptr))
          putpixel(towrite,xxp,yyp,makecol8(getr16(tmv),getg16(tmv),getb16(tmv)));
        }
      }

    }

  int nwid=towrite->w,nhit=towrite->h;
  if (thisgame.spriteflags[sprnum] & SPF_640x400) {
    if (dsc_want_hires == 0) {
      nwid/=2;
      nhit/=2;
    }
  }
  else if (dsc_want_hires) {
    nwid *= 2;
    nhit *= 2;
  }
  wputblock_stretch(atxp,atyp,towrite,nwid,nhit);
  if (needtofree) wfreeblock(towrite);
}

void drawBlock (HDC hdc, block todraw, int x, int y) {
  set_palette_to_hdc (hdc, palette);
  blit_to_hdc (todraw, hdc, 0,0,x,y,todraw->w,todraw->h);
}


enum RoomAreaMask
{
    None,
    Hotspots,
    WalkBehinds,
    WalkableAreas,
    Regions
};

block get_bitmap_for_mask(roomstruct *roomptr, RoomAreaMask maskType) 
{
	if (maskType == RoomAreaMask::None) 
	{
		return NULL;
	}

	block source = NULL;
	switch (maskType) 
	{
	case RoomAreaMask::Hotspots:
		source = roomptr->lookat;
		break;
	case RoomAreaMask::Regions:
		source = roomptr->regions;
		break;
	case RoomAreaMask::WalkableAreas:
		source = roomptr->walls;
		break;
	case RoomAreaMask::WalkBehinds:
		source = roomptr->object;
		break;
	}

	return source;
}

void copy_walkable_to_regions (void *roomptr) {
	roomstruct *theRoom = (roomstruct*)roomptr;
	blit(theRoom->walls, theRoom->regions, 0, 0, 0, 0, theRoom->regions->w, theRoom->regions->h);
}

int get_mask_pixel(void *roomptr, int maskType, int x, int y)
{
	block mask = get_bitmap_for_mask((roomstruct*)roomptr, (RoomAreaMask)maskType);
	return getpixel(mask, x, y);
}

void draw_line_onto_mask(void *roomptr, int maskType, int x1, int y1, int x2, int y2, int color)
{
	block mask = get_bitmap_for_mask((roomstruct*)roomptr, (RoomAreaMask)maskType);
	line(mask, x1, y1, x2, y2, color);
}

void draw_filled_rect_onto_mask(void *roomptr, int maskType, int x1, int y1, int x2, int y2, int color)
{
	block mask = get_bitmap_for_mask((roomstruct*)roomptr, (RoomAreaMask)maskType);
	rectfill(mask, x1, y1, x2, y2, color);
}

void draw_fill_onto_mask(void *roomptr, int maskType, int x1, int y1, int color)
{
	block mask = get_bitmap_for_mask((roomstruct*)roomptr, (RoomAreaMask)maskType);
	floodfill(mask, x1, y1, color);
}

void create_undo_buffer(void *roomptr, int maskType) 
{
	block mask = get_bitmap_for_mask((roomstruct*)roomptr, (RoomAreaMask)maskType);
  if (undoBuffer != NULL)
  {
    if ((undoBuffer->w != mask->w) || (undoBuffer->h != mask->h)) 
    {
      destroy_bitmap(undoBuffer);
      undoBuffer = NULL;
    }
  }
  if (undoBuffer == NULL)
  {
    undoBuffer = create_bitmap_ex(bitmap_color_depth(mask), mask->w, mask->h);
  }
  blit(mask, undoBuffer, 0, 0, 0, 0, mask->w, mask->h);
}

bool does_undo_buffer_exist()
{
  return (undoBuffer != NULL);
}

void clear_undo_buffer() 
{
  if (does_undo_buffer_exist()) 
  {
    destroy_bitmap(undoBuffer);
    undoBuffer = NULL;
  }
}

void restore_from_undo_buffer(void *roomptr, int maskType)
{
  if (does_undo_buffer_exist())
  {
  	block mask = get_bitmap_for_mask((roomstruct*)roomptr, (RoomAreaMask)maskType);
    blit(undoBuffer, mask, 0, 0, 0, 0, mask->w, mask->h);
  }
}

void setup_greyed_out_palette(int selCol) 
{
    color thisColourOnlyPal[256];

    // The code below makes it so that all the hotspot colours
    // except the selected one are greyed out. It doesn't work
    // in 256-colour games.

    // Blank out the temporary palette, and set a shade of grey
    // for all the hotspot colours
    for (int aa = 0; aa < 256; aa++) {
      int lumin = 0;
      if ((aa < MAX_HOTSPOTS) && (aa > 0))
        lumin = ((MAX_HOTSPOTS - aa) % 30) * 2;
      thisColourOnlyPal[aa].r = lumin;
      thisColourOnlyPal[aa].g = lumin;
      thisColourOnlyPal[aa].b = lumin;
    }
    // Highlight the currently selected area colour
    if (selCol > 0) {
      // if a bright colour, use it
      if ((selCol < 15) && (selCol != 7) && (selCol != 8))
        thisColourOnlyPal[selCol] = palette[selCol];
      else {
        // else, draw in red
        thisColourOnlyPal[selCol].r = 60;
        thisColourOnlyPal[selCol].g = 0;
        thisColourOnlyPal[selCol].b = 0;
      }
    }
    set_palette(thisColourOnlyPal);
}

BITMAP *recycle_bitmap(BITMAP* check, int colDepth, int w, int h)
{
  if ((check != NULL) && (check->w == w) && (check->h == h) &&
      (bitmap_color_depth(check) == colDepth))
  {
    return check;
  }
  if (check != NULL)
    destroy_bitmap(check);

  return create_bitmap_ex(colDepth, w, h);
}

block stretchedSprite = NULL, srcAtRightColDep = NULL;

void draw_area_mask(roomstruct *roomptr, block destination, RoomAreaMask maskType, int selectedArea, int transparency) 
{
	block source = get_bitmap_for_mask(roomptr, maskType);

	if (source == NULL) return;
	
	if (bitmap_color_depth(source) != bitmap_color_depth(destination)) 
	{
    block sourceSprite = source;

    if ((source->w != destination->w) || (source->h != destination->h))
    {
		  stretchedSprite = recycle_bitmap(stretchedSprite, bitmap_color_depth(source), destination->w, destination->h);
		  stretch_blit(source, stretchedSprite, 0, 0, source->w, source->h, 0, 0, stretchedSprite->w, stretchedSprite->h);
      sourceSprite = stretchedSprite;
    }

    if (enable_greyed_out_masks)
    {
      setup_greyed_out_palette(selectedArea);
    }

    if (transparency > 0)
    {
      srcAtRightColDep = recycle_bitmap(srcAtRightColDep, bitmap_color_depth(destination), destination->w, destination->h);
      
      int oldColorConv = get_color_conversion();
      set_color_conversion(oldColorConv | COLORCONV_KEEP_TRANS);

      blit(sourceSprite, srcAtRightColDep, 0, 0, 0, 0, sourceSprite->w, sourceSprite->h);
      set_trans_blender(0, 0, 0, (100 - transparency) + 155);
      draw_trans_sprite(destination, srcAtRightColDep, 0, 0);

      set_color_conversion(oldColorConv);
    }
    else
    {
		  draw_sprite(destination, sourceSprite, 0, 0);
    }

    set_palette(palette);
	}
	else
	{
		Cstretch_sprite(destination, source, 0, 0, destination->w, destination->h);
	}
}

void draw_room_background(void *roomvoidptr, int hdc, int x, int y, int bgnum, float scaleFactor, int maskType, int selectedArea, int maskTransparency) 
{
	roomstruct *roomptr = (roomstruct*)roomvoidptr;

  if (bgnum >= roomptr->num_bscenes)
    return;

  block srcBlock = roomptr->ebscene[bgnum];
  if (srcBlock == NULL)
    return;

	if (drawBuffer != NULL) 
	{
		block depthConverted = create_bitmap_ex(bitmap_color_depth(drawBuffer), srcBlock->w, srcBlock->h);
    if (bitmap_color_depth(srcBlock) == 8)
    {
      select_palette(roomptr->bpalettes[bgnum]);
    }

		blit(srcBlock, depthConverted, 0, 0, 0, 0, srcBlock->w, srcBlock->h);

    if (bitmap_color_depth(srcBlock) == 8)
    {
      unselect_palette();
    }

		draw_area_mask(roomptr, depthConverted, (RoomAreaMask)maskType, selectedArea, maskTransparency);

    int srcX = 0, srcY = 0;
    int srcWidth = srcBlock->w;
    int srcHeight = srcBlock->h;

    if (x < 0)
    {
      srcX = -x / scaleFactor;
      x = 0;
      srcWidth = drawBuffer->w / scaleFactor + 1;
      if (srcX + srcWidth > depthConverted->w)
      {
        srcWidth = depthConverted->w - srcX;
      }
    }
    if (y < 0)
    {
      srcY = -y / scaleFactor;
      y = 0;
      srcHeight = drawBuffer->h / scaleFactor + 1;
      if (srcY + srcHeight > depthConverted->h)
      {
        srcHeight = depthConverted->h - srcY;
      }
    }

		Cstretch_blit(depthConverted, drawBuffer, srcX, srcY, srcWidth, srcHeight, x, y, srcWidth * scaleFactor, srcHeight * scaleFactor);
		destroy_bitmap(depthConverted);
	}
	else {
		drawBlockScaledAt(hdc, srcBlock, x, y, scaleFactor);
	}
	
}

void update_font_sizes() {
  int multiplyWas = wtext_multiply;

  // scale up fonts if necessary
  wtext_multiply = 1;
  if ((thisgame.options[OPT_NOSCALEFNT] == 0) &&
      (thisgame.default_resolution >= 3)) {
    wtext_multiply = 2;
  }

  if (multiplyWas != wtext_multiply) {
    // resolution or Scale Up Fonts has changed, reload at new size
    for (int bb=0;bb<thisgame.numfonts;bb++)
      reload_font (bb);
  }

  if (thisgame.default_resolution >= 3) {
    sxmult = 2;
    symult = 2;
  }
  else {
    sxmult = 1;
    symult = 1;
  }

}

const char* import_sci_font(const char*fnn,int fslot) {
  char wgtfontname[100];
  sprintf(wgtfontname,"agsfnt%d.wfn",fslot);
  FILE*iii=fopen(fnn,"rb");
  if (iii==NULL) {
    return "File not found";
  }
  if (fgetc(iii)!=0x87) {
    fclose(iii);
    return "Not a valid SCI font file";
  }
  fseek(iii,3,SEEK_CUR);
  if (getshort(iii)!=0x80) {
    fclose(iii); 
	  return "Invalid SCI font"; 
  }
  int lineHeight = getshort(iii);
  short theiroffs[0x80];
  fread(theiroffs,2,0x80,iii);
  FILE*ooo=fopen(wgtfontname,"wb");
  fwrite("WGT Font File  ",15,1,ooo);
  putshort(0,ooo);  // will be table address
  short coffsets[0x80];
  char buffer[1000];
  int aa;
  for (aa=0;aa<0x80;aa++) 
  {
    if (theiroffs[aa] < 100)
    {
      fclose(iii);
      fclose(ooo);
      unlink(wgtfontname);
      return "Invalid character found in file";
    }
    fseek(iii,theiroffs[aa]+2,SEEK_SET);
    int wwi=fgetc(iii)-1;
    int hhi=fgetc(iii);
    coffsets[aa]=ftell(ooo);
    putshort(wwi+1,ooo);
    putshort(hhi,ooo);
    if ((wwi<1) | (hhi<1)) continue;
    memset(buffer,0,1000);
    int bytesPerRow = (wwi/8)+1;
    fread(buffer, bytesPerRow, hhi, iii);
    for (int bb=0;bb<hhi;bb++) { 
      int thisoffs = bb * bytesPerRow;
      fwrite(&buffer[thisoffs], bytesPerRow, 1, ooo);
    }
  }
  long tableat=ftell(ooo);
  fwrite(&coffsets[0],2,0x80,ooo);
  fclose(ooo); ooo=fopen(wgtfontname,"r+b");
  fseek(ooo,15,SEEK_SET); putshort(tableat,ooo); 
  fclose(ooo);
  fclose(iii);
  wfreefont(fslot);
  if (!wloadfont_size(fslot, 0))
  {
    return "Unable to load converted WFN file";
  }
  return NULL;
}


#define FONTGRIDSIZE 18*blockSize
void drawFontAt (int hdc, int fontnum, int x,int y) {
  
  if (fontnum >= thisgame.numfonts) 
  {
	  return;
  }

  update_font_sizes();

  int doubleSize = (thisgame.default_resolution < 3) ? 2 : 1;
  int blockSize = (thisgame.default_resolution < 3) ? 1 : 2;
  antiAliasFonts = thisgame.options[OPT_ANTIALIASFONTS];

  // we can't antialias font because changing col dep to 16 here causes
  // it to crash ... why?
  block tempblock = create_bitmap_ex(8, FONTGRIDSIZE*10, FONTGRIDSIZE*10);
  clear_to_color(tempblock, 0);
  block abufwas = abuf;
  abuf = tempblock;
  wtextcolor(15);
  for (int aa=0;aa<96;aa++)
    wgtprintf(5+(aa%10)*FONTGRIDSIZE,5+(aa/10)*FONTGRIDSIZE, fontnum, "%c",aa+32);
  abuf = abufwas;

  if (doubleSize > 1) 
    drawBlockDoubleAt(hdc, tempblock, x, y);
  else
    drawBlock((HDC)hdc, tempblock, x, y);
   
  wfreeblock(tempblock);
}

void proportionalDraw (int newwid, int sprnum, int*newx, int*newy) {
  int newhit = newwid;

  int newsizx=newwid,newsizy=newhit;
  int twid=get_sprite(sprnum)->w,thit = get_sprite(sprnum)->h;
  if ((twid < newwid/2) && (thit < newhit/2)) {
    newsizx = twid * 2;
    newsizy = thit * 2;
  }
  else {
    if (twid >= thit) newsizy=(int)((float)thit/((float)twid/(float)newwid));
    else if (twid < thit) newsizx=(int)((float)twid/((float)thit/(float)newhit));
  }
  newx[0] = newsizx;
  newy[0] = newsizy;
}

static void doDrawViewLoop (int hdc, int numFrames, ViewFrame *frames, int x, int y, int size, int cursel) {
  int wtoDraw = size * numFrames;
  
  if ((numFrames > 0) && (frames[numFrames-1].pic == -1))
    wtoDraw -= size;

  block todraw = create_bitmap_ex (thisgame.color_depth*8, wtoDraw, size);
  clear_to_color (todraw, bitmap_mask_color (todraw));
  int neww, newh;
  for (int i = 0; i < numFrames; i++) {
    // don't draw the Go-To-Next-Frame jibble
    if (frames[i].pic == -1)
      break;
    // work out the dimensions to stretch to
    proportionalDraw (size, frames[i].pic, &neww, &newh);
    block toblt = get_sprite(frames[i].pic);
    bool freeBlock = false;
    if (bitmap_color_depth (toblt) != bitmap_color_depth (todraw)) {
      // 256-col sprite in hi-col game, we need to copy first
      block oldBlt = toblt;
      toblt = create_bitmap_ex (bitmap_color_depth (todraw), toblt->w, toblt->h);
      blit (oldBlt, toblt, 0, 0, 0, 0, oldBlt->w, oldBlt->h);
      freeBlock = true;
    }
    block flipped = NULL;
    if (frames[i].flags & VFLG_FLIPSPRITE) {
      // mirror the sprite
      flipped = create_bitmap_ex (bitmap_color_depth (todraw), toblt->w, toblt->h);
      clear_to_color (flipped, bitmap_mask_color (flipped));
      draw_sprite_h_flip (flipped, toblt, 0, 0);
      if (freeBlock)
        wfreeblock(toblt);
      toblt = flipped;
      freeBlock = true;
    }
    //stretch_sprite(toblt, todraw, 0, 0, toblt->w, toblt->h, size*i, 0, neww, newh);
	Cstretch_sprite(todraw, toblt, size*i, 0, neww, newh);
    if (freeBlock)
      wfreeblock (toblt);
    if (i < numFrames-1) {
      int linecol = makecol_depth(thisgame.color_depth * 8, 0, 64, 200);
      if (thisgame.color_depth == 1)
        linecol = 12;

      // Draw dividing line
      line (todraw, size*(i+1) - 1, 0, size*(i+1) - 1, size-1, linecol);
    }
    if (i == cursel) {
      // Selected item
      int linecol = makecol_depth(thisgame.color_depth * 8, 255, 255,255);
      if (thisgame.color_depth == 1)
        linecol = 14;
      
      rect (todraw, size * i, 0, size * (i+1) - 1, size-1, linecol);
    }
  }
  drawBlock ((HDC)hdc, todraw, x, y);
  wfreeblock(todraw);
}

int get_adjusted_spritewidth(int spr) {
  block tsp = get_sprite(spr);
  if (tsp == NULL) return 0;

  int retval = tsp->w;

  if (thisgame.spriteflags[spr] & SPF_640x400) {
    if (sxmult == 1)
      retval /= 2;
  }
  else {
    if (sxmult == 2)
      retval *= 2;
  }
  return retval;
}

int get_adjusted_spriteheight(int spr) {
  block tsp = get_sprite(spr);
  if (tsp == NULL) return 0;

  int retval = tsp->h;

  if (thisgame.spriteflags[spr] & SPF_640x400) {
    if (symult == 1)
      retval /= 2;
  }
  else {
    if (symult == 2)
      retval *= 2;
  }
  return retval;
}

void drawBlockOfColour(int hdc, int x,int y, int width, int height, int colNum)
{
	__my_setcolor(&colNum, colNum);
  /*if (thisgame.color_depth > 2) {
    // convert to 24-bit colour
    int red = ((colNum >> 11) & 0x1f) * 8;
    int grn = ((colNum >> 5) & 0x3f) * 4;
    int blu = (colNum & 0x1f) * 8;
    colNum = (red << _rgb_r_shift_32) | (grn << _rgb_g_shift_32) | (blu << _rgb_b_shift_32);
  }*/

  block palbmp = create_bitmap_ex(thisgame.color_depth * 8, width, height);
  clear_to_color (palbmp, colNum);
  drawBlockScaledAt(hdc, palbmp, x, y, 1);
  wfreeblock(palbmp);
}

void NewInteractionCommand::remove () 
{
}

void new_font () {
  wloadfont_size(thisgame.numfonts, 0);
  thisgame.fontflags[thisgame.numfonts] = 0;
  thisgame.fontoutline[thisgame.numfonts] = -1;
  thisgame.numfonts++;
}

bool initialize_native()
{
  set_uformat(U_ASCII);  // required to stop ALFONT screwing up text
	install_allegro(SYSTEM_NONE, &errno, atexit);
	//set_gdi_color_format();
	palette = &thisgame.defpal[0];
	thisgame.color_depth = 2;
	abuf = create_bitmap_ex(32, 10, 10);
	thisgame.numfonts = 0;
	new_font();

	spriteset.reset();
	if (spriteset.initFile(sprsetname))
	  return false;
	spriteset.maxCacheSize = 100000000;  // 100 mb cache

	if (!Scintilla_RegisterClasses (GetModuleHandle(NULL)))
      return false;

  init_font_renderer();

	return true;
}

void shutdown_native()
{
  shutdown_font_renderer();
	allegro_exit();
}

void drawBlockScaledAt (int hdc, block todraw ,int x, int y, int scaleFactor) {
  if (bitmap_color_depth (todraw) == 8)
    set_palette_to_hdc ((HDC)hdc, palette);

  stretch_blit_to_hdc (todraw, (HDC)hdc, 0,0,todraw->w,todraw->h,
    x,y,todraw->w * scaleFactor, todraw->h * scaleFactor);
}

void drawSprite(int hdc, int x, int y, int spriteNum, bool flipImage) {
	int scaleFactor = ((thisgame.spriteflags[spriteNum] & SPF_640x400) != 0) ? 1 : 2;
	block theSprite = get_sprite(spriteNum);

  if (theSprite == NULL)
    return;

	if (flipImage) {
		block flipped = create_bitmap_ex (bitmap_color_depth(theSprite), theSprite->w, theSprite->h);
		clear_to_color (flipped, bitmap_mask_color (flipped));
		draw_sprite_h_flip (flipped, theSprite, 0, 0);
		drawBlockScaledAt(hdc, flipped, x, y, scaleFactor);
		wfreeblock(flipped);
	}
	else 
	{
		drawBlockScaledAt(hdc, theSprite, x, y, scaleFactor);
	}
}

void drawSpriteStretch(int hdc, int x, int y, int width, int height, int spriteNum) {
  block todraw = get_sprite(spriteNum);
  block tempBlock = NULL;
	
  if (bitmap_color_depth (todraw) == 8)
    set_palette_to_hdc ((HDC)hdc, palette);

  int hdcBpp = GetDeviceCaps((HDC)hdc, BITSPIXEL);
  if (hdcBpp != bitmap_color_depth(todraw))
  {
	  tempBlock = create_bitmap_ex(hdcBpp, todraw->w, todraw->h);
	  blit(todraw, tempBlock, 0, 0, 0, 0, todraw->w, todraw->h);
	  todraw = tempBlock;
  }

  stretch_blit_to_hdc (todraw, (HDC)hdc, 0,0,todraw->w,todraw->h, x,y, width, height);

  if (tempBlock != NULL)
  {
	  destroy_bitmap(tempBlock);
  }
}

void drawGUIAt (int hdc, int x,int y,int x1,int y1,int x2,int y2, int scaleFactor) {

  if ((tempgui.wid < 1) || (tempgui.hit < 1))
    return;

  //update_font_sizes();

  if (scaleFactor == 1) {
    dsc_want_hires = 1;
  }

  block tempblock = create_bitmap_ex (thisgame.color_depth*8, tempgui.wid, tempgui.hit);
  clear_to_color(tempblock, bitmap_mask_color (tempblock));
  block abufWas = abuf;
  abuf = tempblock;

  tempgui.draw_at (0, 0);

  dsc_want_hires = 0;

  if (x1 >= 0) {
    rect (abuf, x1, y1, x2, y2, 14);
  }
  abuf = abufWas;

  drawBlockScaledAt (hdc, tempblock, x, y, scaleFactor);
  //drawBlockDoubleAt (hdc, tempblock, x, y);
  wfreeblock(tempblock);
}

#define SIMP_INDEX0  0
#define SIMP_TOPLEFT 1
#define SIMP_BOTLEFT 2
#define SIMP_TOPRIGHT 3
#define SIMP_BOTRIGHT 4
#define SIMP_LEAVEALONE 5
#define SIMP_NONE     6

void sort_out_transparency(block toimp, int sprite_import_method, color*itspal, bool useBgSlots, int importedColourDepth) 
{
  if (sprite_import_method == SIMP_LEAVEALONE)
    return;

  int uu,tt;
  wsetpalette(0,255,palette);
  int transcol=bitmap_mask_color(toimp);
  // NOTE: This takes the pixel from the corner of the overall import
  // graphic, NOT just the image to be imported
  if (sprite_import_method == SIMP_TOPLEFT)
    transcol=getpixel(toimp,0,0);
  else if (sprite_import_method==SIMP_BOTLEFT)
    transcol=getpixel(toimp,0,(toimp->h)-1);
  else if (sprite_import_method == SIMP_TOPRIGHT)
    transcol = getpixel(toimp, (toimp->w)-1, 0);
  else if (sprite_import_method == SIMP_BOTRIGHT)
    transcol = getpixel(toimp, (toimp->w)-1, (toimp->h)-1);

  if (sprite_import_method == SIMP_NONE)
  {
    // remove all transparency pixels (change them to
    // a close non-trnasparent colour)
    int changeTransparencyTo;
    if (transcol == 0)
      changeTransparencyTo = 16;
    else
      changeTransparencyTo = transcol - 1;

    for (tt=0;tt<toimp->w;tt++) {
      for (uu=0;uu<toimp->h;uu++) {
        if (getpixel(toimp,tt,uu) == transcol)
          putpixel(toimp,tt,uu, changeTransparencyTo);
      }
    }
  }
  else
  {
	  int bitmapMaskColor = bitmap_mask_color(toimp);
    int replaceWithCol = 16;
	  if (bitmap_color_depth(toimp) > 8)
	  {
      if (importedColourDepth == 8)
        replaceWithCol = makecol_depth(bitmap_color_depth(toimp), itspal[0].r * 4, itspal[0].g * 4, itspal[0].b * 4);
      else
		    replaceWithCol = 0;
	  }
    // swap all transparent pixels with index 0 pixels
    for (tt=0;tt<toimp->w;tt++) {
      for (uu=0;uu<toimp->h;uu++) {
        if (getpixel(toimp,tt,uu)==transcol)
          putpixel(toimp,tt,uu, bitmapMaskColor);
        else if (getpixel(toimp,tt,uu) == bitmapMaskColor)
          putpixel(toimp,tt,uu, replaceWithCol);
      }
    }
  }

  if ((thisgame.color_depth == 1) && (itspal != NULL)) { 
    // 256-colour mode only
    if (transcol!=0)
      itspal[transcol] = itspal[0];
    wsetrgb(0,0,0,0,itspal); // set index 0 to black
    __wremap_keep_transparent = 1;
    color oldpale[256];
    for (uu=0;uu<255;uu++) {
      if (useBgSlots)  //  use background scene palette
        oldpale[uu]=palette[uu];
      else if (thisgame.paluses[uu]==PAL_BACKGROUND)
        wsetrgb(uu,0,0,0,oldpale);
      else 
        oldpale[uu]=palette[uu];
    }
    wremap(itspal,toimp,oldpale); 
    wsetpalette(0,255,palette);
  }
  else if (bitmap_color_depth(toimp) == 8) {  // hi-colour game
    set_palette(itspal);
  }
}

void update_abuf_coldepth() {
  wfreeblock(abuf);
  abuf = create_bitmap_ex (thisgame.color_depth * 8, 10, 10);
}

bool reload_font(int curFont)
{
  wfreefont(curFont);

  int fsize = thisgame.fontflags[curFont] & FFLG_SIZEMASK;
  // if the font is designed for 640x400, half it
  if (thisgame.options[OPT_NOSCALEFNT]) {
    if (thisgame.default_resolution <= 2)
      fsize /= 2;
  }
  else if (thisgame.default_resolution >= 3) {
    // designed for 320x200, double it up
    fsize *= 2;
  }
  return wloadfont_size(curFont, fsize);
}

void load_script_modules_compiled(FILE *inn) {

  numScriptModules = getw(inn);
  scModules = (ScriptModule*)realloc(scModules, sizeof(ScriptModule) * numScriptModules);
  for (int i = 0; i < numScriptModules; i++) {
    scModules[i].init();
    scModules[i].compiled = fread_script(inn);
  }

}

void read_dialogs(FILE*iii, int filever, bool encrypted) {
  int bb;
  dialog = (DialogTopic*)malloc(sizeof(DialogTopic) * thisgame.numdialog);
  fread(&dialog[0],sizeof(DialogTopic),thisgame.numdialog,iii);
  for (bb=0;bb<thisgame.numdialog;bb++) {
    if (dialog[bb].optionscripts!=NULL) {
      dialog[bb].optionscripts=(unsigned char*)malloc(dialog[bb].codesize+10);
      fread(&dialog[bb].optionscripts[0],dialog[bb].codesize,1,iii);
    }
    int lenof=getw(iii);
    if (lenof<=1) { fgetc(iii);
      dlgscript[bb]=NULL;
      continue;
    }
    // add a large buffer because it will get added to if another option is added
    dlgscript[bb]=(char*)malloc(lenof + 20000);
    fread(dlgscript[bb],lenof,1,iii);
    if (encrypted)
      decrypt_text(dlgscript[bb]);
  }
  char stringbuffer[1000];
  for (bb=0;bb<thisgame.numdlgmessage;bb++) {
    if ((filever >= 26) && (encrypted))
      read_string_decrypt(iii, stringbuffer);
    else
      fgetstring(stringbuffer, iii);

    // don't actually do anything with the dlgmessage (it's an obsolete compiled artefact)
  }
}

bool reset_sprite_file() {
  spriteset.reset();
  if (spriteset.initFile(sprsetname))
    return false;
  spriteset.maxCacheSize = 100000000;  // 100 mb cache
  return true;
}

#define MAX_PLUGINS 40
struct PluginData 
{
	char filename[50];
	char data[SAVEBUFFERSIZE];
	int dataSize;
};
PluginData thisgamePlugins[MAX_PLUGINS];
int numThisgamePlugins = 0;

void write_plugins_to_disk (FILE *ooo) {
  int a;
  // version of plugin saving format
  putw (1, ooo);
  putw (numThisgamePlugins, ooo);
  
  for (a = 0; a < numThisgamePlugins; a++) {
      fputstring(thisgamePlugins[a].filename, ooo);
      
      int savesize = thisgamePlugins[a].dataSize;
      
      if ((savesize > SAVEBUFFERSIZE) || (savesize < 0)) {
		  MessageBox(NULL, "Plugin tried to write too much data to game file.", "", MB_OK);
        savesize = 0;
      }

      putw (savesize, ooo);
      if (savesize > 0)
        fwrite (&thisgamePlugins[a].data[0], savesize, 1, ooo);
  }
}

const char * read_plugins_from_disk (FILE *iii) {
  if (getw(iii) != 1) {
    return "ERROR: unable to load game, invalid version of plugin data";
  }

  numThisgamePlugins = getw(iii);

  for (int a = 0; a < numThisgamePlugins; a++) {
    // read the plugin name
    fgetstring (thisgamePlugins[a].filename, iii);
    int datasize = getw(iii);
    if (datasize > SAVEBUFFERSIZE) {
      return "Invalid plugin save data format, plugin data is lost";
    }
    // we don't care if it's an editor-only plugin or not
    if (thisgamePlugins[a].filename[strlen(thisgamePlugins[a].filename) - 1] == '!')
		thisgamePlugins[a].filename[strlen(thisgamePlugins[a].filename) - 1] = 0;

	thisgamePlugins[a].dataSize = datasize;
	if (datasize > 0)
	  fread (thisgamePlugins[a].data, datasize, 1, iii);
  }
  return NULL;
}

void allocate_memory_for_views(int viewCount)
{
  numNewViews = 0;
	oldViews = (ViewStruct272*)calloc(sizeof(ViewStruct272) * viewCount, 1);
  newViews = (ViewStruct*)calloc(sizeof(ViewStruct) * viewCount, 1);
  thisgame.viewNames = (char**)malloc(sizeof(char*) * viewCount);
  thisgame.viewNames[0] = (char*)calloc(MAXVIEWNAMELENGTH * viewCount, 1);

  for (int i = 1; i < viewCount; i++)
  {
    thisgame.viewNames[i] = thisgame.viewNames[0] + (MAXVIEWNAMELENGTH * i);
  }
}

const char *load_dta_file_into_thisgame(const char *fileName)
{
  int bb;
  FILE*iii=fopen(fileName, "rb");
  if (iii == NULL)
    return "Unable to open file";

  char buffer[40];
  fread(buffer,30,1,iii); 
  buffer[30]=0;
  if (strcmp(buffer,game_file_sig)!=0) {
    fclose(iii);
    return "File contains invalid data and is not an AGS game.";
  }
  int filever = _getw(iii);
  if (filever != 32) 
  {
	  fclose(iii);
	  return "This game was saved by an old version of AGS. This version of the editor can only import games saved with AGS 2.72.";
  }

  // skip required engine version
  int stlen = _getw(iii);
  fseek(iii, stlen, SEEK_CUR);

  fread(&thisgame, sizeof (GameSetupStructBase), 1, iii);
  fread(&thisgame.fontflags[0], 1, thisgame.numfonts, iii);
  fread(&thisgame.fontoutline[0], 1, thisgame.numfonts, iii);

  int numSprites = _getw(iii);
  memset(&thisgame.spriteflags[0], 0, MAX_SPRITES);
  fread(&thisgame.spriteflags[0], 1, numSprites, iii);
  fread(&thisgame.invinfo[0], sizeof(InventoryItemInfo), thisgame.numinvitems, iii);
  fread(&thisgame.mcurs[0], sizeof(MouseCursor), thisgame.numcursors, iii);

  thisgame.intrChar = (NewInteraction**)calloc(thisgame.numcharacters, sizeof(NewInteraction*));
  for (bb = 0; bb < thisgame.numcharacters; bb++) {
    thisgame.intrChar[bb] = deserialize_new_interaction (iii);
  }
  for (bb = 0; bb < thisgame.numinvitems; bb++) {
    delete thisgame.intrInv[bb];
    thisgame.intrInv[bb] = deserialize_new_interaction (iii);
  }

  numGlobalVars = getw(iii);
  fread (&globalvars[0], sizeof (InteractionVariable), numGlobalVars, iii);

  if (thisgame.dict != NULL) {
    thisgame.dict = (WordsDictionary*)malloc(sizeof(WordsDictionary));
    read_dictionary (thisgame.dict, iii);
  }

  thisgame.globalscript = NULL;

  if (thisgame.compiled_script != NULL)
    thisgame.compiled_script = fread_script(iii);

  load_script_modules_compiled(iii);

  allocate_memory_for_views(thisgame.numviews);
  fread (&oldViews[0], sizeof(ViewStruct272), thisgame.numviews, iii);

  thisgame.chars = (CharacterInfo*)calloc(sizeof(CharacterInfo) * thisgame.numcharacters, 1);
  fread(&thisgame.chars[0],sizeof(CharacterInfo),thisgame.numcharacters,iii);

  fread (&thisgame.lipSyncFrameLetters[0][0], 20, 50, iii);

  for (bb=0;bb<MAXGLOBALMES;bb++) {
    if (thisgame.messages[bb]==NULL) continue;
    thisgame.messages[bb]=(char*)malloc(500);
    read_string_decrypt(iii, thisgame.messages[bb]);
  }

  read_dialogs(iii, filever, true);
  read_gui(iii,&guis[0],&thisgame, &guis);
  const char *pluginError = read_plugins_from_disk (iii);
  if (pluginError != NULL) return pluginError;

  thisgame.charProps = (CustomProperties*)calloc(thisgame.numcharacters, sizeof(CustomProperties));

  for (bb = 0; bb < thisgame.numcharacters; bb++)
    thisgame.charProps[bb].reset();
  for (bb = 0; bb < MAX_INV; bb++)
    thisgame.invProps[bb].reset();

  if (thisgame.propSchema.UnSerialize (iii))
    return "unable to deserialize prop schema";

  int errors = 0;

  for (bb = 0; bb < thisgame.numcharacters; bb++)
    errors += thisgame.charProps[bb].UnSerialize (iii);
  for (bb = 0; bb < thisgame.numinvitems; bb++)
    errors += thisgame.invProps[bb].UnSerialize (iii);

  if (errors > 0)
    return "errors encountered reading custom props";

  for (bb = 0; bb < thisgame.numviews; bb++)
    fgetstring_limit(thisgame.viewNames[bb], iii, MAXVIEWNAMELENGTH);

  for (bb = 0; bb < thisgame.numinvitems; bb++)
    fgetstring_limit(thisgame.invScriptNames[bb], iii, MAX_SCRIPT_NAME_LEN);

  for (bb = 0; bb < thisgame.numdialog; bb++)
    fgetstring_limit(thisgame.dialogScriptNames[bb], iii, MAX_SCRIPT_NAME_LEN);

  fclose(iii);

  for (bb = 0; bb < thisgame.numgui; bb++)
  {
	  guis[bb].rebuild_array();
  }

  // reset colour 0, it's possible for it to get corrupted
  palette[0].r = 0;
  palette[0].g = 0;
  palette[0].b = 0;
  wsetpalette(0,255,palette);

  if (!reset_sprite_file())
    return "The sprite file could not be loaded. Ensure that all your game files are intact and not corrupt. The game may require a newer version of AGS.";

  for (bb=0;bb<MAX_FONTS;bb++) {
    wfreefont(bb);
  }
  for (bb=0;bb<thisgame.numfonts;bb++) {
    reload_font (bb);
  }

  update_abuf_coldepth();
  spritesModified = false;

  thisgame.filever = filever;
  return NULL;
}

void free_script_module(int index) {
  free(scModules[index].name);
  free(scModules[index].author);
  free(scModules[index].version);
  free(scModules[index].description);
  free(scModules[index].script);
  free(scModules[index].scriptHeader);
  if (scModules[index].compiled != NULL)
    ccFreeScript(scModules[index].compiled);
}

void free_script_modules() {
  for (int i = 0; i < numScriptModules; i++)
    free_script_module(i);

  numScriptModules = 0;
}

void free_old_game_data()
{
  int bb;
  for (bb=0;bb<MAXGLOBALMES;bb++) {
    if (thisgame.messages[bb] != NULL)
      free(thisgame.messages[bb]);
  }
  for (bb = 0; bb < thisgame.numdialog; bb++) 
  {
	  if (dialog[bb].optionscripts != NULL)
		  free(dialog[bb].optionscripts);
  }
  if (thisgame.charProps != NULL)
  {
    for (bb = 0; bb < thisgame.numcharacters; bb++)
      thisgame.charProps[bb].reset();
    free(thisgame.charProps);
    thisgame.charProps = NULL;
  }
  if (thisgame.intrChar != NULL)
  {
    for (bb = 0; bb < thisgame.numcharacters; bb++)
      delete thisgame.intrChar[bb];
    
    free(thisgame.intrChar);
    thisgame.intrChar = NULL;
  }
  for (bb = 0; bb < numNewViews; bb++)
  {
    for (int cc = 0; cc < newViews[bb].numLoops; cc++)
    {
      newViews[bb].loops[cc].Dispose();
    }
    newViews[bb].Dispose();
  }
  free(thisgame.viewNames[0]);
  free(thisgame.viewNames);
  free(oldViews);
  free(newViews);
  free(guis);
  free(thisgame.chars);
  thisgame.dict->free_memory();
  free(thisgame.dict);
  free(dialog);
  free_script_modules();
}

// remap the scene, from its current palette oldpale to palette
void remap_background (block scene, color *oldpale, color*palette, int exactPal) {
  int a;  

  if (exactPal) {
    // exact palette import (for doing palette effects, don't change)
    for (a=0;a<256;a++) 
    {
      if (thisgame.paluses[a] == PAL_BACKGROUND)
      {
        palette[a] = oldpale[a];
      }
    }
    return;
  }

  // find how many slots there are reserved for backgrounds
  int numbgslots=0;
  for (a=0;a<256;a++) { oldpale[a].filler=0;
    if (thisgame.paluses[a]!=PAL_GAMEWIDE) numbgslots++;
  }
  // find which colours from the image palette are actually used
  int imgpalcnt[256],numimgclr=0;
  memset(&imgpalcnt[0],0,sizeof(int)*256);
  if (is_linear_bitmap(scene)==0)
    quit("mem bitmap non-linear?");

  for (a=0;a<(scene->w) * (scene->h);a++) {
    imgpalcnt[scene->line[0][a]]++;
  }
  for (a=0;a<256;a++) {
    if (imgpalcnt[a]>0) numimgclr++;
  }
  // count up the number of unique colours in the image
  int numclr=0,bb;
  color tpal[256];
  for (a=0;a<256;a++) {
    if (thisgame.paluses[a]==PAL_BACKGROUND)
      wsetrgb(a,0,0,0,palette);  // black out the bg slots before starting
    if ((oldpale[a].r==0) & (oldpale[a].g==0) & (oldpale[a].b==0)) {
      imgpalcnt[a]=0;
      continue;
    }
    for (bb=0;bb<numclr;bb++) {
      if ((oldpale[a].r==tpal[bb].r) &
        (oldpale[a].g==tpal[bb].g) &
        (oldpale[a].b==tpal[bb].b)) bb=1000;
    }
    if (bb>300) { 
      imgpalcnt[a]=0;
      continue;
    }
    if (imgpalcnt[a]==0)
      continue;
    tpal[numclr]=oldpale[a];
    numclr++;
  }
  if (numclr>numbgslots) {
    MessageBox(NULL, "WARNING: This image uses more colours than are allocated to backgrounds. Some colours will be lost.", "Warning", MB_OK);
  }

  // fill the background slots in the palette with the colours
  int palslt=255;  // start from end of palette and work backwards
  for (a=0;a<numclr;a++) {
    while (thisgame.paluses[palslt]!=PAL_BACKGROUND) {
      palslt--;
      if (palslt<0) break;
    }
    if (palslt<0) break;
    palette[palslt]=tpal[a];
    palslt--;
    if (palslt<0) break;
  }
  // blank out the sprite colours, then remap the picture
  for (a=0;a<256;a++) {
    if (thisgame.paluses[a]==PAL_GAMEWIDE) {
      tpal[a].r=0;
      tpal[a].g=0; tpal[a].b=0; 
    }
    else tpal[a]=palette[a];
  }
  wremapall(oldpale,scene,tpal); //palette);
}

void validate_mask(block toValidate, const char *name, int maxColour) {
  if ((toValidate == NULL) || (bitmap_color_depth(toValidate) != 8) ||
      (!is_memory_bitmap(toValidate))) {
    quit("Invalid mask passed to validate_mask");
    return;
  }

  bool errFound = false;
  int xx, yy;
  for (yy = 0; yy < toValidate->h; yy++) {
    for (xx = 0; xx < toValidate->w; xx++) {
      if (toValidate->line[yy][xx] >= maxColour) {
        errFound = true;
        toValidate->line[yy][xx] = 0;
      }
    }
  }

  if (errFound) {
	char errorBuf[1000];
    sprintf(errorBuf, "Invalid colours were found in the %s mask. They have now been removed."
       "\n\nWhen drawing a mask in an external paint package, you need to make "
       "sure that the image is set as 256-colour (Indexed Palette), and that "
       "you use the first 16 colours in the palette for drawing your areas. Palette "
       "entry 0 corresponds to No Area, palette index 1 corresponds to area 1, and "
       "so forth.", name);
	MessageBox(NULL, errorBuf, "Mask Error", MB_OK);
    roomModified = true;
  }
}

void copy_room_palette_to_global_palette()
{
  for (int ww = 0; ww < 256; ww++) 
  {
    if (thisgame.paluses[ww] == PAL_BACKGROUND)
    {
      thisroom.pal[ww] = thisroom.bpalettes[0][ww];
      palette[ww] = thisroom.bpalettes[0][ww];
    }
  }
}

void copy_global_palette_to_room_palette()
{
  for (int ww = 0; ww < 256; ww++) 
  {
    if (thisgame.paluses[ww] != PAL_BACKGROUND)
      thisroom.bpalettes[0][ww] = palette[ww];
  }
}

const char* load_room_file(const char*rtlo) {

  load_room((char*)rtlo, &thisroom, (thisgame.default_resolution > 2));

  if (thisroom.wasversion < 17) 
  {
	  return "This room was saved with an old version of the editor and cannot be opened. Use AGS 2.72 to upgrade this room file.";
  }

  //thisroom.numhotspots = MAX_HOTSPOTS;

  // Allocate enough memory to add extra variables
  InteractionVariable *ivv = (InteractionVariable*)malloc (sizeof(InteractionVariable) * MAX_GLOBAL_VARIABLES);
  if (thisroom.numLocalVars > 0) {
    memcpy (ivv, thisroom.localvars, sizeof(InteractionVariable) * thisroom.numLocalVars);
    free (thisroom.localvars);
  }
  thisroom.localvars = ivv;

  // Update room palette with gamewide colours
  copy_global_palette_to_room_palette();
  // Update current global palette with room background colours
  copy_room_palette_to_global_palette();
  int ww;
  for (ww = 0; ww < thisroom.numsprs; ww++) {
    // change invalid objects to blue cup
    if (spriteset[thisroom.sprs[ww].sprnum] == NULL)
      thisroom.sprs[ww].sprnum = 0;
  }
  // Fix hi-color screens
  for (ww = 0; ww < thisroom.num_bscenes; ww++)
    fix_block (thisroom.ebscene[ww]);

  if ((thisroom.resolution > 1) && (thisroom.object->w < thisroom.width)) {
    // 640x400 room with 320x200-res walkbehind
    // resize it up to 640x400-res
    int oldw = thisroom.object->w, oldh=thisroom.object->h;
    block tempb = create_bitmap_ex(bitmap_color_depth(thisroom.object), thisroom.width, thisroom.height);
    clear(tempb);
    stretch_blit(thisroom.object,tempb,0,0,oldw,oldh,0,0,tempb->w,tempb->h);
    destroy_bitmap(thisroom.object); 
    thisroom.object = tempb;
  }

  wsetpalette(0,255,palette);
  
  if ((bitmap_color_depth (thisroom.ebscene[0]) > 8) &&
      (thisgame.color_depth == 1))
    MessageBox(NULL,"WARNING: This room is hi-color, but your game is currently 256-colour. You will not be able to use this room in this game. Also, the room background will not look right in the editor.", "Colour depth warning", MB_OK);

  roomModified = false;

  validate_mask(thisroom.lookat, "hotspot", MAX_HOTSPOTS);
  validate_mask(thisroom.object, "walk-behind", MAX_WALK_AREAS + 1);
  validate_mask(thisroom.walls, "walkable area", MAX_WALK_AREAS + 1);
  validate_mask(thisroom.regions, "regions", MAX_REGIONS);
  return NULL;
}

void calculate_walkable_areas () {
  int ww, thispix;

  for (ww = 0; ww <= MAX_WALK_AREAS; ww++) {
    thisroom.walk_area_top[ww] = thisroom.height;
    thisroom.walk_area_bottom[ww] = 0;
  }
  for (ww = 0; ww < thisroom.walls->w; ww++) {
    for (int qq = 0; qq < thisroom.walls->h; qq++) {
      thispix = _getpixel (thisroom.walls, ww, qq);
      if (thispix > MAX_WALK_AREAS)
        continue;
      if (thisroom.walk_area_top[thispix] > qq)
        thisroom.walk_area_top[thispix] = qq;
      if (thisroom.walk_area_bottom[thispix] < qq)
        thisroom.walk_area_bottom[thispix] = qq;
    }
  }

}

// Note: we don't use GETW or PUTW for compatibility between 16-bit and
// 32-bit versions; hence all the FREAD/FWRITEs
void save_room(const char *files, roomstruct rstruc) {
  int               f;
  long              xoff, tesl;
  FILE              *opty;
  room_file_header  rfh;

  if (rstruc.wasversion < ROOM_FILE_VERSION)
    quit("save_room: can no longer save old format rooms");

  if (rstruc.wasversion < 9) {
    for (f = 0; f < 11; f++)
      rstruc.password[f] -= 60;
  }
  else
    for (f = 0; f < 11; f++)
      rstruc.password[f] -= passwencstring[f];

  opty = ci_fopen(files, "wb");
  if (opty == NULL)
    quit("save_room: unable to open room file for writing.");

  rfh.version = rstruc.wasversion; //ROOM_FILE_VERSION;
  fwrite(&rfh,sizeof(room_file_header),1,opty);

  if (rfh.version >= 5) {
    long blsii = 0;

    fputc(BLOCKTYPE_MAIN, opty);
    fwrite(&blsii, 4, 1, opty);
  }

  putw(rstruc.bytes_per_pixel, opty);  // colour depth bytes per pixel
  fwrite(&rstruc.numobj, 2, 1, opty);
  fwrite(&rstruc.objyval[0], 2, rstruc.numobj, opty);

  fwrite(&rstruc.numhotspots, sizeof(int), 1, opty);
  fwrite(&rstruc.hswalkto[0], sizeof(_Point), rstruc.numhotspots, opty);
  for (f = 0; f < rstruc.numhotspots; f++)
  {
	  fputstring(rstruc.hotspotnames[f], opty);
  }

  if (rfh.version >= 24)
    fwrite(&rstruc.hotspotScriptNames[0], MAX_SCRIPT_NAME_LEN, rstruc.numhotspots, opty);

  fwrite(&rstruc.numwalkareas, 4, 1, opty);
  fwrite(&rstruc.wallpoints[0], sizeof(PolyPoints), rstruc.numwalkareas, opty);

  fwrite(&rstruc.top, 2, 1, opty);
  fwrite(&rstruc.bottom, 2, 1, opty);
  fwrite(&rstruc.left, 2, 1, opty);
  fwrite(&rstruc.right, 2, 1, opty);
  fwrite(&rstruc.numsprs, 2, 1, opty);
  fwrite(&rstruc.sprs[0], sizeof(sprstruc), rstruc.numsprs, opty);

  putw (rstruc.numLocalVars, opty);
  if (rstruc.numLocalVars > 0) 
    fwrite (&rstruc.localvars[0], sizeof(InteractionVariable), rstruc.numLocalVars, opty);
/*
  for (f = 0; f < rstruc.numhotspots; f++)
    serialize_new_interaction (rstruc.intrHotspot[f], opty);
  for (f = 0; f < rstruc.numsprs; f++)
    serialize_new_interaction (rstruc.intrObject[f], opty);
  serialize_new_interaction (rstruc.intrRoom, opty);
*/
  putw (MAX_REGIONS, opty);
  /*
  for (f = 0; f < MAX_REGIONS; f++)
    serialize_new_interaction (rstruc.intrRegion[f], opty);
	*/
  serialize_room_interactions(opty);

  fwrite(&rstruc.objbaseline[0], sizeof(int), rstruc.numsprs, opty);
  fwrite(&rstruc.width, 2, 1, opty);
  fwrite(&rstruc.height, 2, 1, opty);

  if (rfh.version >= 23)
    fwrite(&rstruc.objectFlags[0], sizeof(short), rstruc.numsprs, opty);

  if (rfh.version >= 11)
    fwrite(&rstruc.resolution,2,1,opty);

  // write the zoom and light levels
  putw (MAX_WALK_AREAS + 1, opty);
  fwrite(&rstruc.walk_area_zoom[0], sizeof(short), MAX_WALK_AREAS + 1, opty);
  fwrite(&rstruc.walk_area_light[0], sizeof(short), MAX_WALK_AREAS + 1, opty);
  fwrite(&rstruc.walk_area_zoom2[0], sizeof(short), MAX_WALK_AREAS + 1, opty);
  fwrite(&rstruc.walk_area_top[0], sizeof(short), MAX_WALK_AREAS + 1, opty);
  fwrite(&rstruc.walk_area_bottom[0], sizeof(short), MAX_WALK_AREAS + 1, opty);

  fwrite(&rstruc.password[0], 11, 1, opty);
  fwrite(&rstruc.options[0], 10, 1, opty);
  fwrite(&rstruc.nummes, 2, 1, opty);

  if (rfh.version >= 25)
    putw(rstruc.gameId, opty);
 
  if (rfh.version >= 3)
    fwrite(&rstruc.msgi[0], sizeof(MessageInfo), rstruc.nummes, opty);

  for (f = 0; f < rstruc.nummes; f++)
    write_string_encrypt(opty, rstruc.message[f]);
//    fputstring(rstruc.message[f], opty);

  if (rfh.version >= 6) {
    // we no longer use animations, remove them
    rstruc.numanims = 0;
    fwrite(&rstruc.numanims, 2, 1, opty);

    if (rstruc.numanims > 0)
      fwrite(&rstruc.anims[0], sizeof(FullAnimation), rstruc.numanims, opty);
  }

  if ((rfh.version >= 4) && (rfh.version < 16)) {
    save_script_configuration(opty);
    save_graphical_scripts(opty, &rstruc);
  }

  if (rfh.version >= 8)
    fwrite(&rstruc.shadinginfo[0], sizeof(short), 16, opty);

  if (rfh.version >= 21) {
    fwrite(&rstruc.regionLightLevel[0], sizeof(short), MAX_REGIONS, opty);
    fwrite(&rstruc.regionTintLevel[0], sizeof(int), MAX_REGIONS, opty);
  }

  xoff = ftell(opty);
  fclose(opty);

  tesl = save_lzw((char*)files, rstruc.ebscene[0], rstruc.pal, xoff);

  tesl = savecompressed_allegro((char*)files, rstruc.regions, rstruc.pal, tesl);
  tesl = savecompressed_allegro((char*)files, rstruc.walls, rstruc.pal, tesl);
  tesl = savecompressed_allegro((char*)files, rstruc.object, rstruc.pal, tesl);
  tesl = savecompressed_allegro((char*)files, rstruc.lookat, rstruc.pal, tesl);

  if (rfh.version >= 5) {
    long  lee;

    opty = ci_fopen(files,"r+b");
    lee = filelength(fileno(opty))-7;

    fseek(opty, 3, SEEK_SET);
    fwrite(&lee, 4, 1, opty);
    fseek(opty, 0, SEEK_END);

    if (rstruc.scripts != NULL) {
      int hh;

      fputc(BLOCKTYPE_SCRIPT, opty);
      lee = (int)strlen(rstruc.scripts) + 4;
      fwrite(&lee, 4, 1, opty);
      lee -= 4;

      for (hh = 0; hh < lee; hh++)
        rstruc.scripts[hh]-=passwencstring[hh % 11];

      fwrite(&lee, 4, 1, opty);
      fwrite(rstruc.scripts, lee, 1, opty);

      for (hh = 0; hh < lee; hh++)
        rstruc.scripts[hh]+=passwencstring[hh % 11];

    }
   
    if (rstruc.compiled_script != NULL) {
      long  leeat, wasat;

      fputc(BLOCKTYPE_COMPSCRIPT3, opty);
      lee = 0;
      leeat = ftell(opty);
      fwrite(&lee, 4, 1, opty);
      fwrite_script(rstruc.compiled_script, opty);
     
      wasat = ftell(opty);
      fseek(opty, leeat, SEEK_SET);
      lee = (wasat - leeat) - 4;
      fwrite(&lee, 4, 1, opty);
      fseek(opty, 0, SEEK_END);
    }

    if (rstruc.numsprs > 0) {
      fputc(BLOCKTYPE_OBJECTNAMES, opty);
      lee=rstruc.numsprs * MAXOBJNAMELEN + 1;
      fwrite(&lee, 4, 1, opty);
      fputc(rstruc.numsprs, opty);
      fwrite(&rstruc.objectnames[0][0], MAXOBJNAMELEN, rstruc.numsprs, opty);

      fputc(BLOCKTYPE_OBJECTSCRIPTNAMES, opty);
      lee = rstruc.numsprs * MAX_SCRIPT_NAME_LEN + 1;
      fwrite(&lee, 4, 1, opty);
      fputc(rstruc.numsprs, opty);
      fwrite(&rstruc.objectscriptnames[0][0], MAX_SCRIPT_NAME_LEN, rstruc.numsprs, opty);
    }

    long lenpos, lenis;
    int gg;

    if (rstruc.num_bscenes > 1) {
      long  curoffs;

      fputc(BLOCKTYPE_ANIMBKGRND, opty);
      lenpos = ftell(opty);
      lenis = 0;
      fwrite(&lenis, 4, 1, opty);
      fputc(rstruc.num_bscenes, opty);
      fputc(rstruc.bscene_anim_speed, opty);
      
      fwrite (&rstruc.ebpalShared[0], 1, rstruc.num_bscenes, opty);

      fclose(opty);

      curoffs = lenpos + 6 + rstruc.num_bscenes;
      for (gg = 1; gg < rstruc.num_bscenes; gg++)
        curoffs = save_lzw((char*)files, rstruc.ebscene[gg], rstruc.bpalettes[gg], curoffs);

      opty = ci_fopen(files, "r+b");
      lenis = (curoffs - lenpos) - 4;
      fseek(opty, lenpos, SEEK_SET);
      fwrite(&lenis, 4, 1, opty);
      fseek(opty, 0, SEEK_END);
    }

    // Write custom properties
    fputc (BLOCKTYPE_PROPERTIES, opty);
    lenpos = ftell(opty);
    lenis = 0;
    fwrite(&lenis, 4, 1, opty);
    putw (1, opty);  // Version 1 of properties block
    rstruc.roomProps.Serialize (opty);
    for (gg = 0; gg < rstruc.numhotspots; gg++)
      rstruc.hsProps[gg].Serialize (opty);
    for (gg = 0; gg < rstruc.numsprs; gg++)
      rstruc.objProps[gg].Serialize (opty);

    lenis = (ftell(opty) - lenpos) - 4;
    fseek(opty, lenpos, SEEK_SET);
    fwrite(&lenis, 4, 1, opty);
    fseek(opty, 0, SEEK_END);


    // Write EOF block
    fputc(BLOCKTYPE_EOF, opty);
    fclose(opty);
  }
 
  if (rfh.version < 9) {
    for (f = 0; f < 11; f++)
      rstruc.password[f]+=60;
  }
  else
    for (f = 0; f < 11; f++)
      rstruc.password[f] += passwencstring[f];

//  fclose(opty);
//  return SUCCESS;
}

void save_room_file(const char*rtsa) 
{
  thisroom.wasversion=ROOM_FILE_VERSION;
  copy_room_palette_to_global_palette();
  
  thisroom.password[0] = 0;

  calculate_walkable_areas();

  thisroom.bytes_per_pixel = bmp_bpp(thisroom.ebscene[0]);
  int ww;
  // Fix hi-color screens
  for (ww = 0; ww < thisroom.num_bscenes; ww++)
    fix_block (thisroom.ebscene[ww]);

  thisroom.numobj = MAX_OBJ;
  save_room((char*)rtsa,thisroom);

  // Fix hi-color screens back again
  for (ww = 0; ww < thisroom.num_bscenes; ww++)
    fix_block (thisroom.ebscene[ww]);
}



// ****** CLIB MAKER **** //

#define MAX_FILES 10000
#define MAXMULTIFILES 25
#define MAX_FILENAME_LENGTH 100
#define MAX_DATAFILENAME_LENGTH 50
struct MultiFileLibNew {
  char data_filenames[MAXMULTIFILES][MAX_DATAFILENAME_LENGTH];
  int  num_data_files;
  char filenames[MAX_FILES][MAX_FILENAME_LENGTH];
  long offset[MAX_FILES];
  long length[MAX_FILES];
  char file_datafile[MAX_FILES];  // number of datafile
  int  num_files;
  };
MultiFileLibNew ourlib;
static const char *tempSetting = "My\x1\xde\x4Jibzle";  // clib password
extern void init_pseudo_rand_gen(int seed);
extern int get_pseudo_rand();
const int RAND_SEED_SALT = 9338638;  // must update clib32.cpp if this changes

void fwrite_data_enc(const void *data, int dataSize, int dataCount, FILE *ooo)
{
  const unsigned char *dataChar = (const unsigned char*)data;
  for (int i = 0; i < dataSize * dataCount; i++)
  {
    fputc(dataChar[i] + get_pseudo_rand(), ooo);
  }
}

void fputstring_enc(const char *sss, FILE *ooo) 
{
  fwrite_data_enc(sss, 1, strlen(sss) + 1, ooo);
}

void putw_enc(int numberToWrite, FILE *ooo)
{
  fwrite_data_enc(&numberToWrite, 4, 1, ooo);
}

void write_clib_header(FILE*wout) {
  int ff;
  int randSeed = (int)time(NULL);
  putw(randSeed - RAND_SEED_SALT, wout);
  init_pseudo_rand_gen(randSeed);
  putw_enc(ourlib.num_data_files,wout);
  for (ff = 0; ff < ourlib.num_data_files; ff++)
  {
    fputstring_enc(ourlib.data_filenames[ff], wout);
  }
  putw_enc(ourlib.num_files, wout);
  for (ff = 0; ff < ourlib.num_files; ff++) 
  {
    fputstring_enc(ourlib.filenames[ff], wout);
  }
  fwrite_data_enc(&ourlib.offset[0],4,ourlib.num_files,wout);
  fwrite_data_enc(&ourlib.length[0],4,ourlib.num_files,wout);
  fwrite_data_enc(&ourlib.file_datafile[0],1,ourlib.num_files,wout);
}


#define CHUNKSIZE 256000
int copy_file_across(FILE*inlibb,FILE*coppy,long leftforthis) {
  int success = 1;
  char*diskbuffer=(char*)malloc(CHUNKSIZE+10);
  while (leftforthis>0) {
    if (leftforthis>CHUNKSIZE) {
      fread(diskbuffer,CHUNKSIZE,1,inlibb);
      success = fwrite(diskbuffer,CHUNKSIZE,1,coppy);
      leftforthis-=CHUNKSIZE;
    }
    else {
      fread(diskbuffer,leftforthis,1,inlibb);
      success = fwrite(diskbuffer,leftforthis,1,coppy);
      leftforthis=0;
    }
    if (success < 1)
      break;
  }
  free(diskbuffer);
  return success;
}

const char* make_old_style_data_file(const char* dataFileName, int numfile, char * const*filenames)
{
  const char *errorMsg = NULL;
  int a;
  int passwmod = 20;
  long *filesizes = (long*)malloc(4*numfile);
  char**writefname = (char**)malloc(4*numfile);
  writefname[0]=(char*)malloc(14*numfile);

  for (a=0;a<numfile;a++) {
    if (a>0) writefname[a]=&writefname[0][a*13];
	if (strrchr(filenames[a], '\\') != NULL)
		strcpy(writefname[a], strrchr(filenames[a], '\\') + 1);
	else if (strrchr(filenames[a], '/') != NULL)
		strcpy(writefname[a], strrchr(filenames[a], '/') + 1);
	else
		strcpy(writefname[a],filenames[a]);

	if (strlen(writefname[a]) > 12)
    {
		char buffer[500];
		sprintf(buffer, "Filename too long: %s", writefname[a]);
		free(filesizes);
		free(writefname);
		ThrowManagedException(buffer);
    }

    FILE*ddd = fopen(filenames[a],"rb");
    if (ddd==NULL) { 
      filesizes[a] = 0;
      continue;
    }
    filesizes[a] = _filelength(_fileno(ddd));
    fclose(ddd);

    for (int bb = 0; writefname[a][bb] != 0; bb++)
      writefname[a][bb] += passwmod;
  }
  // write the header
  FILE*wout=fopen(dataFileName, "wb");
  fwrite("CLIB\x1a",5,1,wout);
  fputc(6,wout);  // version
  fputc(passwmod,wout);  // password modifier
  fputc(0,wout);  // reserved
  fwrite(&numfile,2,1,wout);
  for (a=0;a<13;a++) fputc(0,wout);  // the password
  fwrite(&writefname[0][0],13,numfile,wout);
  fwrite(&filesizes[0],4,numfile,wout);
  for (a=0;a<2*numfile;a++) fputc(0,wout);  // comp.ratio

  // now copy the data
  for (a=0;a<numfile;a++) {

	FILE*iii = fopen(filenames[a],"rb");
    if (iii==NULL) {
      errorMsg = "unable to add one of the files to data file.";
      continue;
    }
    if (copy_file_across(iii,wout,filesizes[a]) < 1) {
      errorMsg = "Error writing file: possibly disk full";
      fclose(iii);
      break;
    }
    fclose(iii);
  }
  fclose(wout);
  free(filesizes);
  free(writefname[0]);
  free(writefname);

  if (errorMsg != NULL) 
  {
	unlink(dataFileName);
  }

  return errorMsg;
}

FILE* find_file_in_path(char *buffer, const char *fileName)
{
	char tomake[MAX_PATH];
	strcpy(tomake, fileName);
	FILE* iii = clibfopen(tomake, "rb");
	if (iii == NULL) {
	  // try in the Audio folder if not found
	  sprintf(tomake, "AudioCache\\%s", fileName);
	  iii = clibfopen(tomake, "rb");
	}
	if (iii == NULL) {
	  // no? maybe Speech then, templates include this
	  sprintf(tomake, "Speech\\%s", fileName);
	  iii = clibfopen(tomake, "rb");
	}

	if (buffer != NULL)
	  strcpy(buffer, tomake);

	return iii;
}

const char* make_data_file(int numFiles, char * const*fileNames, long splitSize, const char *baseFileName, bool makeFileNameAssumptionsForEXE)
{
  int a,b;
  FILE*wout;
  char tomake[MAX_PATH];
  ourlib.num_data_files = 0;
  ourlib.num_files = numFiles;
  cfopenpriority = 2;

  int currentDataFile = 0;
  long sizeSoFar = 0;
  bool doSplitting = false;

  for (a = 0; a < numFiles; a++)
  {
	  if (splitSize > 0)
	  {
		  if (stricmp(fileNames[a], sprsetname) == 0) 
		  {
			  // the sprite file's appearance signifies it's time to start splitting
			  doSplitting = true;
			  currentDataFile++;
			  sizeSoFar = 0;
		  }
		  else if ((sizeSoFar > splitSize) && (doSplitting) && 
			  (currentDataFile < MAXMULTIFILES - 1))
		  {
			  currentDataFile++;
			  sizeSoFar = 0;
		  }
	  }
	  long thisFileSize = 0;
	  FILE *tf = fopen(fileNames[a], "rb");
	  thisFileSize = _filelength(fileno(tf));
	  fclose(tf);
	  
	  sizeSoFar += thisFileSize;

    const char *fileNameSrc = fileNames[a];

  	if (strrchr(fileNames[a], '\\') != NULL)
		  fileNameSrc = strrchr(fileNames[a], '\\') + 1;
	  else if (strrchr(fileNames[a], '/') != NULL)
		  fileNameSrc = strrchr(fileNames[a], '/') + 1;

    if (strlen(fileNameSrc) >= MAX_FILENAME_LENGTH)
    {
      char buffer[500];
      sprintf(buffer, "Filename too long: %s", fileNames[a]);
      ThrowManagedException(buffer);
    }
		strcpy(ourlib.filenames[a], fileNameSrc);

	  ourlib.file_datafile[a] = currentDataFile;
	  ourlib.length[a] = thisFileSize;
  }

  ourlib.num_data_files = currentDataFile + 1;

  long startOffset = 0;
  long mainHeaderOffset = 0;
  char outputFileName[MAX_PATH];
  char firstDataFileFullPath[MAX_PATH];

  if (makeFileNameAssumptionsForEXE)
  {
	  _mkdir("Compiled");
  }

  // First, set up the ourlib.data_filenames array with all the filenames
  // so that write_clib_header will write the correct amount of data
  for (a = 0; a < ourlib.num_data_files; a++) 
  {
	  if (makeFileNameAssumptionsForEXE) 
	  {
		  sprintf(ourlib.data_filenames[a], "%s.%03d", baseFileName, a);
		  if (a == 0)
		  {
			  strcpy(&ourlib.data_filenames[a][strlen(ourlib.data_filenames[a]) - 3], "exe");
		  }
	  }
	  else 
	  {
    	if (strrchr(baseFileName, '\\') != NULL)
		    strcpy(ourlib.data_filenames[a], strrchr(baseFileName, '\\') + 1);
	    else if (strrchr(baseFileName, '/') != NULL)
		    strcpy(ourlib.data_filenames[a], strrchr(baseFileName, '/') + 1);
	    else
		    strcpy(ourlib.data_filenames[a], baseFileName);
	  }
  }

  // adjust the file paths if necessary, so that write_clib_header will
  // write the correct amount of data
  for (b = 0; b < ourlib.num_files; b++) 
  {
	FILE *iii = find_file_in_path(tomake, ourlib.filenames[b]);
	if (iii != NULL)
	{
		fclose(iii);

		if (!makeFileNameAssumptionsForEXE)
		  strcpy(ourlib.filenames[b], tomake);
	}
  }

  // now, create the actual files
  for (a = 0; a < ourlib.num_data_files; a++) 
  {
	  if (makeFileNameAssumptionsForEXE) 
	  {
		  sprintf(outputFileName, "Compiled\\%s", ourlib.data_filenames[a]);
	  }
	  else 
	  {
		  strcpy(outputFileName, baseFileName);
      }
      if (a == 0) strcpy(firstDataFileFullPath, outputFileName);

	  wout = fopen(outputFileName, (a == 0) ? "ab" : "wb");
	  if (wout == NULL) 
	  {
		  return "ERROR: unable to open file for writing";
	  }

	  startOffset = _filelength(_fileno(wout));
    fwrite("CLIB\x1a",5,1,wout);
    fputc(21, wout);  // version
    fputc(a, wout);   // file number

    if (a == 0) 
	{
      mainHeaderOffset = ftell(wout);
      write_clib_header(wout);
    }

    for (b=0;b<ourlib.num_files;b++) {
      if (ourlib.file_datafile[b] == a) {
        ourlib.offset[b] = ftell(wout) - startOffset;

		FILE *iii = find_file_in_path(NULL, ourlib.filenames[b]);
        if (iii == NULL) {
          fclose(wout);
          unlink(outputFileName);

		  char buffer[500];
		  sprintf(buffer, "Unable to find file '%s' for compilation. Do not remove files during the compilation process.", ourlib.filenames[b]);
		  ThrowManagedException(buffer);
        }

        if (copy_file_across(iii,wout,ourlib.length[b]) < 1) {
          fclose(iii);
          return "Error writing file: possibly disk full";
        }
        fclose(iii);
      }
    }
	if (startOffset > 0)
	{
		putw(startOffset, wout);
		fwrite(clibendsig, 12, 1, wout);
	}
    fclose(wout);
  }

  wout = fopen(firstDataFileFullPath, "r+b");
  fseek(wout, mainHeaderOffset, SEEK_SET);
  write_clib_header(wout);
  fclose(wout);
  return NULL;
}



// **** MANAGED CODE ****

#pragma managed

 void set_rgb_mask_from_alpha_channel(block image)
{
    for (int y = 0; y < image->h; y++)
    {
        unsigned long* thisLine = (unsigned long*)image->line[y];
        for (int x = 0; x < image->w; x++)
        {
	        if ((thisLine[x] & 0xff000000) == 0)
	        {
		        thisLine[x] = MASK_COLOR_32;
	        }
        }
    }
}

void set_opaque_alpha_channel(block image)
{
    for (int y = 0; y < image->h; y++)
    {
        unsigned long* thisLine = (unsigned long*)image->line[y];
        for (int x = 0; x < image->w; x++)
        {
	        if (thisLine[x] != MASK_COLOR_32)
	          thisLine[x] |= 0xff000000;
        }
    }
}


void ThrowManagedException(const char *message) 
{
#if 0
	throw gcnew AGS::Types::AGSEditorException(gcnew String((const char*)message));
#endif
}

void serialize_room_interactions(FILE *ooo) 
{
#if 0
	Room ^roomBeingSaved = TempDataStorage::RoomBeingSaved;
	serialize_interaction_scripts(roomBeingSaved->Interactions, ooo);
	for each (RoomHotspot ^hotspot in roomBeingSaved->Hotspots) 
	{
		serialize_interaction_scripts(hotspot->Interactions, ooo);
	}
	for each (RoomObject ^obj in roomBeingSaved->Objects) 
	{
		serialize_interaction_scripts(obj->Interactions, ooo);
	}
	for each (RoomRegion ^region in roomBeingSaved->Regions) 
	{
		serialize_interaction_scripts(region->Interactions, ooo);
	}
#endif
}


#pragma unmanaged


void quit(char * message) 
{
	ThrowManagedException((const char*)message);
}



// ** GRAPHICAL SCRIPT LOAD/SAVE ROUTINES ** //

long getlong(FILE*iii) {
  long tmm;
  fread(&tmm,4,1,iii);
  return tmm;
}

#define putlong putw

void save_script_configuration(FILE*iii) {
  // no variable names
  putlong (1, iii);
  putlong (0, iii);
}

void load_script_configuration(FILE*iii) { int aa;
  if (getlong(iii)!=1) quit("ScriptEdit: invliad config version");
  int numvarnames=getlong(iii);
  for (aa=0;aa<numvarnames;aa++) {
    int lenoft=getc(iii);
    fseek(iii,lenoft,SEEK_CUR);
  }
}

void save_graphical_scripts(FILE*fff,roomstruct*rss) {
  // no script
  putlong (-1, fff);
}

char*scripttempn="~acsc%d.tmp";
void load_graphical_scripts(FILE*iii,roomstruct*rst) {
  long ct;
  bool doneMsg = false;
  while (1) {
    fread(&ct,4,1,iii);
    if ((ct==-1) | (feof(iii)!=0)) break;
    if (!doneMsg) {
//      infoBox("WARNING: This room uses graphical scripts, which have been removed from this version. If you save the room now, all graphical scripts will be lost.");
      doneMsg = true;
    }
    // skip the data
    long lee; fread(&lee,4,1,iii);
    fseek (iii, lee, SEEK_CUR);
  }
}

