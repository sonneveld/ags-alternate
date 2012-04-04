#include "ac_text.h"

#include "sdlwrap/allegro.h"

#include "ac.h"
#include "ac_context.h"
#include "ac_string.h"
#include "acchars.h"



/* *** SCRIPT SYMBOL: [Text] DisplayAt *** */
void DisplayAt(int xxp,int yyp,int widd,char*texx, ...) {
  char displbuf[STD_BUFFER_SIZE];
  va_list ap;
  va_start(ap,texx);
  my_sprintf(displbuf, get_translation(texx), ap);
  va_end(ap);

  multiply_up_coordinates(&xxp, &yyp);
  widd = multiply_up_coordinate(widd);
  
  if (widd<1) widd=scrnwid/2;
  if (xxp<0) xxp=scrnwid/2-widd/2;
  _display_at(xxp,yyp,widd,displbuf,1,0, 0, 0, false);
  }




/* *** SCRIPT SYMBOL: [Text] SetSpeechStyle *** */
void SetSpeechStyle (int newstyle) {
  if ((newstyle < 0) || (newstyle > 3))
    quit("!SetSpeechStyle: must use a SPEECH_* constant as parameter");
  game.options[OPT_SPEECHTYPE] = newstyle;
}

/* *** SCRIPT SYMBOL: [Text] PlaySpeech *** */
void __scr_play_speech(int who, int which) {
  // *** implement this - needs to call stop_speech as well
  // to reset the volume
  quit("PlaySpeech not yet implemented");
}



int user_to_internal_skip_speech(int userval) {
  // 0 = click mouse or key to skip
  if (userval == 0)
    return SKIP_AUTOTIMER | SKIP_KEYPRESS | SKIP_MOUSECLICK;
  // 1 = key only
  else if (userval == 1)
    return SKIP_AUTOTIMER | SKIP_KEYPRESS;
  // 2 = can't skip at all
  else if (userval == 2)
    return SKIP_AUTOTIMER;
  // 3 = only on keypress, no auto timer
  else if (userval == 3)
    return SKIP_KEYPRESS | SKIP_MOUSECLICK;
  // 4 = mouse only
  else if (userval == 4)
    return SKIP_AUTOTIMER | SKIP_MOUSECLICK;
  else
    quit("user_to_internal_skip_speech: unknown userval");

  return 0;
}


// 0 = click mouse or key to skip
// 1 = key only
// 2 = can't skip at all
// 3 = only on keypress, no auto timer
// 4 = mouseclick only
/* *** SCRIPT SYMBOL: [Text] SetSkipSpeech *** */
void SetSkipSpeech (int newval) {
  if ((newval < 0) || (newval > 4))
    quit("!SetSkipSpeech: invalid skip mode specified (0-4)");

  DEBUG_CONSOLE("SkipSpeech style set to %d", newval);
  play.cant_skip_speech = user_to_internal_skip_speech(newval);
}

/* *** SCRIPT SYMBOL: [Text] DisplayAtY *** */
void DisplayAtY (int ypos, char *texx) {
  if ((ypos < -1) || (ypos >= GetMaxScreenHeight()))
    quitprintf("!DisplayAtY: invalid Y co-ordinate supplied (used: %d; valid: 0..%d)", ypos, GetMaxScreenHeight());

  // Display("") ... a bit of a stupid thing to do, so ignore it
  if (texx[0] == 0)
    return;

  if (ypos > 0)
    ypos = multiply_up_coordinate(ypos);

  if (game.options[OPT_ALWAYSSPCH])
    DisplaySpeechAt(-1, (ypos > 0) ? divide_down_coordinate(ypos) : ypos, -1, game.playercharacter, texx);
  else { 
    // Normal "Display" in text box

    if (screen_is_dirty) {
      // erase any previous DisplaySpeech
      play.disabled_user_interface ++;
      mainloop();
      play.disabled_user_interface --;
    }

    _display_at(-1,ypos,scrnwid/2+scrnwid/4,get_translation(texx),1,0, 0, 0, false);
  }
}

/* *** SCRIPT SYMBOL: [Text] Display *** */
void Display(char*texx, ...) {
  char displbuf[STD_BUFFER_SIZE];
  va_list ap;
  va_start(ap,texx);
  my_sprintf(displbuf, get_translation(texx), ap);
  va_end(ap);
  DisplayAtY (-1, displbuf);
}


/* *** SCRIPT SYMBOL: [Text] DisplayTopBar *** */
void DisplayTopBar(int ypos, int ttexcol, int backcol, char *title, char*texx, ...) {

  strcpy(topBar.text, get_translation(title));

  char displbuf[3001];
  va_list ap;
  va_start(ap,texx);
  my_sprintf(displbuf, get_translation(texx), ap);
  va_end(ap);

  if (ypos > 0)
    play.top_bar_ypos = ypos;
  if (ttexcol > 0)
    play.top_bar_textcolor = ttexcol;
  if (backcol > 0)
    play.top_bar_backcolor = backcol;

  topBar.wantIt = 1;
  topBar.font = FONT_NORMAL;
  topBar.height = wgetfontheight(topBar.font);
  topBar.height += multiply_up_coordinate(play.top_bar_borderwidth) * 2 + get_fixed_pixel_size(1);

  // they want to customize the font
  if (play.top_bar_font >= 0)
    topBar.font = play.top_bar_font;

  // DisplaySpeech normally sets this up, but since we're not going via it...
  if (play.cant_skip_speech & SKIP_AUTOTIMER)
    play.messagetime = GetTextDisplayTime(texx);

  DisplayAtY(play.top_bar_ypos, displbuf);
}

// Display a room/global message in the bar
/* *** SCRIPT SYMBOL: [Text] DisplayMessageBar *** */
void DisplayMessageBar(int ypos, int ttexcol, int backcol, char *title, int msgnum) {
  char msgbufr[3001];
  get_message_text(msgnum, msgbufr);
  DisplayTopBar(ypos, ttexcol, backcol, title, "%s", msgbufr);
}



void replace_tokens(char*srcmes,char*destm, int maxlen) {
  int indxdest=0,indxsrc=0;
  char*srcp,*destp;
  while (srcmes[indxsrc]!=0) {
    srcp=&srcmes[indxsrc];
    destp=&destm[indxdest];
    if ((strncmp(srcp,"@IN",3)==0) | (strncmp(srcp,"@GI",3)==0)) {
      int tokentype=0;
      if (srcp[1]=='I') tokentype=1;
      else tokentype=2;
      int inx=atoi(&srcp[3]);
      srcp++;
      indxsrc+=2;
      while (srcp[0]!='@') {
        if (srcp[0]==0) quit("!Display: special token not terminated");
        srcp++;
        indxsrc++;
        }
      char tval[10];
      if (tokentype==1) {
        if ((inx<1) | (inx>=game.numinvitems))
          quit("!Display: invalid inv item specified in @IN@");
        sprintf(tval,"%d",playerchar->inv[inx]);
        }
      else {
        if ((inx<0) | (inx>=MAXGSVALUES))
          quit("!Display: invalid global int index speicifed in @GI@");
        sprintf(tval,"%d",GetGlobalInt(inx));
        }
      strcpy(destp,tval);
      indxdest+=strlen(tval);
      }
    else {
      destp[0]=srcp[0];
      indxdest++;
      indxsrc++;
      }
    if (indxdest >= maxlen - 3)
      break;
    }
  destm[indxdest]=0;
  }

char *get_global_message (int msnum) {
  if (game.messages[msnum-500] == NULL)
    return "";
  return get_translation(game.messages[msnum-500]);
}

int display_message_aschar=0;
void get_message_text (int msnum, char *buffer, char giveErr) {
  int maxlen = 9999;
  if (!giveErr)
    maxlen = MAX_MAXSTRLEN;

  if (msnum>=500) { //quit("global message requseted, nto yet supported");

    if ((msnum >= MAXGLOBALMES + 500) || (game.messages[msnum-500]==NULL)) {
      if (giveErr)
        quit("!DisplayGlobalMessage: message does not exist");
      buffer[0] = 0;
      return;
    }
    buffer[0] = 0;
    replace_tokens(get_translation(game.messages[msnum-500]), buffer, maxlen);
    return;
  }
  else if (msnum >= thisroom.nummes) {
    if (giveErr)
      quit("!DisplayMessage: Invalid message number to display");
    buffer[0] = 0;
    return;
  }

  buffer[0]=0;
  replace_tokens(get_translation(thisroom.message[msnum]), buffer, maxlen);

}


/* *** SCRIPT SYMBOL: [Game] Game::geti_GlobalMessages *** */
const char* Game_GetGlobalMessages(int index) {
  if ((index < 500) || (index >= MAXGLOBALMES + 500)) {
    return NULL;
  }
  char buffer[STD_BUFFER_SIZE];
  buffer[0] = 0;
  replace_tokens(get_translation(get_global_message(index)), buffer, STD_BUFFER_SIZE);
  return CreateNewScriptString(buffer);
}

/* *** SCRIPT SYMBOL: [Text] DisplayMessageAtY *** */
void DisplayMessageAtY(int msnum, int ypos) {
  char msgbufr[3001];
  if (msnum>=500) { //quit("global message requseted, nto yet supported");
    get_message_text (msnum, msgbufr);
    if (display_message_aschar > 0)
      DisplaySpeech(msgbufr, display_message_aschar);
    else
      DisplayAtY(ypos, msgbufr);
    display_message_aschar=0;
    return;
  }

  if (display_message_aschar > 0) {
    display_message_aschar=0;
    quit("!DisplayMessage: data column specified a character for local\n"
         "message; use the message editor to select the character for room\n"
         "messages.\n");
  }

  int repeatloop=1;
  while (repeatloop) {
    get_message_text (msnum, msgbufr);

    if (thisroom.msgi[msnum].displayas>0) {
      DisplaySpeech(msgbufr, thisroom.msgi[msnum].displayas - 1);
    }
    else {
      // time out automatically if they have set that
      int oldGameSkipDisp = play.skip_display;
      if (thisroom.msgi[msnum].flags & MSG_TIMELIMIT)
        play.skip_display = 0;

      DisplayAtY(ypos, msgbufr);

      play.skip_display = oldGameSkipDisp;
    }
    if (thisroom.msgi[msnum].flags & MSG_DISPLAYNEXT) {
      msnum++;
      repeatloop=1;
    }
    else
      repeatloop=0;
  }

}

/* *** SCRIPT SYMBOL: [Text] DisplayMessage *** */
void DisplayMessage(int msnum) {
  DisplayMessageAtY (msnum, -1);
}



void register_text_script_functions() {
  scAdd_External_Symbol("Game::geti_GlobalMessages",(void *)Game_GetGlobalMessages);
  scAdd_External_Symbol("Display",(void *)Display);
  scAdd_External_Symbol("DisplayAt",(void *)DisplayAt);
  scAdd_External_Symbol("DisplayAtY",(void *)DisplayAtY);
  scAdd_External_Symbol("DisplayMessage",(void *)DisplayMessage);
  scAdd_External_Symbol("DisplayMessageAtY",(void *)DisplayMessageAtY);
  scAdd_External_Symbol("DisplayMessageBar",(void *)DisplayMessageBar);
  scAdd_External_Symbol("DisplayTopBar",(void *)DisplayTopBar);
  scAdd_External_Symbol("PlaySpeech",(void *)__scr_play_speech);
  scAdd_External_Symbol("SetSkipSpeech",(void *)SetSkipSpeech);
  scAdd_External_Symbol("SetSpeechStyle", (void *)SetSpeechStyle);

}

