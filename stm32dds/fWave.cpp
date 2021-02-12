#include "main.h"

void CreateWave(unsigned __int16* aCalculatedWave)
{
    for (int i = 0; i < 360; i=i+1)
        aCalculatedWave[i] = (signed short)(0x7FFF + (0x7FFF * sin(i * PI / 180)));
}