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

let Group = "Sampler flag constants" in {
  def : OCLConstant<"CLK_NORMALIZED_COORDS_FALSE", "0x00">;
  def : OCLConstant<"CLK_NORMALIZED_COORDS_TRUE", "0x10">;

  def : OCLConstant<"CLK_ADDRESS_NONE", "0x00">;
  def : OCLConstant<"CLK_ADDRESS_MIRRORED_REPEAT", "0x01">;
  def : OCLConstant<"CLK_ADDRESS_REPEAT", "0x02">;
  def : OCLConstant<"CLK_ADDRESS_CLAMP_TO_EDGE", "0x03">;
  def : OCLConstant<"CLK_ADDRESS_CLAMP", "0x04">;

  def : OCLConstant<"CLK_FILTER_NEAREST", "0x000">;
  def : OCLConstant<"CLK_FILTER_LINEAR", "0x100">;
}

let Group = "Image format constants" in {
  def : OCLConstant<"CLK_R", "0x10B0">;
  def : OCLConstant<"CLK_A", "0x10B1">;
  def : OCLConstant<"CLK_RG", "0x10B2">;
  def : OCLConstant<"CLK_RA", "0x10B3">;
  def : OCLConstant<"CLK_RGB", "0x10B4">;
  def : OCLConstant<"CLK_RGBA", "0x10B5">;
  def : OCLConstant<"CLK_BGRA", "0x10B6">;
  def : OCLConstant<"CLK_ARGB", "0x10B7">;
  def : OCLConstant<"CLK_INTENSITY", "0x10B8">;
  def : OCLConstant<"CLK_LUMINANCE", "0x10B9">;
  def : OCLConstant<"CLK_Rx", "0x10BA">;
  def : OCLConstant<"CLK_RGx", "0x10BB">;
  def : OCLConstant<"CLK_RGBx", "0x10BC">;
  def : OCLConstant<"CLK_DEPTH", "0x10BD">;
  def : OCLConstant<"CLK_DEPTH_STENCIL", "0x10BE">;

  def : OCLConstant<"CLK_SNORM_INT8", "0x10D0">;
  def : OCLConstant<"CLK_SNORM_INT16", "0x10D1">;
  def : OCLConstant<"CLK_UNORM_INT8", "0x10D2">;
  def : OCLConstant<"CLK_UNORM_INT16", "0x10D3">;
  def : OCLConstant<"CLK_UNORM_SHORT_565", "0x10D4">;
  def : OCLConstant<"CLK_UNORM_SHORT_555", "0x10D5">;
  def : OCLConstant<"CLK_UNORM_INT_101010", "0x10D6">;
  def : OCLConstant<"CLK_SIGNED_INT8", "0x10D7">;
  def : OCLConstant<"CLK_SIGNED_INT16", "0x10D8">;
  def : OCLConstant<"CLK_SIGNED_INT32", "0x10D9">;
  def : OCLConstant<"CLK_UNSIGNED_INT8", "0x10DA">;
  def : OCLConstant<"CLK_UNSIGNED_INT16", "0x10DB">;
  def : OCLConstant<"CLK_UNSIGNED_INT32", "0x10DC">;
  def : OCLConstant<"CLK_HALF_FLOAT", "0x10DD">;
  def : OCLConstant<"CLK_FLOAT", "0x10DE">;
  def : OCLConstant<"CLK_UNORM_INT24", "0x10DF">;
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
