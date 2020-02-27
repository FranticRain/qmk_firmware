#pragma once

#include "config_common.h"

/* USB Device Info */
#define VENDOR_ID 0x16D0
#define PRODUCT_ID 0x0A9B
#define DEVICE_VER 0x0001

#define MANUFACTURER  Vestigl
#define PRODUCT       Dopre Test
#define DESCRIPTION   Electrostatic Capacitive 100% Southpaw


/* Key Matrix Config */
#define MATRIX_ROWS 6
// #define MATRIX_COLS 1
#define MATRIX_COLS 18
#define MATRIX_ROW_PINS { }
// #define MATRIX_COL_PINS { B0, B1, B2, B3, B4, B5, B6, B7, A9, A10, A8, A15, A13, A14, B11, B12, B15, B14 }
// #define MATRIX_COL_PINS { B5 }
// #define MATRIX_COL_PINS { A2 }
#define MATRIX_COL_PINS { A2, A4, A5, A6, A7, A8, A15, A13, A14, B12, B11, B10, B9, B15, B14, B13, B8, A0 }
#define UNUSED_PINS


/* Electrostatic Capacitive-specific Config */
// #define HIGH_LATCH 58
// #define HIGH_UNLATCH 48
#define DRAIN_PIN B3
#define ADC_READ_PIN A1
#define MATRIX_MUXS 3
#define MUX_PINS { B2, B1, B0 }


/* ARM ADC Config */
#define ADC_BUFFER_DEPTH 1
#define ADC_RESOLUTION ADC_CFGR1_RES_6BIT
