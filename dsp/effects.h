/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 *
 * based on synthv1 (http://synthv1.sourceforge.net/)
 * Copyright 2013 Rui Nuno Capela
 */

#ifndef DSP_EFFECTS_H
#define DSP_EFFECTS_H

namespace dsp {

class Effect {
  protected:
    float sample_rate;

  public:
    void setSamplerate(float r) { sample_rate = r; }
    virtual void reset() = 0;
    virtual void process(float* l, float* r, int samples) = 0;
};

// Flanger

class Flanger {
    static const int MAX_SIZE = 4096;  //= (1 << 12);
    static const int MAX_MASK = MAX_SIZE - 1;

    float buffer[MAX_SIZE];
    int frames;

  public:
    void reset();
    float output(float in, float delay, float feedb);
};

// Chorus

class Chorus : Effect {
    // conf
    float delay, feedback, rate, mod, wet;

    Flanger flanger1, flanger2;
    float lfo_phase = 0.0f;

  public:
    void configure(float delay_, float feedback_, float rate_, float mod_, float wet_) {
        delay = delay_;
        feedback = feedback_;
        rate = rate_;
        mod = mod_;
        wet = wet_;
    }

    void reset();
    void process(float* l, float* r, int samples);
};

// Phaser

class APDelay {
    float a = 0.0, m = 0.0;

  public:
    void set (float a_) { a = a_;}

    void reset() { m = 0.0f; }

    float process (float x) {
        float y = -a * x + m;
        m = a * y + x;
        return y;
    }
};

class Phaser : Effect {
    // conf
    float lfo_rate, depth, feedback, wet;

    APDelay ap_l[6], ap_r[6];
    float lfo_phase = 0.0, out_l = 0.0, out_r;

  public:
    void configure(float lfo_rate_, float depth_, float fb_, float wet_) {
        lfo_rate = lfo_rate_;
        depth = depth_;
        feedback = fb_;
        wet = wet_;
    }

    void reset();
    void process(float* l, float* r, int samples);
};


// Delay

class Delay : Effect {
    static const int MAX_SIZE = 65536; //= (1 << 16);
    static const int MAX_MASK = MAX_SIZE - 1;

    // conf
    float wet, delay, feedb, bpm = 0.0f;

    float out_l, out_r;
    float buffer_l[MAX_SIZE], buffer_r[MAX_SIZE];
    int frames;

  public:
    void configure(float wet_, float delay_, float feedb_, float bpm_) {
        wet = wet_;
        delay = delay_;
        feedb = feedb_;
        bpm = bpm_;
    }

    void reset();
    void process(float* l, float* r, int samples);
};


// Reverb

class Reverb : Effect {

  public:
    void process(float* l, float* r, int samples);
};

}

#endif
