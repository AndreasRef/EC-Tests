/*
 
 //What is this?
 - A test of controlling playback of samples so they don't "POP"
 - Making the transition between samples elegant, and support a playback of longer/shorter duration of each sample played.
 
 //To do:
 - Test it with short samples
 - Test it with longer samples
 - Combine it with the radial layout
 
*/


#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    synth.resize(numSamples); //
    volumens.resize(numSamples);
    
    for (int i = 0; i< numSamples; i++) {
    synth[i].setMultiPlay(true); //Is this necessary?
    synth[i].load("../../../samples/strings/" + ofToString(i) + ".wav");
    volumens[i] = 0.99;
    synth[i].setVolume(volumens[i]);
    }
}

//--------------------------------------------------------------
void ofApp::update(){
    pos = round(ofMap(ofGetMouseX(), 0, ofGetWidth(),0,8));
    
  for (int i = 0; i< numSamples; i++) {
      if (pos != i && volumens[i] > 0) {
          volumens[i] -=0.02; //Turn down all samples but the selected one...
      }
      synth[i].setVolume(volumens[i]);
  }
}

//--------------------------------------------------------------
void ofApp::draw(){

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

    volumens[pos] = 0.99;
    synth[pos].play();
    
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
