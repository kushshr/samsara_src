#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "ofxGui.h"
#include "ofxOsc.h"
#include "ofxGif.h"
#include "ofxBlur.h"

#include "ofAtom.h"

#define HOST "10.80.138.34"
#define PORT 9455

#define MAXCreator 5
#define MAXPreserver 5
#define MAXDestroyer 2
#define MAXAtoms 12
#define RADIUS 30
#define FRAMERATE 60
#define BEATRATE 2 
#define DYINGTIME 1


class ofApp : public ofBaseApp {
public:
    void setup();
    void update();
    void draw();
    void mousePressed(int x, int y, int button);
    void keyPressed(int key);
    
    ofxBlur blur;

    double max1;
    int indx1;
    ofVec2f centroidmax1;
    int n1;
    ofVec2f velocity1;
    
    double max2;
    int indx2;
    ofVec2f centroidmax2;
    int n2;
    ofVec2f velocity2;
    
    double max3;
    int indx3;
    ofVec2f centroidmax3;
    int n3;
    ofVec2f velocity3;
    
    //Atom Code
    // Initializing the Different atoms dynamically
    ofAtom **CAtom;
    ofAtom **PAtom;
    ofAtom **DAtom;

    //Initializing Virtual Atoms for Image Processing Gesture
    ofAtom **virtualAtom;

    //Position of each dying creator atom
    ofVec2f dyingCreatorPosition[MAXCreator];

    //Distances of the two creators from the effect ball. -> For MILESTONE.
    float FxMatrix[MAXCreator][MAXPreserver];

    //Collision Status of each Creator with all other Atoms    
    int isCollidedPreserver[MAXCreator][MAXPreserver];
    int isCollidedDestroyer[MAXCreator][MAXDestroyer];
   
    //Index to identify a Creator
    int indexCreator = 0;

    //Life of the Creators -> LifeCreator[i] = 1 (i exists in the system), = 0 (i not in the system)
    int LifeCreator[MAXCreator];

    //Dying State of the Creators -> DyingCreator[i] = 1 (i is currently dying)  
    int DyingCreator[MAXCreator];   

    //Destruction Function for the Atom that gets hit
    void Destroy(ofAtom* Atom);

 
    ofxGIF::fiGifLoader gifloader;
    ofVideoGrabber cam;
    ofxCv::ContourFinder contourFinder1;
    ofColor targetColor1;
    ofColor targetColor2;
    ofxCv::ContourFinder contourFinder2;
    ofColor targetColor3;
    ofxCv::ContourFinder contourFinder3;
    cv::Point2f center1;
    cv::Point2f center2;
    cv::Point2f center3;
    
    ofxPanel gui;
    ofParameter<float> threshold1;
    ofParameter<float> threshold2;
    ofParameter<float> threshold3;
    ofParameter<bool> trackHs;
    
    //OSC code
    ofTrueTypeFont font;
    ofxOscSender sender;
    ofBuffer imgAsBuffer;
    ofImage img;
};
