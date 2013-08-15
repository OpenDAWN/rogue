#include "wavutils.h"

void oscillator_test() {
    char msg[50];
    char filename[50];
    float buffer[SIZE];
    float buffer2[SIZE];
    float buffer3[SIZE];

    for (int i = 0; i < SIZE; i++) {
        buffer2[i] = 0.0;
    }

    dsp::Virtual va;
    va.setSamplerate(SR);
    va.setFreq(440.0f);

    dsp::AS as;
    as.setSamplerate(SR);
    as.setFreq(440.0f);

    dsp::Noise no;
    no.setSamplerate(SR);
    no.setFreq(440.0f);

    // va
    for (int i = 0; i < 34; i++) {
        va.reset();
        va.setType(i);
        va.setModulation(buffer2, 0.0, false);
        va.process(buffer, SIZE);
        if (count_clicks(buffer) > 0) {
            sprintf(msg, "va click error %i", i);
            error(msg);
        }

        sprintf(filename, "wavs/va_%i.wav", i);
        write_wav(filename, buffer);

        // not bandlimited
        va.reset();
        va.setModulation(buffer2, 0.1, false);
        va.process(buffer3, SIZE);

        int diff = 0;
        for (int j = 0; j < SIZE; j++) {
            if (buffer[j] != buffer3[j]) diff++;
        }
        if (diff > 2000 && (i < 20 || i > 23)) {
            sprintf(msg, "too many diffs: %i %i", i, diff);
            std::cout << msg << std::endl;
        }

        sprintf(filename, "wavs/vapm_%i.wav", i);
        write_wav(filename, buffer3);
    }

    // as
    for (int i = 0; i < 3; i++) {
        as.reset();
        as.setType(i);
        as.process(buffer, SIZE);
        if (count_clicks(buffer) > 0) {
            sprintf(msg, "as click error %i", i);
            error(msg);
        }

        sprintf(filename, "wavs/as_%i.wav", i);
        write_wav(filename, buffer);
    }

    // noise
    for (int i = 0; i < 4; i++) {
        no.reset();
        no.setType(i);
        no.process(buffer, SIZE);

        sprintf(filename, "wavs/no_%i.wav", i);
        write_wav(filename, buffer);
    }
}
