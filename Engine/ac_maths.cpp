#include <math.h>
#include "ac.h"
#include "ac_maths.h"

enum RoundDirections {
  eRoundDown = 0,
  eRoundNearest = 1,
  eRoundUp = 2
};

/* *** SCRIPT SYMBOL: [Maths] FloatToInt *** */
static int FloatToInt(SCRIPT_FLOAT(value), int roundDirection) {
  INIT_SCRIPT_FLOAT(value);

  int intval;

  if (value >= 0.0) {
    if (roundDirection == eRoundDown)
      intval = (int)value;
    else if (roundDirection == eRoundNearest)
      intval = (int)(value + 0.5);
    else if (roundDirection == eRoundUp)
      intval = (int)(value + 0.999999);
    else
      quit("!FloatToInt: invalid round direction");
  }
  else {
    // negative number
    if (roundDirection == eRoundUp)
      intval = (int)value; // this just truncates
    else if (roundDirection == eRoundNearest)
      intval = (int)(value - 0.5);
    else if (roundDirection == eRoundDown)
      intval = (int)(value - 0.999999);
    else
      quit("!FloatToInt: invalid round direction");
  }

  return intval;
}

/* *** SCRIPT SYMBOL: [Maths] IntToFloat *** */
static FLOAT_RETURN_TYPE IntToFloat(int value) {
  float fval = (float)value;

  RETURN_FLOAT(fval);
}



/* *** SCRIPT SYMBOL: [Maths] Maths::Cos^1 *** */
static FLOAT_RETURN_TYPE Math_Cos(SCRIPT_FLOAT(value)) {
  INIT_SCRIPT_FLOAT(value);

  value = ::cos(value);

  RETURN_FLOAT(value);
}

/* *** SCRIPT SYMBOL: [Maths] Maths::Sin^1 *** */
static FLOAT_RETURN_TYPE Math_Sin(SCRIPT_FLOAT(value)) {
  INIT_SCRIPT_FLOAT(value);

  value = ::sin(value);

  RETURN_FLOAT(value);
}

/* *** SCRIPT SYMBOL: [Maths] Maths::Tan^1 *** */
static FLOAT_RETURN_TYPE Math_Tan(SCRIPT_FLOAT(value)) {
  INIT_SCRIPT_FLOAT(value);

  value = ::tan(value);

  RETURN_FLOAT(value);
}

/* *** SCRIPT SYMBOL: [Maths] Maths::ArcCos^1 *** */
static FLOAT_RETURN_TYPE Math_ArcCos(SCRIPT_FLOAT(value)) {
  INIT_SCRIPT_FLOAT(value);

  value = ::acos(value);

  RETURN_FLOAT(value);
}

/* *** SCRIPT SYMBOL: [Maths] Maths::ArcSin^1 *** */
static FLOAT_RETURN_TYPE Math_ArcSin(SCRIPT_FLOAT(value)) {
  INIT_SCRIPT_FLOAT(value);

  value = ::asin(value);

  RETURN_FLOAT(value);
}

/* *** SCRIPT SYMBOL: [Maths] Maths::ArcTan^1 *** */
static FLOAT_RETURN_TYPE Math_ArcTan(SCRIPT_FLOAT(value)) {
  INIT_SCRIPT_FLOAT(value);

  value = ::atan(value);

  RETURN_FLOAT(value);
}

/* *** SCRIPT SYMBOL: [Maths] Maths::ArcTan2^2 *** */
static FLOAT_RETURN_TYPE Math_ArcTan2(SCRIPT_FLOAT(yval), SCRIPT_FLOAT(xval)) {
  INIT_SCRIPT_FLOAT(yval);
  INIT_SCRIPT_FLOAT(xval);

  float value = ::atan2(yval, xval);

  RETURN_FLOAT(value);
}

/* *** SCRIPT SYMBOL: [Maths] Maths::Log^1 *** */
static FLOAT_RETURN_TYPE Math_Log(SCRIPT_FLOAT(num)) {
  INIT_SCRIPT_FLOAT(num);

  float value = ::log(num);

  RETURN_FLOAT(value);
}

/* *** SCRIPT SYMBOL: [Maths] Maths::Log10^1 *** */
static FLOAT_RETURN_TYPE Math_Log10(SCRIPT_FLOAT(num)) {
  INIT_SCRIPT_FLOAT(num);

  float value = ::log10(num);

  RETURN_FLOAT(value);
}

/* *** SCRIPT SYMBOL: [Maths] Maths::Exp^1 *** */
static FLOAT_RETURN_TYPE Math_Exp(SCRIPT_FLOAT(num)) {
  INIT_SCRIPT_FLOAT(num);

  float value = ::exp(num);

  RETURN_FLOAT(value);
}

/* *** SCRIPT SYMBOL: [Maths] Maths::Cosh^1 *** */
static FLOAT_RETURN_TYPE Math_Cosh(SCRIPT_FLOAT(num)) {
  INIT_SCRIPT_FLOAT(num);

  float value = ::cosh(num);

  RETURN_FLOAT(value);
}

/* *** SCRIPT SYMBOL: [Maths] Maths::Sinh^1 *** */
static FLOAT_RETURN_TYPE Math_Sinh(SCRIPT_FLOAT(num)) {
  INIT_SCRIPT_FLOAT(num);

  float value = ::sinh(num);

  RETURN_FLOAT(value);
}

/* *** SCRIPT SYMBOL: [Maths] Maths::Tanh^1 *** */
static FLOAT_RETURN_TYPE Math_Tanh(SCRIPT_FLOAT(num)) {
  INIT_SCRIPT_FLOAT(num);

  float value = ::tanh(num);

  RETURN_FLOAT(value);
}

/* *** SCRIPT SYMBOL: [Maths] Maths::RaiseToPower^2 *** */
static FLOAT_RETURN_TYPE Math_RaiseToPower(SCRIPT_FLOAT(base), SCRIPT_FLOAT(exp)) {
  INIT_SCRIPT_FLOAT(base);
  INIT_SCRIPT_FLOAT(exp);

  float value = ::pow(base, exp);

  RETURN_FLOAT(value);
}

/* *** SCRIPT SYMBOL: [Maths] Maths::DegreesToRadians^1 *** */
static FLOAT_RETURN_TYPE Math_DegreesToRadians(SCRIPT_FLOAT(value)) {
  INIT_SCRIPT_FLOAT(value);

  value = value * (M_PI / 180.0);

  RETURN_FLOAT(value);
}

/* *** SCRIPT SYMBOL: [Maths] Maths::RadiansToDegrees^1 *** */
static FLOAT_RETURN_TYPE Math_RadiansToDegrees(SCRIPT_FLOAT(value)) {
  INIT_SCRIPT_FLOAT(value);

  value = value * (180.0 / M_PI);

  RETURN_FLOAT(value);
}

/* *** SCRIPT SYMBOL: [Maths] Maths::get_Pi *** */
static FLOAT_RETURN_TYPE Math_GetPi() {
  float pi = M_PI;

  RETURN_FLOAT(pi);
}

/* *** SCRIPT SYMBOL: [Maths] Maths::Sqrt^1 *** */
static FLOAT_RETURN_TYPE Math_Sqrt(SCRIPT_FLOAT(value)) {
  INIT_SCRIPT_FLOAT(value);

  if (value < 0.0)
    quit("!Sqrt: cannot perform square root of negative number");

  value = ::sqrt(value);

  RETURN_FLOAT(value);
}


void register_maths_script_functions() {
  scAdd_External_Symbol("Maths::ArcCos^1", (void*)Math_ArcCos);
  scAdd_External_Symbol("Maths::ArcSin^1", (void*)Math_ArcSin);
  scAdd_External_Symbol("Maths::ArcTan^1", (void*)Math_ArcTan);
  scAdd_External_Symbol("Maths::ArcTan2^2", (void*)Math_ArcTan2);
  scAdd_External_Symbol("Maths::Cos^1", (void*)Math_Cos);
  scAdd_External_Symbol("Maths::Cosh^1", (void*)Math_Cosh);
  scAdd_External_Symbol("Maths::DegreesToRadians^1", (void*)Math_DegreesToRadians);
  scAdd_External_Symbol("Maths::Exp^1", (void*)Math_Exp);
  scAdd_External_Symbol("Maths::Log^1", (void*)Math_Log);
  scAdd_External_Symbol("Maths::Log10^1", (void*)Math_Log10);
  scAdd_External_Symbol("Maths::RadiansToDegrees^1", (void*)Math_RadiansToDegrees);
  scAdd_External_Symbol("Maths::RaiseToPower^2", (void*)Math_RaiseToPower);
  scAdd_External_Symbol("Maths::Sin^1", (void*)Math_Sin);
  scAdd_External_Symbol("Maths::Sinh^1", (void*)Math_Sinh);
  scAdd_External_Symbol("Maths::Sqrt^1", (void*)Math_Sqrt);
  scAdd_External_Symbol("Maths::Tan^1", (void*)Math_Tan);
  scAdd_External_Symbol("Maths::Tanh^1", (void*)Math_Tanh);
  scAdd_External_Symbol("Maths::get_Pi", (void*)Math_GetPi);
  scAdd_External_Symbol("FloatToInt",(void *)FloatToInt);
  scAdd_External_Symbol("IntToFloat",(void *)IntToFloat);
}