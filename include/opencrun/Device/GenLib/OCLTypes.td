
//===----------------------------------------------------------------------===//
/// OCLType - Root of the OpenCL C type system.
//===----------------------------------------------------------------------===//
class OCLType { }

//===----------------------------------------------------------------------===//
/// OCLScalareType - An OpenCL C scalar type.
//===----------------------------------------------------------------------===//
class OCLScalarType : OCLType { }

//===----------------------------------------------------------------------===//
/// OCLVectType - An OpenCL C vector type. The vector contains width elements
///  of base_type.
//===----------------------------------------------------------------------===//
class OCLVectType <OCLScalarType base_type, int width> : OCLType {
  OCLScalarType BaseType = base_type;
  int Width = width;
}

//===----------------------------------------------------------------------===//
/// OCLGenType - A generic OpenCL C type. This type is used to mark generic
///  arguments in builtin definitions.
//===----------------------------------------------------------------------===//
class OCLGenType <int n> : OCLType {
  int N = n;
}

//===----------------------------------------------------------------------===//
/// OCLVectorizableType - Declare standard OpenCL C vectors for the given
///  base_type.
//===----------------------------------------------------------------------===//
multiclass OCLVectorizableType <OCLScalarType base_type> {
  def  _2 : OCLVectType<base_type,  2>;
  def  _3 : OCLVectType<base_type,  3>;
  def  _4 : OCLVectType<base_type,  4>;
  def  _8 : OCLVectType<base_type,  8>;
  def _16 : OCLVectType<base_type, 16>;
}

//===----------------------------------------------------------------------===//
/// OCLGenericType - Declare a bunch of OCLGenType.
//===----------------------------------------------------------------------===//
multiclass OCLGenericType {
  def _1 : OCLGenType<1>;
  def _2 : OCLGenType<2>;
  def _3 : OCLGenType<3>;
  def _4 : OCLGenType<4>;
  def _5 : OCLGenType<5>;
  def _6 : OCLGenType<6>;
  def _7 : OCLGenType<7>;
}

// Integer scalar types.
def ocl_char    : OCLScalarType;
def ocl_uchar   : OCLScalarType;
def ocl_short   : OCLScalarType;
def ocl_ushort  : OCLScalarType;
def ocl_int     : OCLScalarType;
def ocl_uint    : OCLScalarType;
def ocl_long    : OCLScalarType;
def ocl_ulong   : OCLScalarType;

// Integer vector types.
defm ocl_char_v     : OCLVectorizableType<ocl_char>;
defm ocl_uchar_v    : OCLVectorizableType<ocl_uchar>;
defm ocl_short_v    : OCLVectorizableType<ocl_short>;
defm ocl_ushort_v   : OCLVectorizableType<ocl_ushort>;
defm ocl_int_v      : OCLVectorizableType<ocl_int>;
defm ocl_uint_v     : OCLVectorizableType<ocl_uint>;
defm ocl_long_v     : OCLVectorizableType<ocl_long>;
defm ocl_ulong_v    : OCLVectorizableType<ocl_ulong>;

// Floating point scalar types.
def ocl_float   : OCLScalarType;

// Floating point vector types.
defm ocl_float_v    : OCLVectorizableType<ocl_float>;

// Generic types.
defm ocl_gentype : OCLGenericType;

//===----------------------------------------------------------------------===//
/// OCLAttribute - An attribute that can be attached to GenBuiltin. Support a
///  restricted syntax with respect to OpenCL C attributes.
//===----------------------------------------------------------------------===//
class OCLAttribute <string name> {
  string Name = name;
}

// Declare attributes.
def overloadable : OCLAttribute<"overloadable">;
def pure         : OCLAttribute<"pure">;

//===----------------------------------------------------------------------===//
/// GenBuiltin - An OpenCL C built-in that can take generic arguments and
///  that can occur with different alternatives, each one with one with
///  different paramter/return types. It is composed by:
///    
///    ret_types : a list containing the return types of the various built-in 
///                alternative forms;
///
///    param_types : a list containing a list for each of the alternative
///                  built-in forms; each one of these contains paramter types
///                  for the alternative;
///
///    base_impl : a code block containing the built-in implementation for the
///                scalar version;
///
///    gentype_subs : a list containing a list of mappings for each gentype 
///                   required by built-ins belonging to this class. 
///                   
///  If generic_type_i is used in [ret_type] or in param_types, the i-th entry 
///  of gentype_subs contains a list of types to expand to.
//===----------------------------------------------------------------------===//
class GenBuiltin <list<OCLType> ret_types = [],
                  list<list<OCLType>> param_types = [],
                  code base_impl = [{}],
                  list<list<OCLType>> gentype_subs = []> {
  list<OCLType> RetTypes = ret_types;
  list<list<OCLType>> ParamTypes = param_types;

  code BaseImpl = base_impl;

  list<list<OCLType>> GentypeSubs = gentype_subs;

  list<OCLAttribute> Attributes = [overloadable];
}
