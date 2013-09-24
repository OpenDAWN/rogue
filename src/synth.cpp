/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 *
 * uses code from https://github.com/rekado/lv2-mdaPiano
 */

#include "synth.h"

namespace rogue {

rogueSynth::rogueSynth(double rate)
  : lvtk::Synth<rogueVoice, rogueSynth>(p_n_ports, p_control) {

    sample_rate = rate;
    ldcBlocker.setSamplerate(sample_rate);
    rdcBlocker.setSamplerate(sample_rate);

    for (uint i = 0; i < NVOICES; i++) {
        voices[i] = new rogueVoice(rate, &data, left, right);
        add_voices(voices[i]);
    }

    ::Plugin* effects[] = {&chorus, &phaser, &delay, &reverb};
    for (uint i = 0; i < 4; i++) {
    	// instantiate
    	effects[i]->fs = rate;
    	effects[i]->over_fs = 1.0/rate;
    	effects[i]->normal = NOISE_FLOOR;
    }

    chorus.ports = chorus_ports;
    chorus.init();
    phaser.ports = phaser_ports;
    phaser.init();
    delay.ports = delay_ports;
    delay.init();
    reverb.ports = reverb_ports;
    reverb.init();

    add_audio_outputs(p_left, p_right);

    converter_l = src_new(SRC_SINC_MEDIUM_QUALITY, 1, 0);
    converter_r = src_new(SRC_SINC_MEDIUM_QUALITY, 1, 0);

    converter_data.src_ratio = 1.0 / float(data.oversample);
    converter_data.end_of_input = 0;
}

rogueSynth::~rogueSynth() {
    src_delete(converter_l);
    src_delete(converter_r);
}

unsigned rogueSynth::find_free_voice(unsigned char key, unsigned char velocity) {
    //take the next free voice if
    // ... notes are sustained but not this new one
    // ... notes are not sustained
    if (data.playmode == POLY) {
        for (uint i = 0; i < NVOICES; i++) {
            if (voices[i]->get_key() == lvtk::INVALID_KEY) {
                return i;
            }
        }
    }

    return 0;
}

//parameter change
void rogueSynth::update() {
    // TODO scale dB parameters
    data.bus_a_level = v(p_bus_a_level); // scale
    data.bus_a_pan   = v(p_bus_a_pan);
    data.bus_b_level = v(p_bus_b_level); // scale
    data.bus_b_pan   = v(p_bus_b_pan);
    data.volume      = v(p_volume); // scale
    data.playmode    = v(p_play_mode);
    data.glide_time  = v(p_glide_time);
    data.bend_range  = v(p_pitchbend_range);

    const float rate = sample_rate;

    // XXX skip conf copying if element is off?

    // oscs
    for (uint i = 0; i < NOSC; i++) {
        uint off = i * OSC_OFF;
        data.oscs[i].on          = v(p_osc1_on + off);
        data.oscs[i].type        = v(p_osc1_type + off);
        data.oscs[i].inv         = v(p_osc1_inv + off);
        data.oscs[i].free        = v(p_osc1_free + off);
        data.oscs[i].tracking    = v(p_osc1_tracking + off);
        data.oscs[i].ratio       = v(p_osc1_ratio + off);
        data.oscs[i].coarse      = v(p_osc1_coarse + off);
        data.oscs[i].fine        = v(p_osc1_fine + off);
        data.oscs[i].start       = v(p_osc1_start + off);
        data.oscs[i].width       = v(p_osc1_width + off);
        data.oscs[i].level_a     = v(p_osc1_level_a + off); // scale
        data.oscs[i].level_b     = v(p_osc1_level_b + off); // scale
        data.oscs[i].level       = v(p_osc1_level + off);   // scale

        data.oscs[i].input       = v(p_osc1_input + off);
        data.oscs[i].pm          = v(p_osc1_pm + off);
        data.oscs[i].sync        = v(p_osc1_sync + off);
        data.oscs[i].input2      = v(p_osc1_input2 + off);
        data.oscs[i].out_mod     = v(p_osc1_out_mod + off);
    }

    // filters
    for (uint i = 0; i < NDCF; i++) {
        uint off = i * DCF_OFF;
        data.filters[i].on       = v(p_filter1_on + off);
        data.filters[i].type     = v(p_filter1_type + off);
        data.filters[i].source   = v(p_filter1_source + off);
        data.filters[i].freq     = v(p_filter1_freq + off);
        data.filters[i].q        = v(p_filter1_q + off);
        data.filters[i].distortion = v(p_filter1_distortion + off);
        data.filters[i].level    = v(p_filter1_level + off); // scale
        data.filters[i].pan      = v(p_filter1_pan + off);

        data.filters[i].key_to_f = v(p_filter1_key_to_f + off);
        data.filters[i].vel_to_f = v(p_filter1_vel_to_f + off);
    }

    // lfos
    for (uint i = 0; i < NLFO; i++) {
        uint off = i * LFO_OFF;
        data.lfos[i].on          = v(p_lfo1_on + off);
        data.lfos[i].type        = v(p_lfo1_type + off);
        data.lfos[i].inv         = v(p_lfo1_inv + off);
        data.lfos[i].reset_type  = v(p_lfo1_reset_type + off);
        data.lfos[i].freq        = v(p_lfo1_freq + off);
        data.lfos[i].start       = v(p_lfo1_start + off);
        data.lfos[i].width       = v(p_lfo1_width + off);
        data.lfos[i].humanize    = v(p_lfo1_humanize + off);
    }

    // envs
    for (uint i = 0; i < NENV; i++) {
        uint off = i * ENV_OFF;
        data.envs[i].on          = v(p_env1_on + off);
        data.envs[i].pre_delay   = v(p_env1_pre_delay + off) * rate;
        data.envs[i].attack      = v(p_env1_attack + off) * rate;
        data.envs[i].hold        = v(p_env1_hold + off) * rate;
        data.envs[i].decay       = v(p_env1_decay + off) * rate;
        data.envs[i].sustain     = v(p_env1_sustain + off);
        data.envs[i].release     = v(p_env1_release + off) * rate;
        data.envs[i].curve       = v(p_env1_curve + off);
        data.envs[i].retrigger   = v(p_env1_retrigger + off);
    }

    // mods
    int mod_count = 0;
    for (uint i = 0; i < NMOD; i++) {
        uint off = i * MOD_OFF;
        data.mods[i].src         = v(p_mod1_src + off);
        data.mods[i].target      = v(p_mod1_target + off);
        data.mods[i].amount      = v(p_mod1_amount + off);
        if (data.mods[i].src > 0 && data.mods[i].target > 0) {
            mod_count = i + 1;
        }
    }

    data.mod_count = mod_count;
}

void rogueSynth::pre_process(uint from, uint to) {
    update();

    from = data.oversample * from;
    to = data.oversample * to;

    uint samples = to - from;
    std::memset(left + from, 0, sizeof(float) * samples);
    std::memset(right + from, 0, sizeof(float) * samples);
}

void rogueSynth::post_process(uint from, uint to) {
    float* pleft = p(p_left) + from;
    float* pright = p(p_right) + from;

    const uint samples = to - from;

    // downsample
    converter_data.data_in = left + (data.oversample * from);
    converter_data.input_frames = data.oversample * samples;
    converter_data.data_out = pleft;
    converter_data.output_frames = samples;
    src_process(converter_l, &converter_data);

    converter_data.data_in = right + (data.oversample * from);
    converter_data.input_frames = data.oversample * samples;
    converter_data.data_out = pright;
    converter_data.output_frames = samples;
    src_process(converter_r, &converter_data);

    // shift global LFO phases
    for (uint i = 0; i < NLFO; i++) {
        if (data.lfos[i].reset_type == 1) {
            float inc = data.lfos[i].freq / sample_rate;
            float phase = data.lfos[i].phase;
            phase = fmod(phase + samples * inc, 1.0f);
            data.lfos[i].phase = phase;
        }
    }

    // DC blocking
    ldcBlocker.process(pleft, pleft, samples);
    rdcBlocker.process(pright, pright, samples);

    if (!effects_activated) {
        chorus_ports[0] = p(p_chorus_t);
        chorus_ports[1] = p(p_chorus_width);
        chorus_ports[2] = p(p_chorus_rate);
        chorus_ports[3] = p(p_chorus_blend);
        chorus_ports[4] = p(p_chorus_feedforward);
        chorus_ports[5] = p(p_chorus_feedback);
        chorus.activate();

        phaser_ports[2] = p(p_phaser_rate);
        phaser_ports[3] = p(p_phaser_depth);
        phaser_ports[4] = p(p_phaser_spread);
        phaser_ports[5] = p(p_phaser_resonance);
        phaser.activate();

        delay_ports[2] = p(p_delay_bpm);
        delay_ports[3] = p(p_delay_divider);
        delay_ports[4] = p(p_delay_feedback);
        delay_ports[5] = p(p_delay_dry);
        delay_ports[6] = p(p_delay_blend);
        delay_ports[7] = p(p_delay_tune);
        delay.activate();

        reverb_ports[2] = p(p_reverb_bandwidth);
        reverb_ports[3] = p(p_reverb_tail);
        reverb_ports[4] = p(p_reverb_damping);
        reverb_ports[5] = p(p_reverb_blend);
        reverb.activate();

    	effects_activated = true;
    }

    // chorus
    if (*p(p_chorus_on) > 0.0) {
    	chorus_ports[6] = pleft;
    	chorus_ports[7] = pright;
    	chorus_ports[8] = pleft;
    	chorus_ports[9] = pright;
    	chorus.run(samples);
    }
    // phaser
    if (*p(p_phaser_on) > 0.0) {
    	phaser_ports[0] = pleft;
    	phaser_ports[1] = pright;
    	phaser_ports[6] = pleft;
    	phaser_ports[7] = pright;
    	phaser.run(samples);
    }
    // delay
    if (*p(p_delay_on) > 0.0) {
        delay_ports[0] = pleft;
        delay_ports[1] = pright;
        delay_ports[8] = pleft;
        delay_ports[9] = pright;
        delay.run(samples);
    }
    // reverb
    if (*p(p_reverb_on) > 0.0) {
    	reverb_ports[0] = pleft;
    	reverb_ports[1] = pright;
    	reverb_ports[6] = pleft;
    	reverb_ports[7] = pright;
    	reverb.run(samples);
    }

    // volume
    for (uint i = 0; i < samples; i++) {
        left[i] = data.volume * pleft[i];
        right[i] = data.volume * pright[i];
    }

    // TODO limiter
}

void rogueSynth::handle_midi(uint size, unsigned char* data) {

    //discard invalid midi messages
    if (size != 3) {
        return;
    }

    //receive on all channels
    switch(data[0] & 0xf0) {
    case 0x80: //note off
        for (uint i = 0; i < NVOICES; ++i) {
            if (voices[i]->get_key() == data[1]) {
                voices[i]->off(data[2]);
           }
        }
        break;

    case 0x90: //note on
        voices[ find_free_voice(data[1], data[2]) ]->on(data[1], data[2]);
        break;

    case 0xE0: // pitchbend
        this->data.pitch_bend = this->data.bend_range * float(128 * data[2] + data[1] - 8192) / 8192.0;
        break;

    //controller
    case 0xB0:
        switch (data[1]) {
        case 0x01:  //mod wheel
        case 0x43:  //soft pedal
            // TODO
            break;

        case 0x07:  //volume
            // TODO
            break;

        case 0x40:  //sustain pedal
        case 0x42:  //sostenuto pedal
            // TODO
            break;

        //all sound off
        case 0x78:
        //all notes off
        case 0x7b:
            for (uint v = 0; v < NVOICES; v++) {
                voices[v]->reset();
            }
            break;

        default: break;
        }
        break;

    default: break;
    }
}

static int _ = rogueSynth::register_class(p_uri);

}
