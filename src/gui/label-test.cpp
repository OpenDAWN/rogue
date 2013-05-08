/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#include <gtkmm/main.h>
#include "gui/label.h"

int main(int argc, char* argv[]) {
    Gtk::Main kit(argc, argv);

    rogue::LabelBox label(0.0, 1.0, 0.01);
    Gtk::Alignment alignment(0.0, 0.0, 0.0, 0.0);
    alignment.add(label);

    Gtk::Window window;
    window.set_title("Label");
    window.set_default_size(800, 400);
    window.add(alignment);
    window.show_all();

    Gtk::Main::run(window);

    return 0;
}
