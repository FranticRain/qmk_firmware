#include "onekey.h"

void keyboard_post_init_kb(void)
{
    wasInitializationCalled = 1;
    keyboard_post_init_user();
}
