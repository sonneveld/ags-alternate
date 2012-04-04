#include "dynobj/cc_gui_object.h"

#include "sdlwrap/allegro.h"

#include "ac.h"
#include "ac_context.h"
#include "acgui.h"
#include "cscomp.h"




// return the type name of the object
const char *CCGUIObject::GetType() {
  return "GUIObject";
}

// serialize the object into BUFFER (which is BUFSIZE bytes)
// return number of bytes used
int CCGUIObject::Serialize(const char *address, char *buffer, int bufsize) {
  GUIObject *guio = (GUIObject*)address;
  StartSerialize(buffer);
  SerializeInt(guio->guin);
  SerializeInt(guio->objn);
  return EndSerialize();
}

void CCGUIObject::Unserialize(int index, const char *serializedData, int dataSize) {
  StartUnserialize(serializedData, dataSize);
  int guinum = UnserializeInt();
  int objnum = UnserializeInt();
  ccRegisterUnserializedObject(index, guis[guinum].objs[objnum], this);
}