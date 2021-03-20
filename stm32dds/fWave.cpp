#include "main.h"

void CreateWave(unsigned __int16* aCalculatedWave,
    WaveType eWaveType, unsigned __int16 VppSP, unsigned __int8 uPwmSP)
{
    if (eWaveType == WaveType::Sine) //Create sine wave
    {
        for (int i = 0; i < 360; i = i + 1)
            aCalculatedWave[i] = (signed short)(0x7FFF + (VppSP * sin(i * PI / 180)));
        //Original working prototype
     /*   for (int i = 0; i < 360; i = i + 1)
            aCalculatedWave[i] = (signed short)(0x7FFF + (0x7FFF * sin(i * PI / 180))); */
    }
    if (eWaveType == WaveType::Square) //Create square wave
    {
        for (int i = 0; i < 360; i = i + 1)
        {
            if (i < ((360 * (100-uPwmSP)) / 100))
                aCalculatedWave[i] = (0x7FFF-VppSP);
            else aCalculatedWave[i] =(0x7FFF + VppSP);
        }
    }
    if (eWaveType == WaveType::Triangle) //Create Triangle wave
    {
        for (int i = 0; i < 360; i = i + 1)
        {
            if (i < 180) aCalculatedWave[i] =
                (unsigned short)((0x7FFF - VppSP) + ((2 * (double)VppSP) / 179) * i);
            else aCalculatedWave[i] =
                (unsigned short)((0x7FFF + VppSP) - (((2 * (double)VppSP) / 179) *((double)i-180))+1);
        }
    }
    if (eWaveType == WaveType::SawTooth) //Create SawTooth wave
    {
        for (int i = 0; i < 360; i = i + 1)
            aCalculatedWave[i] =
            (unsigned short)((0x7FFF-VppSP) + ((2*(double)VppSP) / 359) * i);
    }
    if (eWaveType == WaveType::RewSawTooth) //Create Reversed SawTooth wave
    {
        for (int i = 0; i < 360; i = i + 1)
            aCalculatedWave[i] =
            (unsigned short)((0x7FFF + VppSP) - ((2 * (double)VppSP) / 359) * i);
    }
    if (eWaveType == WaveType::Random) //Create random wave
    {
        for (int i = 0; i < 360; i = i + 1)
            aCalculatedWave[i] = (unsigned short)((0x7FFF-VppSP)+(double)2*rand()*VppSP/0x7FFF);
    }
    if (eWaveType == WaveType::Zero) //Create "null"wave for calibration
        for (int i = 0; i < 360; i = i + 1)aCalculatedWave[i] = 0x7FFF; 
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

double CalcWavDspOffs(unsigned __int8 uOffsSP, AmpPower  eAmpPow)
{
    double odiv = 2;
    double CalcWOffs=0.0;
    // x2_0, x1_5, x1_0, x0_5 
    switch (eAmpPow)
    {
    case AmpPower::x2_0: odiv = 2; break;
    case AmpPower::x1_5: odiv = 1.5; break;
    case AmpPower::x1_0: odiv = 1; break;
    case AmpPower::x0_5: odiv = 0.5; break;
    }
    if (uOffsSP < 0x1F)
        CalcWOffs = -3.3 / (double)0x3F * ((double)0x1F-(double)uOffsSP) * odiv;
    else
        CalcWOffs = 3.3 / (double)0x3F * ((double)uOffsSP-(double)0x1F) * odiv; 
    return CalcWOffs;
}

double CalcWavDspVpp(unsigned __int16 VppSP, AmpPower eAmpPow)
{
    double vdiv = 2;
    double CalcWVpp = 0.0;
    // x2_0, x1_5, x1_0, x0_5 
    switch (eAmpPow)
    {
    case AmpPower::x2_0: vdiv = 2; break;
    case AmpPower::x1_5: vdiv = 1.5; break;
    case AmpPower::x1_0: vdiv = 1; break;
    case AmpPower::x0_5: vdiv = 0.5; break;
    }
    CalcWVpp = (3.3 / 0x7FFF*vdiv) * VppSP;
    return CalcWVpp;
}