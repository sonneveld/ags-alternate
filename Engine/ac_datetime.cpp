
#include "ac_datetime.h"

#include "sdlwrap/allegro.h"

#include "ac.h"
#include "ac_context.h"
#include "dynobj/script_date_time.h"
#include "cscomp.h"

ScriptDateTime* DateTime_Now_Core() {
  ScriptDateTime *sdt = new ScriptDateTime();
  sdt->rawUnixTime = time(NULL);

  platform->GetSystemTime(sdt);

  return sdt;
}

/* *** SCRIPT SYMBOL: [DateTime] DateTime::get_Now *** */
ScriptDateTime* DateTime_Now() {
  ScriptDateTime *sdt = DateTime_Now_Core();
  ccRegisterManagedObject(sdt, sdt);
  return sdt;
}

/* *** SCRIPT SYMBOL: [DateTime] DateTime::get_Year *** */
int DateTime_GetYear(ScriptDateTime *sdt) {
  return sdt->year;
}

/* *** SCRIPT SYMBOL: [DateTime] DateTime::get_Month *** */
int DateTime_GetMonth(ScriptDateTime *sdt) {
  return sdt->month;
}

/* *** SCRIPT SYMBOL: [DateTime] DateTime::get_DayOfMonth *** */
int DateTime_GetDayOfMonth(ScriptDateTime *sdt) {
  return sdt->day;
}

/* *** SCRIPT SYMBOL: [DateTime] DateTime::get_Hour *** */
int DateTime_GetHour(ScriptDateTime *sdt) {
  return sdt->hour;
}

/* *** SCRIPT SYMBOL: [DateTime] DateTime::get_Minute *** */
int DateTime_GetMinute(ScriptDateTime *sdt) {
  return sdt->minute;
}

/* *** SCRIPT SYMBOL: [DateTime] DateTime::get_Second *** */
int DateTime_GetSecond(ScriptDateTime *sdt) {
  return sdt->second;
}

/* *** SCRIPT SYMBOL: [DateTime] DateTime::get_RawTime *** */
int DateTime_GetRawTime(ScriptDateTime *sdt) {
  return sdt->rawUnixTime;
}

/* *** SCRIPT SYMBOL: [DateTime] GetTime *** */
int sc_GetTime(int whatti) {
  ScriptDateTime *sdt = DateTime_Now_Core();
  int returnVal;

  if (whatti == 1) returnVal = sdt->hour;
  else if (whatti == 2) returnVal = sdt->minute;
  else if (whatti == 3) returnVal = sdt->second;
  else if (whatti == 4) returnVal = sdt->day;
  else if (whatti == 5) returnVal = sdt->month;
  else if (whatti == 6) returnVal = sdt->year;
  else quit("!GetTime: invalid parameter passed");

  delete sdt;

  return returnVal;
}

/* *** SCRIPT SYMBOL: [DateTime] GetRawTime *** */
int GetRawTime () {
  return time(NULL);
}


void register_datetime_script_functions() {
  scAdd_External_Symbol("DateTime::get_Now", (void*)DateTime_Now);
  scAdd_External_Symbol("DateTime::get_DayOfMonth", (void*)DateTime_GetDayOfMonth);
  scAdd_External_Symbol("DateTime::get_Hour", (void*)DateTime_GetHour);
  scAdd_External_Symbol("DateTime::get_Minute", (void*)DateTime_GetMinute);
  scAdd_External_Symbol("DateTime::get_Month", (void*)DateTime_GetMonth);
  scAdd_External_Symbol("DateTime::get_RawTime", (void*)DateTime_GetRawTime);
  scAdd_External_Symbol("DateTime::get_Second", (void*)DateTime_GetSecond);
  scAdd_External_Symbol("DateTime::get_Year", (void*)DateTime_GetYear);
  scAdd_External_Symbol("GetRawTime",(void *)GetRawTime);
  scAdd_External_Symbol("GetTime",(void *)sc_GetTime);
}