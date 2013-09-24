/*
 * rogue - multimode synth
 *
 * contains main class for synth
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#ifndef ROGUE_SYNTH_H
#define ROGUE_SYNTH_H

#include <samplerate.h>

#include "common.h"
#include "config.h"
#include "voice.h"
#include "rogue.gen"

#include <lvtk/synth.hpp>
#include <stdio.h>

// effects
#include "basics.h"
#include "Chorus.h"
#include "Phaser.h"
#include "Scape.h"
#include "Reverb.h"
#include "Descriptor.h"

namespace rogue {

class rogueSynth : public lvtk::Synth<rogueVoice, rogueSynth> {

    enum {POLY, MONO, LEGATO};

  public:
    rogueSynth(double);
    ~rogueSynth();

    unsigned find_free_voice(unsigned char, unsigned char);
    void handle_midi(uint, unsigned char*);
    void pre_process(uint from, uint to);
    void post_process(uint from, uint to);
    void update();

    template <typename T>
    T v(uint port) {
        float* pv = p(port);
        return (T)pv;
    }

    float v(uint port) {
        return *p(port);
    }

  private:
    float sample_rate;
    dsp::DCBlocker ldcBlocker, rdcBlocker;
    rogueVoice *voices[NVOICES];
    bool sustain;
    SynthData data;

    SRC_STATE* converter_l;
    SRC_STATE* converter_r;
    SRC_DATA converter_data;
    float left[8192];
    float right[8192];

    StereoChorusII2x2 chorus;
    float* chorus_ports[10];
    StereoPhaserII2x2 phaser;
    float* phaser_ports[8];
    Scape delay;
    float* delay_ports[9];
    Plate2x2 reverb;
    float* reverb_ports[8];
    bool effects_activated = false;
};

}

#endif
