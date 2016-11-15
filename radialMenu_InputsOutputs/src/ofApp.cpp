/*To do
 
 
 //Fullscreen
 
 //Draw a basic layout of the different elements on paper before doing anything else...
 
 Use ofxFacetracker2 instead
 
 
 //Enable both Classification and Regression:
 See - SimpleRegressionAndRegressionDebug
 
 //Main input (x/y)
 Select the input (mouse, eyetracking, head position, head orientation)
 
 
 //Secondary input - preset
 Save selected presets that people can load up
 
 
 //Secondary input - REGRESSION
 Find some effect that could be controlled by a self trained regression model
 
 
 //Transpose everything up/down - CLASSIFICATION
 Transpose up and down using a self trained classifier
 
 //Sound output
 Output MIDI
 Output samples (OK)
 Output synth
 
 
 //Select more / fever notes in the radial layout

 Sequencer mode
 
*/

#include "ofApp.h"

using namespace ofxCv; //From ofxFacetracker empty example

//--------------------------------------------------------------
void ofApp::setup(){
    ofSetBackgroundColor(0);
    ofSetCircleResolution(100);
    
    
    //GUI
    gui.setup();
    gui.ofxBaseGui::setPosition(5, 5);
    gui.add(outputMidi.setup("output MIDI", false));
    gui.add(blinkTrigg.setup("blink trigger", false));
    gui.add(blinkThreshold.setup("blink threshold", 5.0,3.5,7.0));
    
    //MIDI
    ofSetLogLevel(OF_LOG_VERBOSE);

    midiOut.listPorts(); // via instance
    midiOut.openPort(0);
    channel = 1;
    currentPgm = 0;
    note = 0;
    velocity = 0;
    pan = 0;
    bend = 0;
    touch = 0;
    polytouch = 0;
    
    
    //SOUNDPLAYER
    synth.setMultiPlay(false);
    synth.load("sounds/synth.wav");
    synth.setVolume(0.99f);
    
    sDrum.setMultiPlay(false);
    sDrum.load("sounds/SD.wav");
    sDrum.setVolume(0.99f);
    
    bDrum.setMultiPlay(false);
    bDrum.load("sounds/BD.mp3");
    bDrum.setVolume(0.99f);

    
    //FACETRACKER
    cam.initGrabber(1280, 720);
    tracker.setup();
    
}

//--------------------------------------------------------------
void ofApp::update(){
    
     ofSoundUpdate();
    
    //Facetracker
    cam.update();
    if(cam.isFrameNew()) {
        tracker.update(toCv(cam));
    }
    
    //Set the controlPoint to be controlled by mousePosition
    controlPoint("mouse");
    
    blinkTimer++;
    
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofSetBackgroundColor(0);
    
    //Layout variables
    float diam =ofGetHeight()*0.9;
    int numOptions = 5;
    float innerCircle = ofGetHeight()/8;
    int selected = -1;
    
    //Declare the active arc
    ofPath activeArc = ofPath();
    activeArc.setCircleResolution(100);
    activeArc.setFillColor(ofColor(0,255,0));
    activeArc.setStrokeWidth(5);
    activeArc.setStrokeColor(ofColor(255));
    
    //Draw outer circle
    ofSetColor(122);
    ofDrawCircle(ofGetWidth()/2, ofGetHeight()/2, diam/2);
    
    //Variables for trigonomitry calculations
    float mouseTheta = atan2(control.y-ofGetHeight()/2, control.x-ofGetWidth()/2);
    float piTheta = mouseTheta>=0?mouseTheta:mouseTheta+TWO_PI;
    float op = numOptions/TWO_PI;
    
    //Run through all the options
    for (int i=0; i<numOptions; i++) {
        float s = i/op;
        float e = (i+1)/op;
        
        //If the control point is inside an arc, update selected
        if (piTheta>= s && piTheta <= e && ofDist(control.x, control.y, ofGetWidth()/2, ofGetHeight()/2) > innerCircle) {
            selected = i;
        }
        
        //Check if our control point is outside the arcs
        if (ofDist(control.x, control.y, ofGetWidth()/2, ofGetHeight()/2) < innerCircle || ofDist(control.x, control.y, ofGetWidth()/2, ofGetHeight()/2) > diam/2) {
            selected = -1;
        }
        
        //Declare the passive arcs
        ofPath passiveArcs = ofPath();
        passiveArcs.setCircleResolution(100);
        passiveArcs.setFillColor(ofColor(126));
        passiveArcs.setStrokeWidth(5);
        passiveArcs.setStrokeColor(ofColor(255));
        
        //Draw passive arcs
        passiveArcs.arc(ofGetWidth()/2,  ofGetHeight()/2,  diam/2, diam/2, 360/numOptions*i, 360/numOptions* i + 360/numOptions);
        passiveArcs.close();
        passiveArcs.draw();
        
        //Draw active arc
        if (selected == i && blinkTrigg && blinked) {
            activeArc.arc(ofGetWidth()/2,  ofGetHeight()/2,  diam/2, diam/2, 360/numOptions*selected, 360/numOptions* selected + 360/numOptions);
            activeArc.close();
            activeArc.draw();
        }
        
        //Draw text indicating the number/note of all the arcs
        ofSetColor(255);
        ofDrawBitmapString(ofToString(i), ofGetWidth()/2 + cos(s+ofDegToRad(360/(numOptions*2)))*diam/2.75, ofGetHeight()/2 + sin(s+ofDegToRad(360/(numOptions*2)))*diam/2.75);
    }
    
    
    //Draw inner circle
    ofSetColor(0);
    ofDrawCircle(ofGetWidth()/2, ofGetHeight()/2, innerCircle);
    
    //Draw selected text indicator
    ofSetColor(255);
    ofDrawBitmapString(ofToString(selected), ofGetWidth()/2-5, ofGetHeight()/2);
    
    //Draw control point
    ofSetColor(255);
    ofDrawCircle(control.x, control.y, 10);
    
    
    //Facetracker
    ofSetColor(255);
    //cam.draw(0, 0);
    ofSetLineWidth(2);
    tracker.draw();
    ofDrawBitmapString(ofToString((int) ofGetFrameRate()), 10, 420);
    
    float lOpen = tracker.getGesture(ofxFaceTracker::LEFT_EYE_OPENNESS);
    float rOpen = tracker.getGesture(ofxFaceTracker::LEFT_EYE_OPENNESS);
    float summedOpen = rOpen + lOpen;
    ofDrawBitmapString("LEFT_EYE_OPENNESS: " +  ofToString( lOpen), 10, 440);
    ofDrawBitmapString("RIGHT_EYE_OPENNESS: " +  ofToString( rOpen), 10, 460);
    ofDrawBitmapString("SUMMED_EYE_OPENNESS: " +  ofToString(summedOpen), 10, 490);
    
    if (summedOpen<blinkThreshold && summedOpen>0.1) {
        ofSetColor(0,255,0);
        ofDrawRectangle(10, 520, 150, 150);
        blinked = true;
        
        
        if (blinkTrigg && blinkTimer > 10) {
            
            if (selected % 2 == 0) {
                synth.play();
            } else if ( selected > 0) {
                sDrum.play();
            }
            blinkTimer = 0;
            
        } else {
            blinked = false;
        }
    }
    
    
    
    if(!bHide){
        gui.draw();
    }
    
}

//--------------------------------------------------------------
void ofApp::controlPoint(string inputSelector){
    
    //Let user can specify input device (mouse, eye tracker, head position, head orientation etc) to control the main control point.
    
    //Redo this as switch statement
    if (inputSelector == "mouse") {
    control.x =ofGetMouseX();
    control.y =ofGetMouseY();
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if (key=='h') {
    bHide = !bHide;
    }
}


