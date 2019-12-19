#include "analog.h"
#include "matrix.h"

#if (MATRIX_COLS <= 8)
#    define print_matrix_header() print("\nr/c 01234567\n")
#    define print_matrix_row(row) print_bin_reverse8(matrix_get_row(row))
#    define matrix_bitpop(i) bitpop(matrix[i])
#    define ROW_SHIFTER ((uint8_t)1)
#elif (MATRIX_COLS <= 16)
#    define print_matrix_header() print("\nr/c 0123456789ABCDEF\n")
#    define print_matrix_row(row) print_bin_reverse16(matrix_get_row(row))
#    define matrix_bitpop(i) bitpop16(matrix[i])
#    define ROW_SHIFTER ((uint16_t)1)
#elif (MATRIX_COLS <= 32)
#    define print_matrix_header() print("\nr/c 0123456789ABCDEF0123456789ABCDEF\n")
#    define print_matrix_row(row) print_bin_reverse32(matrix_get_row(row))
#    define matrix_bitpop(i) bitpop32(matrix[i])
#    define ROW_SHIFTER ((uint32_t)1)
#endif


#if !defined(MUX_PINS)
#    error "You must define MUX_PINS to define the pins used by the analog multiplexer when building an electrostatic capacitive keyboard."
#endif
#if !defined(MATRIX_MUXS)
#    error "You must define MATRIX_MUXS to define the number of multiplexer pins you are using."
#endif
#if !defined(DRAIN_PIN)
#    error "You must define a DRAIN_PIN for your electrostatic capacitive keyboard."
#endif
#if !defined(ADC_READ_PIN)
#    error "You must define an ADC_READ_PIN to be the ADC pin that your electrostatic capacitive keyboard is read through."
#endif
#if !defined(HIGH_LATCH)
#    error "For electrostatic capacitive keyboards, you need to define a high latch point. Please refer to the documentation for additional details."
#endif
#if !defined(HIGH_UNLATCH)
#    error "For electrostatic capacitive keyboards, you need to define a high unlatch point. Please refer to the documentation for additional details."
#endif

#if (MATRIX_ROWS <= 2)
#    if (MATRIX_MUXS != 1)
#        error "You have defined an incorrect amount of matrix mux pins. For 2 or less rows, you need only one mux pin."
#    endif
#elif (MATRIX_ROWS <= 4)
#    if (MATRIX_MUXS != 2)
#        error "You have defined an incorrect amount of matrix mux pins. For 3 or 4 rows, you need two mux pins."
#    endif
#elif (MATRIX_ROWS <= 8)
#    if (MATRIX_MUXS != 3)
#        error "You have defined an incorrect amount of matrix mux pins. For 5, 6, 7, and 8 rows, you need three mux pins."
#    endif
#elif (MATRIX_ROWS <= 16)
#    if (MATRIX_MUXS != 4)
#        error "You have defined an incorrect amount of matrix mux pins. For 9 - 16 rows, you need four mux pins."
#    endif
#else
#    error "QMK's implementation of electrostatic capacitive keyboards doesn't support more than 16 rows at this time."
#endif


static matrix_row_t raw_matrix[MATRIX_ROWS] = {};
static const pin_t mux_pins[MATRIX_MUXS] = MUX_PINS;

#if defined(HAL_USE_ADC) // If we're using a ChibiOS compatible chip (ARM)
    static pin_and_adc mux = {};
#else // We can assume an AVR implementation
    static uint8_t mux = 0;
#endif


__attribute__ ((weak)) void matrix_init_user(void) { }
__attribute__ ((weak)) void matrix_scan_user(void) { }
__attribute__ ((weak)) void matrix_init_kb(void) { matrix_init_user(); }
__attribute__ ((weak)) void matrix_scan_kb(void) { matrix_scan_user(); }


inline uint8_t matrix_rows(void) { return MATRIX_ROWS; }
inline uint8_t matrix_cols(void) { return MATRIX_COLS; }
inline matrix_row_t matrix_get_row(uint8_t row) { return raw_matrix[row]; }


void matrix_print(void) {
    print_matrix_header();

    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        phex(row);
        print(": ");
        print_matrix_row(row);
        print("\n");
    }
}



bool analog_strobe(pin_t strobe_pin, bool was_active) {

    // Note that no delays are used to charge the RC circuit.
    // This is because the process is so slow that waiting the charge time is not required.
    // It is possible that faster uCs or faster ADCs might be present that require waiting.
    // This potential can be evaluated when the time comes.

    // Set the drain pin to floating so it doesn't interfere with our RC charging
    // Note that no pull-up/down resistor is selected. We need the high-Z state.
    setPinInput(DRAIN_PIN);

    // If you needed to disable interrupts, you would do so here.

    // Set our strobe pin high, so we can start charging the circuit
    writePinHigh(strobe_pin);

    adcsample_t value = adc_read(mux);

    // Stop charging the circuit so we can discharge it.
    writePinLow(strobe_pin);

    // If you needed to disable interrupts, you would re-enable them here.

    // Set the drain pin low to add a higher resistance load, increasing discharge rate.
    setPinOutput(DRAIN_PIN);
    writePinLow(DRAIN_PIN);

    // We use different actuation points for activation and release so that noise
    // in our ADC reading doesn't cause the switch to actuate accidentally.
    uint16_t comparison_point = was_active ? HIGH_UNLATCH : HIGH_LATCH;
    return (value > comparison_point);
}


void set_mux_state(uint8_t target_mux_io) {

    // We intentionally fall-through in this switch statement
    switch (MATRIX_MUXS) {
        case 4: writePin(mux_pins[3], target_mux_io & 0x08);
        case 3: writePin(mux_pins[2], target_mux_io & 0x04);
        case 2: writePin(mux_pins[1], target_mux_io & 0x02);
        case 1: writePin(mux_pins[0], target_mux_io & 0x01);
        default: break;
    }
}



#if DIODE_DIRECTION == COL2ROW

static const pin_t col_pins[MATRIX_COLS] = MATRIX_COL_PINS;


void matrix_init(void) {

    for (int col = 0; col < MATRIX_COLS; col++) {
        if (col != NO_PIN) {
            setPinOutput(col_pins[col]);
            writePinLow(col_pins[col]);
        }
    }
    for (int mux = 0; mux < MATRIX_MUXS; mux++) {
        if (mux != NO_PIN) {
            setPinOutput(mux_pins[mux]);
            writePinHigh(mux_pins[mux]);
        }
    }

    setPinInput(ADC_READ_PIN);
    mux = pinToMux(ADC_READ_PIN);

    // Drain pin needs to be pulled to ground to start to make sure that we are all drained before we start.
    // The scan routine will take care of floating and un-floating it from here.
    // Note that we want the high-Z case of not using a pull-up/down resistor here, otherwise it won't float.
    setPinInput(DRAIN_PIN);
    writePinLow(DRAIN_PIN);

    matrix_init_quantum();
}

bool read_cols_on_row(matrix_row_t current_matrix[], uint8_t current_row) {

    // Store last value of row prior to reading
    matrix_row_t last_row_value = current_matrix[current_row];

    // Clear data in matrix row
    current_matrix[current_row] = 0;

    // We need to alter the multiplexer for every new row that we are reading on.
    // We select the row we will be reading, and then read all of the columns on that row.
    set_mux_state(current_row);

    // For each col...
    for (uint8_t col_index = 0; col_index < MATRIX_COLS; col_index++) {

        // Find out if this switch was active previously.
        // Since the release point is different from the activation point,
        // we need to know which actuation point to look for.
        bool wasActive = (last_row_value >> col_index) & 1;

        // Strobe the target row/col combination to get a reading of depth.
        bool pin_state = analog_strobe(col_pins[col_index], wasActive);

        // Populate the matrix row with the state of the column pin
        current_matrix[current_row] |= pin_state ? (ROW_SHIFTER << col_index) : 0;
    }

    return (last_row_value != current_matrix[current_row]);
}

#endif /* DIODE_DIRECTION == COL2ROW */




#if DIODE_DIRECTION == ROW2COL

static const pin_t row_pins[MATRIX_ROWS] = MATRIX_ROW_PINS;


void matrix_init(void) {

    for (int row = 0; row < MATRIX_ROWS; row++) {
        if (row != NO_PIN) {
            setPinOutput(row_pins[row]);
            writePinLow(row_pins[row]);
        }
    }
    for (int mux = 0; mux < MATRIX_MUXS; mux++) {
        if (mux != NO_PIN) {
            setPinOutput(mux_pins[mux]);
            writePinHigh(mux_pins[mux]);
        }
    }

    setPinInput(ADC_READ_PIN);
    mux = pinToMux(ADC_READ_PIN);

    // Drain pin needs to be pulled to ground to start to make sure that we are all drained before we start.
    // The scan routine will take care of floating and un-floating it from here.
    // Note that we want the high-Z case of not using a pull-up/down resistor here, otherwise it won't float.
    setPinInput(DRAIN_PIN);
    writePinLow(DRAIN_PIN);

    matrix_init_quantum();
}

bool read_rows_on_col(matrix_row_t current_matrix[], uint8_t current_col) {

    bool matrix_changed = false;

    // For each col...
    for (uint8_t row_index = 0; row_index < MATRIX_COLS; row_index++) {

        // Store last value of row prior to reading
        matrix_row_t last_row_value = current_matrix[row_index];

        // Find out if this switch was active previously.
        // Since the release point is different from the activation point,
        // we need to know which actuation point to look for.
        bool wasActive = (last_row_value >> row_index) & 1;

        // We need to alter the multiplexer for every new row that we are reading on.
        set_mux_state(row_index);

        // Strobe the target row/col combination to get a reading of depth.
        bool pin_state = analog_strobe(row_pins[row_index], wasActive);

        if (pin_state) {
            // Pin HI, set col bit
            current_matrix[row_index] |= (ROW_SHIFTER << current_col);
        } else {
            // Pin LO, clear col bit
            current_matrix[row_index] &= ~(ROW_SHIFTER << current_col);
        }

        // Determine if the matrix changed state
        if ((last_row_value != current_matrix[row_index]) && !(matrix_changed)) {
            matrix_changed = true;
        }
    }

    return matrix_changed;
}

#endif /* DIODE_DIRECTION == ROW2COL */




uint8_t matrix_scan(void) {

    bool changed = false;

#if (DIODE_DIRECTION == COL2ROW)
    // Set row, read cols
    for (uint8_t current_row = 0; current_row < MATRIX_ROWS; current_row++) {
        changed |= read_cols_on_row(raw_matrix, current_row);
    }
#elif (DIODE_DIRECTION == ROW2COL)
    // Set col, read rows
    for (uint8_t current_col = 0; current_col < MATRIX_COLS; current_col++) {
        changed |= read_rows_on_col(raw_matrix, current_col);
    }
#endif

    // Note that if you wanted to re-add debounce to this circuitry, you would do so here.
    // Our scan is so slow that it isn't necessary.
    // This isn't to mention that we have the latch/unlatch levels.
    // These different activation/deactivation levels help fight the same symptoms of debouncing.

    matrix_scan_quantum();
    return (uint8_t)changed;
}
