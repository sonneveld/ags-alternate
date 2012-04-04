#include "dynobj/cc_gui.h"

#include "sdlwrap/allegro.h"

#include "ac.h"
#include "ac_context.h"
#include "cscomp.h"


  // return the type name of the object
const char *CCGUI::GetType() {
  return "GUI";
}

// serialize the object into BUFFER (which is BUFSIZE bytes)
// return number of bytes used
int CCGUI::Serialize(const char *address, char *buffer, int bufsize) {
  ScriptGUI *shh = (ScriptGUI*)address;
  StartSerialize(buffer);
  SerializeInt(shh->id);
  return EndSerialize();
}

void CCGUI::Unserialize(int index, const char *serializedData, int dataSize) {
  StartUnserialize(serializedData, dataSize);
  int num = UnserializeInt();
  ccRegisterUnserializedObject(index, &scrGui[num], this);
}
