#include "cc_character.h"

#include "allegro.h"

#include "ac.h"
#include "ac_context.h"
#include "cscomp.h"


  // return the type name of the object
const char *CCCharacter::GetType() {
  return "Character";
}

// serialize the object into BUFFER (which is BUFSIZE bytes)
// return number of bytes used
 int CCCharacter::Serialize(const char *address, char *buffer, int bufsize) {
  CharacterInfo *chaa = (CharacterInfo*)address;
  StartSerialize(buffer);
  SerializeInt(chaa->index_id);
  return EndSerialize();
}

 void CCCharacter::Unserialize(int index, const char *serializedData, int dataSize) {
  StartUnserialize(serializedData, dataSize);
  int num = UnserializeInt();
  ccRegisterUnserializedObject(index, &game.chars[num], this);
}
