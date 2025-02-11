#pragma once

#include "mun.h"
#include "touch_encoder.h"

// clang-format off
#define LAYOUT( \
    L00, L01, L02, L03, L04, L05, L06, R00, R01, R02, R03, R04, R05, R06, \
    L10, L11, L12, L13, L14, L15, L16, R10, R11, R12, R13, R14, R15, R16, \
    L20, L21, L22, L23, L24, L25, L26, R20, R21, R22, R23, R24, R25, R26, \
    L30, L31, L32, L33, L34, L35, L36, R30, R31, R32, R33, R34, R35, R36, \
    L40, L41, L42, L43, L44, L45, L46, R40, R41, R42, R43, R44, R45, R46  \
) \
{ \
    /* Left Half */ \
    { L00, L01, L02, L03, L04, L05, L06 }, \
    { L10, L11, L12, L13, L14, L15, L16 }, \
    { L20, L21, L22, L23, L24, L25, L26 }, \
    { L30, L31, L32, L33, L34, L35, L36 }, \
    { L40, L41, L42, L43, L44, L45, L46 }, \
    /* Right Half */ \
    { R06, R05, R04, R03, R02, R01, R00 }, \
    { R16, R15, R14, R13, R12, R11, R10 }, \
    { R26, R25, R24, R23, R22, R21, R20 }, \
    { R36, R35, R34, R33, R32, R31, R30 }, \
    { R46, R45, R44, R43, R42, R41, R40 }  \
}

#define NUMBER_OF_TOUCH_ENCODERS 2
#define TOUCH_ENCODER_OPTIONS TOUCH_SEGMENTS + 2

#define TOUCH_ENCODER_LAYOUT( \
    E10, E11, E12, E13, E14, \
    E20, E21, E22, E23, E24 \
) \
{ \
    { E10, E11, E12, E13, E14 }, \
    { E20, E21, E22, E23, E24 } \
}

#define NUMBER_OF_ENCODERS 4
#define ENCODER_OPTIONS 2

#define ENCODER_LAYOUT( \
    E10, E11, \
    E20, E21, \
    E30, E31, \
    E40, E41 \
) \
{ \
    { E10, E11 }, \
    { E20, E21 }, \
    { E30, E31 }, \
    { E40, E41 } \
}
// clang-format on
