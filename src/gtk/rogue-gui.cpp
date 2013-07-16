/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#include <stdio.h>
#include <string.h>
#include <gtkmm.h>
#include <lvtk/gtkui.hpp>

#include "common.h"
#include "rogue.gen"
#include "gui/config.gen"
#include "gui/knob.h"
#include "gui/label.h"
#include "gui/panel.h"
#include "gui/select.h"
#include "gui/toggle.h"
#include "gui/wavedraw.h"

#include "wrappers.h"

#define OSC_DRAW_WIDTH 140
#define LFO_DRAW_WIDTH 105
#define ENV_DRAW_WIDTH 105

#define DRAW_HEIGHT 60

using namespace sigc;
using namespace Gtk;

namespace rogue {

class rogueGUI : public lvtk::UI<rogueGUI, lvtk::GtkUI<true>, lvtk::URID<true> > {
  public:
    rogueGUI(const char* URI);
    Widget* createOSC(int i);
    Widget* createFilter(int i);
    Widget* createLFO(int i);
    Widget* createEnv(int i);
    Widget* createMain();
    Widget* createEffects();
    Widget* createModulation();
    Widget* smallFrame(const char* label, Table* content);
    Widget* frame(const char* label, int toggle, Table* content);
    Alignment* align(Widget* widget);
    void control(Table* table, const char* label, int port_index, int left, int top);
    void port_event(uint32_t port, uint32_t buffer_size, uint32_t format, const void* buffer);
    void change_status_bar(uint32_t port, float value);
    void update_osc(int i, float value);
    void update_lfo(int i, float value);
    Widget& get_widget() { return container(); }

  protected:
    VBox mainBox;
    Statusbar statusbar;
    char statusBarText[100];
    Changeable* scales[p_n_ports];

    Wavedraw* oscWaves[NOSC];
    Wavedraw* lfoWaves[NLFO];
    Wavedraw* envWaves[NENV];

    float oscBuffers[NOSC][OSC_DRAW_WIDTH];
    float lfoBuffers[NLFO][LFO_DRAW_WIDTH];
    float envBuffers[NENV][ENV_DRAW_WIDTH];

    Osc osc;
    dsp::LFO lfo;
};

// implementation

// TODO replace with vectors
#define CHARS static const char*

// checkbox content
CHARS osc_types[] = {
        // V
        "VA Saw", "VA Tri Saw", "VA Pulse",
        "PD Saw", "PD Square", "PD Pulse", "PD Double Sine", "PD Saw Pulse", "PD Res 1", "PD Res 2", "PD Res 3", "PD Half Sine",
        "Saw", "Double Saw", "Tri", "Tri 2", "Tri 3", "Pulse", "Pulse Saw", "Slope", "Alpha 1", "Alpha 2",
        "Beta 1", "Beta 2", "Pulse Tri", "Exp",
        "FM 1", "FM 2", "FM 3", "FM 4", "FM 5", "FM 6", "FM 7", "FM 8",
        // AS
        "AS Saw", "AS Square", "AS Impulse",
        // Noise
        "Noise", "Pink Noise", "LP Noise", "BP Noise"};

CHARS filter_types[] = {"LP 24dB", "LP 18dB", "LP 12dB", "LP 6dB", "HP 24dB",
        "BP 12dB", "BP 18dB", "Notch",
        "SVF LP", "SVF HP", "SVF BP", "SVF Notch"};

CHARS filter1_sources[] = {"Bus A", "Bus B"};

CHARS filter2_sources[] = {"Bus A", "Bus B", "Filter1"};

CHARS lfo_types[] = {"Sin", "Tri", "Saw", "Square", "S/H"};

CHARS lfo_reset_types[] = {"Poly", "Free", "Mono"};

// labels

CHARS nums[] = {"1", "2", "3", "4", "5", "6"};

CHARS osc_labels[] = {"OSC 1", "OSC 2", "OSC 3", "OSC 4"};

CHARS filter_labels[] = {"Filter 1", "Filter 2"};

CHARS lfo_labels[] = {"LFO 1", "LFO 2", "LFO 3"};

CHARS env_labels[] = {"Env 1", "Env 2", "Env 3", "Env 4", "Env 5"};

CHARS mod_src_labels[] = {
        "--   ",
        "Mod", "Pressure", "Key", "Velocity",
        "LFO 0", "LFO 0+", "LFO 1", "LFO 1+", "LFO 2", "LFO 2+", "LFO 3", "LFO 3+",
        "Env 1", "Env 2", "Env 3", "Env 4", "Env 5"};

CHARS mod_target_labels[]  = {
        "--   ",
        // osc
        "OSC 1 Pitch", "OSC 1 Mod", "OSC 1 PWM", "OSC 1 Amp",
        "OSC 2 Pitch", "OSC 2 Mod", "OSC 2 PWM", "OSC 2 Amp",
        "OSC 3 Pitch", "OSC 3 Mod", "OSC 3 PWM", "OSC 3 Amp",
        "OSC 4 Pitch", "OSC 4 Mod", "OSC 4 PWM", "OSC 4 Amp",
        // dcf
        "Filter 1 Freq", "Filter 1 Q", "Filter 1 Pan", "Filter 1 Amp",
        "Filter 2 Freq", "Filter 2 Q", "Filter 2 Pan", "Filter 2 Amp",
        // lfo
        "LFO 1 Speed", "LFO 1 Amp",
        "LFO 2 Speed", "LFO 2 Amp",
        "LFO 3 Speed", "LFO 3 Amp",
        // env
        "Env 1 Speed", "Env 1 Amp",
        "Env 2 Speed", "Env 2 Amp",
        "Env 3 Speed", "Env 3 Amp",
        "Env 4 Speed", "Env 4 Amp",
        "Env 5 Speed", "Env 5 Amp",
        // bus
        "Bus A Pan", "Bus B Pan"};


rogueGUI::rogueGUI(const char* URI) {
    std::cout << "starting GUI" << std::endl;

    // initialize sliders
    for (int i = 3; i < p_n_ports; i++) {
        uint32_t type = p_port_meta[i].type;
        if (type == KNOB) {
            Knob* knob = new Knob(p_port_meta[i].min, p_port_meta[i].max, p_port_meta[i].step);
            knob->set_radius(12.0);
            knob->set_size(38);
            scales[i] = manage(knob);
        } else if (type == LABEL) {
            scales[i] = manage(new LabelBox(p_port_meta[i].min, p_port_meta[i].max, p_port_meta[i].step));
        } else if (type == TOGGLE) {
            scales[i] = manage(new Toggle());
        } else if (type != SELECT) {
            std::cout << i << std::endl;
        } else if (i == p_osc1_type || i == p_osc2_type || i == p_osc3_type || i == p_osc4_type) {
            scales[i] = manage(new SelectBox(osc_types, 34 + 3 + 4));
        } else if (i == p_filter1_type || i == p_filter2_type) {
            scales[i] = manage(new SelectBox(filter_types, 12));
        } else if (i == p_filter1_source) {
            scales[i] = manage(new SelectBox(filter1_sources, 2));
        } else if (i == p_filter2_source) {
            scales[i] = manage(new SelectBox(filter2_sources, 3));
        } else if (i == p_lfo1_type || i == p_lfo2_type || i == p_lfo3_type) {
            scales[i] = manage(new SelectBox(lfo_types, 5));
        } else if (i == p_lfo1_reset_type || i == p_lfo2_reset_type || i == p_lfo3_reset_type) {
            scales[i] = manage(new SelectBox(lfo_reset_types, 3));
        } else if (i >= p_mod1_src || i <= p_mod20_amount) {
            if ((i - p_mod1_src) % 3 == 0) {
                scales[i] = manage(new SelectBox(mod_src_labels, M_SIZE));
            } else {
                scales[i] = manage(new SelectBox(mod_target_labels, M_TARGET_SIZE));
            }
        } else {
            std::cout << i << std::endl;
        }
    }

    // connect widgets to ports
    for (int i = 3; i < p_n_ports; i++) {
        slot<void> slot1 = compose(bind<0>(mem_fun(*this, &rogueGUI::write_control), i),
            mem_fun(*scales[i], &Changeable::get_value));
        scales[i]->connect(slot1);
        int type = p_port_meta[i].type;
        // connect knobs to statusbar
        if (type == KNOB) {
            slot<void> slot2 = compose(bind<0>(mem_fun(*this, &rogueGUI::change_status_bar), i),
                mem_fun(*scales[i], &Changeable::get_value));
            scales[i]->connect(slot2);
        }
    }

    //osc.setFreq(1.0f);
    osc.setSamplerate(OSC_DRAW_WIDTH);

    lfo.setFreq(1.0);
    lfo.setSamplerate(LFO_DRAW_WIDTH);

    // clear buffers
    for (int i = 0; i < NOSC; i++) {
        std::memset(oscBuffers[i], 0, sizeof(float) * OSC_DRAW_WIDTH);
    }
    for (int i = 0; i < NLFO; i++) {
        std::memset(lfoBuffers[i], 0, sizeof(float) * LFO_DRAW_WIDTH);
    }
    for (int i = 0; i < NENV; i++) {
        std::memset(envBuffers[i], 0, sizeof(float) * ENV_DRAW_WIDTH);
    }

    // osc visualizations
    for (int i = 0; i < NOSC; i++) {
        Wavedraw* draw = manage(new Wavedraw(OSC_DRAW_WIDTH, DRAW_HEIGHT, oscBuffers[i], OSC_DRAW_WIDTH));
        oscWaves[i] = draw;

        // connect type, inv, width, param1, param2
        slot<void> slot1 = compose(bind<0>(mem_fun(*this, &rogueGUI::update_osc), i),
                    mem_fun(*scales[p_osc1_type + i * OSC_OFF], &Changeable::get_value));
        scales[p_osc1_type + i * OSC_OFF]->connect(slot1);
        scales[p_osc1_width + i * OSC_OFF]->connect(slot1);
        update_osc(i, 0.0);
    }

    // lfo visualizations
    for (int i = 0; i < NLFO; i++) {
        Wavedraw* draw = manage(new Wavedraw(LFO_DRAW_WIDTH, DRAW_HEIGHT, lfoBuffers[i], LFO_DRAW_WIDTH));
        lfoWaves[i] = draw;

        // connect type, inv, width, param1, param2
        slot<void> slot1 = compose(bind<0>(mem_fun(*this, &rogueGUI::update_lfo), i),
                    mem_fun(*scales[p_lfo1_type + i * LFO_OFF], &Changeable::get_value));
        scales[p_lfo1_type + i * LFO_OFF]->connect(slot1);
        scales[p_lfo1_width + i * LFO_OFF]->connect(slot1);
        update_lfo(i, 0.0);
    }

    // env visualizations
    for (int i = 0; i < NENV; i++) {
        Wavedraw* draw = manage(new Wavedraw(ENV_DRAW_WIDTH, DRAW_HEIGHT, envBuffers[i], ENV_DRAW_WIDTH));
        envWaves[i] = draw;

        // TODO connect and update
    }

    Table* table = manage(new Table(4, 3));
    table->set_spacings(5);

    // main
    table->attach(*createMain(), 1, 2, 0, 1);

    // effects
    table->attach(*createEffects(), 2, 3, 0, 1);

    // oscs
    Table* oscTable = manage(new Table(2, 2));
    oscTable->attach(*createOSC(0), 0, 1, 0, 1);
    oscTable->attach(*createOSC(1), 1, 2, 0, 1);
    oscTable->attach(*createOSC(2), 0, 1, 1, 2);
    oscTable->attach(*createOSC(3), 1, 2, 1, 2);
    Frame* oscFrame = manage(new Frame("Oscillators"));
    oscFrame->add(*oscTable);
    table->attach(*oscFrame, 0, 2, 1, 3);

    // filters
    Table* filterTable = manage(new Table(2, 1));
    filterTable->attach(*createFilter(0), 0, 1, 0, 1);
    filterTable->attach(*createFilter(1), 0, 1, 1, 2);
    Frame* filterFrame = manage(new Frame("Filters"));
    filterFrame->add(*filterTable);
    table->attach(*filterFrame, 2, 3, 1, 3);

    // envelopes
    Notebook* envelopes = manage(new Notebook());
    envelopes->set_tab_pos(POS_LEFT);
    for (int i = 0; i < NENV; i++) {
        envelopes->append_page(*createEnv(i), nums[i]);
    }
    Frame* envelopesFrame = manage(new Frame("Envelopes"));
    envelopesFrame->add(*envelopes);
    table->attach(*envelopesFrame, 0, 1, 3, 4);

    // modulation
    table->attach(*createModulation(), 1, 2, 3, 4);

    // lfos
    Notebook* lfos = manage(new Notebook());
    lfos->set_tab_pos(POS_LEFT);
    for (int i = 0; i < NLFO; i++) {
        lfos->append_page(*createLFO(i), nums[i]);
    }
    Frame* lfosFrame = manage(new Frame("LFOs"));
    lfosFrame->add(*lfos);
    table->attach(*lfosFrame, 2, 3, 3, 4);

    mainBox.pack_start(*table);
    mainBox.pack_end(statusbar);
    add(*align(&mainBox));
    std::cout << "GUI ready" << std::endl;
}

Widget* rogueGUI::createOSC(int i) {
    int off = i * OSC_OFF;
    Table* table = manage(new Table(4, 8));
    // row 1
    control(table, "Type", p_osc1_type + off, 0, 1);
    control(table, "Inv", p_osc1_inv + off, 1, 1);
    control(table, "Track", p_osc1_tracking + off, 2, 1);
    control(table, "Level", p_osc1_level + off, 3, 1);

    table->attach(*oscWaves[i], 4, 8, 1, 3);

    // row 2
    control(table, "Coarse", p_osc1_coarse + off, 0, 3);
    control(table, "Fine", p_osc1_fine + off, 1, 3);
    control(table, "Ratio", p_osc1_ratio + off, 2, 3);
    control(table, "Tone", p_osc1_tone + off, 3, 3);
    control(table, "Width", p_osc1_width + off, 4, 3);
    control(table, "Vol A", p_osc1_level_a + off, 5, 3);
    control(table, "Vol B", p_osc1_level_b + off, 6, 3);

    // TODO
    //control(table, "Free", p_osc1_free + off, 2, 1);

    return frame(osc_labels[i], p_osc1_on + off, table);
}

Widget* rogueGUI::createFilter(int i) {
    int off = i * DCF_OFF;
    Table* table = manage(new Table(4,6));
    // row 1
    control(table, "Type", p_filter1_type + off, 0, 1);
    control(table, "Source", p_filter1_source + off, 1, 1);
    control(table, "Freq", p_filter1_freq + off, 2, 1);
    control(table, "Res", p_filter1_q + off, 3, 1);
    control(table, "Vol", p_filter1_level + off, 4, 1);

    // row 2
    control(table, "Dist", p_filter1_distortion + off, 0, 3);
    control(table, "Key>F", p_filter1_key_to_f + off, 1, 3);
    control(table, "Vel>F", p_filter1_vel_to_f + off, 2, 3);
    // empty
    control(table, "Pan", p_filter1_pan + off, 4, 3);

    return frame(filter_labels[i], p_filter1_on + off, table);
}

Widget* rogueGUI::createLFO(int i) {
    int off = i * LFO_OFF;
    Table* table = manage(new Table(4, 7));
    // row 1
    control(table, "Type", p_lfo1_type + off, 0, 1);
    control(table, "Reset", p_lfo1_reset_type + off, 1, 1);
    control(table, "Freq", p_lfo1_freq + off, 2, 1);
    control(table, "Width", p_lfo1_width + off, 3, 1);

    table->attach(*lfoWaves[i], 4, 7, 1, 3);

    // row 2
    control(table, "Rand", p_lfo1_humanize + off, 0, 3);

    return frame(lfo_labels[i], p_lfo1_on + off, table);
}

Widget* rogueGUI::createEnv(int i) {
    int off = i * ENV_OFF;
    Table* table = manage(new Table(4, 7));
    // row 1
    control(table, "A", p_env1_attack + off, 0, 1);
    control(table, "D", p_env1_decay + off, 1, 1);
    control(table, "S", p_env1_sustain + off, 2, 1);
    control(table, "R", p_env1_release + off, 3, 1);

    table->attach(*envWaves[i], 4, 7, 1, 3);

    // row 2
    control(table, "Hold", p_env1_hold + off, 0, 3);
    control(table, "Pre", p_env1_pre_delay + off, 1, 3);
    control(table, "Curve", p_env1_curve + off, 2, 3);
    control(table, "Retr", p_env1_retrigger + off, 3, 3);

    return frame(env_labels[i], p_env1_on + off, table);
}

Widget* rogueGUI::createMain() {
    Table* table = manage(new Table(2, 7));
    table->set_spacings(5);
    // row 1
    control(table, "Volume", p_volume, 0, 1);
    control(table, "Vol A", p_bus_a_level, 1, 1);
    control(table, "Pan A", p_bus_a_pan, 2, 1);
    control(table, "Vol B", p_bus_b_level, 3, 1);
    control(table, "Pan B", p_bus_b_pan, 4, 1);
    control(table, "Glide", p_glide_time, 5, 1);
    control(table, "Bend", p_bend_range, 6, 1);

    Frame* mainFrame = manage(new Frame("Main"));
    mainFrame->add(*align(table));

    //return frame(env_labels[i], p_env1_on + off, table);
    return mainFrame;
}

Widget* rogueGUI::createEffects() {
	Notebook* effects = manage(new Notebook());
	effects->set_tab_pos(POS_LEFT);
    // chorus - t, width, rate, blend, feedfoward, feedback
	Table* chorus = manage(new Table(2, 6));
	control(chorus, "T", p_chorus_t, 0, 1);
	control(chorus, "Width", p_chorus_width, 1, 1);
	control(chorus, "Rate", p_chorus_rate, 2, 1);
	control(chorus, "Blend", p_chorus_blend, 3, 1);
	control(chorus, "Feedfoward", p_chorus_feedforward, 4, 1);
	control(chorus, "Feedback", p_chorus_feedback, 5, 1);
	effects->append_page(*frame("Chorus", p_chorus_on, chorus), "C");

	// phaser - rate, depth, spread, resonance
	Table* phaser = manage(new Table(2, 4));
	control(phaser, "Rate", p_phaser_rate, 0, 1);
	control(phaser, "Depth", p_phaser_depth, 1, 1);
	control(phaser, "Spread", p_phaser_spread, 2, 1);
	control(phaser, "Resonance", p_phaser_resonance, 3, 1);
	effects->append_page(*frame("Phaser", p_phaser_on, phaser), "P");

	// delay - bpm, divider, feedback, dry, blend, tune
	Table* delay = manage(new Table(2, 6));
	control(delay, "BPM", p_delay_bpm, 0, 1);
	control(delay, "Divider", p_delay_divider, 1, 1);
	control(delay, "Feedback", p_delay_feedback, 2, 1);
	control(delay, "Dry", p_delay_dry, 3, 1);
	control(delay, "Blend", p_delay_blend, 4, 1);
	control(delay, "Tune", p_delay_tune, 5, 1);
	effects->append_page(*frame("Delay", p_delay_on, delay), "D");

	// reverb - bandwidth, tail, damping, blend
	Table* reverb = manage(new Table(2, 4));
	control(reverb, "Bandwidth", p_reverb_bandwidth, 0, 1);
	control(reverb, "Tail", p_reverb_tail, 1, 1);
	control(reverb, "Damping", p_reverb_damping, 2, 1);
	control(reverb, "Blend", p_reverb_blend, 3, 1);
	effects->append_page(*frame("Reverb", p_reverb_on, reverb), "R");

	Frame* effectsFrame = manage(new Frame("Effects"));
	effectsFrame->add(*effects);
	return effectsFrame;
}


Widget* rogueGUI::createModulation() {
    Table* table1 = manage(new Table(5, 6));
    Table* table2 = manage(new Table(5, 6));
    //table->set_spacings(5);
    for (int i = 0; i < 5; i++) {
        int off = i * 3;
        // table 1
        // col 1
        table1->attach(*scales[p_mod1_src + off]->get_widget(), 0, 1, i + 1, i + 2);
        table1->attach(*scales[p_mod1_target + off]->get_widget(), 1, 2, i + 1, i + 2);
        table1->attach(*scales[p_mod1_amount + off]->get_widget(), 2, 3, i + 1, i + 2);
        // col 2
        table1->attach(*scales[p_mod6_src + off]->get_widget(), 3, 4, i + 1, i + 2);
        table1->attach(*scales[p_mod6_target + off]->get_widget(), 4, 5, i + 1, i + 2);
        table1->attach(*scales[p_mod6_amount + off]->get_widget(), 5, 6, i + 1, i + 2);

        // table 2
        // col 1
        table2->attach(*scales[p_mod11_src + off]->get_widget(), 0, 1, i + 1, i + 2);
        table2->attach(*scales[p_mod11_target + off]->get_widget(), 1, 2, i + 1, i + 2);
        table2->attach(*scales[p_mod11_amount + off]->get_widget(), 2, 3, i + 1, i + 2);
        // col 2
        table2->attach(*scales[p_mod16_src + off]->get_widget(), 3, 4, i + 1, i + 2);
        table2->attach(*scales[p_mod16_target + off]->get_widget(), 4, 5, i + 1, i + 2);
        table2->attach(*scales[p_mod16_amount + off]->get_widget(), 5, 6, i + 1, i + 2);
    }

    Notebook* mod = manage(new Notebook());
    mod->set_tab_pos(POS_LEFT);
    mod->append_page(*table1, "1");
    mod->append_page(*table2, "2");

    Frame* modFrame = manage(new Frame("Modulation"));
    modFrame->add(*mod);
    return modFrame;
}


void rogueGUI::control(Table* table, const char* label, int port_index, int left, int top) {
    table->attach(*scales[port_index]->get_widget(), left, left + 1, top, top + 1);
    table->attach(*manage(new Label(label)), left, left + 1, top + 1, top + 2);
}

Widget* rogueGUI::smallFrame(const char* label, Table* content) {
    content->set_border_width(2);
    content->set_col_spacings(2);
    content->set_spacings(2);

    Frame* frame = manage(new Frame());
    frame->set_label_align(0.0f, 0.0f);
    frame->set_border_width(5);
    frame->set_label(label);
    frame->add(*content);

    Alignment* alignment = manage(new Alignment(0.0, 0.0, 1.0, 0.0));
    alignment->add(*frame);
    return alignment;
}

Widget* rogueGUI::frame(const char* label, int toggle, Table* content) {
    content->set_border_width(2);
    content->set_col_spacings(5);
    content->set_spacings(2);

    Panel* panel = manage(new Panel(label, scales[toggle]->get_widget(), align(content)));

    Alignment* alignment = manage(new Alignment(0.0, 0.0, 1.0, 0.0));
    alignment->add(*panel);
    return alignment;
}

Alignment* rogueGUI::align(Widget* widget) {
    Alignment* alignment = manage(new Alignment(0.0, 0.0, 0.0, 0.0));
    alignment->add(*widget);
    return alignment;
}

void rogueGUI::port_event(uint32_t port, uint32_t buffer_size, uint32_t format, const void* buffer) {
    if (port > 2) {
        scales[port]->set_value(*static_cast<const float*>(buffer));
    }
}

void rogueGUI::change_status_bar(uint32_t port, float value) {
   //if (p_port_meta[port-3].step >= 1.0f) {
   //    sprintf(statusBarText, "%s = %3.0f", p_port_meta[port].symbol, value);
   //} else {
       sprintf(statusBarText, "%s = %3.3f", p_port_meta[port].symbol, value);
   //}
   statusbar.remove_all_messages();
   statusbar.push(statusBarText);
}

void rogueGUI::update_osc(int i, float value) {
    float t = scales[p_osc1_tone + i * OSC_OFF]->get_value();
    float w = scales[p_osc1_width + i * OSC_OFF]->get_value();
    osc.process((int)value, 1.0f, t, w, w, oscBuffers[i], OSC_DRAW_WIDTH);
    oscWaves[i]->refresh();
}

void rogueGUI::update_lfo(int i, float value) {
    float w = scales[p_lfo1_width + i * LFO_OFF]->get_value();

    lfo.setType((int)value);
    lfo.setWidth(w);
    lfo.reset();
    float* buffer = lfoBuffers[i];
    for (int j = 0; j < LFO_DRAW_WIDTH; j++) {
        buffer[j] = lfo.tick();
    }

    lfoWaves[i]->refresh();
}

static int _ = rogueGUI::register_class("http://www.github.com/timowest/rogue/ui");
}