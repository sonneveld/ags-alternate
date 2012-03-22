#include "ac_gui_label.h"

#include "ac.h"
#include "ac_context.h"
#include "acgui.h"
#include "ac_string.h"

// ** LABEL FUNCTIONS

/* *** SCRIPT SYMBOL: [Label] Label::get_Text *** */
const char* Label_GetText_New(GUILabel *labl) {
  return CreateNewScriptString(labl->GetText());
}

/* *** SCRIPT SYMBOL: [Label] Label::GetText^1 *** */
void Label_GetText(GUILabel *labl, char *buffer) {
  strcpy(buffer, labl->GetText());
}

/* *** SCRIPT SYMBOL: [Label] Label::SetText^1 *** */
/* *** SCRIPT SYMBOL: [Label] Label::set_Text *** */
void Label_SetText(GUILabel *labl, const char *newtx) {
  newtx = get_translation(newtx);

  if (strcmp(labl->GetText(), newtx)) {
    guis_need_update = 1;
    labl->SetText(newtx);
  }
}

/* *** SCRIPT SYMBOL: [Label] Label::get_TextColor *** */
int Label_GetColor(GUILabel *labl) {
  return labl->textcol;
}

/* *** SCRIPT SYMBOL: [Label] Label::set_TextColor *** */
void Label_SetColor(GUILabel *labl, int colr) {
  if (labl->textcol != colr) {
    labl->textcol = colr;
    guis_need_update = 1;
  }
}

/* *** SCRIPT SYMBOL: [Label] Label::get_Font *** */
int Label_GetFont(GUILabel *labl) {
  return labl->font;
}

/* *** SCRIPT SYMBOL: [Label] Label::set_Font *** */
void Label_SetFont(GUILabel *guil, int fontnum) {
  if ((fontnum < 0) || (fontnum >= game.numfonts))
    quit("!SetLabelFont: invalid font number.");

  if (fontnum != guil->font) {
    guil->font = fontnum;
    guis_need_update = 1;
  }
}


/* *** SCRIPT SYMBOL: [Label] SetLabelColor *** */
void SetLabelColor(int guin,int objn, int colr) {
  if ((guin<0) | (guin>=game.numgui))
    quit("!SetLabelColor: invalid GUI number");
  if ((objn<0) | (objn>=guis[guin].numobjs))
    quit("!SetLabelColor: invalid object number");
  if (guis[guin].get_control_type(objn)!=GOBJ_LABEL)
    quit("!SetLabelColor: specified control is not a label");

  GUILabel*guil=(GUILabel*)guis[guin].objs[objn];
  Label_SetColor(guil, colr);
}

/* *** SCRIPT SYMBOL: [Label] SetLabelText *** */
void SetLabelText(int guin,int objn,char*newtx) {
  VALIDATE_STRING(newtx);
  if ((guin<0) | (guin>=game.numgui)) quit("!SetLabelText: invalid GUI number");
  if ((objn<0) | (objn>=guis[guin].numobjs)) quit("!SetLabelTexT: invalid object number");
  if (guis[guin].get_control_type(objn)!=GOBJ_LABEL)
    quit("!SetLabelText: specified control is not a label");

  GUILabel*guil=(GUILabel*)guis[guin].objs[objn];
  Label_SetText(guil, newtx);
}

/* *** SCRIPT SYMBOL: [Label] SetLabelFont *** */
void SetLabelFont(int guin,int objn, int fontnum) {

  if ((guin<0) | (guin>=game.numgui)) quit("!SetLabelFont: invalid GUI number");
  if ((objn<0) | (objn>=guis[guin].numobjs)) quit("!SetLabelFont: invalid object number");
  if (guis[guin].get_control_type(objn)!=GOBJ_LABEL)
    quit("!SetLabelFont: specified control is not a label");

  GUILabel*guil=(GUILabel*)guis[guin].objs[objn];
  Label_SetFont(guil, fontnum);
}

void register_label_script_functions() {
  scAdd_External_Symbol("Label::GetText^1", (void *)Label_GetText);
  scAdd_External_Symbol("Label::SetText^1", (void *)Label_SetText);
  scAdd_External_Symbol("Label::get_Font", (void *)Label_GetFont);
  scAdd_External_Symbol("Label::set_Font", (void *)Label_SetFont);
  scAdd_External_Symbol("Label::get_Text", (void *)Label_GetText_New);
  scAdd_External_Symbol("Label::set_Text", (void *)Label_SetText);
  scAdd_External_Symbol("Label::get_TextColor", (void *)Label_GetColor);
  scAdd_External_Symbol("Label::set_TextColor", (void *)Label_SetColor);
  scAdd_External_Symbol("SetLabelColor",(void *)SetLabelColor);
  scAdd_External_Symbol("SetLabelFont",(void *)SetLabelFont);
  scAdd_External_Symbol("SetLabelText",(void *)SetLabelText);
}

