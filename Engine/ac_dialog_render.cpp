
#include "ac_dialog_render.h"

#include "allegro.h"

#include "ac.h"
#include "ac_context.h"
#include "script_dialog_options_rendering.h"

// ** SCRIPT DIALOGOPTIONSRENDERING OBJECT

/* *** SCRIPT SYMBOL: [DialogOptionsRenderingInfo] DialogOptionsRenderingInfo::get_X *** */
int DialogOptionsRendering_GetX(ScriptDialogOptionsRendering *dlgOptRender)
{
  return dlgOptRender->x;
}

/* *** SCRIPT SYMBOL: [DialogOptionsRenderingInfo] DialogOptionsRenderingInfo::set_X *** */
void DialogOptionsRendering_SetX(ScriptDialogOptionsRendering *dlgOptRender, int newX)
{
  dlgOptRender->x = newX;
}

/* *** SCRIPT SYMBOL: [DialogOptionsRenderingInfo] DialogOptionsRenderingInfo::get_Y *** */
int DialogOptionsRendering_GetY(ScriptDialogOptionsRendering *dlgOptRender)
{
  return dlgOptRender->y;
}

/* *** SCRIPT SYMBOL: [DialogOptionsRenderingInfo] DialogOptionsRenderingInfo::set_Y *** */
void DialogOptionsRendering_SetY(ScriptDialogOptionsRendering *dlgOptRender, int newY)
{
  dlgOptRender->y = newY;
}

/* *** SCRIPT SYMBOL: [DialogOptionsRenderingInfo] DialogOptionsRenderingInfo::get_Width *** */
int DialogOptionsRendering_GetWidth(ScriptDialogOptionsRendering *dlgOptRender)
{
  return dlgOptRender->width;
}

/* *** SCRIPT SYMBOL: [DialogOptionsRenderingInfo] DialogOptionsRenderingInfo::set_Width *** */
void DialogOptionsRendering_SetWidth(ScriptDialogOptionsRendering *dlgOptRender, int newWidth)
{
  dlgOptRender->width = newWidth;
}

/* *** SCRIPT SYMBOL: [DialogOptionsRenderingInfo] DialogOptionsRenderingInfo::get_Height *** */
int DialogOptionsRendering_GetHeight(ScriptDialogOptionsRendering *dlgOptRender)
{
  return dlgOptRender->height;
}

/* *** SCRIPT SYMBOL: [DialogOptionsRenderingInfo] DialogOptionsRenderingInfo::set_Height *** */
void DialogOptionsRendering_SetHeight(ScriptDialogOptionsRendering *dlgOptRender, int newHeight)
{
  dlgOptRender->height = newHeight;
}

/* *** SCRIPT SYMBOL: [DialogOptionsRenderingInfo] DialogOptionsRenderingInfo::get_ParserTextBoxX *** */
int DialogOptionsRendering_GetParserTextboxX(ScriptDialogOptionsRendering *dlgOptRender)
{
  return dlgOptRender->parserTextboxX;
}

/* *** SCRIPT SYMBOL: [DialogOptionsRenderingInfo] DialogOptionsRenderingInfo::set_ParserTextBoxX *** */
void DialogOptionsRendering_SetParserTextboxX(ScriptDialogOptionsRendering *dlgOptRender, int newX)
{
  dlgOptRender->parserTextboxX = newX;
}

/* *** SCRIPT SYMBOL: [DialogOptionsRenderingInfo] DialogOptionsRenderingInfo::get_ParserTextBoxY *** */
int DialogOptionsRendering_GetParserTextboxY(ScriptDialogOptionsRendering *dlgOptRender)
{
  return dlgOptRender->parserTextboxY;
}

/* *** SCRIPT SYMBOL: [DialogOptionsRenderingInfo] DialogOptionsRenderingInfo::set_ParserTextBoxY *** */
void DialogOptionsRendering_SetParserTextboxY(ScriptDialogOptionsRendering *dlgOptRender, int newY)
{
  dlgOptRender->parserTextboxY = newY;
}

/* *** SCRIPT SYMBOL: [DialogOptionsRenderingInfo] DialogOptionsRenderingInfo::get_ParserTextBoxWidth *** */
int DialogOptionsRendering_GetParserTextboxWidth(ScriptDialogOptionsRendering *dlgOptRender)
{
  return dlgOptRender->parserTextboxWidth;
}

/* *** SCRIPT SYMBOL: [DialogOptionsRenderingInfo] DialogOptionsRenderingInfo::set_ParserTextBoxWidth *** */
void DialogOptionsRendering_SetParserTextboxWidth(ScriptDialogOptionsRendering *dlgOptRender, int newWidth)
{
  dlgOptRender->parserTextboxWidth = newWidth;
}

/* *** SCRIPT SYMBOL: [DialogOptionsRenderingInfo] DialogOptionsRenderingInfo::get_DialogToRender *** */
ScriptDialog* DialogOptionsRendering_GetDialogToRender(ScriptDialogOptionsRendering *dlgOptRender)
{
  return &scrDialog[dlgOptRender->dialogID];
}

/* *** SCRIPT SYMBOL: [DialogOptionsRenderingInfo] DialogOptionsRenderingInfo::get_Surface *** */
ScriptDrawingSurface* DialogOptionsRendering_GetSurface(ScriptDialogOptionsRendering *dlgOptRender)
{
  dlgOptRender->surfaceAccessed = true;
  return dlgOptRender->surfaceToRenderTo;
}

/* *** SCRIPT SYMBOL: [DialogOptionsRenderingInfo] DialogOptionsRenderingInfo::get_ActiveOptionID *** */
int DialogOptionsRendering_GetActiveOptionID(ScriptDialogOptionsRendering *dlgOptRender)
{
  return dlgOptRender->activeOptionID + 1;
}

/* *** SCRIPT SYMBOL: [DialogOptionsRenderingInfo] DialogOptionsRenderingInfo::set_ActiveOptionID *** */
void DialogOptionsRendering_SetActiveOptionID(ScriptDialogOptionsRendering *dlgOptRender, int activeOptionID)
{
  int optionCount = dialog[scrDialog[dlgOptRender->dialogID].id].numoptions;
  if ((activeOptionID < 0) || (activeOptionID > optionCount))
    quitprintf("DialogOptionsRenderingInfo.ActiveOptionID: invalid ID specified for this dialog (specified %d, valid range: 1..%d)", activeOptionID, optionCount);

  dlgOptRender->activeOptionID = activeOptionID - 1;
}



void register_dialog_options_rendering_info_script_functions() {

  scAdd_External_Symbol("DialogOptionsRenderingInfo::get_ActiveOptionID", (void *)DialogOptionsRendering_GetActiveOptionID);
  scAdd_External_Symbol("DialogOptionsRenderingInfo::set_ActiveOptionID", (void *)DialogOptionsRendering_SetActiveOptionID);
  scAdd_External_Symbol("DialogOptionsRenderingInfo::get_DialogToRender", (void *)DialogOptionsRendering_GetDialogToRender);
  scAdd_External_Symbol("DialogOptionsRenderingInfo::get_Height", (void *)DialogOptionsRendering_GetHeight);
  scAdd_External_Symbol("DialogOptionsRenderingInfo::set_Height", (void *)DialogOptionsRendering_SetHeight);
  scAdd_External_Symbol("DialogOptionsRenderingInfo::get_ParserTextBoxX", (void *)DialogOptionsRendering_GetParserTextboxX);
  scAdd_External_Symbol("DialogOptionsRenderingInfo::set_ParserTextBoxX", (void *)DialogOptionsRendering_SetParserTextboxX);
  scAdd_External_Symbol("DialogOptionsRenderingInfo::get_ParserTextBoxY", (void *)DialogOptionsRendering_GetParserTextboxY);
  scAdd_External_Symbol("DialogOptionsRenderingInfo::set_ParserTextBoxY", (void *)DialogOptionsRendering_SetParserTextboxY);
  scAdd_External_Symbol("DialogOptionsRenderingInfo::get_ParserTextBoxWidth", (void *)DialogOptionsRendering_GetParserTextboxWidth);
  scAdd_External_Symbol("DialogOptionsRenderingInfo::set_ParserTextBoxWidth", (void *)DialogOptionsRendering_SetParserTextboxWidth);
  scAdd_External_Symbol("DialogOptionsRenderingInfo::get_Surface", (void *)DialogOptionsRendering_GetSurface);
  scAdd_External_Symbol("DialogOptionsRenderingInfo::get_Width", (void *)DialogOptionsRendering_GetWidth);
  scAdd_External_Symbol("DialogOptionsRenderingInfo::set_Width", (void *)DialogOptionsRendering_SetWidth);
  scAdd_External_Symbol("DialogOptionsRenderingInfo::get_X", (void *)DialogOptionsRendering_GetX);
  scAdd_External_Symbol("DialogOptionsRenderingInfo::set_X", (void *)DialogOptionsRendering_SetX);
  scAdd_External_Symbol("DialogOptionsRenderingInfo::get_Y", (void *)DialogOptionsRendering_GetY);
  scAdd_External_Symbol("DialogOptionsRenderingInfo::set_Y", (void *)DialogOptionsRendering_SetY);
}