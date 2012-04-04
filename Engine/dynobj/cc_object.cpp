#include "dynobj/cc_object.h"

#include "sdlwrap/allegro.h"

#include "ac.h"
#include "ac_context.h"
#include "cscomp.h"


// return the type name of the object
const char *CCObject::GetType() {
  return "Object";
}

// serialize the object into BUFFER (which is BUFSIZE bytes)
// return number of bytes used
int CCObject::Serialize(const char *address, char *buffer, int bufsize) {
  ScriptObject *shh = (ScriptObject*)address;
  StartSerialize(buffer);
  SerializeInt(shh->id);
  return EndSerialize();
}

void CCObject::Unserialize(int index, const char *serializedData, int dataSize) {
  StartUnserialize(serializedData, dataSize);
  int num = UnserializeInt();
  ccRegisterUnserializedObject(index, &scrObj[num], this);
}
