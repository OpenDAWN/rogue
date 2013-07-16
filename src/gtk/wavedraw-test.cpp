/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#include <gtkmm/main.h>
#include "gui/wavedraw.h"

int main(int argc, char* argv[]) {
    Gtk::Main kit(argc, argv);

    float samples[40];
    for (int i = 0; i < 40; i++) {
        samples[i] = -1.0 + i / 20.0;
    }

    rogue::Wavedraw draw(160, 80, samples, 40);
    Gtk::Alignment alignment(0.0, 0.0, 0.0, 0.0);
    alignment.add(draw);

    Gtk::Window window;
    window.set_title("Label");
    window.set_default_size(800, 400);
    window.add(alignment);
    window.show_all();

    Gtk::Main::run(window);

    return 0;
}
