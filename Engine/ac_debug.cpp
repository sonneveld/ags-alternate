#include "ac_debug.h"


#define UNICODE

#include "allegro.h"

#include "ac.h"
#include "ac_context.h"

#include "acgui.h"
#include "AGSEditorDebugger.h"
#include "cscomp.h"

extern const char* ccGetSectionNameAtOffs(ccScript *scri, long offs);

#ifdef WINDOWS_VERSION
#include <crtdbg.h>
#elif defined(LINUX_VERSION) || defined(MAC_VERSION)
#define HWND long
#endif  // WINDOWS_VERSION

// ============================================================================
// DEBUG MESSAGES AND LOGGING?
// ============================================================================

void quitprintf(const char*texx, ...) {
  char displbuf[STD_BUFFER_SIZE];
  va_list ap;
  va_start(ap,texx);
  vsprintf(displbuf,texx,ap);
  va_end(ap);
  quit(displbuf);
}

void write_log(char*msg) {
  FILE*ooo=fopen("ac.log","at");
  fprintf(ooo,"%s\n",msg);
  fclose(ooo);
}

// this function is only enabled for special builds if a startup
// issue needs to be checked
//#define write_log_debug(msg) platform->WriteDebugString(msg)  //<-- moved to ac.h
/*extern "C" {
void write_log_debug(const char*msg) {
  //if (play.debug_mode == 0)
    //return;

  char toWrite[300];
  sprintf(toWrite, "%02d/%02d/%04d %02d:%02d:%02d (EIP=%4d) %s", sc_GetTime(4), sc_GetTime(5),
     sc_GetTime(6), sc_GetTime(1), sc_GetTime(2), sc_GetTime(3), our_eip, msg);

  FILE*ooo=fopen("ac.log","at");
  fprintf(ooo,"%s\n", toWrite);
  fclose(ooo);
}
}*/

/* The idea of this is that non-essential errors such as "sound file not
   found" are logged instead of exiting the program.
*/
void debug_log(const char*texx, ...) {
  // if not in debug mode, don't print it so we don't worry the
  // end player
  if (play.debug_mode == 0)
    return;
  static int first_time = 1;
  char displbuf[STD_BUFFER_SIZE];
  va_list ap;
  va_start(ap,texx);
  vsprintf(displbuf,texx,ap);
  va_end(ap);

  /*if (true) {
    char buffer2[STD_BUFFER_SIZE];
    strcpy(buffer2, "%");
    strcat(buffer2, displbuf);
    quit(buffer2);
  }*/

  char*openmode = "at";
  if (first_time) {
    openmode = "wt";
    first_time = 0;
  }
  FILE*outfil = fopen("warnings.log",openmode);
  if (outfil == NULL)
  {
    debug_write_console("* UNABLE TO WRITE TO WARNINGS.LOG");
    debug_write_console(displbuf);
  }
  else
  {
    fprintf(outfil,"(in room %d): %s\n",displayed_room,displbuf);
    fclose(outfil);
  }
}


void debug_write_console (const char *msg, ...) {
  char displbuf[STD_BUFFER_SIZE];
  va_list ap;
  va_start(ap,msg);
  vsprintf(displbuf,msg,ap);
  va_end(ap);
  displbuf[99] = 0;

  strcpy (debug_line[last_debug_line].text, displbuf);
  ccInstance*curinst = ccGetCurrentInstance();
  if (curinst != NULL) {
    char scriptname[20];
    if (curinst->instanceof == gamescript)
      strcpy(scriptname,"G ");
    else if (curinst->instanceof == thisroom.compiled_script)
      strcpy (scriptname, "R ");
    else if (curinst->instanceof == dialogScriptsScript)
      strcpy(scriptname,"D ");
    else
      strcpy(scriptname,"? ");
    sprintf(debug_line[last_debug_line].script,"%s%d",scriptname,currentline);
  }
  else debug_line[last_debug_line].script[0] = 0;

  platform->WriteDebugString("%s (%s)", displbuf, debug_line[last_debug_line].script);

  last_debug_line = (last_debug_line + 1) % DEBUG_CONSOLE_NUMLINES;

  if (last_debug_line == first_debug_line)
    first_debug_line = (first_debug_line + 1) % DEBUG_CONSOLE_NUMLINES;

}

// usually for debug messages
const char *get_cur_script(int numberOfLinesOfCallStack) {
  ccGetCallStack(ccGetCurrentInstance(), pexbuf, numberOfLinesOfCallStack);

  if (pexbuf[0] == 0)
    strcpy(pexbuf, ccErrorCallStack);

  return &pexbuf[0];
}



// ============================================================================
// EDITOR INTEGRATION
// ============================================================================

static const char* BREAK_MESSAGE = "BREAK";

struct Breakpoint
{
  char scriptName[80];
  int lineNumber;
};

DynamicArray<Breakpoint> breakpoints;
int numBreakpoints = 0;

bool send_message_to_editor(const char *msg, const char *errorMsg) 
{
  const char *callStack = get_cur_script(25);
  if (callStack[0] == 0)
    return false;

  char messageToSend[STD_BUFFER_SIZE];
  sprintf(messageToSend, "<?xml version=\"1.0\" encoding=\"Windows-1252\"?><Debugger Command=\"%s\">", msg);
#ifdef WINDOWS_VERSION
  sprintf(&messageToSend[strlen(messageToSend)], "  <EngineWindow>%d</EngineWindow> ", alw_win_get_window());
#endif
  sprintf(&messageToSend[strlen(messageToSend)], "  <ScriptState><![CDATA[%s]]></ScriptState> ", callStack);
  if (errorMsg != NULL)
  {
    sprintf(&messageToSend[strlen(messageToSend)], "  <ErrorMessage><![CDATA[%s]]></ErrorMessage> ", errorMsg);
  }
  strcat(messageToSend, "</Debugger>");

  editor_debugger->SendMessageToEditor(messageToSend);

  return true;
}

bool send_message_to_editor(const char *msg) 
{
  return send_message_to_editor(msg, NULL);
}

bool init_editor_debugging() 
{
  editor_debugger = GetEditorDebugger(editor_debugger_instance_token);

  if (editor_debugger == NULL)
    quit("editor_debugger is NULL but debugger enabled");

  if (editor_debugger->Initialize())
  {
    editor_debugging_initialized = 1;

    // Wait for the editor to send the initial breakpoints
    // and then its READY message
    while (check_for_messages_from_editor() != 2) 
    {
      platform->Delay(10);
    }

    send_message_to_editor("START");
    return true;
  }

  return false;
}

int check_for_messages_from_editor()
{
  if (editor_debugger->IsMessageAvailable())
  {
    char *msg = editor_debugger->GetNextMessage();
    if (msg == NULL)
    {
      return 0;
    }

    if (strncmp(msg, "<Engine Command=\"", 17) != 0) 
    {
      //OutputDebugString("Faulty message received from editor:");
      //OutputDebugString(msg);
      free(msg);
      return 0;
    }

    const char *msgPtr = &msg[17];

    if (strncmp(msgPtr, "START", 5) == 0)
    {
      const char *windowHandle = strstr(msgPtr, "EditorWindow") + 14;
#ifdef WINDOWS_VERSION
      editor_window_handle = (HWND)atoi(windowHandle);
#endif
    }
    else if (strncmp(msgPtr, "READY", 5) == 0)
    {
      free(msg);
      return 2;
    }
    else if ((strncmp(msgPtr, "SETBREAK", 8) == 0) ||
             (strncmp(msgPtr, "DELBREAK", 8) == 0))
    {
      bool isDelete = (msgPtr[0] == 'D');
      // Format:  SETBREAK $scriptname$lineNumber$
      msgPtr += 10;
      char scriptNameBuf[100];
      int i = 0;
      while (msgPtr[0] != '$')
      {
        scriptNameBuf[i] = msgPtr[0];
        msgPtr++;
        i++;
      }
      scriptNameBuf[i] = 0;
      msgPtr++;

      int lineNumber = atoi(msgPtr);

      if (isDelete) 
      {
        for (i = 0; i < numBreakpoints; i++)
        {
          if ((breakpoints[i].lineNumber == lineNumber) &&
              (strcmp(breakpoints[i].scriptName, scriptNameBuf) == 0))
          {
            numBreakpoints--;
            for (int j = i; j < numBreakpoints; j++)
            {
              breakpoints[j] = breakpoints[j + 1];
            }
            break;
          }
        }
      }
      else 
      {
        strcpy(breakpoints[numBreakpoints].scriptName, scriptNameBuf);
        breakpoints[numBreakpoints].lineNumber = lineNumber;
        numBreakpoints++;
      }
    }
    else if (strncmp(msgPtr, "RESUME", 6) == 0) 
    {
      game_paused_in_debugger = 0;
    }
    else if (strncmp(msgPtr, "STEP", 4) == 0) 
    {
      game_paused_in_debugger = 0;
      break_on_next_script_step = 1;
    }
    else if (strncmp(msgPtr, "EXIT", 4) == 0) 
    {
      want_exit = 1;
      abort_engine = 1;
      check_dynamic_sprites_at_exit = 0;
    }

    free(msg);
    return 1;
  }

  return 0;
}



// ============================================================================
// DEBUGGER
// ============================================================================

void break_into_debugger() 
{
#ifdef WINDOWS_VERSION

  if (editor_window_handle != NULL)
    SetForegroundWindow(editor_window_handle);

  send_message_to_editor("BREAK");
  game_paused_in_debugger = 1;

  while (game_paused_in_debugger) 
  {
    update_polled_stuff();
    platform->YieldCPU();
  }

#endif
}

int scrDebugWait = 0;
// allow LShift to single-step,  RShift to pause flow
void scriptDebugHook (ccInstance *ccinst, int linenum) {

  if (pluginsWantingDebugHooks > 0) {
    // a plugin is handling the debugging
    char scname[40];
    get_script_name(ccinst, scname);
    platform->RunPluginDebugHooks(scname, linenum);
    return;
  }

  // no plugin, use built-in debugger

  if (ccinst == NULL) 
  {
    // come out of script
    return;
  }

  if (break_on_next_script_step) 
  {
    break_on_next_script_step = 0;
    break_into_debugger();
    return;
  }

  const char *scriptName = ccGetSectionNameAtOffs(ccinst->runningInst->instanceof, ccinst->pc);

  for (int i = 0; i < numBreakpoints; i++)
  {
    if ((breakpoints[i].lineNumber == linenum) &&
        (strcmp(breakpoints[i].scriptName, scriptName) == 0))
    {
      break_into_debugger();
      break;
    }
  }
}

void check_debug_keys() {
    if (play.debug_mode) {
      // do the run-time script debugging

      alw_poll_keyboard();
      
      if ((!alw_get_key(KEY_SCRLOCK)) && (scrlockWasDown))
        scrlockWasDown = 0;
      else if ((alw_get_key(KEY_SCRLOCK)) && (!scrlockWasDown)) {

        break_on_next_script_step = 1;
        scrlockWasDown = 1;
      }

    }

}



// ============================================================================
// DEBUG - MINIDUMP
// ============================================================================

#ifdef WINDOWS_VERSION
CONTEXT cpustate;
EXCEPTION_RECORD excinfo;
int miniDumpResultCode = 0;

typedef enum _MINIDUMP_TYPE {
    MiniDumpNormal                         = 0x0000,
    MiniDumpWithDataSegs                   = 0x0001,
    MiniDumpWithFullMemory                 = 0x0002,
    MiniDumpWithHandleData                 = 0x0004,
    MiniDumpFilterMemory                   = 0x0008,
    MiniDumpScanMemory                     = 0x0010,
    MiniDumpWithUnloadedModules            = 0x0020,
    MiniDumpWithIndirectlyReferencedMemory = 0x0040,
    MiniDumpFilterModulePaths              = 0x0080,
    MiniDumpWithProcessThreadData          = 0x0100,
    MiniDumpWithPrivateReadWriteMemory     = 0x0200,
    MiniDumpWithoutOptionalData            = 0x0400,
} MINIDUMP_TYPE;

typedef struct _MINIDUMP_EXCEPTION_INFORMATION {
    DWORD ThreadId;
    PEXCEPTION_POINTERS ExceptionPointers;
    BOOL ClientPointers;
} MINIDUMP_EXCEPTION_INFORMATION, *PMINIDUMP_EXCEPTION_INFORMATION;

typedef BOOL (WINAPI * MINIDUMPWRITEDUMP)(HANDLE hProcess, DWORD ProcessId, 
  HANDLE hFile, MINIDUMP_TYPE DumpType, 
  CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam, 
  CONST void* UserStreamParam, 
  CONST void* CallbackParam); 

MINIDUMPWRITEDUMP _MiniDumpWriteDump;


void CreateMiniDump( EXCEPTION_POINTERS* pep ) 
{
  HMODULE dllHandle = LoadLibrary(L"dbghelp.dll");
  if (dllHandle == NULL)
  {
    miniDumpResultCode = 1;
    return;
  }

  _MiniDumpWriteDump = (MINIDUMPWRITEDUMP)GetProcAddress(dllHandle, "MiniDumpWriteDump");
  if (_MiniDumpWriteDump == NULL)
  {
    FreeLibrary(dllHandle);
    miniDumpResultCode = 2;
    return;
  }

  char fileName[80];
  sprintf(fileName, "CrashInfo.%s.dmp", ACI_VERSION_TEXT);
  HANDLE hFile = CreateFileA(fileName, GENERIC_READ | GENERIC_WRITE, 
    0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL); 

  if((hFile != NULL) && (hFile != INVALID_HANDLE_VALUE))
  {
    MINIDUMP_EXCEPTION_INFORMATION mdei; 

    mdei.ThreadId = GetCurrentThreadId();
    mdei.ExceptionPointers = pep;
    mdei.ClientPointers = FALSE;

    MINIDUMP_TYPE mdt = MiniDumpNormal; //MiniDumpWithPrivateReadWriteMemory;

    BOOL rv = _MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), 
      hFile, mdt, (pep != 0) ? &mdei : 0, NULL, NULL); 

    if (!rv)
      miniDumpResultCode = 4;

    CloseHandle(hFile); 
  }
  else
    miniDumpResultCode = 3;

  FreeLibrary(dllHandle);
}

int CustomExceptionHandler (LPEXCEPTION_POINTERS exinfo) {
  cpustate = exinfo->ContextRecord[0];
  excinfo = exinfo->ExceptionRecord[0];
  CreateMiniDump(exinfo);
  
  return EXCEPTION_EXECUTE_HANDLER;
}

FILE *logfile;
int OurReportingFunction( int reportType, char *userMessage, int *retVal ) {

  fprintf(logfile,"%s: %s\n",(reportType == _CRT_ASSERT) ? "Assertion failed" : "Warning",userMessage);
  fflush (logfile);
  return 0;
}
#endif

#if defined(WINDOWS_VERSION)
#include <new.h>
char tempmsg[100];
char*printfworkingspace;
int malloc_fail_handler(size_t amountwanted) {
  free(printfworkingspace);
  sprintf(tempmsg,"Out of memory: failed to allocate %ld bytes (at PP=%d)",amountwanted, our_eip);
  quit(tempmsg);
  return 0;
}
#endif

void install_debug_handlers() {
  #if defined(WINDOWS_VERSION)
  _set_new_handler(malloc_fail_handler);
  _set_new_mode(1);
  printfworkingspace=(char*)malloc(7000);
#endif
}