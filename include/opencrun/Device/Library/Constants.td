class OCLConstant<string name, string value> {
  string Name = name;
  string Value = value;
  string Group = "";
  bit isTarget = 0;
  list<OCLPredicate> Predicates = [];
}

class OCLTargetConstant<string name, string value>
 : OCLConstant<name, value> {
  let isTarget = 1;
}

let Group = "Memory flag constants" in {
  def : OCLConstant<"CLK_GLOBAL_MEM_FENCE", "0">;
  def : OCLConstant<"CLK_LOCAL_MEM_FENCE", "1">;
}

let Group = "Integer types constants" in {
  def : OCLConstant<"CHAR_BIT", "8">;
  def : OCLConstant<"CHAR_MAX", "SCHAR_MAX">;
  def : OCLConstant<"CHAR_MIN", "SCHAR_MIN">;
  def : OCLConstant<"INT_MAX", "2147483647">;
  def : OCLConstant<"INT_MIN", "-2147483647 - 1">;
  def : OCLConstant<"LONG_MAX", "0x7FFFFFFFFFFFFFFFL">;
  def : OCLConstant<"LONG_MIN", "-0x7FFFFFFFFFFFFFFFL - 1">;
  def : OCLConstant<"SCHAR_MAX", "127">;
  def : OCLConstant<"SCHAR_MIN", "-127 - 1">;
  def : OCLConstant<"SHRT_MAX", "32767">;
  def : OCLConstant<"SHRT_MIN", "-32767 - 1">;
  def : OCLConstant<"UCHAR_MAX", "255">;
  def : OCLConstant<"USHRT_MAX", "65535">;
  def : OCLConstant<"UINT_MAX", "0xffffffffU">;
  def : OCLConstant<"ULONG_MAX", "0xFFFFFFFFFFFFFFFFUL">;
}

let Group = "FP32 type constants" in {
  def : OCLConstant<"FLT_DIG", "6">;
  def : OCLConstant<"FLT_MANT_DIG", "24">;
  def : OCLConstant<"FLT_MAX_10_EXP", "+38">;
  def : OCLConstant<"FLT_MAX_EXP", "+128">;
  def : OCLConstant<"FLT_MIN_10_EXP", "-37">;
  def : OCLConstant<"FLT_MIN_EXP", "-125">;
  def : OCLConstant<"FLT_RADIX", "2">;
  def : OCLConstant<"FLT_MAX", "0x1.FFFFFEp127f">;
  def : OCLConstant<"FLT_MIN", "0x1.0p-126f">;
  def : OCLConstant<"FLT_EPSILON", "0x1.0p-23f">;
}

let Group = "FP32 math constants" in {
  def : OCLConstant<"M_E_F", "2.71828174591064f">;
  def : OCLConstant<"M_LOG2E_F", "1.44269502162933f">;
  def : OCLConstant<"M_LOG10E_F", "0.43429449200630f">;
  def : OCLConstant<"M_LN2_F", "0.69314718246460f">;
  def : OCLConstant<"M_LN10_F", "2.30258512496948f">;
  def : OCLConstant<"M_PI_F", "3.14159274101257f">;
  def : OCLConstant<"M_PI_2_F", "1.57079637050629f">;
  def : OCLConstant<"M_PI_4_F", "0.78539818525314f">;
  def : OCLConstant<"M_1_PI_F", "0.31830987334251f">;
  def : OCLConstant<"M_2_PI_F", "0.63661974668503f">;
  def : OCLConstant<"M_2_SQRTPI_F", "1.12837922573090f">;
  def : OCLConstant<"M_SQRT2_F", "1.41421353816986f">;
  def : OCLConstant<"M_SQRT1_2_F", "0.70710676908493f">;
}

let Group = "FP32 util constants" in {
  def : OCLConstant<"HUGE_VALF", "__builtin_huge_valf()">;
  def : OCLConstant<"INFINITY", "HUGE_VALF">;
  def : OCLConstant<"MAX_FLOAT", "FLT_MAX">;
  def : OCLConstant<"NAN", "__builtin_nanf(\"\")">;
}

let Predicates = [ocl_ext_cl_khr_fp64] in {
let Group = "FP64 type constants" in {
  def : OCLConstant<"DBL_DIG", "15">;
  def : OCLConstant<"DBL_MANT_DIG", "53">;
  def : OCLConstant<"DBL_MAX_10_EXP", "+308">;
  def : OCLConstant<"DBL_MAX_EXP", "+1024">;
  def : OCLConstant<"DBL_MIN_10_EXP", "-307">;
  def : OCLConstant<"DBL_MIN_EXP", "-1021">;
  def : OCLConstant<"DBL_RADIX", "2">;
  def : OCLConstant<"DBL_MAX", "0x1.FFFFFFFFFFFFFp1023">;
  def : OCLConstant<"DBL_MIN", "0x1.0p-1022">;
  def : OCLConstant<"DBL_EPSILON", "0x1.0p-52">;
}
let Group = "FP64 math constants" in {
  def : OCLConstant<"M_E", "2.718281828459045090796">;
  def : OCLConstant<"M_LOG2E", "1.442695040888963387005">;
  def : OCLConstant<"M_LOG10E", "0.434294481903251816668">;
  def : OCLConstant<"M_LN2", "0.693147180559945286227">;
  def : OCLConstant<"M_LN10", "2.302585092994045901094">;
  def : OCLConstant<"M_PI", "3.141592653589793115998">;
  def : OCLConstant<"M_PI_2", "1.570796326794896557999">;
  def : OCLConstant<"M_PI_4", "0.785398163397448278999">;
  def : OCLConstant<"M_1_PI", "0.318309886183790691216">;
  def : OCLConstant<"M_2_PI", "0.636619772367581382433">;
  def : OCLConstant<"M_2_SQRTPI", "1.128379167095512558561">;
  def : OCLConstant<"M_SQRT2", "1.414213562373095145475">;
  def : OCLConstant<"M_SQRT1_2", "0.707106781186547572737">;
}
let Group = "FP64 util constants" in {
  def : OCLConstant<"HUGE_VAL", "__builtin_huge_val()">;
}
}
