#include "onekey.h"

uint8_t wasInitializationCalled = 0;

void keyboard_post_init_kb(void)
{
    wasInitializationCalled = 1;
    keyboard_post_init_user();
}
