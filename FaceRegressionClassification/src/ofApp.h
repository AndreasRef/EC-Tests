

#pragma once

#include "ofMain.h"
#include "ofxGrt.h"
#include "ofxFaceTracker2.h"
#include "ofxGui.h"
#include <stdio.h>

//State that we want to use the GRT namespace
using namespace GRT;

#define PRE_RECORDING_COUNTDOWN_TIME 500
#define RECORDING_TIME 2000

class ofApp : public ofBaseApp{
    
public:
    
    enum RegressifierType{ LINEAR_REGRESSION=0, LOGISTIC_REGRESSION, NEURAL_NET, NUM_REGRESSIFIERS };

    
    void setup();
    void update();
    void draw();
    
    void keyPressed  (int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    
    bool setClassifier( const int type );
    bool setRegressifier( const int type );
    

    //GENERAL
    
    ofTrueTypeFont largeFont;
    ofTrueTypeFont smallFont;
    ofTrueTypeFont hugeFont;
    bool drawInfo;

    //REGRESSION
    int regressifierType;
    Timer trainingTimer_R;
    
    RegressionData trainingData_R;      		//This will store our training data
    GestureRecognitionPipeline pipeline_R;        //This is a wrapper for our classifier and any pre/post processing modules
    bool recordTrainingData_R;                                //This is a flag that keeps track of when we should record training data
    bool trainingModeActive_R;
    bool predictionModeActive_R;
    
    
    
    GRT::VectorFloat targetVector;              //This will hold the current label for when we are training the classifier //DON'T KNOW IF IT IS NEEDED??
    
    string infoText_R;                            //This string will be used to draw some info messages to the main app window
    
    
    string regressifierTypeToString( const int type ){
        switch( type ){
            case LINEAR_REGRESSION:
                return "LINEAR_REGRESSION";
                break;
            case LOGISTIC_REGRESSION:
                return "LOGISTIC_REGRESSION";
                break;
            case NEURAL_NET:
                return "NEURAL_NET";
                break;
        }
        return "UNKOWN_CLASSIFIER";
    }
    
    
    //CLASSIFICATION
    
    bool predictionModeActive_C;
    ClassificationData trainingData_C;      		//This will store our training data
    GestureRecognitionPipeline pipeline_C;        //This is a wrapper for our classifier and any pre/post processing modules
    bool record_C;                                //This is a flag that keeps track of when we should record training data
    UINT trainingClassLabel_C;                    //This will hold the current label for when we are training the classifier
    string infoText_C;                            //This string will be used to draw some info messages to the main app window
    
    ofxGrtTimeseriesPlot mouseDataPlot_C;
    ofxGrtTimeseriesPlot predictionPlot_C;
    
    
        enum ClassifierType{ ADABOOST=0, DECISION_TREE, KKN, GAUSSIAN_MIXTURE_MODEL, NAIVE_BAYES, MINDIST, RANDOM_FOREST_10, RANDOM_FOREST_100, RANDOM_FOREST_200, SOFTMAX, SVM_LINEAR, SVM_RBF, NUM_CLASSIFIERS };
    int classifierType;
    
    string classifierTypeToString( const int type ){
        switch( type ){
            case ADABOOST:
                return "ADABOOST";
                break;
            case DECISION_TREE:
                return "DECISION_TREE";
                break;
            case KKN:
                return "KKN";
                break;
            case GAUSSIAN_MIXTURE_MODEL:
                return "GMM";
                break;
            case NAIVE_BAYES:
                return "NAIVE_BAYES";
                break;
            case MINDIST:
                return "MINDIST";
                break;
            case RANDOM_FOREST_10:
                return "RANDOM_FOREST_10";
                break;
            case RANDOM_FOREST_100:
                return "RANDOM_FOREST_100";
                break;
            case RANDOM_FOREST_200:
                return "RANDOM_FOREST_200";
                break;
            case SOFTMAX:
                return "SOFTMAX";
                break;
            case SVM_LINEAR:
                return "SVM_LINEAR";
                break;
            case SVM_RBF:
                return "SVM_RBF";
                break;
        }
        return "UNKOWN_CLASSIFIER";
    }
    
    
    //FaceTracker2 stuff
    
    enum inputSelector{ GESTUREINPUTS=5, ORIENTATIONINPUTS=9, RAWINPUTS=136 };
    
    int trainingInputs = 5;
    
    enum Gesture {
        MOUTH_WIDTH, MOUTH_HEIGHT,
        LEFT_EYEBROW_HEIGHT, RIGHT_EYEBROW_HEIGHT,
        LEFT_EYE_OPENNESS, RIGHT_EYE_OPENNESS,
        JAW_OPENNESS,
        NOSTRIL_FLARE
    };
    
    float getGesture (Gesture gesture);
    
    void updateControlPoint(int inputSelector, float smoothFactor=0.0);
    
    ofxFaceTracker2 tracker;
    ofVideoGrabber grabber;
    
    bool rawBool;
    bool gestureBool;
    bool orientationBool;
    
    bool drawNumbers;
    bool drawFace;
    bool drawPose;
    bool drawVideo;
    
    
    //Debug stuff
    int numOrientationValues = 0;
    int numRawValues = 0;
    
    //GUI
    ofxPanel gui;
    ofxFloatSlider val1;
    ofxFloatSlider val2;
    ofxFloatSlider smoothing;
    
    float rawVal1 = 0.5;
    float rawVal2 = 0.5;
    
    
};
