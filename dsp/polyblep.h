/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#ifndef DSP_POLYBLEP_H
#define DSP_POLYBLEP_H

#include "types.h"

namespace dsp {

static float polyblep_values[8001];

static float polyblep_slow(float t) {
    if (t > 0.0f) {
        return t - (t*t)/2.0f - 0.5f;
    } else {
        return (t*t)/2.0f + t + 0.5f;
    }
}

static void init_polyblep() {
    for (uint i = 0; i < 8001; i++) {
        polyblep_values[i] = polyblep_slow((i - 4000) / 4000.0f);
    }
}

static float polyblep(float t) {
    return polyblep_values[int(4000 * t) + 4000];
}

static float saw_polyblep(float p, float max, float inc) {
    float mod = 0.0f;
    if (p < inc) { // start
        mod = polyblep(p / inc);
    } else if (p > (max - inc)) { // end
        mod = polyblep((p - max) / inc);
    }
    return mod;
}

static float pulse_polyblep(float p, float width, float inc) {
    float mod = 0.0f;
    if (p < width) {
        if (p < inc) { // start
            mod = polyblep(p / inc);
        } else if (p > (width - inc)) {
            mod = -polyblep( (p - width) / inc);
        }
    } else {
        if (p > (1.0f - inc)) { // end
            mod = polyblep( (p - 1.0f) / inc);
        } else if (p < (width + inc)) {
            mod = -polyblep( (p - width) / inc);
        }
    }
    return mod;
}

}

#endif