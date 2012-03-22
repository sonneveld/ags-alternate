#include "ac_gui_textbox.h"

#include "ac.h"
#include "ac_context.h"
#include "acgui.h"
#include "ac_string.h"

// ** TEXT BOX FUNCTIONS

/* *** SCRIPT SYMBOL: [TextBox] TextBox::get_Text *** */
const char* TextBox_GetText_New(GUITextBox *texbox) {
  return CreateNewScriptString(texbox->text);
}

/* *** SCRIPT SYMBOL: [TextBox] TextBox::GetText^1 *** */
void TextBox_GetText(GUITextBox *texbox, char *buffer) {
  strcpy(buffer, texbox->text);
}

/* *** SCRIPT SYMBOL: [TextBox] TextBox::SetText^1 *** */
/* *** SCRIPT SYMBOL: [TextBox] TextBox::set_Text *** */
void TextBox_SetText(GUITextBox *texbox, const char *newtex) {
  if (strlen(newtex) > 190)
    quit("!SetTextBoxText: text too long");

  if (strcmp(texbox->text, newtex)) {
    strcpy(texbox->text, newtex);
    guis_need_update = 1;
  }
}

/* *** SCRIPT SYMBOL: [TextBox] TextBox::get_TextColor *** */
int TextBox_GetTextColor(GUITextBox *guit) {
  return guit->textcol;
}

/* *** SCRIPT SYMBOL: [TextBox] TextBox::set_TextColor *** */
void TextBox_SetTextColor(GUITextBox *guit, int colr)
{
  if (guit->textcol != colr) 
  {
    guit->textcol = colr;
    guis_need_update = 1;
  }
}

/* *** SCRIPT SYMBOL: [TextBox] TextBox::get_Font *** */
int TextBox_GetFont(GUITextBox *guit) {
  return guit->font;
}

/* *** SCRIPT SYMBOL: [TextBox] TextBox::set_Font *** */
void TextBox_SetFont(GUITextBox *guit, int fontnum) {
  if ((fontnum < 0) || (fontnum >= game.numfonts))
    quit("!SetTextBoxFont: invalid font number.");

  if (guit->font != fontnum) {
    guit->font = fontnum;
    guis_need_update = 1;
  }
}


/* *** SCRIPT SYMBOL: [TextBox] SetTextBoxFont *** */
void SetTextBoxFont(int guin,int objn, int fontnum) {

  if ((guin<0) | (guin>=game.numgui)) quit("!SetTextBoxFont: invalid GUI number");
  if ((objn<0) | (objn>=guis[guin].numobjs)) quit("!SetTextBoxFont: invalid object number");
  if (guis[guin].get_control_type(objn) != GOBJ_TEXTBOX)
    quit("!SetTextBoxFont: specified control is not a text box");

  GUITextBox *guit = (GUITextBox*)guis[guin].objs[objn];
  TextBox_SetFont(guit, fontnum);
}

/* *** SCRIPT SYMBOL: [TextBox] GetTextBoxText *** */
void GetTextBoxText(int guin, int objn, char*txbuf) {
  VALIDATE_STRING(txbuf);
  if ((guin<0) | (guin>=game.numgui)) quit("!GetTextBoxText: invalid GUI number");
  if ((objn<0) | (objn>=guis[guin].numobjs)) quit("!GetTextBoxText: invalid object number");
  if (guis[guin].get_control_type(objn)!=GOBJ_TEXTBOX)
    quit("!GetTextBoxText: specified control is not a text box");

  GUITextBox*guisl=(GUITextBox*)guis[guin].objs[objn];
  TextBox_GetText(guisl, txbuf);
}

/* *** SCRIPT SYMBOL: [TextBox] SetTextBoxText *** */
void SetTextBoxText(int guin, int objn, char*txbuf) {
  if ((guin<0) | (guin>=game.numgui)) quit("!SetTextBoxText: invalid GUI number");
  if ((objn<0) | (objn>=guis[guin].numobjs)) quit("!SetTextBoxText: invalid object number");
  if (guis[guin].get_control_type(objn)!=GOBJ_TEXTBOX)
    quit("!SetTextBoxText: specified control is not a text box");

  GUITextBox*guisl=(GUITextBox*)guis[guin].objs[objn];
  TextBox_SetText(guisl, txbuf);
}

void register_textbox_script_functions() {
  scAdd_External_Symbol("TextBox::GetText^1", (void *)TextBox_GetText);
  scAdd_External_Symbol("TextBox::SetText^1", (void *)TextBox_SetText);
  scAdd_External_Symbol("TextBox::get_Font", (void *)TextBox_GetFont);
  scAdd_External_Symbol("TextBox::set_Font", (void *)TextBox_SetFont);
  scAdd_External_Symbol("TextBox::get_Text", (void *)TextBox_GetText_New);
  scAdd_External_Symbol("TextBox::set_Text", (void *)TextBox_SetText);
  scAdd_External_Symbol("TextBox::get_TextColor", (void *)TextBox_GetTextColor);
  scAdd_External_Symbol("TextBox::set_TextColor", (void *)TextBox_SetTextColor);
  scAdd_External_Symbol("GetTextBoxText",(void *)GetTextBoxText);
  scAdd_External_Symbol("SetTextBoxFont",(void *)SetTextBoxFont);
  scAdd_External_Symbol("SetTextBoxText",(void *)SetTextBoxText);
}

