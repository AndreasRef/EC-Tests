/*
 This extended example shows how to use the sequencer to:
 
 - playback MIDI
 - playback audio samples
 - integrate GUI
 
 
 MAJOR ISSUE:
 It does not feel "tight?!". I'm not sure it is working as it should
 
 
 //Attempt to read the timing from the soundcard as suggested here:
 https://forum.openframeworks.cc/t/music-sequencer-bpm-slightly-off/25067
 
 Still to do:

 -Make sure samples from ofSound don't pop...
 (See: http://openframeworks.cc/ofBook/chapters/sound.html )
 
 -General cleanup
 
 */

#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    
    //ofSetFrameRate(50);
    

    cols = 16;
    rows = 3;
    
    sequencer.setup(cols, 120, rows); //columns, bpm, rows
    sequencer.setBpm(120); //Somehow needed, otherwise it seems to run at half speed...
    
    p1.set("SNARE DRUM (f)", 0.2, 0, 1);
    p2.set("BASS DRUM 2 (f)", 0.2, 0, 1);
    p3.set("SYNTH 3 (f)", 0.2, 0, 1);
    
    
    sequencer.addRow(&p1);
    sequencer.addRow(&p2);
    sequencer.addRow(&p3);
    
    emptySequencer();
    
    sequencer.setSmooth(true); //Figure out what this does exactly?
    
    sequencer.start();
    
    sequencer.setPosition(0, 0, ofGetWidth(), ofGetHeight()); //x, y, w, h
    
    displayString = "No beats yet";
    ofAddListener(sequencer.beatEvent, this, &ofApp::beatsIn);
    
    
    //MIDI
    ofSetLogLevel(OF_LOG_VERBOSE);
    
    // print the available output ports to the console
    midiOut.listPorts(); // via instance
    //ofxMidiOut::listPorts(); // via static too
    
    // connect
    midiOut.openPort(0); // by number
    //midiOut.openPort("IAC Driver Pure Data In"); // by name
    //midiOut.openVirtualPort("ofxMidiOut"); // open a virtual port
    
    channel = 1;
    currentPgm = 0;
    note = 0;
    velocity = 0;
    pan = 0;
    bend = 0;
    touch = 0;
    polytouch = 0;
    
    //SoundPlayer
    synth.setMultiPlay(true);
    synth.load("sounds/synth.wav");
    synth.setVolume(0.75f);
    
    sDrum.setMultiPlay(true);
    sDrum.load("sounds/snare.wav");
    sDrum.setVolume(0.75f);
    
    bDrum.setMultiPlay(true);
    bDrum.load("sounds/kick.wav");
    bDrum.setVolume(0.75f);
    
    metronome1.setMultiPlay(false);
    metronome1.load("sounds/metronomes/cbClick2.wav");
    metronome1.setVolume(0.75f);
    metronome2.load("sounds/metronomes/cbClick1.wav");
    metronome2.setVolume(0.75f);
    
    
    //GUI
    gui.setup(); // most of the time you don't need a name
    gui.ofxBaseGui::setPosition(ofGetWidth()-225, 10);
    gui.add(metronomeOn.setup("metronome", false));
    gui.add(metronomeVol.setup("metronomeVol", 0.5,0.0,1.0));
    gui.add(outputMidi.setup("output MIDI", false));
    gui.add(bpmSlider.setup("bpm", 120,1,360));
    
    sequencerStart.addListener(this, &ofApp::startSequencer);
    gui.add(sequencerStart.setup("start sequencer"));
    sequencerStop.addListener(this, &ofApp::stopSequencer);
    gui.add(sequencerStop.setup("stop sequencer"));
    
    emptySequencerButton.addListener(this, &ofApp::emptySequencer);
    gui.add(emptySequencerButton.setup("clear sequencer"));
    
}

//--------------------------------------------------------------
void ofApp::beatsIn (int & eventInt){
    //Don't draw stuff in this function, it will mess up things...
    //Also it is not allowed to call stuff without excluding conditions
    //Or perhaps it is on setup...... I think it is...
    
//    cout << "we got an event!" << endl;
//    cout << "integer is:" <<endl;
//    cout << eventInt << endl;
//    
//    displayString = "last beat received: \n" + ofToString(eventInt);
    
    if (outputMidi == true) {
        if (p1 > 0 ) {
            note = 84;
            velocity = ofMap(p1, 0, 1, 0, 127);
            midiOut.sendNoteOn(channel, note,  velocity);
        } else {
            note = 84;
            velocity = 0;
            midiOut << NoteOff(channel, note, velocity);
        }
        
        if (p2 > 0 ) {
            note = 85;
            velocity = ofMap(p2, 0, 1, 0, 127);
            midiOut.sendNoteOn(channel, note,  velocity);
        } else {
            note = 85;
            velocity = 0;
            midiOut << NoteOff(channel, note, velocity);
        }
        
        if (p3 > 0 ) {
            note = 86;
            velocity = ofMap(p3, 0, 1, 0, 127);
            midiOut.sendNoteOn(channel, note,  velocity);
        } else {
            note = 86;
            velocity = 0;
            midiOut << NoteOff(channel, note, velocity);
        }
        
        
        
    } else if (outputMidi == false) {
        
        if (p1 > 0 ) {
            sDrum.setVolume(p1);
            sDrum.play();
        }
        
        if (p2 > 0 ) {
            bDrum.play();
            bDrum.setVolume(p2);
        }
        
        if (p3 > 0 ) {
            synth.play();
            synth.setVolume(p3);
        }
    }
    
    
    if (metronomeOn) {
        if (eventInt % 4 == 0) {
            metronome2.play();
        } else if ((ofGetFrameNum() > 1 && eventInt % 4 == 1) || eventInt % 4 == 2 || eventInt % 4 == 3) {
            metronome1.play(); //Hacky way of avoiding a crash on setup by not running if frameNumber == 0
        }
    }
    
}

//--------------------------------------------------------------
void ofApp::update(){
    //sequencer.update();
    
    // update the sound playing system:
    metronome1.setVolume(metronomeVol);
    metronome2.setVolume(metronomeVol);
    sequencer.setBpm(bpmSlider);
    
    ofSoundUpdate();
    
    //cout << ofGetFrameRate() << endl;
    
    
}

//--------------------------------------------------------------
void ofApp::draw(){
    sequencer.draw();
    
    
    //MIDI
    // let's see something -- OBS: Displayed informations is not correct when sequencer is playing (since it only outputs values for the last "p")
    
    if (outputMidi) {
    ofSetColor(0);
    stringstream text;
    text << "connected to port " << midiOut.getPort() << endl
    << midiOut.getName() << "\"" << endl
    << "is virtual?: " << midiOut.isVirtual() << endl << endl
    << "sending to channel " << channel << endl << endl
    << "current program: " << currentPgm << endl << endl
    << "note: " << note << endl
    << "velocity: " << velocity << endl
    << "pan: " << pan << endl
    << "bend: " << bend << endl
    << "touch: " << touch << endl
    << "polytouch: " << polytouch;
    ofDrawBitmapString(text.str(), 570, 20);
    }
        
    if(!bHide){
        gui.draw();
    }
    
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if (key==' ') {
        // set row 0, column 1 to random number
        sequencer.setValue<float>(0, 1, ofRandom(1));
        // set row 2, column 3 to random integer (since p3 is an int)
        sequencer.setValue<int>(2, 3, int(ofRandom(4)));
        // set row 3, column 3 to random integer (since p3 is an int)
        sequencer.setValue<Boolean>(3, 3, true); //Does not work...
    }
    
    
    else if (key=='r') {
        sequencer.randomize();
    }
    
    
    else if(key == 'h'){
        bHide = !bHide;
    }
    
    
    
    // send a note on if the key is a letter or a number
    else if(isalnum((unsigned char) key)) {
        
        // scale the ascii values to midi velocity range 0-127
        // see an ascii table: http://www.asciitable.com/
        note = ofMap(key, 48, 122, 0, 127);
        velocity = 64;
        midiOut.sendNoteOn(channel, note,  velocity);
        
        // print out both the midi note and the frequency
        ofLogNotice() << "note: " << note
        << " freq: " << ofxMidi::mtof(note) << " Hz";
    }
    
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
    
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
    
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){
    
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){
    
}

//--------------------------------------------------------------
void ofApp::exit() {
    
    // clean up
    velocity = 0;
    midiOut << NoteOff(channel, note, velocity);
    
    midiOut.closePort();
}



//--------------------------------------------------------------
void ofApp:: emptySequencer() {
    //Set all values to zero in the sequencer
    for (int i = 0; i<cols; i++) {
        for (int j = 0; j<rows; j++) {
            sequencer.setValue<float>(j, i, 0.0);
        }
    }
}

//--------------------------------------------------------------
void ofApp:: startSequencer() {
    cout << "Start";
    sequencer.start();
}

//--------------------------------------------------------------
void ofApp:: stopSequencer() {
    cout << "Stop";
    sequencer.stop();
}


