//This sketch gets the gets the seconds count from the soundcard and shows how to do a more stable bpm count - see this discussion https://forum.openframeworks.cc/t/music-sequencer-bpm-slightly-off/25067

#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    ofSoundStreamSetup(2,0,this,44100,512,4);
    bpm.addListener(this, &ofApp::bpmChanged);
    gui.setup();
    gui.setPosition(ofGetWidth()-210, 10);
    gui.add(bpm.setup("bpm", 120, 30, 300));
}

//--------------------------------------------------------------
void ofApp::update(){
    currentTime = seconds;
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    ofBackground(255);
    //bpm = 121;
    
    if (floor(currentTime*bpm/60.0) >= lastTime) {
        lastTime = seconds*bpm/60.0;
        ofSetColor(255, 0, 0);
        ofDrawEllipse(100, 100, 100, 100);
        count +=1;
        cout<< "tick " << count <<endl;
    }
    //ofDrawBitmapString(bpm.get, 10, 20);
    
    gui.draw();
}

//--------------------------------------------------------------
void ofApp::bpmChanged(int &bpm){
    lastTime = 0;
}

//--------------------------------------------------------------
void ofApp::audioOut(ofSoundBuffer & buffer){
    
    //Get the seconds count from the soundcard
    seconds = buffer.getTickCount() * buffer.getNumFrames() / double(buffer.getSampleRate());
    
    //Set all sounds to zero
    float * samples = buffer.getBuffer().data();
    int max = buffer.getNumFrames() * buffer.getNumChannels();
    for (int n=0; n<max; ++n){
        samples[n] = 0.0f;
    }
    
    
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    
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
void ofApp::mouseEntered(int x, int y){
    
}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){
    
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