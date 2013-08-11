#include "wavutils.h"

void filter_test() {
    char filename[50];
    float buffer[SIZE];
    float buffer2[SIZE];

    // for input
    dsp::Noise no;
    no.setSamplerate(SR);
    no.setFreq(440.0f);

    dsp::MoogFilter moog;
    moog.setSamplerate(SR);
    moog.setCoefficients(1000.0, 0.5);

    dsp::StateVariableFilter svf;
    svf.setSamplerate(SR);
    svf.setCoefficients(1000.0, 0.5);

    dsp::StateVariableFilter2 svf2;
    svf2.setSamplerate(SR);
    svf2.setCoefficients(1000.0, 0.5);

    // noise input
    float noise[SIZE];
    no.setFreq(1000.0);
    no.setType(0);
    no.process(noise, SIZE);

    // moog
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 10; j++) {
            moog.clear();
            moog.setType(i);
            moog.setCoefficients(1000.0, float(j) * 0.1);
            moog.process(noise, buffer, SIZE);

            sprintf(filename, "wavs/moog_%i%i.wav", i, j);
            write_wav(filename, buffer);

            // TODO verify that output is not zero
        }
    }

    // svf
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 10; j++) {
            svf.clear();
            svf.setType(i);
            svf.setCoefficients(1000.0, float(j) * 0.1);
            svf.process(noise, buffer, SIZE);

            sprintf(filename, "wavs/svf_%i%i.wav", i, j);
            write_wav(filename, buffer);

            // TODO verify that output is not zero
        }
    }

    // svf2
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 10; j++) {
            svf2.clear();
            svf2.setType(i);
            svf2.setCoefficients(1000.0, float(j) * 0.1);
            svf2.process(noise, buffer, SIZE);

            sprintf(filename, "wavs/svf2_%i%i.wav", i, j);
            write_wav(filename, buffer);

            // TODO verify that output is not zero
        }
    }
}