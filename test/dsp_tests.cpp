#include <sndfile.hh>
#include "oscillator.cpp"
#include "filter.cpp"
#include "lfo.cpp"
#include "envelope.cpp"
#include "tables.cpp"

#define SR 44100.0
#define SIZE 44100
#define CHANNELS 1

void write_wav(char* filename, float* buffer) {
    static const int FORMAT = SF_FORMAT_WAV | SF_FORMAT_FLOAT;
    SndfileHandle outfile(filename, SFM_WRITE, FORMAT, CHANNELS, SR);
    if (outfile) {
        outfile.write(&buffer[0], SIZE);
    }
}

int main() {
    char filename[50];
    float buffer[SIZE];
    dsp::PhaseShaping osc;
    osc.setSamplerate(SR);
    osc.setFreq(440.0f);
    osc.setParams(0.5, 0.5);

    dsp::MoogFilter moog;
    moog.setSamplerate(SR);
    moog.setCoefficients(1000.0, 0.5);

    dsp::StateVariableFilter svf;
    svf.setSamplerate(SR);
    svf.setCoefficients(1000.0, 0.5);

    dsp::LFO lfo;
    lfo.setSamplerate(SR);
    lfo.setFreq(10.0);

    dsp::AHDSR env;

    // oscs
    for (int i = 0; i < 10; i++) {
        osc.reset();
        osc.setType(i);
        osc.process(buffer, SIZE);

        sprintf(filename, "osc_%i.wav", i);
        write_wav(filename, buffer);
    }

    // noise input
    float noise[SIZE];
    osc.reset();
    osc.setFreq(1000.0);
    osc.setType(9);
    osc.process(noise, SIZE);

    // moog
    for (int i = 0; i < 8; i++) {
        moog.clear();
        moog.setType(i);
        moog.process(noise, buffer, SIZE);

        sprintf(filename, "moog_%i.wav", i);
        write_wav(filename, buffer);
    }

    // svf
    for (int i = 0; i < 4; i++) {
        svf.clear();
        svf.setType(i);
        svf.process(noise, buffer, SIZE);

        sprintf(filename, "svf_%i.wav", i);
        write_wav(filename, buffer);
    }

    // lfos
    for (int i = 0; i < 6; i++) {
        lfo.reset();
        lfo.setType(i);
        //lfo.process(buffer, SIZE);
        for (int j = 0; j < SIZE; j++) {
            buffer[j] = lfo.tick();
        }

        sprintf(filename, "lfo_%i.wav", i);
        write_wav(filename, buffer);
    }

    // envs
    env.setAHDSR(0.1 * SR, 0.2 * SR, 0.3 * SR, 0.9, 0.5 * SR);
    env.on();
    for (int j = 0; j < 0.5 * SIZE; j++) {
        buffer[j] = env.tick();
    }
    env.off();
    for (int j = 0.5* SIZE; j < SIZE; j++) {
            buffer[j] = env.tick();
    }
    sprintf(filename, "env_%i.wav", 0);
    write_wav(filename, buffer);

    env.on();
    for (int j = 0; j < 0.25 * SIZE; j++) {
        buffer[j] = env.tick();
    }
    env.off();
    for (int j = 0.25 * SIZE; j < 0.5 * SIZE; j++) {
        buffer[j] = env.tick();
    }
    env.on();
    for (int j = 0.5 * SIZE; j < 0.75 * SIZE; j++) {
        buffer[j] = env.tick();
    }
    env.off();
    for (int j = 0.75 * SIZE; j < SIZE; j++) {
        buffer[j] = env.tick();
    }
    sprintf(filename, "env_%i.wav", 1);
    write_wav(filename, buffer);


    return 0;
}