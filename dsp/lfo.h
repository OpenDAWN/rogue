/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#ifndef DSP_LFO_H
#define DSP_LFO_H

namespace dsp {

class LFO {

    enum {SIN, TRI, SAW, PULSE, SH, NOISE};

  public:
    void clear ();
    void reset ();
    void setType(int t) { type = t; }
    void setStart(float s) { start = s; }
    void setFreq(float f) { freq = f; }
    void setSamplerate(float r) { sample_rate = r; }
    void setWidth(float w) { width = w; }
    float getValue(float p);
    float tick(int samples);
    float tick();

  private:
    int type = 0;
    float phase = 0.0, start = 0.0, freq, width = 0.5;
    float prev_phase;
    float sample_rate;
};

}

#endif
