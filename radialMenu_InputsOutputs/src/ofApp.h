#pragma once

#include "ofMain.h"

#include "ofxMidi.h"
#include "ofxGui.h"
#include "ofxCv.h"
#include "ofxFaceTracker.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
        void keyPressed(int key);
    
    void controlPoint(string inputSelector);
    
    //Global variables
    ofPoint control = ofPoint();
    
    
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
    
    
    
    //GUI
    ofxPanel gui;
    ofxFloatSlider blinkThreshold;
    ofxToggle blinkTrigg;
    ofxToggle outputMidi;
    bool bHide;
    
    
    
    //Facetracker
    ofVideoGrabber cam;
    ofxFaceTracker tracker;
    
    bool blinked = false;
    
    long blinkTimer = 0;
    

    
		
};
