#include "Arduino.h"
#include "NESpad.h"

NESpad nintendo = NESpad(A3, A2, A1);

int check_button()
{
    byte state = nintendo.buttons();
    if (state & NES_A) return 4;
    else if (state & NES_RIGHT) return 0;
    else if (state & NES_UP) return 1;
    else if (state & NES_DOWN) return 2;
    else if (state & NES_LEFT) return 3;
    else return -1;
}

