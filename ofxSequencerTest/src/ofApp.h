#pragma once

#include "ofMain.h"
#include "ofxSequencer.h"
#include "ofxMidi.h"
#include "ofxGui.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
    	void exit();
    
    //note the type of the argument has to be the same as the type of ofEvent - in this case an integer
    //also note the & - without it the notification will not happen
    void beatsIn ( int & eventInt);
    void emptySequencer();
    void startSequencer();
    void stopSequencer();
    

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		
    ofxSequencer sequencer;
    
    ofParameter<float> p1;
    ofParameter<float> p2;
    ofParameter<float> p3;
    ofParameter<float> p4;
    
    int cols;
    int rows;
    
    
    //incorporating beatEvent
    ofEvent <int> beatEvent;
    string displayString;
    
    
    //MIDI
    ofxMidiOut midiOut;
    int channel;
    
    unsigned int currentPgm;
    int note, velocity;
    int pan, bend, touch, polytouch;
    
    //SoundPlayer
    ofSoundPlayer  synth;
    ofSoundPlayer  bDrum;
    ofSoundPlayer  sDrum;
    
    ofSoundPlayer metronome1;
    ofSoundPlayer metronome2;
    
    
    
    //GUI
    ofxPanel gui;
    ofxIntSlider bpmSlider;
    ofxFloatSlider metronomeVol;
    ofxToggle metronomeOn;
    ofxToggle outputMidi;
    
    ofxButton emptySequencerButton;
    
    ofxButton sequencerStart;
    ofxButton sequencerStop;
    
    bool bHide;
    
    
    
    
};
