#include "ac_string.h"

#include "sdlwrap/allegro.h"

#include "ac.h"
#include "acruntim.h"
#include "ac_maths.h"
#include "dynobj/script_string.h"

// ** SCRIPT STRING

const char *CreateNewScriptString(const char *fromText, bool reAllocate) {
  ScriptString *str;
  if (reAllocate) {
    str = new ScriptString(fromText);
  }
  else {
    str = new ScriptString();
    str->text = (char*)fromText;
  }

  ccRegisterManagedObject(str->text, str);

  /*long handle = ccRegisterManagedObject(str->text, str);
  char buffer[1000];
  sprintf(buffer, "String %p (handle %d) allocated: '%s'", str->text, handle, str->text);
  write_log(buffer);*/

  return str->text;
}


/* *** SCRIPT SYMBOL: [String] String::IsNullOrEmpty^1 *** */
static int String_IsNullOrEmpty(const char *thisString) 
{
  if ((thisString == NULL) || (thisString[0] == 0))
    return 1;

  return 0;
}

/* *** SCRIPT SYMBOL: [String] String::Copy^0 *** */
static const char* String_Copy(const char *srcString) {
  return CreateNewScriptString(srcString);
}

/* *** SCRIPT SYMBOL: [String] String::Append^1 *** */
static const char* String_Append(const char *thisString, const char *extrabit) {
  char *buffer = (char*)malloc(strlen(thisString) + strlen(extrabit) + 1);
  strcpy(buffer, thisString);
  strcat(buffer, extrabit);
  return CreateNewScriptString(buffer, false);
}

/* *** SCRIPT SYMBOL: [String] String::AppendChar^1 *** */
static const char* String_AppendChar(const char *thisString, char extraOne) {
  char *buffer = (char*)malloc(strlen(thisString) + 2);
  sprintf(buffer, "%s%c", thisString, extraOne);
  return CreateNewScriptString(buffer, false);
}

/* *** SCRIPT SYMBOL: [String] String::ReplaceCharAt^2 *** */
static const char* String_ReplaceCharAt(const char *thisString, int index, char newChar) {
  if ((index < 0) || (index >= (int)strlen(thisString)))
    quit("!String.ReplaceCharAt: index outside range of string");

  char *buffer = (char*)malloc(strlen(thisString) + 1);
  strcpy(buffer, thisString);
  buffer[index] = newChar;
  return CreateNewScriptString(buffer, false);
}

/* *** SCRIPT SYMBOL: [String] String::Truncate^1 *** */
static const char* String_Truncate(const char *thisString, int length) {
  if (length < 0)
    quit("!String.Truncate: invalid length");

  if (length >= (int)strlen(thisString))
    return thisString;

  char *buffer = (char*)malloc(length + 1);
  strncpy(buffer, thisString, length);
  buffer[length] = 0;
  return CreateNewScriptString(buffer, false);
}

/* *** SCRIPT SYMBOL: [String] String::Substring^2 *** */
static const char* String_Substring(const char *thisString, int index, int length) {
  if (length < 0)
    quit("!String.Substring: invalid length");
  if ((index < 0) || (index > (int)strlen(thisString)))
    quit("!String.Substring: invalid index");

  char *buffer = (char*)malloc(length + 1);
  strncpy(buffer, &thisString[index], length);
  buffer[length] = 0;
  return CreateNewScriptString(buffer, false);
}

/* *** SCRIPT SYMBOL: [String] String::CompareTo^2 *** */
static int String_CompareTo(const char *thisString, const char *otherString, bool caseSensitive) {

  if (caseSensitive) {
    return strcmp(thisString, otherString);
  }
  else {
    return stricmp(thisString, otherString);
  }
}

/* *** SCRIPT SYMBOL: [String] String::StartsWith^2 *** */
static int String_StartsWith(const char *thisString, const char *checkForString, bool caseSensitive) {

  if (caseSensitive) {
    return (strncmp(thisString, checkForString, strlen(checkForString)) == 0) ? 1 : 0;
  }
  else {
    return (strnicmp(thisString, checkForString, strlen(checkForString)) == 0) ? 1 : 0;
  }
}

/* *** SCRIPT SYMBOL: [String] String::EndsWith^2 *** */
static int String_EndsWith(const char *thisString, const char *checkForString, bool caseSensitive) {

  int checkAtOffset = strlen(thisString) - strlen(checkForString);

  if (checkAtOffset < 0)
  {
    return 0;
  }

  if (caseSensitive) 
  {
    return (strcmp(&thisString[checkAtOffset], checkForString) == 0) ? 1 : 0;
  }
  else 
  {
    return (stricmp(&thisString[checkAtOffset], checkForString) == 0) ? 1 : 0;
  }
}

/* *** SCRIPT SYMBOL: [String] String::Replace^3 *** */
static const char* String_Replace(const char *thisString, const char *lookForText, const char *replaceWithText, bool caseSensitive)
{
  char resultBuffer[STD_BUFFER_SIZE] = "";
  int thisStringLen = (int)strlen(thisString);
  int outputSize = 0;
  for (int i = 0; i < thisStringLen; i++)
  {
    bool matchHere = false;
    if (caseSensitive)
    {
      matchHere = (strncmp(&thisString[i], lookForText, strlen(lookForText)) == 0);
    }
    else
    {
      matchHere = (strnicmp(&thisString[i], lookForText, strlen(lookForText)) == 0);
    }

    if (matchHere)
    {
      strcpy(&resultBuffer[outputSize], replaceWithText);
      outputSize += strlen(replaceWithText);
      i += strlen(lookForText) - 1;
    }
    else
    {
      resultBuffer[outputSize] = thisString[i];
      outputSize++;
    }
  }

  resultBuffer[outputSize] = 0;

  return CreateNewScriptString(resultBuffer, true);
}

/* *** SCRIPT SYMBOL: [String] String::LowerCase^0 *** */
static const char* String_LowerCase(const char *thisString) {
  char *buffer = (char*)malloc(strlen(thisString) + 1);
  strcpy(buffer, thisString);
  strlwr(buffer);
  return CreateNewScriptString(buffer, false);
}

/* *** SCRIPT SYMBOL: [String] String::UpperCase^0 *** */
static const char* String_UpperCase(const char *thisString) {
  char *buffer = (char*)malloc(strlen(thisString) + 1);
  strcpy(buffer, thisString);
  strupr(buffer);
  return CreateNewScriptString(buffer, false);
}

/* *** SCRIPT SYMBOL: [String] String::Format^101 *** */
static const char* String_Format(const char *texx, ...) {
  char displbuf[STD_BUFFER_SIZE];

  va_list ap;
  va_start(ap,texx);
  my_sprintf(displbuf, get_translation(texx), ap);
  va_end(ap);

  return CreateNewScriptString(displbuf);
}

/* *** SCRIPT SYMBOL: [String] String::geti_Chars *** */
static int String_GetChars(const char *texx, int index) {
  if ((index < 0) || (index >= (int)strlen(texx)))
    return 0;
  return texx[index];
}


/* *** SCRIPT SYMBOL: [String] String::get_AsFloat *** */
static FLOAT_RETURN_TYPE StringToFloat(const char *theString) {
  float fval = atof(theString);

  RETURN_FLOAT(fval);
}

/* *** SCRIPT SYMBOL: [String] String::get_AsInt *** */
/* *** SCRIPT SYMBOL: [String] StringToInt *** */
static int StringToInt(char*stino) {
  return atoi(stino);
  }


/* *** SCRIPT SYMBOL: [String] StrGetCharAt *** */
static int StrGetCharAt (char *strin, int posn) {
  if ((posn < 0) || (posn >= (int)strlen(strin)))
    return 0;
  return strin[posn];
}

/* *** SCRIPT SYMBOL: [String] StrSetCharAt *** */
static void StrSetCharAt (char *strin, int posn, int nchar) {
  if ((posn < 0) || (posn > (int)strlen(strin)) || (posn >= MAX_MAXSTRLEN))
    quit("!StrSetCharAt: tried to write past end of string");

  if (posn == (int)strlen(strin))
    strin[posn+1] = 0;
  strin[posn] = nchar;
}


int MAXSTRLEN = MAX_MAXSTRLEN;
void check_strlen(char*ptt) {
  MAXSTRLEN = MAX_MAXSTRLEN;
  long charstart = (long)&game.chars[0];
  long charend = charstart + sizeof(CharacterInfo)*game.numcharacters;
  if (((long)&ptt[0] >= charstart) && ((long)&ptt[0] <= charend))
    MAXSTRLEN=30;
}


static void my_strncpy(char *dest, const char *src, int len) {
  // the normal strncpy pads out the string with zeros up to the
  // max length -- we don't want that
  if (strlen(src) >= (unsigned)len) {
    strncpy(dest, src, len);
    dest[len] = 0;
  }
  else
    strcpy(dest, src);
}

/* *** SCRIPT SYMBOL: [String] StrCat *** */
static void _sc_strcat(char*s1,char*s2) {
  // make sure they don't try to append a char to the string
  VALIDATE_STRING (s2);
  check_strlen(s1);
  int mosttocopy=(MAXSTRLEN-strlen(s1))-1;
//  int numbf=game.iface[4].numbuttons;
  my_strncpy(&s1[strlen(s1)], s2, mosttocopy);
}

/* *** SCRIPT SYMBOL: [String] StrCopy *** */
static void _sc_strcpy(char*s1,char*s2) {
  check_strlen(s1);
  my_strncpy(s1, s2, MAXSTRLEN - 1);
}

/* *** SCRIPT SYMBOL: [String] String::Contains^1 *** */
/* *** SCRIPT SYMBOL: [String] String::IndexOf^1 *** */
/* *** SCRIPT SYMBOL: [String] StrContains *** */
static int StrContains (const char *s1, const char *s2) {
  VALIDATE_STRING (s1);
  VALIDATE_STRING (s2);
  char *tempbuf1 = (char*)malloc(strlen(s1) + 1);
  char *tempbuf2 = (char*)malloc(strlen(s2) + 1);
  strcpy(tempbuf1, s1);
  strcpy(tempbuf2, s2);
  strlwr(tempbuf1);
  strlwr(tempbuf2);

  char *offs = strstr (tempbuf1, tempbuf2);
  free(tempbuf1);
  free(tempbuf2);

  if (offs == NULL)
    return -1;

  return (offs - tempbuf1);
}

#ifdef WINDOWS_VERSION
#define strlwr _strlwr
#define strupr _strupr
#endif

/* *** SCRIPT SYMBOL: [String] StrToLowerCase *** */
static void _sc_strlower (char *desbuf) {
  VALIDATE_STRING(desbuf);
  check_strlen (desbuf);
  strlwr (desbuf);
}

/* *** SCRIPT SYMBOL: [String] StrToUpperCase *** */
static void _sc_strupper (char *desbuf) {
  VALIDATE_STRING(desbuf);
  check_strlen (desbuf);
  strupr (desbuf);
}

/*int _sc_strcmp (char *s1, char *s2) {
  return strcmp (get_translation (s1), get_translation(s2));
}

int _sc_stricmp (char *s1, char *s2) {
  return stricmp (get_translation (s1), get_translation(s2));
}*/



// Custom printf, needed because floats are pushed as 8 bytes
void my_sprintf(char *buffer, const char *fmt, va_list ap) {
  int bufidx = 0;
  const char *curptr = fmt;
  const char *endptr;
  char spfbuffer[STD_BUFFER_SIZE];
  char fmtstring[100];
  int numargs = -1;

  while (1) {
    // copy across everything until the next % (or end of string)
    endptr = strchr(curptr, '%');
    if (endptr == NULL)
      endptr = &curptr[strlen(curptr)];
    while (curptr < endptr) {
      buffer[bufidx] = *curptr;
      curptr++;
      bufidx++;
    }
    // at this point, curptr and endptr should be equal and pointing
    // to the % or \0
    if (*curptr == 0)
      break;
    if (curptr[1] == '%') {
      // "%%", so just write a % to the output
      buffer[bufidx] = '%';
      bufidx++;
      curptr += 2;
      continue;
    }
    // find the end of the % clause
    while ((*endptr != 'd') && (*endptr != 'f') && (*endptr != 'c') &&
           (*endptr != 0) && (*endptr != 's') && (*endptr != 'x') &&
           (*endptr != 'X'))
      endptr++;

    if (numargs >= 0) {
      numargs--;
      // if there are not enough arguments, just copy the %d
      // to the output string rather than trying to format it
      if (numargs < 0)
        endptr = &curptr[strlen(curptr)];
    }

    if (*endptr == 0) {
      // something like %p which we don't support, so just write
      // the % to the output
      buffer[bufidx] = '%';
      bufidx++;
      curptr++;
      continue;
    }
    // move endptr to 1 after the end character
    endptr++;

    // copy the %d or whatever
    strncpy(fmtstring, curptr, (endptr - curptr));
    fmtstring[endptr - curptr] = 0;

    unsigned int theArg = va_arg(ap, unsigned int);

    // use sprintf to parse the actual %02d type thing
    if (endptr[-1] == 'f') {
      // floats are pushed as 8-bytes, so ensure that it knows this is a float
      float floatArg = *((float*)&theArg);
      sprintf(spfbuffer, fmtstring, floatArg);
    }
    else if ((theArg == (int)buffer) && (endptr[-1] == 's'))
      quit("Cannot use destination as argument to StrFormat");
    else if ((theArg < 0x10000) && (endptr[-1] == 's'))
      quit("!One of the string arguments supplied was not a string");
    else if (endptr[-1] == 's')
    {
      strncpy(spfbuffer, (const char*)theArg, STD_BUFFER_SIZE);
      spfbuffer[STD_BUFFER_SIZE - 1] = 0;
    }
    else 
      sprintf(spfbuffer, fmtstring, theArg);

    // use the formatted text
    buffer[bufidx] = 0;

    if (bufidx + strlen(spfbuffer) >= STD_BUFFER_SIZE)
      quitprintf("!String.Format: buffer overrun: maximum formatted string length %d chars, this string: %d chars", STD_BUFFER_SIZE, bufidx + strlen(spfbuffer));

    strcat(buffer, spfbuffer);
    bufidx += strlen(spfbuffer);
    curptr = endptr;
  }
  buffer[bufidx] = 0;

}

/* *** SCRIPT SYMBOL: [String] StrFormat *** */
static void _sc_sprintf(char*destt,char*texx, ...) {
  char displbuf[STD_BUFFER_SIZE];
  VALIDATE_STRING(destt);
  check_strlen(destt);
  va_list ap;
  va_start(ap,texx);
  my_sprintf(displbuf, get_translation(texx), ap);
  va_end(ap);

  my_strncpy(destt, displbuf, MAXSTRLEN - 1);
}


void register_string_script_functions() {
  scAdd_External_Symbol("String::IsNullOrEmpty^1", (void*)String_IsNullOrEmpty);
  scAdd_External_Symbol("String::Append^1", (void*)String_Append);
  scAdd_External_Symbol("String::AppendChar^1", (void*)String_AppendChar);
  scAdd_External_Symbol("String::CompareTo^2", (void*)String_CompareTo);
  scAdd_External_Symbol("String::Contains^1", (void*)StrContains);
  scAdd_External_Symbol("String::Copy^0", (void*)String_Copy);
  scAdd_External_Symbol("String::EndsWith^2", (void*)String_EndsWith);
  scAdd_External_Symbol("String::Format^101", (void*)String_Format);
  scAdd_External_Symbol("String::IndexOf^1", (void*)StrContains);
  scAdd_External_Symbol("String::LowerCase^0", (void*)String_LowerCase);
  scAdd_External_Symbol("String::Replace^3", (void*)String_Replace);
  scAdd_External_Symbol("String::ReplaceCharAt^2", (void*)String_ReplaceCharAt);
  scAdd_External_Symbol("String::StartsWith^2", (void*)String_StartsWith);
  scAdd_External_Symbol("String::Substring^2", (void*)String_Substring);
  scAdd_External_Symbol("String::Truncate^1", (void*)String_Truncate);
  scAdd_External_Symbol("String::UpperCase^0", (void*)String_UpperCase);
  scAdd_External_Symbol("String::get_AsFloat", (void*)StringToFloat);
  scAdd_External_Symbol("String::get_AsInt", (void*)StringToInt);
  scAdd_External_Symbol("String::geti_Chars", (void*)String_GetChars);
  scAdd_External_Symbol("String::get_Length", (void*)strlen);
  scAdd_External_Symbol("StrCat",(void *)_sc_strcat);
  scAdd_External_Symbol("StrCaseComp",(void *)stricmp);
  scAdd_External_Symbol("StrComp",(void *)strcmp);
  scAdd_External_Symbol("StrContains",(void *)StrContains);
  scAdd_External_Symbol("StrCopy",(void *)_sc_strcpy);
  scAdd_External_Symbol("StrFormat",(void *)_sc_sprintf);
  scAdd_External_Symbol("StrGetCharAt", (void *)StrGetCharAt);
  scAdd_External_Symbol("StringToInt",(void *)StringToInt);
  scAdd_External_Symbol("StrLen",(void *)strlen);
  scAdd_External_Symbol("StrSetCharAt", (void *)StrSetCharAt);
  scAdd_External_Symbol("StrToLowerCase", (void *)_sc_strlower);
  scAdd_External_Symbol("StrToUpperCase", (void *)_sc_strupper);
}