#include "main.h"

void CreateWave(unsigned __int16* aCalculatedWave)
{
    for (int i = 0; i < 360; i=i+1)
        aCalculatedWave[i] = (signed short)(0x7FFF + (0x7FFF * sin(i * PI / 180)));
}

double CalcWavDspFrq(unsigned __int16 uFrqSP, SamplesPerWave  eSPW)
{
    int fdiv=1;
    //SPW360, SPW180, SPW90, SPW45, SPW24
    switch (eSPW)
    {
        case SamplesPerWave::SPW360: fdiv = 360; break;
        case SamplesPerWave::SPW180: fdiv = 180; break;
        case SamplesPerWave::SPW90: fdiv = 90; break;
        case SamplesPerWave::SPW45: fdiv = 45; break;
        case SamplesPerWave::SPW24: fdiv = 24; break;
    }
    double CalcWFrq = (double)72000000 / (2 * (double)uFrqSP * (double)fdiv);
    return CalcWFrq;
}