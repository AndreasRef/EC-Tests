/*
Master sketch for Eye Conductor combining everything :D
*/

#include "ofApp.h"


//--------------------------------------------------------------
void ofApp::setup(){
    
    ofSetFrameRate(60);
    ofSetVerticalSync(true);
    
    largeFont.load("verdana.ttf", 12, true, true);
    largeFont.setLineHeight(14.0f);
    smallFont.load("verdana.ttf", 10, true, true);
    smallFont.setLineHeight(12.0f);
    hugeFont.load("verdana.ttf", 36, true, true);
    hugeFont.setLineHeight(38.0f);
    
    //Initialize the training and info variables
    infoText = "";
    trainingModeActive = false;
    recordTrainingData = false;
    predictionModeActive = false;
    drawInfo = true;
    
    //Select the inputs: Gestures, orientations or raw inputs
    rawBool = false;
    gestureBool = true;
    orientationBool = false;
    
    trainingInputs = GESTUREINPUTS*gestureBool+ORIENTATIONINPUTS*orientationBool+RAWINPUTS*rawBool; //RAWINPUTS=136, ORIENTATIONINPUTS=9, GESTUREINPUTS=5
    
    trainingData.setInputAndTargetDimensions( trainingInputs, 2 );
    
    
    //set the default classifier
    setRegressifier( LINEAR_REGRESSION );
    
    
    
    //Facetracker
    
    //Select the inputs: Gestures, orientations or raw inputs
    rawBool = false;
    gestureBool = true;
    orientationBool = false;
    
    //What to draw
    drawNumbers = false;
    drawFace = true;
    drawPose = false;
    drawVideo = true;
    
    
    // Setup grabber
    grabber.setup(1280,720);
    //grabber.setup(960,540);
    
    // Setup tracker
    tracker.setup("../../../model/shape_predictor_68_face_landmarks.dat");
    
    
    //GUI
    gui.setup();
    gui.setPosition(ofGetWidth()-200, 4);
//    gui.setSize(300, 75);
//    gui.setDefaultWidth(300);
//    gui.setDefaultHeight(75);
    gui.add(val1.setup("output value 1", 0.7, 0.0, 1.0));
    gui.add(val2.setup("output value 2", 0.7, 0.0, 1.0));
    gui.add(smoothing.setup("smoothing", 0.85, 0.0, 1.0));
    
}

//--------------------------------------------------------------
void ofApp::update(){
    
    
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
        if( trainingModeActive){
            
            //Check to see if the countdown timer has elapsed, if so then start the recording
            if( !recordTrainingData ){
                if( trainingTimer.timerReached() ){
                    recordTrainingData = true;
                    trainingTimer.start( RECORDING_TIME );
                }
            }else{
                //We should be recording the training data - check to see if we should stop the recording
                if( trainingTimer.timerReached() ){
                    trainingModeActive = false;
                    recordTrainingData = false;
                }
            }
            
            if( recordTrainingData ){
                
                

                
                VectorFloat targetVector(2);
                targetVector[0] = val1;
                targetVector[1] = val2;

                
                if( !trainingData.addSample(trainingSample,targetVector) ){
                    infoText = "WARNING: Failed to add training sample to training data!";
                }
            }
        }
        
        inputVector = trainingSample;
        
        //Update the prediction mode if active
        if( predictionModeActive ){
            
            if( pipeline.predict( inputVector ) ){
                rawVal1 = ofClamp(pipeline.getRegressionData()[0],0.0, 1.0);
                rawVal2 = ofClamp(pipeline.getRegressionData()[1],0.0, 1.0);

            }else{
                infoText = "ERROR: Failed to run prediction!";
            }
        }
    }
    
    if (predictionModeActive) {
        val1 = val1 + ( rawVal1 - val1 ) * (1- smoothing);
        val2 = val2 + ( rawVal2 - val2 ) * (1- smoothing);
    }
    
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    
    ofBackground(225, 225, 225);
    
    
    
    //FaceTracker draw
    ofSetColor(255);
    // Draw camera image
    if (drawVideo) grabber.draw(0, 0); //Default
    
    // Draw estimated 3d pose
    if (drawPose) tracker.drawDebugPose();
    
    //Draw all the individual points / numbers for all tracked faces
    for (int i = 0; i<tracker.size(); i ++) {
        
        if(drawFace) {
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
        }
        
        for (int j = 0; j< tracker.getInstances()[i].getLandmarks().getImagePoints().size(); j++) {
            if (drawNumbers) ofDrawBitmapString(j, tracker.getInstances()[i].getLandmarks().getImagePoint(j));
        }
    }
    

    if( trainingModeActive ){
        if( !recordTrainingData ){
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
        ofDrawRectangle( 5, 5, 265, 620 );
        ofSetColor( 255, 255, 255 );

        smallFont.drawString( "FACETRACKER2 REGRESSION EXAMPLE", textX, textY ); textY += textSpacer*2;
        smallFont.drawString( "[i]: Toogle Info", textX, textY ); textY += textSpacer;
        
        smallFont.drawString( "[r]: Record Sample", textX, textY ); textY += textSpacer;
        smallFont.drawString( "[l]: Load Model", textX, textY ); textY += textSpacer;
        smallFont.drawString( "[s]: Save Model", textX, textY ); textY += textSpacer;
        smallFont.drawString( "[t]: Train Model", textX, textY ); textY += textSpacer;
        smallFont.drawString( "[d]: Pause Model", textX, textY ); textY += textSpacer;
        smallFont.drawString( "[c]: Clear Training Data", textX, textY ); textY += textSpacer;
        textY += textSpacer;
        
        
        smallFont.drawString( "Classifier: " + regressifierTypeToString( regressifierType ), textX, textY ); textY += textSpacer;
        smallFont.drawString( "[tab]: Select Regressifier", textX, textY ); textY += textSpacer;
        
        smallFont.drawString( "Recording: " + ofToString( recordTrainingData ), textX, textY ); textY += textSpacer;
        smallFont.drawString( "Num Samples: " + ofToString( trainingData.getNumSamples() ), textX, textY ); textY += textSpacer;
        smallFont.drawString( "Prediction mode active: " + ofToString( predictionModeActive), textX, textY ); textY += textSpacer;
        textY += textSpacer;
        
        ofSetColor(255,241,0);
        smallFont.drawString( infoText, textX, textY ); textY += textSpacer;
        ofSetColor(255);
        
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
        
 
        
        smallFont.drawString( "Framerate : "+ofToString(ofGetFrameRate()), textX, textY ); textY += textSpacer;
        smallFont.drawString( "Tracker thread framerate : "+ofToString(tracker.getThreadFps()), textX, textY ); textY += textSpacer;
        textY += textSpacer;
        
        
        ofSetColor(0);
        smallFont.drawString( "INSTRUCTIONS regression: \n \n 1) Set the height and width of the output sliders 2) Press [r] to record some training samples containing your selected facial features (gestures/orientation/raw points) and output slider values. \n \n 3) Repeat step 1) and 2) with different output slider values and height and different facial expressions / head orientations \n \n 4) Move your face and see the changes in the output slider values based on your facial orientation and expression",  textX, 750 );
        
    }
    gui.draw();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    
    infoText = "";
    bool buildTexture = false;
    
    switch ( key) {
        case 'r':
            predictionModeActive = false;
            trainingModeActive = true;
            recordTrainingData = false;
            trainingTimer.start( PRE_RECORDING_COUNTDOWN_TIME );
            break;
        case 't':
            if( pipeline.train( trainingData ) ){
                infoText = "Pipeline Trained";
                predictionModeActive = true;
            }else infoText = "WARNING: Failed to train pipeline";
            break;
        case 's':
            if( trainingData.save( ofToDataPath("TrainingData.grt") ) ){
                infoText = "Training data saved to file";
            }else infoText = "WARNING: Failed to save training data to file";
            break;
        case 'l':
            if( trainingData.load( ofToDataPath("TrainingData.grt") ) ){
                infoText = "Training data loaded from file";
            }else infoText = "WARNING: Failed to load training data from file";
            break;
        
        case 'd':
            predictionModeActive = false;
            infoText = "Model paused";
            break;
            
        case 'c':
            trainingData.clear();
            infoText = "Training data cleared";
            break;
            
        case 'g': //Toogle gesture inputs and update length of trainingInputs
            gestureBool =! gestureBool;
            trainingInputs = GESTUREINPUTS*gestureBool+ORIENTATIONINPUTS*orientationBool+RAWINPUTS*rawBool; //GESTUREINPUTS=5, ORIENTATIONINPUTS=9, RAWINPUTS=136
            trainingData.setInputAndTargetDimensions( trainingInputs, 2 );
            break;
            
        case 'o': //Toogle orientation inputs and update length of trainingInputs
            orientationBool =! orientationBool;
            trainingInputs = GESTUREINPUTS*gestureBool+ORIENTATIONINPUTS*orientationBool+RAWINPUTS*rawBool; //GESTUREINPUTS=5, ORIENTATIONINPUTS=9, RAWINPUTS=136
            trainingData.setInputAndTargetDimensions( trainingInputs, 2 );
            break;
            
        case 'a': //Toogle raw inputs and update length of trainingInputs
            rawBool =! rawBool;
            trainingInputs = GESTUREINPUTS*gestureBool+ORIENTATIONINPUTS*orientationBool+RAWINPUTS*rawBool; //GESTUREINPUTS=5, ORIENTATIONINPUTS=9, RAWINPUTS=136
            trainingData.setInputAndTargetDimensions( trainingInputs, 2 );
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
        
        case OF_KEY_TAB:
            setRegressifier( ++this->regressifierType % NUM_REGRESSIFIERS );
            break;
            
        default:
            break;
    }
    
}

bool ofApp::setRegressifier( const int type ){
    
    LinearRegression linearRegression;
    LogisticRegression logisticRegression;
    MLP mlp;
    
    this->regressifierType = type;
    
    pipeline.clear();
    
    switch( regressifierType ){
        case LINEAR_REGRESSION:
            pipeline << MultidimensionalRegression(linearRegression,true);
            break;
        case LOGISTIC_REGRESSION:
            pipeline << MultidimensionalRegression(logisticRegression,true);
            break;
        case NEURAL_NET:
        {
            unsigned int numInputNeurons = trainingData.getNumInputDimensions();
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
            
            pipeline << MultidimensionalRegression(mlp,true);
        }
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