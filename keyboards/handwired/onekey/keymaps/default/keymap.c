#include QMK_KEYBOARD_H

enum macroKeycodes {
    INIT_TEST = SAFE_RANGE,
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
  LAYOUT( INIT_TEST )
};

bool process_record_user(uint16_t keycode, keyrecord_t* record)
{
    switch (keycode)
    {
        case INIT_TEST:
        {
            if (record->event.pressed)
            {
                if (wasInitializationCalled)
                {
                    SEND_STRING("Yes");
                }
                else
                {
                    SEND_STRING("No");
                }
            }
        } break;
    }

    return true;
}
