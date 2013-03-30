/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#ifndef DSP_PHASE_H
#define DSP_PHASE_H

#include <stdlib.h>
#include <math.h>

namespace dsp {

float gb(float x) {
    return 2.0 * x - 1.0;
}

float gu(float x) {
    return 0.5 * x + 0.5;
}

float glin(float x, float a1) {
    return a1 * x;
}

float glin(float x, float a1, float a0) {
    return a1 * x + a0;
}

float mod1(float x) {
    return fmod(x, 1.0);
}

float modm(float x, float m) {
    return fmod(x, m);
}

float gramp(float x, float a1, float a0) {
    return mod1(glin(x, a1, a0));
}

float gtri(float x) {
    return fabs(gb(x));
}

float gtri(float x, float a1, float a0) {
    return mod1(glin(fabs(gb(x)),a1,a0));
}

float stri(float x) {
    if (x < 0.5) {
        return 2.0 * x;
    } else {
        return 2.0 - 2.0 * x;
    }
}

float gpulse(float x, float w) {
    return x < w ? 0.0 : 1.0;
}

float gvslope(float x, float w) {
    if (x < w) {
        return x;
    } else {
        return 2.0 * x - 1.0;
    }
}

float svtri(float x, float w) {
    return gb(x) - gb(fabs(x - w));
}

float gvtri(float x, float w, float a1, float a0) {
    return mod1(glin(svtri(x, w), a1, a0));
}

float gripple(float x, float m) {
    return x + fmod(x, m);
}

float gripple2(float x, float m1, float m2) {
    return fmod(x, m1) + fmod(x, m2);
}

}

#endif
