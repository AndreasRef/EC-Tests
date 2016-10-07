#include "ofApp.h"


//--------------------------------------------------------------
void ofApp::setup(){
    ofSetBackgroundColor(0);
    ofSetCircleResolution(100);
}

//--------------------------------------------------------------
void ofApp::update(){
    
    //Set the controlPoint to be controlled by mousePosition
    controlPoint("mouse");
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
        if (selected == i) {
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
}

//--------------------------------------------------------------
void ofApp::controlPoint(string inputSelector){
    
    //This is not very elegant yet, but the idea is that the user can specify a range of input devices (mouse, eye tracker, head position, head orientation etc) to control the main control point. 
    
    if (inputSelector == "mouse") {
    control.x =ofGetMouseX();
    control.y =ofGetMouseY();
    }
}


