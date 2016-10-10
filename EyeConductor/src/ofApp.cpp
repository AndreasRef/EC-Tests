/*
Main project for building Eye Conductor
 
 
 //Fullscreen
 
 //Draw a basic layout of the different elements on paper before doing anything else...
 
 
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
 
 Select more / fever notes in the radial layout
 Sequencer mode
 
 
*/

#include "ofApp.h"


//--------------------------------------------------------------
void ofApp::setup(){
    
    ofSetFrameRate(60);
    ofSetVerticalSync(true);
    ofSetCircleResolution(100);
    
    largeFont.load("verdana.ttf", 12, true, true);
    largeFont.setLineHeight(14.0f);
    smallFont.load("verdana.ttf", 9, true, true);
    smallFont.setLineHeight(10.0f);
    hugeFont.load("verdana.ttf", 36, true, true);
    hugeFont.setLineHeight(38.0f);
    
    //Initialize the training and info variables
    infoText_R = "";
    trainingModeActive_R = false;
    recordTrainingData_R = false;
    predictionModeActive_R = false;
    drawInfo = true;
    
    //Select the inputs: Gestures, orientations or raw inputs
    rawBool = false;
    gestureBool = true;
    orientationBool = false;
    
    trainingInputs = GESTUREINPUTS*gestureBool+ORIENTATIONINPUTS*orientationBool+RAWINPUTS*rawBool; //RAWINPUTS=136, ORIENTATIONINPUTS=9, GESTUREINPUTS=5
    trainingData_R.setInputAndTargetDimensions( trainingInputs, 2 );
    
    //set the default classifier
    setRegressifier( LINEAR_REGRESSION );
    
    //CLASSIFICATION
    //Initialize the training and info variables
    infoText_C = "";
    trainingClassLabel_C = 1;
    record_C = false;
    //drawInfo_C = true;
    
    trainingData_C.setNumDimensions( trainingInputs );
    
    //set the default classifier
    ANBC naiveBayes;
    naiveBayes.enableNullRejection( false );
    naiveBayes.setNullRejectionCoeff( 3 );
    pipeline_C.setClassifier( naiveBayes );
    
    
    //Facetracker
    //Select the inputs: Gestures, orientations or raw inputs
    rawBool = false;
    gestureBool = true;
    orientationBool = false;
    
    //What to draw
    drawNumbers = false;
    drawFace = true;
    drawPose = false;
    drawVideo = false;
    
    
    // Setup grabber
    grabber.setup(1280,720);
    //grabber.setup(960,540);
    
    // Setup tracker
    tracker.setup("../../../model/shape_predictor_68_face_landmarks.dat");
    
    
    //GUI
    gui.setup();
    gui.setPosition(ofGetWidth()-200, 110);
//    gui.setSize(300, 75);
//    gui.setDefaultWidth(300);
//    gui.setDefaultHeight(75);
    gui.add(val1.setup("output value 1", 0.7, 0.0, 1.0));
    gui.add(val2.setup("output value 2", 0.7, 0.0, 1.0));
    gui.add(smoothing.setup("smoothing", 0.85, 0.0, 1.0));
    
    
    
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
    synth.load("../../../samples/synth.wav");
    synth.setVolume(0.99f);
    
    sDrum.setMultiPlay(false);
    sDrum.load("../../../samples/SD.wav");
    sDrum.setVolume(0.99f);
    
    bDrum.setMultiPlay(false);
    bDrum.load("../../../samples/BD.mp3");
    bDrum.setVolume(0.99f);
    
}

//--------------------------------------------------------------
void ofApp::update(){
    
    ofSoundUpdate();
    
    //FaceTracker2
    grabber.update();
    
    // Update tracker when there are new frames
    if(grabber.isFrameNew()){
        tracker.update(grabber);
    }
    VectorFloat trainingSample(trainingInputs);
    VectorFloat inputVector(trainingInputs);
    
    if ( tracker.size()>0) {
        
        
        //RAW (136 values)
        if (rawBool) {
            
            //Send all raw points (68 facepoints with x+y = 136 values) (only 1 face)
            auto facePoints = tracker.getInstances()[0].getLandmarks().getImageFeature(ofxFaceTracker2Landmarks::ALL_FEATURES);
            
            for (int i = 0; i<facePoints.size(); i++) {
                ofPoint p = facePoints.getVertices()[i].getNormalized(); //only values from 0-1. Experiment with this, and try to send non-normalized as well
                
                trainingSample[i] = p.x;
                trainingSample[i + facePoints.size()] = p.y; //Not that elegant...
            }
        }
        
        
        //ORIENTATION (9 values)
        if (orientationBool) {
            for (int i = 0; i<=2; i++) {
                for (int j = 0; j<=2; j++) {
                    trainingSample[i+RAWINPUTS*rawBool] = tracker.getInstances()[0].getPoseMatrix().getRowAsVec3f(i)[j];
                }
            }
        }
        
        
        //GESTURES (5 values)
        if (gestureBool) {
            trainingSample[0+RAWINPUTS*rawBool+ORIENTATIONINPUTS*orientationBool] = getGesture(RIGHT_EYE_OPENNESS);
            trainingSample[1+RAWINPUTS*rawBool+ORIENTATIONINPUTS*orientationBool] = getGesture(LEFT_EYE_OPENNESS);
            trainingSample[2+RAWINPUTS*rawBool+ORIENTATIONINPUTS*orientationBool] = getGesture(RIGHT_EYEBROW_HEIGHT);
            trainingSample[3+RAWINPUTS*rawBool+ORIENTATIONINPUTS*orientationBool] = getGesture(LEFT_EYEBROW_HEIGHT);
            trainingSample[4+RAWINPUTS*rawBool+ORIENTATIONINPUTS*orientationBool] = getGesture(MOUTH_HEIGHT);

        }
        
        
        //Update the training mode if needed
        if( trainingModeActive_R){
            
            //Check to see if the countdown timer has elapsed, if so then start the recording
            if( !recordTrainingData_R ){
                if( trainingTimer_R.timerReached() ){
                    recordTrainingData_R = true;
                    trainingTimer_R.start( RECORDING_TIME );
                }
            }else{
                //We should be recording the training data - check to see if we should stop the recording
                if( trainingTimer_R.timerReached() ){
                    trainingModeActive_R = false;
                    recordTrainingData_R = false;
                }
            }
            
            if( recordTrainingData_R ){
 
                VectorFloat targetVector(2);
                targetVector[0] = val1;
                targetVector[1] = val2;

                
                if( !trainingData_R.addSample(trainingSample,targetVector) ){
                    infoText_R = "WARNING: Failed to add training sample to training data!";
                }
            }
        }
        
        inputVector = trainingSample;
        
        //Update the prediction mode if active
        if( predictionModeActive_R ){
            
            if( pipeline_R.predict( inputVector ) ){
                rawVal1 = ofClamp(pipeline_R.getRegressionData()[0],0.0, 1.0);
                rawVal2 = ofClamp(pipeline_R.getRegressionData()[1],0.0, 1.0);

            }else{
                infoText_R = "ERROR: Failed to run prediction!";
            }
        }
    }
    
    if (predictionModeActive_R) {
        val1 = val1 + ( rawVal1 - val1 ) * (1- smoothing);
        val2 = val2 + ( rawVal2 - val2 ) * (1- smoothing);
    }
    
    
    //If we are recording training data, then add the current sample to the training data set
    if( record_C ){
        trainingData_C.addSample( trainingClassLabel_C, trainingSample );
    }
    
    //If the pipeline has been trained, then run the prediction
    if( pipeline_C.getTrained() && predictionModeActive_C ){
        pipeline_C.predict( trainingSample );
        predictionPlot_C.update( pipeline_C.getClassLikelihoods() );
    }
    
    updateControlPoint(inputSelector, 0.9);
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    ofBackground(0);
    
    //FaceTracker draw
    ofSetColor(255);
    // Draw camera image
    
    
    if (drawVideo) {
        
        ofPushMatrix();
        ofScale(-1,1);
        ofTranslate(-ofGetWidth(), 0);
        grabber.draw(0, 0); //Default
        ofPopMatrix();
    
    }
    
    radialLayoutDraw();
    
    
    
    // Draw estimated 3d pose - //This is still not mirrored... pushMatrix, Scale and Translate does not work...
    if (drawPose) {
        tracker.drawDebugPose();
    }
    
    //Draw all the individual points / numbers for all tracked faces
    for (int i = 0; i<tracker.size(); i ++) {
        
        if(drawFace) {
            
            ofPushMatrix();
            ofScale(-1,1);
            ofTranslate(-ofGetWidth(), 0);
            
            ofPolyline jaw = tracker.getInstances()[i].getLandmarks().getImageFeature(ofxFaceTracker2Landmarks::JAW);
            ofPolyline left_eyebrow = tracker.getInstances()[i].getLandmarks().getImageFeature(ofxFaceTracker2Landmarks::LEFT_EYEBROW);
            ofPolyline right_eyebrow = tracker.getInstances()[i].getLandmarks().getImageFeature(ofxFaceTracker2Landmarks::RIGHT_EYEBROW);
            ofPolyline left_eye_top = tracker.getInstances()[i].getLandmarks().getImageFeature(ofxFaceTracker2Landmarks::LEFT_EYE_TOP);
            ofPolyline right_eye_top = tracker.getInstances()[i].getLandmarks().getImageFeature(ofxFaceTracker2Landmarks::RIGHT_EYE_TOP);
            ofPolyline left_eye = tracker.getInstances()[i].getLandmarks().getImageFeature(ofxFaceTracker2Landmarks::LEFT_EYE);
            ofPolyline right_eye = tracker.getInstances()[i].getLandmarks().getImageFeature(ofxFaceTracker2Landmarks::RIGHT_EYE);
            ofPolyline outher_mouth = tracker.getInstances()[i].getLandmarks().getImageFeature(ofxFaceTracker2Landmarks::OUTER_MOUTH);
            ofPolyline inner_mouth = tracker.getInstances()[i].getLandmarks().getImageFeature(ofxFaceTracker2Landmarks::INNER_MOUTH);
            ofPolyline nose_bridge = tracker.getInstances()[i].getLandmarks().getImageFeature(ofxFaceTracker2Landmarks::NOSE_BRIDGE);
            ofPolyline nose_base = tracker.getInstances()[i].getLandmarks().getImageFeature(ofxFaceTracker2Landmarks::NOSE_BASE);
            
            jaw.draw();
            left_eyebrow.draw();
            right_eyebrow.draw();
            left_eye_top.draw();
            right_eye_top.draw();
            left_eye.draw();
            right_eye.draw();
            outher_mouth.draw();
            inner_mouth.draw();
            nose_bridge.draw();
            nose_base.draw();
            
            ofPopMatrix();
        }
        
        for (int j = 0; j< tracker.getInstances()[i].getLandmarks().getImagePoints().size(); j++) {
            if (drawNumbers) {
                ofPushMatrix();
                ofScale(-1,1);
                ofTranslate(-ofGetWidth(), 0);
                ofDrawBitmapString(j, tracker.getInstances()[i].getLandmarks().getImagePoint(j));
                ofPopMatrix();
            }
        }
    }
    

    if( trainingModeActive_R ){
        if( !recordTrainingData_R ){
            ofSetColor(255, 204, 0);
            string txt = "PREP";
            ofRectangle bounds = hugeFont.getStringBoundingBox(txt,0,0);
            hugeFont.drawString(txt,ofGetWidth()-25-bounds.width,ofGetHeight()-25-bounds.height);
        }else{
            ofSetColor(255,0,0);
            string txt = "REC";
            ofRectangle bounds = hugeFont.getStringBoundingBox(txt,0,0);
            hugeFont.drawString(txt,ofGetWidth()-25-bounds.width,ofGetHeight()-25-bounds.height);
        }
    }
    
    
    
    
    
    
    
    //Draw the info text
    if( drawInfo ){
        float textX = 10;
        float textY = 25;
        float textSpacer = smallFont.getLineHeight() * 1.5;
        
        ofFill();
        ofSetColor(100,100,100);
        ofDrawRectangle( 5, 5, 290, 720 -8 );
        ofSetColor( 255, 255, 255 );

        largeFont.drawString( "EYE CONDUCTOR ML EXAMPLE", textX, textY ); textY += textSpacer;
        
        smallFont.drawString( "Framerate : "+ofToString(ofGetFrameRate()), textX, textY ); textY += textSpacer;
        smallFont.drawString( "Tracker thread framerate : "+ofToString(tracker.getThreadFps()), textX, textY ); textY += textSpacer;
        textY += textSpacer;
        
        largeFont.drawString( "GLOBAL CONTROLS", textX, textY ); textY += textSpacer;
        smallFont.drawString( "[i]: Toogle Info", textX, textY ); textY += textSpacer;
        smallFont.drawString( "[a] Input all raw points: "+ofToString(rawBool), textX, textY ); textY += textSpacer;
        smallFont.drawString( "[g] Input gestures: "+ofToString(gestureBool), textX, textY ); textY += textSpacer;
        smallFont.drawString( "[o] Input orientation: "+ofToString(orientationBool), textX, textY ); textY += textSpacer;
        smallFont.drawString( "Total input values: "+ofToString(trainingInputs), textX, textY ); textY += textSpacer;
        textY += textSpacer;
        
        smallFont.drawString( "[n] draw numbers: "+ofToString(drawNumbers), textX, textY ); textY += textSpacer;
        smallFont.drawString( "[f] draw face: "+ofToString(drawFace), textX, textY ); textY += textSpacer;
        smallFont.drawString( "[p] draw pose: "+ofToString(drawPose), textX, textY ); textY += textSpacer;
        smallFont.drawString( "[v] draw video: "+ofToString(drawVideo), textX, textY ); textY += textSpacer;
        textY += textSpacer;
        
        //REGRESSION
        largeFont.drawString( "REGRESSION CONTROLS", textX, textY ); textY += textSpacer;
        smallFont.drawString( "[r]: Record Sample", textX, textY ); textY += textSpacer;
        smallFont.drawString( "[l]: Load Model", textX, textY ); textY += textSpacer;
        smallFont.drawString( "[s]: Save Model", textX, textY ); textY += textSpacer;
        smallFont.drawString( "[t]: Train Model", textX, textY ); textY += textSpacer;
        smallFont.drawString( "[d]: Pause Model", textX, textY ); textY += textSpacer;
        smallFont.drawString( "[c]: Clear Training Data", textX, textY ); textY += textSpacer;
        smallFont.drawString( "[tab]: Select Regressifier", textX, textY ); textY += textSpacer;
        textY += textSpacer;
        
        smallFont.drawString( "Regressifier: " + regressifierTypeToString( regressifierType ), textX, textY ); textY += textSpacer;
        
        smallFont.drawString( "Recording: " + ofToString( recordTrainingData_R ), textX, textY ); textY += textSpacer;
        smallFont.drawString( "Num Samples: " + ofToString( trainingData_R.getNumSamples() ), textX, textY ); textY += textSpacer;
        smallFont.drawString( "Prediction mode active: " + ofToString( predictionModeActive_R), textX, textY ); textY += textSpacer;
        ofSetColor(255,241,0);
        smallFont.drawString( infoText_R, textX, textY ); textY += textSpacer;
        ofSetColor(255);
        textY += textSpacer;
        
        
        //CLASSIFICATION
        largeFont.drawString( "CLASSIFICATION CONTROLS", textX, textY ); textY += textSpacer;
        smallFont.drawString( "[1-9]: Set Class Label", textX, textY ); textY += textSpacer;
        smallFont.drawString( "[R]: Record Sample", textX, textY ); textY += textSpacer;
        smallFont.drawString( "[L]: Load Model", textX, textY ); textY += textSpacer;
        smallFont.drawString( "[S]: Save Model", textX, textY ); textY += textSpacer;
        smallFont.drawString( "[T]: Train Model", textX, textY ); textY += textSpacer;
        smallFont.drawString( "[D]: Pause Model", textX, textY ); textY += textSpacer;
        smallFont.drawString( "[C]: Clear Training Data", textX, textY ); textY += textSpacer;
        smallFont.drawString( "[enter/return]: Select Classifier", textX, textY ); textY += textSpacer*2;
        
        smallFont.drawString( "Classifier: " + classifierTypeToString(classifierType), textX, textY ); textY += textSpacer;
        smallFont.drawString( "Class Label: " + ofToString( trainingClassLabel_C ), textX, textY ); textY += textSpacer;
        smallFont.drawString( "Recording: " + ofToString( record_C ), textX, textY ); textY += textSpacer;
        smallFont.drawString( "Num Samples: " + ofToString( trainingData_C.getNumSamples() ), textX, textY ); textY += textSpacer;
        smallFont.drawString( "Prediction mode active: " + ofToString( predictionModeActive_C), textX, textY ); textY += textSpacer;
        ofSetColor(255,241,0);
        smallFont.drawString( infoText_C, textX, textY ); textY += textSpacer;
        ofSetColor(255);
        textY += textSpacer;
        
        ofSetColor(255);
        smallFont.drawString( "INSTRUCTIONS REGRESSION: \n \n 1) Set the height and width of the output sliders \n \n 2) Press [r] to record some training samples containing your selected facial features (gestures/orientation/raw points) and output slider values. \n \n 3) Repeat step 1) and 2) with different output slider values and height and different facial expressions / head orientations \n \n 4) Press [t] to train. Move your face and see the changes in the output slider values based on your facial orientation and expression \n \n \n \n 5) INSTRUCTIONS CLASSIFICATION: Like regression but using keys [1-9], [R] (as a toogle) and [T]",  textX, 750 );
        
    }
    
    
    //CLASSIFICATION
    int graphX = ofGetWidth() - 200 ;
    int graphY = 5;
    int graphW = 200;
    int graphH = 100;
    
    if( pipeline_C.getTrained() ){
        predictionPlot_C.draw( graphX, graphY, graphW, graphH ); graphY += graphH * 1.1;
    }
    
    ofSetColor(255);
    hugeFont.drawString(ofToString(pipeline_C.getPredictedClassLabel()), ofGetWidth()/2-10, ofGetHeight()/2+30);

    
    gui.draw();
}


//--------------------------------------------------------------
void ofApp::radialLayoutDraw() {
    
    //RADIAL LAYOUT
    //Layout variables
    float diam =ofGetHeight()*0.9;
    int numOptions = 5;
    float innerCircle = ofGetHeight()/8;
    
    
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
    float mouseTheta = atan2(smoothControl.y-ofGetHeight()/2, smoothControl.x-ofGetWidth()/2);
    float piTheta = mouseTheta>=0?mouseTheta:mouseTheta+TWO_PI;
    float op = numOptions/TWO_PI;
    
    //Run through all the options
    for (int i=0; i<numOptions; i++) {
        float s = i/op;
        float e = (i+1)/op;
        
        //If the control point is inside an arc, update selected
        if (piTheta>= s && piTheta <= e && ofDist(smoothControl.x, smoothControl.y, ofGetWidth()/2, ofGetHeight()/2) > innerCircle) {
            selected = i;
        }
        
        //Check if our control point is outside the arcs
        if (ofDist(smoothControl.x, smoothControl.y, ofGetWidth()/2, ofGetHeight()/2) < innerCircle || ofDist(smoothControl.x, smoothControl.y, ofGetWidth()/2, ofGetHeight()/2) > diam/2) {
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
    ofFill();
    ofDrawCircle(ofGetWidth()/2, ofGetHeight()/2, innerCircle);
    
    //Draw selected text indicator
    ofSetColor(255);
    ofDrawBitmapString("Selected note", ofGetWidth()/2-50, ofGetHeight()/2-50);
    ofDrawBitmapString(ofToString(selected), ofGetWidth()/2-5, ofGetHeight()/2-25);
    
    //Draw control point
    ofSetColor(255);
    ofDrawCircle(smoothControl.x, smoothControl.y, 10);
    
    
    
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    
    infoText_R = "";
    infoText_C = "";
    bool buildTexture = false;
    
    switch ( key) {
            
            //GENERAL
        case 'g': //Toogle gesture inputs and update length of trainingInputs
            gestureBool =! gestureBool;
            trainingInputs = GESTUREINPUTS*gestureBool+ORIENTATIONINPUTS*orientationBool+RAWINPUTS*rawBool; //GESTUREINPUTS=5, ORIENTATIONINPUTS=9, RAWINPUTS=136
            trainingData_R.setInputAndTargetDimensions( trainingInputs, 2 );
            break;
            
        case 'o': //Toogle orientation inputs and update length of trainingInputs
            orientationBool =! orientationBool;
            trainingInputs = GESTUREINPUTS*gestureBool+ORIENTATIONINPUTS*orientationBool+RAWINPUTS*rawBool; //GESTUREINPUTS=5, ORIENTATIONINPUTS=9, RAWINPUTS=136
            trainingData_R.setInputAndTargetDimensions( trainingInputs, 2 );
            break;
            
        case 'a': //Toogle raw inputs and update length of trainingInputs
            rawBool =! rawBool;
            trainingInputs = GESTUREINPUTS*gestureBool+ORIENTATIONINPUTS*orientationBool+RAWINPUTS*rawBool; //GESTUREINPUTS=5, ORIENTATIONINPUTS=9, RAWINPUTS=136
            trainingData_R.setInputAndTargetDimensions( trainingInputs, 2 );
            break;
            
        case 'i':
            drawInfo = !drawInfo;
            break;
            
        case 'n':
            drawNumbers = !drawNumbers;
            break;
            
        case 'f':
            drawFace = !drawFace;
            break;
            
        case 'p':
            drawPose = !drawPose;
            break;
            
        case 'v':
            drawVideo = !drawVideo;
            break;

            
            //REGRESSION
        case 'r':
            predictionModeActive_R = false;
            trainingModeActive_R = true;
            recordTrainingData_R = false;
            trainingTimer_R.start( PRE_RECORDING_COUNTDOWN_TIME );
            break;
        case 't':
            if( pipeline_R.train( trainingData_R ) ){
                infoText_R = "Pipeline Trained";
                predictionModeActive_R = true;
            }else infoText_R = "WARNING: Failed to train pipeline";
            break;
        case 's':
            if( trainingData_R.save( ofToDataPath("TrainingData_R.grt") ) ){
                infoText_R = "Training data saved to file";
            }else infoText_R = "WARNING: Failed to save training data to file";
            break;
        case 'l':
            if( trainingData_R.load( ofToDataPath("TrainingData_R.grt") ) ){
                infoText_R = "Training data loaded from file";
            }else infoText_R = "WARNING: Failed to load training data from file";
            break;
        
        case 'd':
            predictionModeActive_R =! predictionModeActive_R;
            infoText_R = "Model paused";
            break;
            
        case 'c':
            trainingData_R.clear();
            infoText_R = "Training data cleared";
            break;
            
            
        case OF_KEY_TAB:
            setRegressifier( ++this->regressifierType % NUM_REGRESSIFIERS );
            break;
            
            
        //CLASSIFICATION
            
        case OF_KEY_RETURN:
            setClassifier( ++this->classifierType % NUM_CLASSIFIERS );
            break;
            
        case '1':
            trainingClassLabel_C = 1;
            break;
        case '2':
            trainingClassLabel_C = 2;
            break;
        case '3':
            trainingClassLabel_C = 3;
            break;
        case '4':
            trainingClassLabel_C = 4;
            break;
        case '5':
            trainingClassLabel_C = 5;
            break;
        case '6':
            trainingClassLabel_C = 6;
            break;
        case '7':
            trainingClassLabel_C = 7;
            break;
        case '8':
            trainingClassLabel_C = 8;
            break;
        case '9':
            trainingClassLabel_C = 9;
            break;
        
        case 'R':
            
            record_C = !record_C;
            
            break;
        
        case 'T':
            if( pipeline_C.train( trainingData_C ) ){
                infoText_C = "Pipeline Trained";
                predictionPlot_C.setup( 500, pipeline_C.getNumClasses(), "prediction likelihoods" );
                predictionPlot_C.setDrawGrid( true );
                predictionPlot_C.setDrawInfoText( true );
                predictionPlot_C.setFont( smallFont );
                
                predictionModeActive_C = true;
            }else infoText_C = "WARNING: Failed to train pipeline";
            break;
        
        case 'L':
            if( trainingData_C.load( ofToDataPath("TrainingData_C.grt") ) ){
                infoText_C = "Training data loaded from file";
            }else infoText_C = "WARNING: Failed to load training data from file";
            break;

        case 'S':
            if( trainingData_C.save( ofToDataPath("TrainingData_C.grt") ) ){
                infoText_C = "Training data saved to file";
            }else infoText_C = "WARNING: Failed to save training data to file";
            break;
        
        case 'C':
            trainingData_C.clear();
            infoText_C = "Training data cleared";
            break;
            
        case 'D':
            predictionModeActive_C =! predictionModeActive_C;
            infoText_C = "Model paused";
            break;
        
        case 'H':
            inputSelector = MOUSE;
            break;
            
        case 'J':
            inputSelector = HEAD_ORIENTATION;
            break;
            
        case 'K':
            inputSelector = HEAD_POSITION;
            break;

        default:
            break;
    }
    
}

//--------------------------------------------------------------
//void ofApp::updateControlPoint(const int type){
//    
//    //Let user can specify input device (MOUSE=0, HEAD_ORIENTATION, HEAD_POSITION, EYE_TRACKER etc) to control the main control point.
//    
//    switch( type ){
//        case MOUSE:
//            control.x =ofGetMouseX();
//            control.y =ofGetMouseY();
//            break;
//        case HEAD_ORIENTATION:
//            control.x =ofGetMouseX();
//            control.y =ofGetMouseY();
//            break;
//        case HEAD_POSITION:
//            control.x =ofGetMouseX();
//            control.y =ofGetMouseY();
//            break;
//        case EYE_TRACKER: //TO DO
//            //control.x =ofGetMouseX();
//            //control.y =ofGetMouseY();
//            break;
//    }
//    
//    
//
//}

//--------------------------------------------------------------
void ofApp::updateControlPoint(int inputSelector, float smoothFactor){
    
    //CLEAN UP THIS FUNCTION!!!
    
    //Control the input
    switch (inputSelector) {
        case 0:
            rawControl = ofPoint(ofGetMouseX(),ofGetMouseY());
            break;
        case 1:
            if (tracker.size() > 0) {
                ofPoint poseOrientation = ofPoint(tracker.getInstances()[0].getPoseMatrix().getRowAsVec3f(2)[0],tracker.getInstances()[0].getPoseMatrix().getRowAsVec3f(2)[1]);
                rawControl.x = ofMap(poseOrientation.x, -0.4, 0.4, 0, ofGetWidth());
                rawControl.y = ofMap(poseOrientation.y, -0.25, 0.25, 0, ofGetHeight());
            }
            
            break;
        case 2:
            
            //Track point 27 (just above the nose) and map that point...
            if (tracker.size() > 0) {
                //To do: Make a function so you can (re)set the center of the head in an intelligent (dynamic) way
                ofPoint centerFacePos = tracker.getInstances()[0].getLandmarks().getImagePoint(27);
                rawControl.x = ofMap(centerFacePos.x, 0.65*ofGetWidth(), 0.35*ofGetWidth(), 0, ofGetWidth());
                rawControl.y = ofMap(centerFacePos.y, 0.35*ofGetHeight(), 0.65*ofGetHeight(), 0, ofGetHeight());
            }
            
            break;
        case 3:
            //To do: EyeControl via OSC
            break;
        default:
            ;
    }
    //Perform smoothing
    smoothControl.x = smoothControl.x + (1-smoothFactor) * (rawControl.x-smoothControl.x);
    smoothControl.y = smoothControl.y + (1-smoothFactor) * (rawControl.y-smoothControl.y);
}




//--------------------------------------------------------------
bool ofApp::setRegressifier( const int type ){
    
    LinearRegression linearRegression;
    LogisticRegression logisticRegression;
    MLP mlp;
    
    this->regressifierType = type;
    
    pipeline_R.clear();
    
    switch( regressifierType ){
        case LINEAR_REGRESSION:
            pipeline_R << MultidimensionalRegression(linearRegression,true);
            break;
        case LOGISTIC_REGRESSION:
            pipeline_R << MultidimensionalRegression(logisticRegression,true);
            break;
        case NEURAL_NET:
        {
            unsigned int numInputNeurons = trainingData_R.getNumInputDimensions();
            unsigned int numHiddenNeurons = 10; 
            unsigned int numOutputNeurons = 1; //1 as we are using multidimensional regression
            
            //Initialize the MLP
            mlp.init(numInputNeurons, numHiddenNeurons, numOutputNeurons, Neuron::LINEAR, Neuron::SIGMOID, Neuron::SIGMOID );
            
            //Set the training settings
            mlp.setMaxNumEpochs( 1000 ); //This sets the maximum number of epochs (1 epoch is 1 complete iteration of the training data) that are allowed
            mlp.setMinChange( 1.0e-10 ); //This sets the minimum change allowed in training error between any two epochs
            mlp.setLearningRate( 0.001 ); //This sets the rate at which the learning algorithm updates the weights of the neural network
            mlp.setNumRandomTrainingIterations( 5 ); //This sets the number of times the MLP will be trained, each training iteration starts with new random values
            mlp.setUseValidationSet( true ); //This sets aside a small portiion of the training data to be used as a validation set to mitigate overfitting
            mlp.setValidationSetSize( 20 ); //Use 20% of the training data for validation during the training phase
            mlp.setRandomiseTrainingOrder( true ); //Randomize the order of the training data so that the training algorithm does not bias the training
            
            //The MLP generally works much better if the training and prediction data is first scaled to a common range (i.e. [0.0 1.0])
            mlp.enableScaling( true );
            
            pipeline_R << MultidimensionalRegression(mlp,true);
        }
            break;
        default:
            return false;
            break;
    }
    
    return true;
}




//--------------------------------------------------------------
bool ofApp::setClassifier( const int type ){
    
    int nullRejection = 0; //Change this if you want to enable nullrejection
    
    AdaBoost adaboost;
    DecisionTree dtree;
    KNN knn;
    GMM gmm;
    ANBC naiveBayes;
    MinDist minDist;
    RandomForests randomForest;
    Softmax softmax;
    SVM svm;
    
    this->classifierType = type;
    
    switch( classifierType ){
        case ADABOOST:
            adaboost.enableNullRejection( nullRejection ); // The GRT AdaBoost algorithm does not currently support null rejection, although this will be added at some point in the near future.
            adaboost.setNullRejectionCoeff( 3 );
            pipeline_C.setClassifier( adaboost );
            break;
        case DECISION_TREE:
            dtree.enableNullRejection( nullRejection );
            dtree.setNullRejectionCoeff( 3 );
            dtree.setMaxDepth( 10 );
            dtree.setMinNumSamplesPerNode( 3 );
            dtree.setRemoveFeaturesAtEachSpilt( false );
            pipeline_C.setClassifier( dtree );
            break;
        case KKN:
            knn.enableNullRejection( nullRejection );
            knn.setNullRejectionCoeff( 3 );
            pipeline_C.setClassifier( knn );
            break;
        case GAUSSIAN_MIXTURE_MODEL:
            gmm.enableNullRejection( nullRejection );
            gmm.setNullRejectionCoeff( 3 );
            pipeline_C.setClassifier( gmm );
            break;
        case NAIVE_BAYES:
            naiveBayes.enableNullRejection( nullRejection );
            naiveBayes.setNullRejectionCoeff( 3 );
            pipeline_C.setClassifier( naiveBayes );
            break;
        case MINDIST:
            minDist.enableNullRejection( nullRejection );
            minDist.setNullRejectionCoeff( 3 );
            pipeline_C.setClassifier( minDist );
            break;
        case RANDOM_FOREST_10:
            randomForest.enableNullRejection( nullRejection );
            randomForest.setNullRejectionCoeff( 3 );
            randomForest.setForestSize( 10 );
            randomForest.setNumRandomSplits( 2 );
            randomForest.setMaxDepth( 10 );
            randomForest.setMinNumSamplesPerNode( 3 );
            randomForest.setRemoveFeaturesAtEachSpilt( false );
            pipeline_C.setClassifier( randomForest );
            break;
        case RANDOM_FOREST_100:
            randomForest.enableNullRejection( nullRejection );
            randomForest.setNullRejectionCoeff( 3 );
            randomForest.setForestSize( 100 );
            randomForest.setNumRandomSplits( 2 );
            randomForest.setMaxDepth( 10 );
            randomForest.setMinNumSamplesPerNode( 3 );
            randomForest.setRemoveFeaturesAtEachSpilt( false );
            pipeline_C.setClassifier( randomForest );
            break;
        case RANDOM_FOREST_200:
            randomForest.enableNullRejection( nullRejection );
            randomForest.setNullRejectionCoeff( 3 );
            randomForest.setForestSize( 200 );
            randomForest.setNumRandomSplits( 2 );
            randomForest.setMaxDepth( 10 );
            randomForest.setMinNumSamplesPerNode( 3 );
            randomForest.setRemoveFeaturesAtEachSpilt( false );
            pipeline_C.setClassifier( randomForest );
            break;
        case SOFTMAX:
            softmax.enableNullRejection( false ); //Does not support null rejection
            softmax.setNullRejectionCoeff( 3 );
            pipeline_C.setClassifier( softmax );
            break;
        case SVM_LINEAR:
            svm.enableNullRejection( nullRejection );
            svm.setNullRejectionCoeff( 3 );
            pipeline_C.setClassifier( SVM(SVM::LINEAR_KERNEL) );
            break;
        case SVM_RBF:
            svm.enableNullRejection( nullRejection );
            svm.setNullRejectionCoeff( 3 );
            pipeline_C.setClassifier( SVM(SVM::RBF_KERNEL) );
            break;
        default:
            return false;
            break;
    }
    
    return true;
}

//--------------------------------------------------------------
float ofApp:: getGesture (Gesture gesture){
    
    //Current issues: How to make it scale accordingly?
    
    if(tracker.size()<1) {
        return 0;
    }
    int start = 0, end = 0;
    int gestureMultiplier = 10;
    
    
    switch(gesture) {
            // left to right of mouth
        case MOUTH_WIDTH: start = 48; end = 54; break;
            // top to bottom of inner mouth
        case MOUTH_HEIGHT: start = 51; end = 57; gestureMultiplier = 10; break;
            // center of the eye to middle of eyebrow
        case LEFT_EYEBROW_HEIGHT: start = 38; end = 20; gestureMultiplier = 10; break;
            // center of the eye to middle of eyebrow
        case RIGHT_EYEBROW_HEIGHT: start = 43; end = 23; gestureMultiplier = 10; break;
            // upper inner eye to lower outer eye
        case LEFT_EYE_OPENNESS: start = 38; end = 40; gestureMultiplier = 25; break;
            // upper inner eye to lower outer eye
        case RIGHT_EYE_OPENNESS: start = 43; end = 47; gestureMultiplier = 25; break;
            // nose center to chin center
        case JAW_OPENNESS: start = 33; end = 8; break;
            // left side of nose to right side of nose
        case NOSTRIL_FLARE: start = 31; end = 35; break;
    }
    
    
    //Normalized
    return (gestureMultiplier*abs(abs(tracker.getInstances()[0].getLandmarks().getImagePoint(start).getNormalized().y - tracker.getInstances()[0].getLandmarks().getImagePoint(end).getNormalized().y)));
    
    
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
    
    // scale the ascii values to midi velocity range 0-127
    // see an ascii table: http://www.asciitable.com/
    note = selected *2 + 48;
    velocity = 122;
    midiOut.sendNoteOn(channel, note,  velocity);
    
    
    // print out both the midi note and the frequency
    ofLogNotice() << "note: " << note
    << " freq: " << ofxMidi::mtof(note) << " Hz";
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    //midiOut.sendNoteOff(channel, note,  velocity);
    
    midiOut << NoteOff(channel, note, velocity); // stream interface
}

//--------------------------------------------------------------
void ofApp::exit() {
    // clean up
    midiOut.closePort();
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