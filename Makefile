BUNDLE = lv2-rogue.lv2
INSTALL_DIR = /usr/local/lib/lv2

SOURCES = dsp/*.cpp src/*.cpp
SOURCES_UI = src/gui/rogue-gui.cpp
FLAGS = -fPIC -DPIC -std=c++11 -O 
LVTK = `pkg-config --cflags --libs lvtk-plugin-1`
LVTK_UI = `pkg-config --cflags --libs lvtk-ui-1`
GTKMM = `pkg-config --cflags --libs gtkmm-2.4`
SNDFILE = -lsndfile

$(BUNDLE): manifest.ttl rogue.ttl rogue.so rogue-gui.so presets
	rm -rf $(BUNDLE)
	mkdir $(BUNDLE)
	cp -r $^ $(BUNDLE)

rogue.so: $(SOURCES) src/rogue.gen
	$(CXX) $(FLAGS) -g -shared $(SOURCES) $(LVTK) -Idsp -Isrc -o $@
	
rogue-gui.so: $(SOURCES_UI) src/rogue.gen src/gui/config.gen
	$(CXX) $(FLAGS) -g -shared $(SOURCES_UI) $(GTKMM) $(LVTK) $(LVTK_UI) -Isrc -o $@	

src/rogue.gen: rogue.ttl
	ttl2c $^ src/rogue.gen
    
rogue.ttl:
	./confgen.py     
	
src/gui/config.gen:
	./confgen.py

install: $(BUNDLE)
	mkdir -p $(INSTALL_DIR)
	rm -rf $(INSTALL_DIR)/$(BUNDLE)
	cp -R $(BUNDLE) $(INSTALL_DIR)

run:
	jalv.gtk http://www.github.com/timowest/rogue

clean:
	rm -rf $(BUNDLE) rogue.so src/rogue.peg rogue.ttl *.out *.wav
	
guitests: src/rogue.gen src/gui/config.gen	
	$(CXX) -g -std=c++11 src/gui/knob-test.cpp $(GTKMM) -Isrc -o knobtest.out		
	$(CXX) -g -std=c++11 src/gui/rogue-gui-test.cpp $(GTKMM) $(LVTK_UI) -Isrc -o guitest.out	
	
tests: src/rogue.gen
	$(CXX) -g -std=c++11 test/dsp_tests.cpp $(SNDFILE) -Idsp -o dsp_tests.out
	$(CXX) -g -std=c++11 test/voice_tests.cpp $(SNDFILE) $(LVTK) -Idsp -Isrc -o voice_tests.out
	./tests.out
	./voice_tests.out