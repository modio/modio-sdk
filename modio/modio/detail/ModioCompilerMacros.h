/*
 *  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

 // Reference: https://www.fluentcpp.com/2019/08/30/how-to-disable-a-warning-in-cpp/

#define MODIO_DO_PRAGMA(X) _Pragma(#X)
#include "modio/detail/HedleyWrapper.h"

#if defined(__GNUC__) || defined(__clang__)

	#if defined(__clang__)
		#define MODIO_DISABLE_WARNING_PUSH MODIO_DO_PRAGMA(clang diagnostic push)
		#define MODIO_DISABLE_WARNING_POP MODIO_DO_PRAGMA(clang diagnostic pop)
		#define MODIO_DISABLE_WARNING(warningName) MODIO_DO_PRAGMA(clang diagnostic ignored #warningName)
	#else 
		#define MODIO_DISABLE_WARNING_PUSH MODIO_DO_PRAGMA(GCC diagnostic push)
		#define MODIO_DISABLE_WARNING_POP MODIO_DO_PRAGMA(GCC diagnostic pop)
		#define MODIO_DISABLE_WARNING(warningName) MODIO_DO_PRAGMA(GCC diagnostic ignored #warningName)
	#endif

	// https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html

	#if HEDLEY_HAS_WARNING("-Wshorten-64-to-32")
		#define MODIO_DISABLE_WARNING_64BIT_TO_32BIT_CONVERSION MODIO_DISABLE_WARNING(-Wshorten-64-to-32)
	#else
		#define MODIO_DISABLE_WARNING_64BIT_TO_32BIT_CONVERSION
	#endif

	#define MODIO_DISABLE_WARNING_ASSIGNMENT_IMPLICITLY_DELETED

	#if HEDLEY_HAS_WARNING("-Wattributes")
		#define MODIO_DISABLE_WARNING_ATTRIBUTES MODIO_DISABLE_WARNING(-Wattributes)
	#else
		#define MODIO_DISABLE_WARNING_ATTRIBUTES
	#endif

	#define MODIO_DISABLE_WARNING_CONSTRUCTOR_IMPLICITLY_DELETED 
	#define MODIO_DISABLE_WARNING_CONVERSION 
	#define MODIO_DISABLE_WARNING_COPY_CONSTRUCTOR_IMPLICITLY_DELETED 
	#define MODIO_DISABLE_WARNING_ENUMERATOR_VALUES

	#if HEDLEY_HAS_WARNING("-Wcast-function-type")
		#define MODIO_DISABLE_WARNING_CAST_FUNCTION_TYPES MODIO_DISABLE_WARNING(-Wcast-function-type)
	#else
		#define MODIO_DISABLE_WARNING_CAST_FUNCTION_TYPES
	#endif

	#if HEDLEY_HAS_WARNING("-Wcast-function-type-strict")
		#define MODIO_DISABLE_WARNING_CAST_FUNCTION_TYPE_STRICT MODIO_DISABLE_WARNING(-Wcast-function-type-strict)
	#else
		#define MODIO_DISABLE_WARNING_CAST_FUNCTION_TYPE_STRICT
	#endif

	#if HEDLEY_HAS_WARNING("-Wcovered-switch-default")
		#define MODIO_DISABLE_WARNING_COVERED_SWITCH_DEFAULT MODIO_DISABLE_WARNING(-Wcovered-switch-default)
	#else
		#define MODIO_DISABLE_WARNING_COVERED_SWITCH_DEFAULT
	#endif

	#if HEDLEY_HAS_WARNING("-Wc++98-compat")
		#define MODIO_DISABLE_WARNING_CPP98_COMPAT \
			MODIO_DISABLE_WARNING(-Wc++98-compat) \
			MODIO_DISABLE_WARNING(-Wc++98-compat-pedantic) \
			MODIO_DISABLE_WARNING(-Wc++98-compat-extra-semi)
	#else
		#define MODIO_DISABLE_WARNING_CPP98_COMPAT
	#endif

	#define MODIO_DISABLE_WARNING_DEFAULT_OPERATOR_USED

	#if HEDLEY_HAS_WARNING("-Wdeprecated-declarations")
		#define MODIO_DISABLE_WARNING_DEPRECATED_DECLARATIONS MODIO_DISABLE_WARNING(-Wdeprecated-declarations)
	#else
		#define MODIO_DISABLE_WARNING_DEPRECATED_DECLARATIONS
	#endif

	#if HEDLEY_HAS_WARNING("-Wdeprecated-literal-operator")
		#define MODIO_DISABLE_WARNING_DEPRECATED_LITERAL_OPERATOR MODIO_DISABLE_WARNING(-Wdeprecated-literal-operator)
	#else
		#define MODIO_DISABLE_WARNING_DEPRECATED_LITERAL_OPERATOR 
	#endif

	#if HEDLEY_HAS_WARNING("-Wdocumentation-unknown-command")
		#define MODIO_DISABLE_WARNING_DOCUMENTATION_UNKNOWN_COMMAND MODIO_DISABLE_WARNING(-Wdocumentation-unknown-command)
	#else
		#define MODIO_DISABLE_WARNING_DOCUMENTATION_UNKNOWN_COMMAND
	#endif

	#if HEDLEY_HAS_WARNING("-Wduplicate-enum")
		#define MODIO_DISABLE_WARNING_DUPLICATE_ENUM MODIO_DISABLE_WARNING(-Wduplicate-enum)
	#else
		#define MODIO_DISABLE_WARNING_DUPLICATE_ENUM
	#endif

	#if HEDLEY_HAS_WARNING("-Wexit-time-destructors")
		#define MODIO_DISABLE_WARNING_EXIT_TIME_DESTRUCTORS MODIO_DISABLE_WARNING(-Wexit-time-destructors)
	#else
		#define MODIO_DISABLE_WARNING_EXIT_TIME_DESTRUCTORS
	#endif

	#define MODIO_DISABLE_WARNING_EXPRESSION_BEFORE_COMMA

	#if HEDLEY_HAS_WARNING("-Wfloat-equal")
		#define MODIO_DISABLE_WARNING_FLOAT_EQUAL MODIO_DISABLE_WARNING(-Wfloat-equal)
	#else
		#define MODIO_DISABLE_WARNING_FLOAT_EQUAL
	#endif

	#define MODIO_DISABLE_WARNING_FUNCTION_NOT_INLINED

	#if HEDLEY_HAS_WARNING("-Wglobal-constructors")
		#define MODIO_DISABLE_WARNING_GLOBAL_CONSTRUCTORS MODIO_DISABLE_WARNING(-Wglobal-constructors)
	#else
		#define MODIO_DISABLE_WARNING_GLOBAL_CONSTRUCTORS
	#endif

	#define MODIO_DISABLE_WARNING_L2R_EVALUATION_ORDER_INITIALIZER
	#define MODIO_DISABLE_WARNING_L2R_EVALUATION_ORDER_OPERATOR
	#define MODIO_DISABLE_WARNING_MACRO_DEFINITION

	#if HEDLEY_HAS_WARNING("-Wmissing-noreturn")
		#define MODIO_DISABLE_WARNING_MISSING_NORETURN MODIO_DISABLE_WARNING(-Wmissing-noreturn)
	#else
		#define MODIO_DISABLE_WARNING_MISSING_NORETURN
	#endif

	#define MODIO_DISABLE_WARNING_MOVE_ASSIGNMENT_IMPLICITLY_DELETED 
	#define MODIO_DISABLE_WARNING_MOVE_CONSTRUCTOR_IMPLICITLY_DELETED

	#if HEDLEY_HAS_WARNING("-Wnested-anon-types")
		#define MODIO_DISABLE_WARNING_NESTED_ANON_TYPES MODIO_DISABLE_WARNING(-Wnested-anon-types)
	#else
		#define MODIO_DISABLE_WARNING_NESTED_ANON_TYPES
	#endif

	#if HEDLEY_HAS_WARNING("-Wnewline-eof")
		#define MODIO_DISABLE_WARNING_NO_NEWLINE_AT_EOF MODIO_DISABLE_WARNING(-Wnewline-eof)
	#else
		#define MODIO_DISABLE_WARNING_NO_NEWLINE_AT_EOF
	#endif

	#if HEDLEY_HAS_WARNING("-Wnonportable-system-include-path")
		#define MODIO_DISABLE_WARNING_NONPORTABLE_INCLUDE_PATH MODIO_DISABLE_WARNING(-Wnonportable-system-include-path)
	#else
		#define MODIO_DISABLE_WARNING_NONPORTABLE_INCLUDE_PATH
	#endif

	#if HEDLEY_HAS_WARNING("-Wnontrivial-memcall")
		#define MODIO_DISABLE_WARNING_NONTRIVIAL_MEMCALL MODIO_DISABLE_WARNING(-Wnontrivial-memcall)
	#else
		#define MODIO_DISABLE_WARNING_NONTRIVIAL_MEMCALL
	#endif

	#define MODIO_DISABLE_WARNING_OBJECT_LAYOUT

	#if HEDLEY_HAS_WARNING("-Wold-style-cast")
		#define MODIO_DISABLE_WARNING_OLD_STYLE_CAST MODIO_DISABLE_WARNING(-Wold-style-cast)
	#else
		#define MODIO_DISABLE_WARNING_OLD_STYLE_CAST
	#endif

	#if HEDLEY_HAS_WARNING("-Wcomma")
		#define MODIO_DISABLE_WARNING_POSSIBLE_COMMA_MISUSE MODIO_DISABLE_WARNING(-Wcomma)
	#else
		#define MODIO_DISABLE_WARNING_POSSIBLE_COMMA_MISUSE
	#endif

	#if HEDLEY_HAS_WARNING("-Wpre-c++14-compat")
		#define MODIO_DISABLE_WARNING_PRE_CPP14_COMPAT MODIO_DISABLE_WARNING(-Wpre-c++14-compat)
	#else
		#define MODIO_DISABLE_WARNING_PRE_CPP14_COMPAT 
	#endif

	#define MODIO_DISABLE_WARNING_PRAGMA_POP_MISMATCH 

	#if HEDLEY_HAS_WARNING("-Wreserved-identifier")
		#define MODIO_DISABLE_WARNING_RESERVED_IDENTIFIER MODIO_DISABLE_WARNING(-Wreserved-identifier)
	#else
		#define MODIO_DISABLE_WARNING_RESERVED_IDENTIFIER 
	#endif

	#if HEDLEY_HAS_WARNING("-Wsign-compare")
		#define MODIO_DISABLE_WARNING_SIGNED_UNSIGNED_INTEGER_COMPARISON MODIO_DISABLE_WARNING(-Wsign-compare)
	#else
		#define MODIO_DISABLE_WARNING_SIGNED_UNSIGNED_INTEGER_COMPARISON
	#endif

	#if HEDLEY_HAS_WARNING("-Wsign-conversion")
		#define MODIO_DISABLE_WARNING_SIGNED_UNSIGNED_INTEGER_CONVERSION MODIO_DISABLE_WARNING(-Wsign-conversion)
	#else
		#define MODIO_DISABLE_WARNING_SIGNED_UNSIGNED_INTEGER_CONVERSION
	#endif

	#define MODIO_DISABLE_WARNING_SPECTRE_MITIGATION
	#define MODIO_DISABLE_WARNING_STRUCTURE_PADDING

	#if HEDLEY_HAS_WARNING("-Wstring-conversion")
		#define MODIO_DISABLE_WARNING_STRING_CONVERSION MODIO_DISABLE_WARNING(-Wstring-conversion)
	#else
		#define MODIO_DISABLE_WARNING_STRING_CONVERSION
	#endif

	#define MODIO_DISABLE_WARNING_SUBOBJECT_BRACES 

	#if HEDLEY_HAS_WARNING("-Wsubobject-linkage")
		#define MODIO_DISABLE_WARNING_SUBOBJECT_LINKAGE MODIO_DISABLE_WARNING(-Wsubobject-linkage)
	#else
		#define MODIO_DISABLE_WARNING_SUBOBJECT_LINKAGE
	#endif

	#if HEDLEY_HAS_WARNING("-Wsuggest-destructor-override")
		#define MODIO_DISABLE_WARNING_SUGGEST_DESTRUCTOR_OVERRIDE MODIO_DISABLE_WARNING(-Wsuggest-destructor-override)
	#else
		#define MODIO_DISABLE_WARNING_SUGGEST_DESTRUCTOR_OVERRIDE
	#endif

	#if HEDLEY_HAS_WARNING("-Wsuggest-override")
		#define MODIO_DISABLE_WARNING_SUGGEST_OVERRIDE MODIO_DISABLE_WARNING(-Wsuggest-override)
	#else
		#define MODIO_DISABLE_WARNING_SUGGEST_OVERRIDE
	#endif

	#if HEDLEY_HAS_WARNING("-Wswitch-enum")
		#define MODIO_DISABLE_WARNING_SWITCH_ENUM MODIO_DISABLE_WARNING(-Wswitch-enum)
	#else
		#define MODIO_DISABLE_WARNING_SWITCH_ENUM
	#endif

	#if HEDLEY_HAS_WARNING("-Wunused-parameter")
		#define MODIO_DISABLE_WARNING_UNREFERENCED_FORMAL_PARAMETER MODIO_DISABLE_WARNING(-Wunused-parameter)
	#else
		#define MODIO_DISABLE_WARNING_UNREFERENCED_FORMAL_PARAMETER
	#endif

	#if HEDLEY_HAS_WARNING("-Wunused-function")
		#define MODIO_DISABLE_WARNING_UNREFERENCED_FUNCTION MODIO_DISABLE_WARNING(-Wunused-function)
	#else
		#define MODIO_DISABLE_WARNING_UNREFERENCED_FUNCTION
	#endif

	#define MODIO_DISABLE_WARNING_UNREFERENCED_INTERNAL_FUNCTION

	#if HEDLEY_HAS_WARNING("-Wunused-private-field")
		#define MODIO_DISABLE_WARNING_UNUSED_PRIVATE_FIELD MODIO_DISABLE_WARNING(-Wunused-private-field)
	#else
		#define MODIO_DISABLE_WARNING_UNUSED_PRIVATE_FIELD
	#endif

	#if HEDLEY_HAS_WARNING("-Wunused-function")
		#define MODIO_DISABLE_WARNING_UNUSED_FUNCTION MODIO_DISABLE_WARNING(-Wunused-function)
	#else
		#define MODIO_DISABLE_WARNING_UNUSED_FUNCTION
	#endif

	#if HEDLEY_HAS_WARNING("-Wunused-but-set-variable")
		#define MODIO_DISABLE_WARNING_UNUSED_BUT_SET_VARIABLE MODIO_DISABLE_WARNING(-Wunused-but-set-variable)
	#else
		#define MODIO_DISABLE_WARNING_UNUSED_BUT_SET_VARIABLE
	#endif

	#define MODIO_DISABLE_WARNING_UNREACHABLE_CODE

	#if HEDLEY_HAS_WARNING("-Wunsafe-buffer-usage")
		#define MODIO_DISABLE_WARNING_UNSAFE_BUFFER_USAGE MODIO_DISABLE_WARNING(-Wunsafe-buffer-usage)
	#else
		#define MODIO_DISABLE_WARNING_UNSAFE_BUFFER_USAGE
	#endif

	#if HEDLEY_HAS_WARNING("-Wzero-as-null-pointer-constant")
		#define MODIO_DISABLE_WARNING_ZERO_AS_NULL_POINTER_CONSTANT MODIO_DISABLE_WARNING(-Wzero-as-null-pointer-constant)
	#else
		#define MODIO_DISABLE_WARNING_ZERO_AS_NULL_POINTER_CONSTANT
	#endif

#elif defined(_MSC_VER)
	#define MODIO_DISABLE_WARNING_PUSH MODIO_DO_PRAGMA(warning(push))
	#define MODIO_DISABLE_WARNING_POP MODIO_DO_PRAGMA(warning(pop))
	#define MODIO_DISABLE_WARNING(warningNumber) MODIO_DO_PRAGMA(warning(disable : warningNumber))

	// https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warnings-c4400-through-c4599?view=msvc-170
	// Compiler Warning (level 1) C4005 macro redefinition
	#define MODIO_DISABLE_WARNING_MACRO_DEFINITION MODIO_DISABLE_WARNING(4005)

	// Compiler Warning (level 4, off) C4061 enumerator 'identifier' in switch of enum 'enumeration' is not explicitly handled by a case label
	#define MODIO_DISABLE_WARNING_SWITCH_ENUM MODIO_DISABLE_WARNING(4061)

    // Compiler warning (level 4) C4100	'identifier': unreferenced formal parameter
	#define MODIO_DISABLE_WARNING_UNREFERENCED_FORMAL_PARAMETER MODIO_DISABLE_WARNING(4100)

	// Compiler warning (level 3) C4191	'operator/operation': unsafe conversion from 'type_of_expression'
	// to 'type_required'\nCalling this function through the result pointer may cause your program to fail
	#define MODIO_DISABLE_WARNING_CAST_FUNCTION_TYPES MODIO_DISABLE_WARNING(4191)

	// Compiler Warning (levels 3 and 4) C4244 : 'conversion' conversion from 'type1' to 'type2', possible loss of data
	#define MODIO_DISABLE_WARNING_CONVERSION MODIO_DISABLE_WARNING(4244)

	// Compiler Warning (level 4) C4365 : conversion from 'a' to 'b', signed/unsigned mismatch
	#define MODIO_DISABLE_WARNING_SIGNED_UNSIGNED_INTEGER_CONVERSION MODIO_DISABLE_WARNING(4365)

    // Compiler warning (level 4) C4435	'class1' : Object layout under /vd2 will change due to virtual base 'class2'
	#define MODIO_DISABLE_WARNING_OBJECT_LAYOUT MODIO_DISABLE_WARNING(4435)

    // Compiler warning (level 4) C4505	'function': unreferenced local function has been removed
	#define MODIO_DISABLE_WARNING_UNREFERENCED_FUNCTION MODIO_DISABLE_WARNING(4505)

	// Compiler Warning (level 4) C4514 : unreferenced inline function has been removed
	#define MODIO_DISABLE_WARNING_UNUSED_FUNCTION MODIO_DISABLE_WARNING(4514)

	// Compiler Warning (level 1) C4548 : expression before comma has no effect
	#define MODIO_DISABLE_WARNING_EXPRESSION_BEFORE_COMMA MODIO_DISABLE_WARNING(4548)

    // Compiler warning (level 4) C4582	'type': constructor is not implicitly called
	#define MODIO_DISABLE_WARNING_GLOBAL_CONSTRUCTORS MODIO_DISABLE_WARNING(4582)

    // Compiler warning (level 4) C4583	'type': destructor is not implicitly called
	#define MODIO_DISABLE_WARNING_EXIT_TIME_DESTRUCTORS MODIO_DISABLE_WARNING(4583)

	// Compiler Warning (level 4) C4623 'type': default constructor was implicitly defined as deleted
	#define MODIO_DISABLE_WARNING_CONSTRUCTOR_IMPLICITLY_DELETED MODIO_DISABLE_WARNING(4623)

	// Compiler Warning (level 4) C4625 'derived class' : copy constructor was implicitly defined as deleted because a base
	// class copy constructor is inaccessible or deleted
	#define MODIO_DISABLE_WARNING_COPY_CONSTRUCTOR_IMPLICITLY_DELETED MODIO_DISABLE_WARNING(4625)

	// Compiler Warning (level 4) C4626 'derived class' : copy constructor was implicitly defined as deleted because a base
	// class copy constructor is inaccessible or deleted
	#define MODIO_DISABLE_WARNING_ASSIGNMENT_IMPLICITLY_DELETED MODIO_DISABLE_WARNING(4626)

	// Compiler Warning (level 4) C4702 : unreachable code
	#define MODIO_DISABLE_WARNING_UNREACHABLE_CODE MODIO_DISABLE_WARNING(4702)

	// Compiler Warning (level 4) C4710 'function' : function not inlined
	#define MODIO_DISABLE_WARNING_FUNCTION_NOT_INLINED MODIO_DISABLE_WARNING(4710)

	// Compiler Warning (level 4) C4820 'bytes' bytes padding added after construct 'member_name'
	#define MODIO_DISABLE_WARNING_STRUCTURE_PADDING MODIO_DISABLE_WARNING(4820)

	// Compiler Warning (level 4) C4866 'file(line_number)' compiler may not enforce left-to-right evaluation order for call to operator_name
	#define MODIO_DISABLE_WARNING_L2R_EVALUATION_ORDER_OPERATOR MODIO_DISABLE_WARNING(4866)

	// Compiler Warning (level 4) C4868 'file(line_number)' compiler may not enforce left-to-right evaluation order in
	// braced initializer list
	#define MODIO_DISABLE_WARNING_L2R_EVALUATION_ORDER_INITIALIZER MODIO_DISABLE_WARNING(4868)

	// Compiler Warning (level 4) C4913: user defined binary operator ',' exists but no overload could convert all operands, default built-in binary operator ',' used
	#define MODIO_DISABLE_WARNING_DEFAULT_OPERATOR_USED MODIO_DISABLE_WARNING(4913)

	// Compiler Warning (level 3) C4996: Your code uses a function, class member, variable, or typedef that's marked deprecated 
	#define MODIO_DISABLE_WARNING_DEPRECATED_DECLARATIONS MODIO_DISABLE_WARNING(4996)

	// Compiler warning (level 1 and level 4, off) C5026 'type': move constructor was implicitly defined as deleted
	#define MODIO_DISABLE_WARNING_MOVE_CONSTRUCTOR_IMPLICITLY_DELETED MODIO_DISABLE_WARNING(5026)

	// Compiler warning (level 1 and level 4, off) C5027 'type': move assignment operator was implicitly defined as deleted
	#define MODIO_DISABLE_WARNING_MOVE_ASSIGNMENT_IMPLICITLY_DELETED MODIO_DISABLE_WARNING(5027)

	// Compiler warning (level 4, off) C5031 #pragma warning(pop): likely mismatch, popping warning state pushed in
	// different file
	#define MODIO_DISABLE_WARNING_PRAGMA_POP_MISMATCH MODIO_DISABLE_WARNING(5031)

	// Compiler warning (level 4, off) C5045 Compiler will insert Spectre mitigation for memory load if /Qspectre switch specified
	#define MODIO_DISABLE_WARNING_SPECTRE_MITIGATION MODIO_DISABLE_WARNING(5045)

	// Compiler warning (level 4, off) C5245 'function': unreferenced function with internal linkage has been removed
	#define MODIO_DISABLE_WARNING_UNREFERENCED_INTERNAL_FUNCTION MODIO_DISABLE_WARNING(5245)

	// Compiler warning (level 1, off) C5246 'member': the initialization of a subobject should be wrapped in braces
	#define MODIO_DISABLE_WARNING_SUBOBJECT_BRACES MODIO_DISABLE_WARNING(5246)

	// Compiler warning (level 1, off) C5249 type has named enumerators with values that cannot be represented in the given bit field width of 'N'
	#define MODIO_DISABLE_WARNING_ENUMERATOR_VALUES MODIO_DISABLE_WARNING(5249)
 
	#define MODIO_DISABLE_WARNING_64BIT_TO_32BIT_CONVERSION
	#define MODIO_DISABLE_WARNING_ATTRIBUTES
	#define MODIO_DISABLE_WARNING_CAST_FUNCTION_TYPE_STRICT
	#define MODIO_DISABLE_WARNING_COVERED_SWITCH_DEFAULT 
	#define MODIO_DISABLE_WARNING_CPP98_COMPAT
	#define MODIO_DISABLE_WARNING_DEPRECATED_LITERAL_OPERATOR
	#define MODIO_DISABLE_WARNING_DOCUMENTATION_UNKNOWN_COMMAND
	#define MODIO_DISABLE_WARNING_DUPLICATE_ENUM
	#define MODIO_DISABLE_WARNING_FLOAT_EQUAL
	#define MODIO_DISABLE_WARNING_MISSING_NORETURN
	#define MODIO_DISABLE_WARNING_NESTED_ANON_TYPES
	#define MODIO_DISABLE_WARNING_NO_NEWLINE_AT_EOF
	#define MODIO_DISABLE_WARNING_NONPORTABLE_INCLUDE_PATH
	#define MODIO_DISABLE_WARNING_NONTRIVIAL_MEMCALL
	#define MODIO_DISABLE_WARNING_OLD_STYLE_CAST
	#define MODIO_DISABLE_WARNING_POSSIBLE_COMMA_MISUSE 
	#define MODIO_DISABLE_WARNING_PRE_CPP14_COMPAT
	#define MODIO_DISABLE_WARNING_RESERVED_IDENTIFIER 
	#define MODIO_DISABLE_WARNING_SIGNED_UNSIGNED_INTEGER_COMPARISON
	#define MODIO_DISABLE_WARNING_STRING_CONVERSION
	#define MODIO_DISABLE_WARNING_SUBOBJECT_LINKAGE
	#define MODIO_DISABLE_WARNING_SUGGEST_DESTRUCTOR_OVERRIDE 
	#define MODIO_DISABLE_WARNING_SUGGEST_OVERRIDE
	#define MODIO_DISABLE_WARNING_UNSAFE_BUFFER_USAGE
	#define MODIO_DISABLE_WARNING_UNUSED_BUT_SET_VARIABLE
	#define MODIO_DISABLE_WARNING_UNUSED_PRIVATE_FIELD 
	#define MODIO_DISABLE_WARNING_ZERO_AS_NULL_POINTER_CONSTANT

#else

	#define MODIO_DISABLE_WARNING_PUSH 
	#define MODIO_DISABLE_WARNING_POP 
	#define MODIO_DISABLE_WARNING(warningNumber)

	#define MODIO_DISABLE_WARNING_64BIT_TO_32BIT_CONVERSION 
	#define MODIO_DISABLE_WARNING_ASSIGNMENT_IMPLICITLY_DELETED
	#define MODIO_DISABLE_WARNING_ATTRIBUTES
	#define MODIO_DISABLE_WARNING_CAST_FUNCTION_TYPES
	#define MODIO_DISABLE_WARNING_CAST_FUNCTION_TYPE_STRICT 
	#define MODIO_DISABLE_WARNING_CONSTRUCTOR_IMPLICITLY_DELETED
	#define MODIO_DISABLE_WARNING_CONVERSION 
	#define MODIO_DISABLE_WARNING_COPY_CONSTRUCTOR_IMPLICITLY_DELETED
	#define MODIO_DISABLE_WARNING_COVERED_SWITCH_DEFAULT 
	#define MODIO_DISABLE_WARNING_CPP98_COMPAT
	#define MODIO_DISABLE_WARNING_DEFAULT_OPERATOR_USED
	#define MODIO_DISABLE_WARNING_DEPRECATED_DECLARATIONS
	#define MODIO_DISABLE_WARNING_DEPRECATED_LITERAL_OPERATOR 
	#define MODIO_DISABLE_WARNING_DOCUMENTATION_UNKNOWN_COMMAND 
	#define MODIO_DISABLE_WARNING_DUPLICATE_ENUM 
	#define MODIO_DISABLE_WARNING_ENUMERATOR_VALUES
	#define MODIO_DISABLE_WARNING_EXIT_TIME_DESTRUCTORS
	#define MODIO_DISABLE_WARNING_EXPRESSION_BEFORE_COMMA
	#define MODIO_DISABLE_WARNING_FLOAT_EQUAL
	#define MODIO_DISABLE_WARNING_GLOBAL_CONSTRUCTORS 
	#define MODIO_DISABLE_WARNING_L2R_EVALUATION_ORDER_INITIALIZER
	#define MODIO_DISABLE_WARNING_L2R_EVALUATION_ORDER_OPERATOR 
	#define MODIO_DISABLE_WARNING_MACRO_DEFINITION
	#define MODIO_DISABLE_WARNING_MISSING_NORETURN
	#define MODIO_DISABLE_WARNING_MOVE_ASSIGNMENT_IMPLICITLY_DELETED
	#define MODIO_DISABLE_WARNING_MOVE_CONSTRUCTOR_IMPLICITLY_DELETED
	#define MODIO_DISABLE_WARNING_NESTED_ANON_TYPES
	#define MODIO_DISABLE_WARNING_NO_NEWLINE_AT_EOF 
	#define MODIO_DISABLE_WARNING_NONPORTABLE_INCLUDE_PATH
	#define MODIO_DISABLE_WARNING_NONTRIVIAL_MEMCALL
	#define MODIO_DISABLE_WARNING_OBJECT_LAYOUT
	#define MODIO_DISABLE_WARNING_OLD_STYLE_CAST 
	#define MODIO_DISABLE_WARNING_POSSIBLE_COMMA_MISUSE 
	#define MODIO_DISABLE_WARNING_PRE_CPP14_COMPAT 
	#define MODIO_DISABLE_WARNING_PRAGMA_POP_MISMATCH 
	#define MODIO_DISABLE_WARNING_RESERVED_IDENTIFIER
	#define MODIO_DISABLE_WARNING_SIGNED_UNSIGNED_INTEGER_COMPARISON 
	#define MODIO_DISABLE_WARNING_SIGNED_UNSIGNED_INTEGER_CONVERSION 
	#define MODIO_DISABLE_WARNING_SPECTRE_MITIGATION
	#define MODIO_DISABLE_WARNING_STRUCTURE_PADDING
	#define MODIO_DISABLE_WARNING_STRING_CONVERSION
	#define MODIO_DISABLE_WARNING_SUBOBJECT_BRACES 
	#define MODIO_DISABLE_WARNING_SUBOBJECT_LINKAGE
	#define MODIO_DISABLE_WARNING_SUGGEST_DESTRUCTOR_OVERRIDE 
	#define MODIO_DISABLE_WARNING_SUGGEST_OVERRIDE 
	#define MODIO_DISABLE_WARNING_SWITCH_ENUM 
	#define MODIO_DISABLE_WARNING_UNREFERENCED_FORMAL_PARAMETER 
	#define MODIO_DISABLE_WARNING_UNREFERENCED_FUNCTION
	#define MODIO_DISABLE_WARNING_UNREFERENCED_INTERNAL_FUNCTION
	#define MODIO_DISABLE_WARNING_UNSAFE_BUFFER_USAGE
	#define MODIO_DISABLE_WARNING_UNUSED_BUT_SET_VARIABLE 
	#define MODIO_DISABLE_WARNING_UNUSED_PRIVATE_FIELD 
	#define MODIO_DISABLE_WARNING_UNUSED_FUNCTION 
	#define MODIO_DISABLE_WARNING_UNREACHABLE_CODE
	#define MODIO_DISABLE_WARNING_ZERO_AS_NULL_POINTER_CONSTANT

#endif
