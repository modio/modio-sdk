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

#if defined(_MSC_VER)
    #define DISABLE_WARNING_PUSH           __pragma(warning( push ))
    #define DISABLE_WARNING_POP            __pragma(warning( pop ))
    #define DISABLE_WARNING(warningNumber) __pragma(warning( disable : warningNumber ))

    // https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warnings-c4400-through-c4599?view=msvc-170
    // Compiler warning (level 4) C4100	'identifier': unreferenced formal parameter
    #define DISABLE_WARNING_UNREFERENCED_FORMAL_PARAMETER       DISABLE_WARNING(4100)

    // Compiler warning (level 3) C4191	'operator/operation': unsafe conversion from 'type_of_expression'
    // to 'type_required'\nCalling this function through the result pointer may cause your program to fail
    #define DISABLE_WARNING_OPERATOR_OPERATION                  DISABLE_WARNING(4191)

    // Compiler warning (level 4) C4505	'function': unreferenced local function has been removed
    #define DISABLE_WARNING_UNREFERENCED_FUNCTION               DISABLE_WARNING(4505)

    // Compiler warning (level 4) C4582	'type': constructor is not implicitly called
    #define DISABLE_WARNING_NOT_IMPLICIT_CONSTRUCTOR            DISABLE_WARNING(4582)

    // Compiler warning (level 4) C4583	'type': destructor is not implicitly called
    #define DISABLE_WARNING_NOT_IMPLICIT_DESCTRUCTOR            DISABLE_WARNING(4583)

#elif defined(__GNUC__) || defined(__clang__)
    #define DO_PRAGMA(X) _Pragma(#X)
    #define DISABLE_WARNING_PUSH           DO_PRAGMA(GCC diagnostic push)
    #define DISABLE_WARNING_POP            DO_PRAGMA(GCC diagnostic pop)
    #define DISABLE_WARNING(warningName)   DO_PRAGMA(GCC diagnostic ignored #warningName)
    // https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html
    #define DISABLE_WARNING_UNREFERENCED_FORMAL_PARAMETER       DISABLE_WARNING(-Wunused-parameter)
    #define DISABLE_WARNING_OPERATOR_OPERATION
    #define DISABLE_WARNING_UNREFERENCED_FUNCTION               DISABLE_WARNING(-Wunused-function)
    #define DISABLE_WARNING_NOT_IMPLICIT_CONSTRUCTOR
    #define DISABLE_WARNING_NOT_IMPLICIT_DESCTRUCTOR            

#else
    #define DISABLE_WARNING_PUSH
    #define DISABLE_WARNING_POP
    #define DISABLE_WARNING_UNREFERENCED_FORMAL_PARAMETER
    #define DISABLE_WARNING_UNREFERENCED_FUNCTION

#endif
