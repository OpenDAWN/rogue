/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#include "oscillator.h"
#include "phase.h"
#include "tables.h"

#define SIN(x) sin_.linear(x)
#define COS(x) cos_.linear(x)
#define CASE(a,b) case a: b(output, samples); break;

namespace dsp {

static float polyblep(float t) {
    if (t > 0.0f) {
        return t - (t*t)/2.0f - 0.5f;
    } else {
        return (t*t)/2.0f + t + 0.5f;
    }
}

static float pd(float x, float w) {
    if (x < w) {
        return 0.5f * x / w;
    } else {
        return 0.5f + 0.5f * (x - w) / (1.0f - w);
    }
}

// VA

void Virtual::va_highpass(float* output, int samples) {
    float b = 2.0f - COS((0.602f * freq) / sample_rate);
    float c2 = b - sqrt(b*b - 1.0f);

    for (int i = 0; i < samples; i++) {
        float x = output[i];
        output[i] = c2 * (prev + x);
        prev = output[i] - x;
    }
}

void Virtual::va_saw(float* output, int samples) {
    // saw
    el_saw(output, samples);
    // highpass
    va_highpass(output, samples);
}

void Virtual::va_tri_saw(float* output, int samples) {
    // tri
    el_tri(output, samples);
    // highpass
    va_highpass(output, samples);
}

void Virtual::va_pulse(float* output, int samples) {
    // pulse
    el_pulse(output, samples);
    // highpass
    va_highpass(output, samples);
}

// PD

void Virtual::pd_saw(float* output, int samples) {
    float inc = freq / sample_rate;
    float mod = 0.5f - wf * 0.5;
    float m_step = 0.5f * (wf - wt) / (float)samples;

    for (int i = 0; i < samples; i++) {
        output[i] = COS(pd(phase, mod));
        phase = fmod(phase + inc, 1.0f);
        mod += m_step;
    }
}

void Virtual::pd_square(float* output, int samples) {
    float inc = freq / sample_rate;
    float mod = 0.5f - wf * 0.5;
    float m_step = 0.5f * (wf - wt) / (float)samples;

    for (int i = 0; i < samples; i++) {
        float p2;
        if (phase < mod) {
            p2 = phase * 0.5f / mod;
        } else if (phase < 0.5f) {
            p2 = 0.5f;
        } else if (phase < 0.5f + mod) {
            p2 = (phase - 0.5f) * 0.5f / mod + 0.5f;
        } else {
            p2 = 1.0f;
        }
        output[i] = COS(p2);
        phase = fmod(phase + inc, 1.0f);
        mod += m_step;
    }
}

void Virtual::pd_pulse(float* output, int samples) {
    float inc = freq / sample_rate;
    float mod = 1.0f - wf;
    float m_step = (wf - wt) / (float)samples;

    for (int i = 0; i < samples; i++) {
        float p2 = phase < mod ? phase / mod : 1.0f;
        output[i] = COS(p2);
        phase = fmod(phase + inc, 1.0f);
        mod += m_step;
    }
}

void Virtual::pd_double_sine(float* output, int samples) {
    float inc = freq / sample_rate;
    float mod = 1.0f - wf;
    float m_step = (wf - wt) / (float)samples;

    for (int i = 0; i < samples; i++) {
        float p2 = 0;
        if (phase < 0.5f) {
            p2 = 2.0f * phase;
        } else {
            p2 = 1.0f - (phase - 0.5f) / (0.5f * mod);
            if (p2 < 0) p2 = 0;
        }
        output[i] = COS(p2);
        phase = fmod(phase + inc, 1.0f);
        mod += m_step;
    }
}

void Virtual::pd_saw_pulse(float* output, int samples) {
    float inc = freq / sample_rate;
    float mod = 1.0f - wf;
    float m_step = (wf - wt) / (float)samples;

    for (int i = 0; i < samples; i++) {
        float p2 = 0;
        if (phase < 0.5f) {
            p2 = phase;
        } else {
            p2 = 0.5f - (phase - 0.5f) / mod;
            if (p2 < 0) p2 = 0;
        }
        output[i] = COS(p2);
        phase = fmod(phase + inc, 1.0f);
        mod += m_step;
    }
}

void Virtual::pd_res1(float* output, int samples) {
    float inc = freq / sample_rate;
    float mod = expf(wf * 6.0f * (float)M_LN2);
    float modt = expf(wt * 6.0f * (float)M_LN2);
    float m_step = (modt - mod) / (float)samples;

    for (int i = 0; i < samples; i++) {
        float p2 = fmod(mod * phase, 1.0f);
        float window = 1.0f - phase;
        output[i] = 1.0f - window * (1.0 - COS(p2));
        phase = fmod(phase + inc, 1.0f);
        mod += m_step;
    }
}

void Virtual::pd_res2(float* output, int samples) {
    float inc = freq / sample_rate;
    float mod = expf(wf * 6.0f * (float)M_LN2);
    float modt = expf(wt * 6.0f * (float)M_LN2);
    float m_step = (modt - mod) / (float)samples;

    for (int i = 0; i < samples; i++) {
        float p2 = fmod(mod * phase, 1.0f);
        float window = phase < 0.5f ? 2.0f * phase : 2.0f * (1.0f - phase);
        output[i] = 1.0f - window * (1.0 - COS(p2));
        phase = fmod(phase + inc, 1.0f);
        mod += m_step;
    }
}

void Virtual::pd_res3(float* output, int samples) {
    float inc = freq / sample_rate;
    float mod = expf(wf * 6.0f * (float)M_LN2);
    float modt = expf(wt * 6.0f * (float)M_LN2);
    float m_step = (modt - mod) / (float)samples;

    for (int i = 0; i < samples; i++) {
        float p2 = fmod(mod * phase, 1.0f);
        float window = phase < 0.5f ? 1.0f : 2.0f * (1.0f - phase);
        output[i] = 1.0f - window * (1.0 - COS(p2));
        phase = fmod(phase + inc, 1.0f);
        mod += m_step;
    }
}

void Virtual::pd_half_sine(float* output, int samples) {
    float inc = freq / sample_rate;
    float mod = 0.5f + wf * 0.5;
    float m_step = 0.5f * (wt - wf) / (float)samples;

    for (int i = 0; i < samples; i++) {
        output[i] = gb(SIN(0.5f * pd(phase, mod)));
        phase = fmod(phase + inc, 1.0f);
        mod += m_step;
    }
}

// EL


#define PHASE_LOOP(calc) \
    float inc = freq / sample_rate; \
    for (int i = 0; i < samples; i++) { \
        calc \
        phase = fmod(phase + inc, 1.0f); \
    }

#define PWIDTH_LOOP(calc) \
    float inc = freq / sample_rate; \
    float width = wf; \
    float w_step = (wt - wf) / (float)samples; \
    for (int i = 0; i < samples; i++) { \
        calc \
        phase = fmod(phase + inc, 1.0f); \
        width += w_step; \
    }

// polyblep
void Virtual::el_saw(float* output, int samples) {
    PHASE_LOOP(
        float mod = 0.0f;
        if (phase < inc) { // start
            mod = polyblep(phase / inc);
        } else if (phase > (1.0f - inc)) { // end
            mod = polyblep( (phase - 1.0) / inc);
        }
        output[i] = gb(phase - mod);
    )
}

// polyblep
void Virtual::el_double_saw(float* output, int samples) {
    PWIDTH_LOOP(
        float p2;
        float mod = 0.0f;
        if (phase < width) {
            float inc2 = inc / width;
            p2 = phase / width;

            if (p2 < inc2) { // start
                mod = polyblep(p2 / inc2);
            } else if (p2 > (1.0f - inc2)) { // end
                mod = polyblep((p2 - 1.0f) / inc2);
            }
        } else {
            float inc2 = inc / (1.0f - width);
            p2 = (phase - width) / (1.0f - width);

            if (p2 < inc2) {
                mod = polyblep(p2 / inc2);
            } else if (p2 > (1.0f - inc2)) {
                mod = polyblep((p2 - 1.0f) / inc2);
            }
        }
        output[i] = gb(p2 - mod);
    )
}

void Virtual::el_tri(float* output, int samples) {
    PWIDTH_LOOP(
        output[i] = gb(gtri(phase, width));
    )
}

void Virtual::el_tri2(float* output, int samples) {
    PWIDTH_LOOP(
        float p = gtri(phase, width);
        output[i] = gb(p * sqrt(p) + p * (1.0-p));
    )
}

void Virtual::el_tri3(float* output, int samples) {
    PWIDTH_LOOP(
        float p = gtri(phase, width);
        output[i] = gb(phase < width ? sqrt(p) : p*p*p);
    )
}

// polyblep
void Virtual::el_pulse(float* output, int samples) {
    float inc2 = freq / sample_rate;
    bool bl = wf > inc2 && wf < (1.0f - inc2);
    PWIDTH_LOOP(
        float p2 = phase < width ? 0.0f : 1.0f;
        float mod = 0.0f;

        if (bl) {
            if (phase < width) {
                if (phase < inc) { // start
                    mod = polyblep(phase / inc);
                } else if (phase > (width - inc)) {
                    mod = -polyblep( (phase - width) / inc);
                }
            } else {
                if (phase > (1.0f - inc)) { // end
                    mod = polyblep( (phase - 1.0f) / inc);
                } else if (phase < (width + inc)) {
                    mod = -polyblep( (phase - width) / inc);
                }
            }
        }
        output[i] = gb(p2 - mod);
    )
}

// TODO polyblep
void Virtual::el_pulse2(float* output, int samples) {
    PWIDTH_LOOP(
        float min = 0.5f - 0.5f * width;
        float max = 0.5f + 0.5f * width;
        float p2 = (phase > min && phase < max) ? 1.0f : 0.0f;
        output[i] = gb(p2);
    )
}

// TODO polyblep
void Virtual::el_pulse_saw(float* output, int samples) {
    PWIDTH_LOOP(
        float p2 = pd(phase, width);
        if (phase < width) {
            output[i] = 2.0f * p2;
        } else {
            output[i] = -2.0f * (p2 - 0.5f);
        }
    )
}

// polyblep
// TODO optimize
void Virtual::el_slope(float* output, int samples) {
    PWIDTH_LOOP(
        float p2 = gvslope(phase, width);
        float mod = 0.0f;

        float inc2 = inc / (1.0f - width);
        if (phase < inc) {               // start
            mod = polyblep(phase / inc);
        } else if (p2 > (1.0f - inc2)) { // end
            mod = polyblep( (p2 - 1.0f) / inc2);
        } else if (phase < width && phase > (width - inc)) {
            mod = width * polyblep( (p2 - width) / inc);
        } else if (phase > width && p2 < inc2) {
            mod = width * polyblep(p2 / inc2);
        }

        output[i] = gb(p2 - mod);
    )
}

void Virtual::el_alpha1(float* output, int samples) {
    // pulse
    float f = freq;
    float p = phase;
    freq = 2.0f * freq;
    el_pulse(output, samples);

    // saw
    phase = p;
    freq = f;
    PHASE_LOOP(
        output[i] = phase * (output[i] + 1.0) - 1.0f;
    )
}

void Virtual::el_alpha2(float* output, int samples) {
    // pulse
    float f = freq;
    float p = phase;
    freq = 4.0f * freq;
    el_pulse(output, samples);

    // saw
    phase = p;
    freq = f;
    PHASE_LOOP(
        output[i] = phase * (output[i] + 1.0) - 1.0f;
    )
}

void Virtual::el_beta1(float* output, int samples) {
    // TODO
}

void Virtual::el_beta2(float* output, int samples) {
    // TODO
}

void Virtual::el_pulse_tri(float* output, int samples) {
    // pulse2
    float p = phase;
    el_pulse2(output, samples);

    // tri
    phase = p;
    PWIDTH_LOOP(
        output[i] = tone * output[i] + (1.0 - tone) * gb(gtri(phase, 0.5f));
    )
}

// copied from lmms triple oscillator
void Virtual::el_exp(float* output, int samples) {
    PHASE_LOOP(
        if (phase > 0.5f) {
            output[i] = -1.0 + 8.0 * (1.0 - phase) * (1.0 - phase);
        } else {
            output[i] -1.0 + 8.0 * phase * phase;
        }
    )
}

// FM

void Virtual::fm1(float* output, int samples) {
    PHASE_LOOP(
        output[i] = SIN(phase);
    )
}

void Virtual::fm2(float* output, int samples) {
    PHASE_LOOP(
        output[i] = SIN(0.5f * phase);
    )
}

void Virtual::fm3(float* output, int samples) {
    PHASE_LOOP(
        float y = SIN(phase);
        output[i] = (phase > 0.25 && phase < 0.75) ? -y : y;
    )
}

void Virtual::fm4(float* output, int samples) {
    PHASE_LOOP(
        output[i] = phase < 0.5f ? SIN(2.0f * phase) : 0.0f;
    )
}

void Virtual::fm5(float* output, int samples) {
    PHASE_LOOP(
        output[i] = phase < 0.5f ? SIN(phase) : 0.0f;
    )
}

void Virtual::fm6(float* output, int samples) {
    PHASE_LOOP(
        if (phase < 0.25f) {
            output[i] = SIN(2.0 * phase);
        } else if (phase > 0.5f && phase < 0.75f) {
            output[i] = SIN(2.0 * (phase - 0.25));
        } else {
            output[i] = 0.0f;
        }
    )
}

void Virtual::fm7(float* output, int samples) {
    PHASE_LOOP(
        if (phase < 0.25 || phase > 0.5 && phase < 0.75) {
            output[i] = SIN(phase);
        } else {
            output[i] = 0.0f;
        }
    )
}

void Virtual::fm8(float* output, int samples) {
    PHASE_LOOP(
        if (phase < 0.25 || phase > 0.5 && phase < 0.75) {
            output[i] = SIN(fmod(phase, 0.25f));
        } else {
            output[i] = 0.0f;
        }
    )
}


void Virtual::process(float* output, int samples) {
    switch (type) {
    // va
    CASE(VA_SAW, va_saw)
    CASE(VA_TRI_SAW, va_tri_saw)
    CASE(VA_PULSE, va_pulse)
    // pd
    CASE(PD_SAW, pd_saw)
    CASE(PD_SQUARE, pd_square)
    CASE(PD_PULSE, pd_pulse)
    CASE(PD_DOUBLE_SINE, pd_double_sine)
    CASE(PD_SAW_PULSE, pd_saw_pulse)
    CASE(PD_RES1, pd_res1)
    CASE(PD_RES2, pd_res2)
    CASE(PD_RES3, pd_res3)
    CASE(PD_HALF_SINE, pd_half_sine)
    // el
    CASE(EL_SAW, el_saw)
    CASE(EL_DOUBLE_SAW, el_double_saw)
    CASE(EL_TRI, el_tri)
    CASE(EL_TRI2, el_tri2)
    CASE(EL_TRI3, el_tri3)
    CASE(EL_PULSE, el_pulse)
    CASE(EL_PULSE_SAW, el_pulse_saw)
    CASE(EL_SLOPE, el_slope)
    CASE(EL_ALPHA1, el_alpha1)
    CASE(EL_ALPHA2, el_alpha2)
    CASE(EL_BETA1, el_beta1)
    CASE(EL_BETA2, el_beta2)
    CASE(EL_PULSE_TRI, el_pulse_tri)
    CASE(EL_EXP, el_exp)
    // fm
    CASE(FM1, fm1)
    CASE(FM2, fm2)
    CASE(FM3, fm3)
    CASE(FM4, fm4)
    CASE(FM5, fm5)
    CASE(FM6, fm6)
    CASE(FM7, fm7)
    CASE(FM8, fm8)
    }
}

// AS

void AS::saw(float* output, int samples) {
    float inc = freq / sample_rate;
    // TODO take sr into account
    float max = 20.0f * tone;

    for (int i = 0.; i < samples; i++) {
        float y = 0.0f;
        for (float j = 1; j < max; j++) {
            y += SIN(fmod(j * phase, 1.0f)) * 1.0/j;
        }
        output[i] = -2.0f/M_PI * y;
        phase = fmod(phase + inc, 1.0f);
    }
}

void AS::square(float* output, int samples) {
    float inc = freq / sample_rate;
    // TODO take sr into account
    float max = 40.0f * tone;

    for (int i = 0; i < samples; i++) {
        float y = 0.0f;
        for (float j = 1; j < max; j += 2.0f) {
            y += SIN(fmod(j * phase, 1.0f)) * 1.0/j;
        }
        output[i] = 4.0/M_PI * y;
        phase = fmod(phase + inc, 1.0f);
    }
}

void AS::impulse(float* output, int samples) {
    float inc = freq / sample_rate;
    // TODO take sr into account
    float max = 20.0f * tone;

    for (int i = 0; i < samples; i++) {
        float y = 0.0f;
        for (float j = 1; j < max; j++) {
            y += SIN(fmod(j * phase, 1.0f));
        }
        // TODO take max into account
        output[i] = 0.05 * y;
        phase = fmod(phase + inc, 1.0f);
    }
}

void AS::process(float* output, int samples) {
    switch (type) {
    CASE(SAW, saw)
    CASE(SQUARE, square)
    CASE(IMPULSE, impulse)
    }
}

// Noise

void Noise::process(float* output, int samples) {
    for (int i = 0; i < samples; i++) {
        output[i] =  (2.0f * rand() / (RAND_MAX + 1.0f) - 1.0f);
    }
}

// TODO pink noise

// TODO brown noise



}
