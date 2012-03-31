#include "ac_gui_slider.h"

#include "allegro_wrapper.h"

#include "ac.h"
#include "ac_context.h"
#include "acgui.h"


// *** SLIDER FUNCTIONS

/* *** SCRIPT SYMBOL: [Slider] Slider::set_Max *** */
void Slider_SetMax(GUISlider *guisl, int valn) {

  if (valn != guisl->max) {
    guisl->max = valn;

    if (guisl->value > guisl->max)
      guisl->value = guisl->max;
    if (guisl->min > guisl->max)
      quit("!Slider.Max: minimum cannot be greater than maximum");

    guis_need_update = 1;
  }

}

/* *** SCRIPT SYMBOL: [Slider] Slider::get_Max *** */
int Slider_GetMax(GUISlider *guisl) {
  return guisl->max;
}

/* *** SCRIPT SYMBOL: [Slider] Slider::set_Min *** */
void Slider_SetMin(GUISlider *guisl, int valn) {

  if (valn != guisl->min) {
    guisl->min = valn;

    if (guisl->value < guisl->min)
      guisl->value = guisl->min;
    if (guisl->min > guisl->max)
      quit("!Slider.Min: minimum cannot be greater than maximum");

    guis_need_update = 1;
  }

}

/* *** SCRIPT SYMBOL: [Slider] Slider::get_Min *** */
int Slider_GetMin(GUISlider *guisl) {
  return guisl->min;
}

/* *** SCRIPT SYMBOL: [Slider] Slider::set_Value *** */
void Slider_SetValue(GUISlider *guisl, int valn) {
  if (valn > guisl->max) valn = guisl->max;
  if (valn < guisl->min) valn = guisl->min;

  if (valn != guisl->value) {
    guisl->value = valn;
    guis_need_update = 1;
  }
}

/* *** SCRIPT SYMBOL: [Slider] Slider::get_Value *** */
int Slider_GetValue(GUISlider *guisl) {
  return guisl->value;
}

/* *** SCRIPT SYMBOL: [Slider] SetSliderValue *** */
void SetSliderValue(int guin,int objn, int valn) {
  if ((guin<0) | (guin>=game.numgui)) quit("!SetSliderValue: invalid GUI number");
  if (guis[guin].get_control_type(objn)!=GOBJ_SLIDER)
    quit("!SetSliderValue: specified control is not a slider");

  GUISlider*guisl=(GUISlider*)guis[guin].objs[objn];
  Slider_SetValue(guisl, valn);
}

/* *** SCRIPT SYMBOL: [Slider] GetSliderValue *** */
int GetSliderValue(int guin,int objn) {
  if ((guin<0) | (guin>=game.numgui)) quit("!GetSliderValue: invalid GUI number");
  if (guis[guin].get_control_type(objn)!=GOBJ_SLIDER)
    quit("!GetSliderValue: specified control is not a slider");

  GUISlider*guisl=(GUISlider*)guis[guin].objs[objn];
  return Slider_GetValue(guisl);
}

/* *** SCRIPT SYMBOL: [Slider] Slider::get_BackgroundGraphic *** */
int Slider_GetBackgroundGraphic(GUISlider *guisl) {
  return (guisl->bgimage > 0) ? guisl->bgimage : 0;
}

/* *** SCRIPT SYMBOL: [Slider] Slider::set_BackgroundGraphic *** */
void Slider_SetBackgroundGraphic(GUISlider *guisl, int newImage) 
{
  if (newImage != guisl->bgimage)
  {
    guisl->bgimage = newImage;
    guis_need_update = 1;
  }
}

/* *** SCRIPT SYMBOL: [Slider] Slider::get_HandleGraphic *** */
int Slider_GetHandleGraphic(GUISlider *guisl) {
  return (guisl->handlepic > 0) ? guisl->handlepic : 0;
}

/* *** SCRIPT SYMBOL: [Slider] Slider::set_HandleGraphic *** */
void Slider_SetHandleGraphic(GUISlider *guisl, int newImage) 
{
  if (newImage != guisl->handlepic)
  {
    guisl->handlepic = newImage;
    guis_need_update = 1;
  }
}

/* *** SCRIPT SYMBOL: [Slider] Slider::get_HandleOffset *** */
int Slider_GetHandleOffset(GUISlider *guisl) {
  return guisl->handleoffset;
}

/* *** SCRIPT SYMBOL: [Slider] Slider::set_HandleOffset *** */
void Slider_SetHandleOffset(GUISlider *guisl, int newOffset) 
{
  if (newOffset != guisl->handleoffset)
  {
    guisl->handleoffset = newOffset;
    guis_need_update = 1;
  }
}


void register_slider_script_functions() {
  scAdd_External_Symbol("Slider::get_BackgroundGraphic", (void *)Slider_GetBackgroundGraphic);
  scAdd_External_Symbol("Slider::set_BackgroundGraphic", (void *)Slider_SetBackgroundGraphic);
  scAdd_External_Symbol("Slider::get_HandleGraphic", (void *)Slider_GetHandleGraphic);
  scAdd_External_Symbol("Slider::set_HandleGraphic", (void *)Slider_SetHandleGraphic);
  scAdd_External_Symbol("Slider::get_HandleOffset", (void *)Slider_GetHandleOffset);
  scAdd_External_Symbol("Slider::set_HandleOffset", (void *)Slider_SetHandleOffset);
  scAdd_External_Symbol("Slider::get_Max", (void *)Slider_GetMax);
  scAdd_External_Symbol("Slider::set_Max", (void *)Slider_SetMax);
  scAdd_External_Symbol("Slider::get_Min", (void *)Slider_GetMin);
  scAdd_External_Symbol("Slider::set_Min", (void *)Slider_SetMin);
  scAdd_External_Symbol("Slider::get_Value", (void *)Slider_GetValue);
  scAdd_External_Symbol("Slider::set_Value", (void *)Slider_SetValue);
  scAdd_External_Symbol("GetSliderValue",(void *)GetSliderValue);
  scAdd_External_Symbol("SetSliderValue",(void *)SetSliderValue);
}

