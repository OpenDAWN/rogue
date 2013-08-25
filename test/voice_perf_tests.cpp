#include <omp.h>

#include "oscillator.cpp"
#include "filter.cpp"
#include "lfo.cpp"
#include "envelope.cpp"
#include "voice.cpp"
#include "tables.cpp"

#define SIZE 64
#define SR 44100
#define ITERATIONS 1000000

void log(const char* label, float duration) {
    std::cout << label << " " << duration << std::endl;
}

int main() {
    float buffer_l[SIZE];
    float buffer_r[SIZE];
    for (int i = 0; i < SIZE; i++) {
        buffer_l[i] = 0.0f;
        buffer_r[i] = 0.0f;
    }

    std::vector<void*> ports;
    ports.push_back(0);
    ports.push_back(buffer_l);
    ports.push_back(buffer_r);

    rogue::SynthData data;
    data.volume = 0.5;
    data.bus_a_level = 0.5;
    data.bus_a_pan = 0.5;

    data.oscs[0].on = true;
    data.oscs[0].type = 0;
    data.oscs[0].start = 0.0;
    data.oscs[0].width = 0.5;
    data.oscs[0].ratio = 1.0;
    data.oscs[0].level = 1.0;
    data.oscs[0].level_a = 1.0;

    data.envs[0].on = true;
    data.envs[0].pre_delay = 0.0f;
    data.envs[0].attack = 0.1 * SR;
    data.envs[0].hold = 0.5 * SR;
    data.envs[0].decay = 0.4 * SR;
    data.envs[0].sustain = 0.8;
    data.envs[0].release = 0.5 * SR;
    data.envs[0].curve = 0.5;

    rogue::rogueVoice voice(SR, &data);
    voice.set_port_buffers(ports);

    double start = omp_get_wtime();
    for (int j = 0; j < ITERATIONS; j++) {
        voice.on(69, 64);
        voice.render(0, SIZE / 2);
        voice.off(0);
        voice.render(SIZE / 2, SIZE);
    }
    double end = omp_get_wtime();
    log("voice", end - start);
}
