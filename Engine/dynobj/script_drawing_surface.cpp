#include "dynobj/script_drawing_surface.h"

#include "ac.h"
#include "ac_context.h"
#include "sprcache.h"
#include "cscomp.h"
#include "ac_drawsurf.h"

void ScriptDrawingSurface::MultiplyCoordinates(int *xcoord, int *ycoord)
{
  if (this->highResCoordinates)
  {
    if (current_screen_resolution_multiplier == 1) 
    {
      // using high-res co-ordinates but game running at low-res
      xcoord[0] /= 2;
      ycoord[0] /= 2;
    }
  }
  else
  {
    if (current_screen_resolution_multiplier > 1) 
    {
      // using low-res co-ordinates but game running at high-res
      xcoord[0] *= 2;
      ycoord[0] *= 2;
    }
  }
}

void ScriptDrawingSurface::MultiplyThickness(int *valueToAdjust)
{
  if (this->highResCoordinates)
  {
    if (current_screen_resolution_multiplier == 1) 
    {
      valueToAdjust[0] /= 2;
      if (valueToAdjust[0] < 1)
        valueToAdjust[0] = 1;
    }
  }
  else
  {
    if (current_screen_resolution_multiplier > 1) 
    {
      valueToAdjust[0] *= 2;
    }
  }
}

// convert actual co-ordinate back to what the script is expecting
void ScriptDrawingSurface::UnMultiplyThickness(int *valueToAdjust)
{
  if (this->highResCoordinates)
  {
    if (current_screen_resolution_multiplier == 1) 
    {
      valueToAdjust[0] *= 2;
    }
  }
  else
  {
    if (current_screen_resolution_multiplier > 1) 
    {
      valueToAdjust[0] /= 2;
      if (valueToAdjust[0] < 1)
        valueToAdjust[0] = 1;
    }
  }
}


BITMAP* ScriptDrawingSurface::GetBitmapSurface()
{
  if (roomBackgroundNumber >= 0)
    return thisroom.ebscene[roomBackgroundNumber];
  else if (dynamicSpriteNumber >= 0)
    return spriteset[dynamicSpriteNumber];
  else if (dynamicSurfaceNumber >= 0)
    return dynamicallyCreatedSurfaces[dynamicSurfaceNumber];
  else if (linkedBitmapOnly != NULL)
    return linkedBitmapOnly;
  else
    quit("!DrawingSurface: attempted to use surface after Release was called");

  return NULL;
}

void ScriptDrawingSurface::StartDrawing()
{
  abufBackup = abuf;
  abuf = this->GetBitmapSurface();
}

void ScriptDrawingSurface::FinishedDrawingReadOnly()
{
  abuf = abufBackup;
}

void ScriptDrawingSurface::FinishedDrawing()
{
  FinishedDrawingReadOnly();
  modified = 1;
}

int ScriptDrawingSurface::Dispose(const char *address, bool force) {

  // dispose the drawing surface
  DrawingSurface_Release(this);
  delete this;
  return 1;
}

const char *ScriptDrawingSurface::GetType() {
  return "DrawingSurface";
}

int ScriptDrawingSurface::Serialize(const char *address, char *buffer, int bufsize) {
  StartSerialize(buffer);
  SerializeInt(roomBackgroundNumber);
  SerializeInt(dynamicSpriteNumber);
  SerializeInt(dynamicSurfaceNumber);
  SerializeInt(currentColour);
  SerializeInt(currentColourScript);
  SerializeInt(highResCoordinates);
  SerializeInt(modified);
  SerializeInt(hasAlphaChannel);
  SerializeInt(isLinkedBitmapOnly ? 1 : 0);
  return EndSerialize();
}

void ScriptDrawingSurface::Unserialize(int index, const char *serializedData, int dataSize) {
  StartUnserialize(serializedData, dataSize);
  roomBackgroundNumber = UnserializeInt();
  dynamicSpriteNumber = UnserializeInt();
  dynamicSurfaceNumber = UnserializeInt();
  currentColour = UnserializeInt();
  currentColourScript = UnserializeInt();
  highResCoordinates = UnserializeInt();
  modified = UnserializeInt();
  hasAlphaChannel = UnserializeInt();
  isLinkedBitmapOnly = (UnserializeInt() != 0);
  ccRegisterUnserializedObject(index, this, this);
}

ScriptDrawingSurface::ScriptDrawingSurface() 
{
  roomBackgroundNumber = -1;
  dynamicSpriteNumber = -1;
  dynamicSurfaceNumber = -1;
  isLinkedBitmapOnly = false;
  linkedBitmapOnly = NULL;
  currentColour = play.raw_color;
  currentColourScript = 0;
  modified = 0;
  hasAlphaChannel = 0;
  highResCoordinates = 0;

  if ((game.options[OPT_NATIVECOORDINATES] != 0) &&
      (game.default_resolution > 2))
  {
    highResCoordinates = 1;
  }
}

