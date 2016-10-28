#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    printNote(48, 0, minor);
    printNote(49, 0, major);
}

//--------------------------------------------------------------
void ofApp::printNote(int startingNote, int step, scale inputscale ){
    
    if (inputscale == major) {
        cout << notes[(startingNote + majorScale[step % sizeof(majorScale)]) % 12] << " " << (floor((startingNote +  majorScale[step]) / 12)-1) << " scale: major" << endl;
        
    } else if ( inputscale == minor) {
        cout << notes[(startingNote + minorScale[step % sizeof(minorScale)]) % 12] << " " << (floor((startingNote +  minorScale[step]) / 12)-1) << " scale: minor" << endl;
    }
}

//--------------------------------------------------------------
void ofApp::update(){

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
