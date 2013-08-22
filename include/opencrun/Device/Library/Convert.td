//===----------------------------------------------------------------------===//
// Convert builtin classes
//===----------------------------------------------------------------------===//

let Group = "CONVERT" in {

class ConvertBuiltin_ii_ir_rtz : OCLConvertBuiltin<
  ocl_rm_rtz, 0, [
  OCLBuiltinVariant<
    [ocl_gentype_integer, ocl_gentype_integer],
    [isSameDimAs<Id<0>, Id<1>>], "ii">,
  OCLBuiltinVariant<
    [ocl_gentype_integer, ocl_gentype_real],
    [isSameDimAs<Id<0>, Id<1>>], "ir">
 ]>;

class ConvertBuiltin_ir_rte : OCLConvertBuiltinSimple<
  ocl_rm_rte, 0,
  [ocl_gentype_integer, ocl_gentype_real],
  [isSameDimAs<Id<0>, Id<1>>]>;

class ConvertBuiltin_ir_rtp : OCLConvertBuiltinSimple<
  ocl_rm_rtp, 0,
  [ocl_gentype_integer, ocl_gentype_real],
  [isSameDimAs<Id<0>, Id<1>>]>;

class ConvertBuiltin_ir_rtn : OCLConvertBuiltinSimple<
  ocl_rm_rtn, 0,
  [ocl_gentype_integer, ocl_gentype_real],
  [isSameDimAs<Id<0>, Id<1>>]>;

class ConvertBuiltin_ir_rtz : OCLConvertBuiltinSimple<
  ocl_rm_rtz, 0,
  [ocl_gentype_integer, ocl_gentype_real],
  [isSameDimAs<Id<0>, Id<1>>]>;

class ConvertBuiltin_fSi_fUi_fd_dSi_dUi_df_rr_rte : OCLConvertBuiltin<
  ocl_rm_rte, 0, [
    OCLBuiltinVariant<
      [ocl_gentype_float, ocl_gentype_integer],
      [isSameDimAs<Id<0>, Id<1>>], "fSi">,
    OCLBuiltinVariant<
      [ocl_gentype_float, ocl_gentype_integer],
      [isSameDimAs<Id<0>, Id<1>>], "fUi">,
    OCLBuiltinVariant<
      [ocl_gentype_float, ocl_gentype_double],
      [isSameDimAs<Id<0>, Id<1>>], "fd">,
    OCLBuiltinVariant<
      [ocl_gentype_double, ocl_gentype_integer],
      [isSameDimAs<Id<0>, Id<1>>], "dSi">,
    OCLBuiltinVariant<
      [ocl_gentype_double, ocl_gentype_float],
      [isSameDimAs<Id<0>, Id<1>>], "df">,
    OCLBuiltinVariant<
      [ocl_gentype_real, ocl_gentype_real],
      [isSameAs<Id<0>, Id<1>>], "rr">
  ]>;

class ConvertBuiltin_fSi_fUi_fd_dSi_dUi_df_rr_rtz : OCLConvertBuiltin<
  ocl_rm_rtz, 0, [
    OCLBuiltinVariant<
      [ocl_gentype_float, ocl_gentype_integer],
      [isSameDimAs<Id<0>, Id<1>>], "fSi">,
    OCLBuiltinVariant<
      [ocl_gentype_float, ocl_gentype_integer],
      [isSameDimAs<Id<0>, Id<1>>], "fUi">,
    OCLBuiltinVariant<
      [ocl_gentype_float, ocl_gentype_double],
      [isSameDimAs<Id<0>, Id<1>>], "fd">,
    OCLBuiltinVariant<
      [ocl_gentype_double, ocl_gentype_integer],
      [isSameDimAs<Id<0>, Id<1>>], "dSi">,
    OCLBuiltinVariant<
      [ocl_gentype_double, ocl_gentype_integer],
      [isSameDimAs<Id<0>, Id<1>>], "dUi">,
    OCLBuiltinVariant<
      [ocl_gentype_double, ocl_gentype_float],
      [isSameDimAs<Id<0>, Id<1>>], "df">,
    OCLBuiltinVariant<
      [ocl_gentype_real, ocl_gentype_real],
      [isSameAs<Id<0>, Id<1>>], "rr">
  ]>;

class ConvertBuiltin_fSi_fUi_fd_dSi_dUi_df_rr_rtp : OCLConvertBuiltin<
  ocl_rm_rtp, 0, [
    OCLBuiltinVariant<
      [ocl_gentype_float, ocl_gentype_integer],
      [isSameDimAs<Id<0>, Id<1>>], "fSi">,
    OCLBuiltinVariant<
      [ocl_gentype_float, ocl_gentype_integer],
      [isSameDimAs<Id<0>, Id<1>>], "fUi">,
    OCLBuiltinVariant<
      [ocl_gentype_float, ocl_gentype_double],
      [isSameDimAs<Id<0>, Id<1>>], "fd">,
    OCLBuiltinVariant<
      [ocl_gentype_double, ocl_gentype_integer],
      [isSameDimAs<Id<0>, Id<1>>], "dSi">,
    OCLBuiltinVariant<
      [ocl_gentype_double, ocl_gentype_integer],
      [isSameDimAs<Id<0>, Id<1>>], "dUi">,
    OCLBuiltinVariant<
      [ocl_gentype_double, ocl_gentype_float],
      [isSameDimAs<Id<0>, Id<1>>], "df">,
    OCLBuiltinVariant<
      [ocl_gentype_real, ocl_gentype_real],
      [isSameAs<Id<0>, Id<1>>], "rr">
  ]>;

class ConvertBuiltin_fSi_fUi_fd_dSi_dUi_df_rr_rtn : OCLConvertBuiltin<
  ocl_rm_rtn, 0, [
    OCLBuiltinVariant<
      [ocl_gentype_float, ocl_gentype_integer],
      [isSameDimAs<Id<0>, Id<1>>], "fSi">,
    OCLBuiltinVariant<
      [ocl_gentype_float, ocl_gentype_integer],
      [isSameDimAs<Id<0>, Id<1>>], "fUi">,
    OCLBuiltinVariant<
      [ocl_gentype_float, ocl_gentype_double],
      [isSameDimAs<Id<0>, Id<1>>], "fd">,
    OCLBuiltinVariant<
      [ocl_gentype_double, ocl_gentype_integer],
      [isSameDimAs<Id<0>, Id<1>>], "dSi">,
    OCLBuiltinVariant<
      [ocl_gentype_double, ocl_gentype_integer],
      [isSameDimAs<Id<0>, Id<1>>], "dUi">,
    OCLBuiltinVariant<
      [ocl_gentype_double, ocl_gentype_float],
      [isSameDimAs<Id<0>, Id<1>>], "df">,
    OCLBuiltinVariant<
      [ocl_gentype_real, ocl_gentype_real],
      [isSameAs<Id<0>, Id<1>>], "rr">
  ]>;


class ConvertBuiltin_SiSi_UiUi_UiSi_SiUi_sat_rtz
 : OCLConvertBuiltin<ocl_rm_rtz, 1, [
  OCLBuiltinVariant<
    [ocl_gentype_s_integer, ocl_gentype_s_integer],
    [isSameDimAs<Id<0>, Id<1>>],
    "SiSi">,
  OCLBuiltinVariant<
    [ocl_gentype_u_integer, ocl_gentype_u_integer],
    [isSameDimAs<Id<0>, Id<1>>],
    "UiUi">,
  OCLBuiltinVariant<
    [ocl_gentype_u_integer, ocl_gentype_s_integer],
    [isSameDimAs<Id<0>, Id<1>>],
    "UiSi">,
  OCLBuiltinVariant<
    [ocl_gentype_s_integer, ocl_gentype_u_integer],
    [isSameDimAs<Id<0>, Id<1>>],
    "SiUi">
  ]>;

class ConvertBuiltin_Sir_Uir_sat_rtz
 : OCLConvertBuiltin<ocl_rm_rtz, 1, [
  OCLBuiltinVariant<
    [ocl_gentype_s_integer, ocl_gentype_real],
    [isSameDimAs<Id<0>, Id<1>>],
    "Sir">,
  OCLBuiltinVariant<
    [ocl_gentype_u_integer, ocl_gentype_real],
    [isSameDimAs<Id<0>, Id<1>>],
    "Uir">
  ]>;

class ConvertBuiltin_Sir_Uir_sat_rte
 : OCLConvertBuiltin<ocl_rm_rte, 1, [
  OCLBuiltinVariant<
    [ocl_gentype_s_integer, ocl_gentype_real],
    [isSameDimAs<Id<0>, Id<1>>],
    "Sir">,
  OCLBuiltinVariant<
    [ocl_gentype_u_integer, ocl_gentype_real],
    [isSameDimAs<Id<0>, Id<1>>],
    "Uir">
  ]>;

class ConvertBuiltin_Sir_Uir_sat_rtp
 : OCLConvertBuiltin<ocl_rm_rtp, 1, [
  OCLBuiltinVariant<
    [ocl_gentype_s_integer, ocl_gentype_real],
    [isSameDimAs<Id<0>, Id<1>>],
    "Sir">,
  OCLBuiltinVariant<
    [ocl_gentype_u_integer, ocl_gentype_real],
    [isSameDimAs<Id<0>, Id<1>>],
    "Uir">
  ]>;

class ConvertBuiltin_Sir_Uir_sat_rtn
 : OCLConvertBuiltin<ocl_rm_rtn, 1, [
  OCLBuiltinVariant<
    [ocl_gentype_s_integer, ocl_gentype_real],
    [isSameDimAs<Id<0>, Id<1>>],
    "Sir">,
  OCLBuiltinVariant<
    [ocl_gentype_u_integer, ocl_gentype_real],
    [isSameDimAs<Id<0>, Id<1>>],
    "Uir">
  ]>;
}

//===----------------------------------------------------------------------===//
// Convert builtin declarations
//===----------------------------------------------------------------------===//

def blt_convert_all_to_integer_rtz : ConvertBuiltin_ii_ir_rtz;

def blt_convert_real_to_integer_rte : ConvertBuiltin_ir_rte;
def blt_convert_real_to_integer_rtp : ConvertBuiltin_ir_rtp;
def blt_convert_real_to_integer_rtn : ConvertBuiltin_ir_rtn;

def blt_convert_all_to_real_rte : ConvertBuiltin_fSi_fUi_fd_dSi_dUi_df_rr_rte;
def blt_convert_all_to_real_rtz : ConvertBuiltin_fSi_fUi_fd_dSi_dUi_df_rr_rtz;
def blt_convert_all_to_real_rtp : ConvertBuiltin_fSi_fUi_fd_dSi_dUi_df_rr_rtp;
def blt_convert_all_to_real_rtn : ConvertBuiltin_fSi_fUi_fd_dSi_dUi_df_rr_rtn;

def blt_convert_integer_to_integer_sat_rtz : ConvertBuiltin_SiSi_UiUi_UiSi_SiUi_sat_rtz;
def blt_convert_real_to_integer_sat_rtz : ConvertBuiltin_Sir_Uir_sat_rtz;
def blt_convert_real_to_integer_sat_rte : ConvertBuiltin_Sir_Uir_sat_rte;
def blt_convert_real_to_integer_sat_rtp : ConvertBuiltin_Sir_Uir_sat_rtp;
def blt_convert_real_to_integer_sat_rtn : ConvertBuiltin_Sir_Uir_sat_rtn;

//===----------------------------------------------------------------------===//
// Convert builtin implementation utils
//===----------------------------------------------------------------------===//

def internal_floating_point_cvt : OCLInclude<"opencrun/Device/Library/InternalFloatingPointCVT.inc">;

//===----------------------------------------------------------------------===//
// Convert builtin implementations
//===----------------------------------------------------------------------===//

def : OCLBuiltinImpl<blt_convert_all_to_integer_rtz, RecursiveSplit<[{
  return (RETTYPE)param1;
}], [TypedefId<"RETTYPE", Id<0>>]>, "ii">;

def : OCLBuiltinImpl<blt_convert_all_to_integer_rtz, RecursiveSplit<[{
  return (RETTYPE)__builtin_ocl_trunc(param1);
}], [TypedefId<"RETTYPE", Id<0>>]>, "ir">;

def : OCLBuiltinImpl<blt_convert_real_to_integer_rte, RecursiveSplit<[{
  return (RETTYPE)param1;
}], [TypedefId<"RETTYPE", Id<0>>]>>;

def : OCLBuiltinImpl<blt_convert_real_to_integer_rtp, RecursiveSplit<[{
  return (RETTYPE)__builtin_ocl_ceil(param1);
}], [TypedefId<"RETTYPE", Id<0>>]>>;

def : OCLBuiltinImpl<blt_convert_real_to_integer_rtn, RecursiveSplit<[{
  return (RETTYPE)__builtin_ocl_floor(param1);
}], [TypedefId<"RETTYPE", Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_convert_all_to_real_rte, RecursiveSplit<[{
  // C99 default rounding mode is nearest even, so a simple cast is enough.
  return (RETTYPE)param1;
}], [TypedefId<"RETTYPE", Id<0>>]>>;

//----------------------------------------------------------------------------//

let Requirements = [internal_floating_point_cvt] in {
def : OCLBuiltinImpl<blt_convert_all_to_real_rtz, RecursiveSplit<[{
  return __internal_ocl_Si2f_rtz(param1);
}]>, "fSi">;

def : OCLBuiltinImpl<blt_convert_all_to_real_rtz, RecursiveSplit<[{
  return __internal_ocl_Si2f_rtz(param1);
}]>, "fUi">;

def : OCLBuiltinImpl<blt_convert_all_to_real_rtz, RecursiveSplit<[{
  return __internal_ocl_d2f_rtz(param1);
}]>, "fd">;

def : OCLBuiltinImpl<blt_convert_all_to_real_rtz, RecursiveSplit<[{
  return __internal_ocl_Si2d_rtz(param1);
}]>, "dSi">;

def : OCLBuiltinImpl<blt_convert_all_to_real_rtz, RecursiveSplit<[{
  return __internal_ocl_Si2d_rtz(param1);
}]>, "dUi">;
}

def : OCLBuiltinImpl<blt_convert_all_to_real_rtz, RecursiveSplit<[{
  // float to double conversion is always exact!
  return (RETTYPE)param1;
}], [TypedefId<"RETTYPE", Id<0>>]>, "df">;

def : OCLBuiltinImpl<blt_convert_all_to_real_rtz, TemplateStrategy<[{
  return param1;
}]>, "rr">;

//----------------------------------------------------------------------------//

let Requirements = [internal_floating_point_cvt] in {
def : OCLBuiltinImpl<blt_convert_all_to_real_rtp, RecursiveSplit<[{
  return __internal_ocl_Si2f_rtp(param1);
}]>, "fSi">;

def : OCLBuiltinImpl<blt_convert_all_to_real_rtp, RecursiveSplit<[{
  return __internal_ocl_Si2f_rtp(param1);
}]>, "fUi">;

def : OCLBuiltinImpl<blt_convert_all_to_real_rtp, RecursiveSplit<[{
  return __internal_ocl_d2f_rtp(param1);
}]>, "fd">;

def : OCLBuiltinImpl<blt_convert_all_to_real_rtp, RecursiveSplit<[{
  return __internal_ocl_Si2d_rtp(param1);
}]>, "dSi">;

def : OCLBuiltinImpl<blt_convert_all_to_real_rtp, RecursiveSplit<[{
  return __internal_ocl_Si2d_rtp(param1);
}]>, "dUi">;
}

def : OCLBuiltinImpl<blt_convert_all_to_real_rtp, RecursiveSplit<[{
  // float to double conversion is always exact!
  return (RETTYPE)param1;
}], [TypedefId<"RETTYPE", Id<0>>]>, "df">;

def : OCLBuiltinImpl<blt_convert_all_to_real_rtp, TemplateStrategy<[{
  return param1;
}]>, "rr">;

//----------------------------------------------------------------------------//

let Requirements = [internal_floating_point_cvt] in {
def : OCLBuiltinImpl<blt_convert_all_to_real_rtn, RecursiveSplit<[{
  return __internal_ocl_Si2f_rtn(param1);
}]>, "fSi">;

def : OCLBuiltinImpl<blt_convert_all_to_real_rtn, RecursiveSplit<[{
  return __internal_ocl_Ui2f_rtn(param1);
}]>, "fUi">;

def : OCLBuiltinImpl<blt_convert_all_to_real_rtn, RecursiveSplit<[{
  return __internal_ocl_d2f_rtn(param1);
}]>, "fd">;

def : OCLBuiltinImpl<blt_convert_all_to_real_rtn, RecursiveSplit<[{
  return __internal_ocl_Si2d_rtn(param1);
}]>, "dSi">;

def : OCLBuiltinImpl<blt_convert_all_to_real_rtn, RecursiveSplit<[{
  return __internal_ocl_Ui2d_rtn(param1);
}]>, "dUi">;
}

def : OCLBuiltinImpl<blt_convert_all_to_real_rtn, RecursiveSplit<[{
  // float to double conversion is always exact!
  return (RETTYPE)param1;
}], [TypedefId<"RETTYPE", Id<0>>]>, "df">;

def : OCLBuiltinImpl<blt_convert_all_to_real_rtn, TemplateStrategy<[{
  return param1;
}]>, "rr">;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_convert_integer_to_integer_sat_rtz, RecursiveSplit<[{
  if (param1 < RetMin) return RetMin;
  else if (param1 > RetMax) return RetMax;
  else return (RETTYPE)param1;
}], [TypedefId<"RETTYPE", Id<0>>, MinValue<"RetMin", Id<0>>, 
     MaxValue<"RetMax", Id<0>>]>, "SiSi">;

def : OCLBuiltinImpl<blt_convert_integer_to_integer_sat_rtz, RecursiveSplit<[{
  if (param1 > RetMax) return RetMax;
  else return (RETTYPE)param1;
}], [TypedefId<"RETTYPE", Id<0>>, MaxValue<"RetMax", Id<0>>]>, "UiUi">;

def : OCLBuiltinImpl<blt_convert_integer_to_integer_sat_rtz, RecursiveSplit<[{
  if (param1 < 0) return 0;
  else if ((UTYPE)param1 > RetMax) return RetMax;
  else return (RETTYPE)param1;
}], [TypedefId<"RETTYPE", Id<0>>, TypedefUnsigned<"UTYPE", Id<1>>,
     MaxValue<"RetMax", Id<0>>]>, "UiSi">;

def : OCLBuiltinImpl<blt_convert_integer_to_integer_sat_rtz, RecursiveSplit<[{
  if (param1 > (UTYPE)RetMax) return RetMax;
  else return (RETTYPE)param1;
}], [TypedefId<"RETTYPE", Id<0>>, TypedefUnsigned<"UTYPE", Id<0>>, 
     MaxValue<"RetMax", Id<0>>]>, "SiUi">;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_convert_real_to_integer_sat_rtz, RecursiveSplit<[{
  if (param1 < RetMin) return RetMin;
  else if (param1 > RetMax) return RetMax;
  else return BASIC_BUILTIN(param1);
}], [BuiltinName<"BASIC_BUILTIN", blt_convert_real_to_integer_rtn>,
     MinValue<"RetMin", Id<0>>, MaxValue<"RetMax", Id<0>>]>, "Sir">;

def : OCLBuiltinImpl<blt_convert_real_to_integer_sat_rtz, RecursiveSplit<[{
  if (param1 > RetMax) return RetMax;
  else if (param1 < 0) return 0;
  else return BASIC_BUILTIN(param1);
}], [BuiltinName<"BASIC_BUILTIN", blt_convert_real_to_integer_rtn>,
     MaxValue<"RetMax", Id<0>>]>, "Uir">;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_convert_real_to_integer_sat_rte, RecursiveSplit<[{
  if (param1 < RetMin) return RetMin;
  else if (param1 > RetMax) return RetMax;
  else return BASIC_BUILTIN(param1);
}], [BuiltinName<"BASIC_BUILTIN", blt_convert_real_to_integer_rtn>,
     MinValue<"RetMin", Id<0>>, MaxValue<"RetMax", Id<0>>]>, "Sir">;

def : OCLBuiltinImpl<blt_convert_real_to_integer_sat_rte, RecursiveSplit<[{
  if (param1 > RetMax) return RetMax;
  else if (param1 < 0) return 0;
  else return BASIC_BUILTIN(param1);
}], [BuiltinName<"BASIC_BUILTIN", blt_convert_real_to_integer_rtn>,
     MaxValue<"RetMax", Id<0>>]>, "Uir">;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_convert_real_to_integer_sat_rtp, RecursiveSplit<[{
  if (param1 < RetMin) return RetMin;
  else if (param1 > RetMax) return RetMax;
  else return BASIC_BUILTIN(param1);
}], [BuiltinName<"BASIC_BUILTIN", blt_convert_real_to_integer_rtn>,
     MinValue<"RetMin", Id<0>>, MaxValue<"RetMax", Id<0>>]>, "Sir">;

def : OCLBuiltinImpl<blt_convert_real_to_integer_sat_rtp, RecursiveSplit<[{
  if (param1 > RetMax) return RetMax;
  else if (param1 < 0) return 0;
  else return BASIC_BUILTIN(param1);
}], [BuiltinName<"BASIC_BUILTIN", blt_convert_real_to_integer_rtn>,
     MaxValue<"RetMax", Id<0>>]>, "Uir">;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_convert_real_to_integer_sat_rtn, RecursiveSplit<[{
  if (param1 < RetMin) return RetMin;
  else if (param1 > RetMax) return RetMax;
  else return BASIC_BUILTIN(param1);
}], [BuiltinName<"BASIC_BUILTIN", blt_convert_real_to_integer_rtn>,
     MinValue<"RetMin", Id<0>>, MaxValue<"RetMax", Id<0>>]>, "Sir">;

def : OCLBuiltinImpl<blt_convert_real_to_integer_sat_rtn, RecursiveSplit<[{
  if (param1 > RetMax) return RetMax;
  else if (param1 < 0) return 0;
  else return BASIC_BUILTIN(param1);
}], [BuiltinName<"BASIC_BUILTIN", blt_convert_real_to_integer_rtn>,
     MaxValue<"RetMax", Id<0>>]>, "Uir">;
