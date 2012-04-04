#include "ac_dialog.h"

#include "sdlwrap/allegro.h"

#include "ac.h"
#include "ac_context.h"
#include "ac_string.h"
#include "ac_exescr.h"


/* *** SCRIPT SYMBOL: [Dialog] RunDialog *** */
void RunDialog(int tum) {
  if ((tum<0) | (tum>=game.numdialog))
    quit("!RunDialog: invalid topic number specified");

  can_run_delayed_command();

  if (play.stop_dialog_at_end != DIALOG_NONE) {
    if (play.stop_dialog_at_end == DIALOG_RUNNING)
      play.stop_dialog_at_end = DIALOG_NEWTOPIC + tum;
    else
      quit("!NewRoom: two NewRoom/RunDiaolg/StopDialog requests within dialog");
    return;
  }

  if (inside_script) 
    curscript->queue_action(ePSARunDialog, tum, "RunDialog");
  else
    do_conversation(tum);
}


/* *** SCRIPT SYMBOL: [Dialog] StopDialog *** */
static void StopDialog() {
  if (play.stop_dialog_at_end == DIALOG_NONE) {
    debug_log("StopDialog called, but was not in a dialog");
    DEBUG_CONSOLE("StopDialog called but no dialog");
    return;
  }
  play.stop_dialog_at_end = DIALOG_STOP;
}

/* *** SCRIPT SYMBOL: [Dialog] SetDialogOption *** */
void SetDialogOption(int dlg,int opt,int onoroff) {
  if ((dlg<0) | (dlg>=game.numdialog))
    quit("!SetDialogOption: Invalid topic number specified");
  if ((opt<1) | (opt>dialog[dlg].numoptions))
    quit("!SetDialogOption: Invalid option number specified");
  opt--;

  dialog[dlg].optionflags[opt]&=~DFLG_ON;
  if ((onoroff==1) & ((dialog[dlg].optionflags[opt] & DFLG_OFFPERM)==0))
    dialog[dlg].optionflags[opt]|=DFLG_ON;
  else if (onoroff==2)
    dialog[dlg].optionflags[opt]|=DFLG_OFFPERM;
}

/* *** SCRIPT SYMBOL: [Dialog] GetDialogOption *** */
static int GetDialogOption (int dlg, int opt) {
  if ((dlg<0) | (dlg>=game.numdialog))
    quit("!GetDialogOption: Invalid topic number specified");
  if ((opt<1) | (opt>dialog[dlg].numoptions))
    quit("!GetDialogOption: Invalid option number specified");
  opt--;

  if (dialog[dlg].optionflags[opt] & DFLG_OFFPERM)
    return 2;
  if (dialog[dlg].optionflags[opt] & DFLG_ON)
    return 1;
  return 0;
}

/* *** SCRIPT SYMBOL: [Game] Game::get_DialogCount *** */
static int Game_GetDialogCount()
{
  return game.numdialog;
}

/* *** SCRIPT SYMBOL: [Dialog] Dialog::Start^0 *** */
static void Dialog_Start(ScriptDialog *sd) {
  RunDialog(sd->id);
}

/* *** SCRIPT SYMBOL: [Dialog] Dialog::DisplayOptions^1 *** */
static int Dialog_DisplayOptions(ScriptDialog *sd, int sayChosenOption) 
{
  if ((sayChosenOption < 1) || (sayChosenOption > 3))
    quit("!Dialog.DisplayOptions: invalid parameter passed");

  int chose = show_dialog_options(sd->id, sayChosenOption, (game.options[OPT_RUNGAMEDLGOPTS] != 0));
  if (chose != CHOSE_TEXTPARSER)
  {
    chose++;
  }
  return chose;
}

/* *** SCRIPT SYMBOL: [Dialog] Dialog::SetOptionState^2 *** */
static void Dialog_SetOptionState(ScriptDialog *sd, int option, int newState) {
  SetDialogOption(sd->id, option, newState);
}

/* *** SCRIPT SYMBOL: [Dialog] Dialog::GetOptionState^1 *** */
static int Dialog_GetOptionState(ScriptDialog *sd, int option) {
  return GetDialogOption(sd->id, option);
}

/* *** SCRIPT SYMBOL: [Dialog] Dialog::HasOptionBeenChosen^1 *** */
static int Dialog_HasOptionBeenChosen(ScriptDialog *sd, int option)
{
  if ((option < 1) || (option > dialog[sd->id].numoptions))
    quit("!Dialog.HasOptionBeenChosen: Invalid option number specified");
  option--;

  if (dialog[sd->id].optionflags[option] & DFLG_HASBEENCHOSEN)
    return 1;
  return 0;
}

/* *** SCRIPT SYMBOL: [Dialog] Dialog::get_OptionCount *** */
static int Dialog_GetOptionCount(ScriptDialog *sd)
{
  return dialog[sd->id].numoptions;
}

/* *** SCRIPT SYMBOL: [Dialog] Dialog::get_ShowTextParser *** */
static int Dialog_GetShowTextParser(ScriptDialog *sd)
{
  return (dialog[sd->id].topicFlags & DTFLG_SHOWPARSER) ? 1 : 0;
}

/* *** SCRIPT SYMBOL: [Dialog] Dialog::GetOptionText^1 *** */
static const char* Dialog_GetOptionText(ScriptDialog *sd, int option)
{
  if ((option < 1) || (option > dialog[sd->id].numoptions))
    quit("!Dialog.GetOptionText: Invalid option number specified");

  option--;

  return CreateNewScriptString(get_translation(dialog[sd->id].optionnames[option]));
}

/* *** SCRIPT SYMBOL: [Dialog] Dialog::get_ID *** */
static int Dialog_GetID(ScriptDialog *sd) {
  return sd->id;
}


void register_dialog_script_functions() {
  scAdd_External_Symbol("Dialog::get_ID", (void *)Dialog_GetID);
  scAdd_External_Symbol("Dialog::get_OptionCount", (void *)Dialog_GetOptionCount);
  scAdd_External_Symbol("Dialog::get_ShowTextParser", (void *)Dialog_GetShowTextParser);
  scAdd_External_Symbol("Dialog::DisplayOptions^1", (void *)Dialog_DisplayOptions);
  scAdd_External_Symbol("Dialog::GetOptionState^1", (void *)Dialog_GetOptionState);
  scAdd_External_Symbol("Dialog::GetOptionText^1", (void *)Dialog_GetOptionText);
  scAdd_External_Symbol("Dialog::HasOptionBeenChosen^1", (void *)Dialog_HasOptionBeenChosen);
  scAdd_External_Symbol("Dialog::SetOptionState^2", (void *)Dialog_SetOptionState);
  scAdd_External_Symbol("Dialog::Start^0", (void *)Dialog_Start);
  scAdd_External_Symbol("Game::get_DialogCount", (void *)Game_GetDialogCount);
  scAdd_External_Symbol("GetDialogOption",(void *)GetDialogOption);
  scAdd_External_Symbol("RunDialog",(void *)RunDialog);
  scAdd_External_Symbol("SetDialogOption",(void *)SetDialogOption);
  scAdd_External_Symbol("StopDialog",(void *)StopDialog);
}