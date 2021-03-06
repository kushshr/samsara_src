
#include "ofApp.h"
#include <iostream>

using namespace ofxCv;
using namespace cv;
using namespace std;

// 1 to Enable velocity control in Image processing
int variableVelocityFlag = 0;
int indx = 0;
int indx2 = 0;
int flag = 0;
int n_Creator = 0;
int n_Preserver = 0;
int n_Destroyer = 0;
int n_Atoms = 0;
int startOfEnd = 0;


void ofApp::setup() {
    cam.setup(1280,720);
    blur.setup(1280,720,5,.2,10);
    
    //targetColor1 is red, targetColor2 is blue, targetColor3 is yellow - Similarly contourFinder1(or 2 or 3), threshold1(or 2 or 3)
    contourFinder1.setMinAreaRadius(10);
    contourFinder1.setMaxAreaRadius(200);
    
    contourFinder2.setMinAreaRadius(10);
    contourFinder2.setMaxAreaRadius(200);
    
    contourFinder3.setMinAreaRadius(10);
    contourFinder3.setMaxAreaRadius(200);
    
    targetColor1.set(0,0,255);
    targetColor2.set(200,250,0);
    targetColor3.set(255,0,0);
    
    contourFinder1.setTargetColor(targetColor1, TRACK_COLOR_HS);
    contourFinder2.setTargetColor(targetColor2, TRACK_COLOR_RGB);
    contourFinder2.setTargetColor(targetColor3, TRACK_COLOR_HSV);
    
    //loading background
    gifloader.load("images/Star_bw.gif");
    gifloader2.load("images/giphy4.gif");
    
    //Atoms code
    //Initializing one Atom of each type
    CAtom = new ofAtom*[MAXCreator];
    PAtom = new ofAtom*[MAXPreserver];
    DAtom = new ofAtom*[MAXDestroyer];
    
    //Initializing FxMatrix (Distances of each Creator from Effects = 0 before the pair exists)
    for(int i = 0; i < MAXCreator; i++)
        for(int j = 0; j < MAXPreserver; j++)
            FxMatrix[i][j] = 0;
    
    //Initializing LifeCreator - status of each Creator
    for(int i = 0; i < MAXCreator; i++)
        LifeCreator[i] = 0;
    
    //Initializing LifePreserver - status of each Preserver
    for(int i = 0; i < MAXPreserver; i++)
        LifePreserver[i] = 0;
    
    //Initializing LifeDestroyer - status
    for(int i = 0; i < MAXDestroyer; i++)
        LifeDestroyer[i] = 0;
    
    //Initializing dying Environment - Status to check if that Creator is currently dying and its pos,vel
    dyingEnvironment = new ofDyingAtom*[MAXCreator];
    dyingEnd = new ofDyingAtom*[MAXAtoms];
    
    for(int i = 0; i < MAXCreator; i++){
        dyingEnvironment[i] = new ofDyingAtom(0,0,0,0,0);
    }
    
    for(int j = 0; j < MAXAtoms; j++){
        dyingEnd[j] = new ofDyingAtom(0,0,0,0,0);
    }
    TheEnd = 0;
    //Initializing 3 virtual atoms for image processing
    virtualAtom = new ofAtom*[3];
    virtualAtom[0] = new ofAtom(3, 0, 0, 0 , RADIUS);
    virtualAtom[1] = new ofAtom(3, 1, 0, 0 , RADIUS);
    virtualAtom[2] = new ofAtom(3, 2, 0, 0 , RADIUS);
    
    
    // Setting Frame Rate for processing
    ofSetFrameRate(FRAMERATE);
    //timeBeginPeriod(1);   // https://forum.openframeworks.cc/t/emptyexample-running-at-50fps-in-release-mode/13731/7
    
    
    //OSC Code
    // open an outgoing connection to HOST:PORT
    sender.setup(HOST, PORT);
    
    // Setting OSC Addresses ---------------------------------------------------
    PowerOn.setAddress("/power");
    M3.setAddress("/collisionPreserver");
    M4.setAddress("/collisionDestroyer");
    
    // FxMatrix
    std::string msg;
    for(int i = 0; i < MAXCreator; i++){
        for(int j = 0; j < MAXPreserver; j++){
            msg.append("/FxMatrix/");
            msg.append(ofToString(i+1));
            msg.append("/");
            msg.append(ofToString(j+1));
            M5[i][j].setAddress(msg);
            msg = "";
        }
    }
    
    // LifeCreator
    msg = "";
    for(int i = 0; i < MAXCreator; i++){
        msg.append("/lifeCreator/");
        msg.append(ofToString(i+1));
        M1[i].setAddress(msg);
        msg = "";
    }
    
    // LifePreserver
    msg = "";
    for(int i = 0; i < MAXPreserver; i++){
        msg.append("/lifePreserver/");
        msg.append(ofToString(i+1));
        M2[i].setAddress(msg);
        msg = "";
    }
    // -------------------------------------------------------------------------------------
    
    gui.setup();
    gui.add(threshold1.set("Threshold1", 50, 0, 255));
    gui.add(threshold2.set("Threshold2", 90, 0, 255));
    gui.add(threshold3.set("Threshold3", 100, 0, 255));
}

void ofApp::update() {
    
    cam.update();
    if(cam.isFrameNew()) {
        contourFinder1.setTargetColor(targetColor1,TRACK_COLOR_HS);
        contourFinder1.setThreshold(threshold1);
        contourFinder1.findContours(cam);
        
        contourFinder2.setTargetColor(targetColor2,TRACK_COLOR_RGB);
        contourFinder2.setThreshold(threshold2);
        contourFinder2.findContours(cam);
        
        contourFinder3.setTargetColor(targetColor3,TRACK_COLOR_HSV);
        contourFinder3.setThreshold(threshold3);
        contourFinder3.findContours(cam);
    }
    
    //Finding and drawing the center of the contour for targetcolor1
    n1 = contourFinder1.size();
    max1 = 0.0;
    for(int i = 0; i < n1; i++) {
        double area1 = contourFinder1.getContourArea(i);
        if(area1 > max1 ){
            max1 = area1;
            indx1 = i;
            centroidmax1 = toOf(contourFinder1.getCenter(i));
            velocity1 = toOf(contourFinder1.getVelocity(i));
        }
    }
    
    //Finding and drawing the center of the contour for targetcolor2
    n2 = contourFinder2.size();
    max2 = 0.0;
    for(int i = 0; i < n2; i++) {
        double area2 = contourFinder2.getContourArea(i);
        if(area2 > max2 ){
            max2 = area2;
            indx2 = i;
            centroidmax2 = toOf(contourFinder2.getCenter(i));
            velocity2 = toOf(contourFinder2.getVelocity(i));
        }
    }
    
    //Finding and drawing the center of the contour for targetcolor3
    n3 = contourFinder3.size();
    max3 = 0.0;
    for(int i = 0; i < n3; i++) {
        double area3 = contourFinder3.getContourArea(i);
        if(area3 > max3 ){
            max3 = area3;
            indx3 = i;
            centroidmax3 = toOf(contourFinder3.getCenter(i));
            velocity3 = toOf(contourFinder3.getVelocity(i));
        }
    }
    
    
    //Creating Virtual Atoms at the detected centroids.
    virtualAtom[0]->assign(3,0,centroidmax1.x, centroidmax1.y,0,0, RADIUS);
    virtualAtom[1]->assign(3,1,centroidmax2.x, centroidmax2.y,0,0, RADIUS);
    virtualAtom[2]->assign(3,2,centroidmax3.x, centroidmax3.y,0,0, RADIUS);
    
    
    // Blurring for graphics
    blur.setScale(0.2);
    
    
    // Creating Atoms at every BEATRATE seconds --------------------------------------------------------------
    if((ofGetFrameNum()%(BEATRATE*FRAMERATE)==0) && TheEnd == 0 )
    {
        indexCreator = MAXCreator;
        for(int i = 0; i < MAXCreator; i++){
            if(LifeCreator[i] == 0){
                indexCreator = i;    // First Available Creator ID
                break;
            }
        }
        
        //Checking if there is space in the environment for creating a Creator Atom
        if( indexCreator < MAXCreator){
            
            CAtom[indexCreator] = new ofAtom(0,indexCreator,centroidmax1.x,centroidmax1.y,RADIUS);
            
            if(variableVelocityFlag)
                CAtom[indexCreator]->assignVelocity(velocity1.x,velocity1.y);
            
            LifeCreator[indexCreator] = 1;
            
            // Update n_Creator if it hasnt reached MAX even once.
            if(n_Creator < MAXCreator)
                n_Creator++;
        }
    }
    
    // Creating Preserver Atoms at every 2 seconds
    if(TheEnd == 0 && n_Preserver < MAXPreserver && (ofGetFrameNum() > (BEATRATE*FRAMERATE*MAXCreator) && ofGetFrameNum()%(BEATRATE*FRAMERATE) == 0))
    {
        PAtom[n_Preserver] = new ofAtom(1,n_Preserver,centroidmax2.x,centroidmax2.y,RADIUS);
        
        LifePreserver[n_Preserver] = 1;
        
        if(variableVelocityFlag)
            PAtom[n_Preserver]->assignVelocity(velocity2.x,velocity2.y);
        n_Preserver++;
    }
    
    // Creating Destroyer Atoms at every 2 seconds
    if(TheEnd == 0 && n_Destroyer < MAXDestroyer && (ofGetFrameNum() > ((BEATRATE*FRAMERATE)*(MAXCreator + MAXPreserver)) && ofGetFrameNum()%(BEATRATE*FRAMERATE) == 0))
    {
        DAtom[n_Destroyer] = new ofAtom(2,n_Destroyer,centroidmax3.x,centroidmax3.y,RADIUS);
        
        LifeDestroyer[n_Destroyer] = 1;
        
        if(variableVelocityFlag)
            DAtom[n_Destroyer]->assignVelocity(velocity3.x,velocity3.y);
        n_Destroyer++;
    }
    
    
    
    //Initializing Collision status matrix --------------------------------------------------------------
    for(int j = 0; j < MAXCreator; j++)
        for(int i = 0; i < MAXPreserver; i++){
            isCollidedPreserver[j][i] = 0;
        }
    
    for(int j = 0; j < MAXCreator; j++)
        for(int i = 0; i < MAXDestroyer; i++){
            isCollidedDestroyer[j][i] = 0;
        }
    
    for(int i = 0; i < MAXCreator; i++)
        isCollidedVirtual[i] = 0;
    
    //Initializing FxMatrix every update frame
    for(int i = 0; i < MAXCreator; i++)
        for(int j = 0; j < MAXPreserver; j++)
            FxMatrix[i][j] = 0;
    
    
    if(startOfEnd == 0){
        //For drawing background
        if (ofGetFrameNum() % 2)
        {
            indx++;
            if (indx > gifloader.pages.size()-1) indx = 0;
        }}
    else {
        //For drawing background
        if (ofGetFrameNum() % 2)
        {
            indx2++;
            if (indx2 > gifloader2.pages.size()-1) indx2 = 0;
        }
    }
    //Atoms Code ------------------------------------------------------------------------------
    
    
    for (int i = 0; i < n_Creator; i++){
        if(LifeCreator[i])
            CAtom[i]->update();
    }
    
    for (int i = 0; i < n_Preserver; i++){
        if(LifePreserver[i])
            PAtom[i]->update();
    }
    for (int i = 0; i < n_Destroyer; i++){
        if(LifeDestroyer[i])
            DAtom[i]->update();
    }
    
    //COLLISION CHECK --------------------------------------------------------------------------
    
    flag = 0;
    //Checking for Atoms Collisions
    //Creator
    for(int j = 0; j < n_Creator; j++) {
        for(int i = 0; i < n_Preserver; i++){
            
            if(LifeCreator[j] && LifePreserver[i]){
                isCollidedPreserver[j][i] = CAtom[j]->collide(PAtom[i]);
                
                if(isCollidedPreserver[j][i] == 1 && TheEnd == 1){
                    Destroy(CAtom[j]);
                    Destroy(PAtom[i]);
                }
            }
        }
        
        // Destruction Collisions
        for(int i = 0; i < n_Destroyer; i++){
            
            if(LifeCreator[j] && LifeDestroyer[i]){
                isCollidedDestroyer[j][i] = CAtom[j]->collide(DAtom[i]);
                if(isCollidedDestroyer[j][i] == 2){
                    Destroy(CAtom[j]);
                    
                    if(TheEnd == 1)
                        Destroy(DAtom[i]);
                }
            }
        }
        // Virtual Atom Collisions
        for(int i = 0; i < 3; i ++){
            if(LifeCreator[j]){
                isCollidedVirtual[j] = CAtom[j]->collide(virtualAtom[i]);
                if(isCollidedVirtual[j] == 3 && i == 2)
                    Destroy(CAtom[j]);
            }
        }
        
        for(int i = 0; i < n_Creator; i++){
            //TODO: Check for distance from other creator atoms
            
            if(LifeCreator[j] && LifeCreator[i])
                if(i!=j){
                    flag = CAtom[j]->collide(CAtom[i]);
                    if(TheEnd == 1 && flag == 1){
                        Destroy(CAtom[j]);
                        Destroy(CAtom[i]);
                    }
                }
        }
    }
    
    //Preserver
    for(int j = 0; j < n_Preserver; j++){
        for(int i = 0; i < n_Destroyer; i++){
            if(LifePreserver[j] && LifeDestroyer[i]){
                flag = PAtom[j]->collide(DAtom[i]);
                if(TheEnd == 1 && flag == 1){
                    Destroy(PAtom[j]);
                    Destroy(DAtom[i]);
                }
            }
        }
        for(int i = 0; i < n_Preserver; i++){
            if(LifePreserver[j] && LifePreserver[i])
                if(i!=j){
                    flag = PAtom[j]->collide(PAtom[i]);
                    if(TheEnd == 1 && flag == 1){
                        Destroy(PAtom[j]);
                        Destroy(PAtom[i]);
                    }
                }
        }
        // Virtual Atom Collisions
        for(int i = 0; i < 3; i ++){
            if(LifePreserver[j])
                PAtom[j]->collide(virtualAtom[i]);
        }
        
    }
    //Destroyer
    for(int j = 0; j < n_Destroyer; j++){
        for(int i = 0; i < n_Destroyer; i++){
            if(LifeDestroyer[j] && LifeDestroyer[i])
                if(i!=j){
                    flag = DAtom[j]->collide(DAtom[i]);
                    if(TheEnd == 1 && flag == 1){
                        Destroy(DAtom[j]);
                        Destroy(DAtom[i]);
                    }
                }
            
        }
        // Virtual Atom Collisions
        for(int i = 0; i < 3; i ++){
            if(LifeDestroyer[j])
                DAtom[j]->collide(virtualAtom[i]);
        }
        
    }
    //Kill the Last atom
    flag = 0;
    if(TheEnd == 1){
	for(int i = 0; i < MAXCreator; i++)
	    if(LifeCreator[i])
	        flag++;

	for(int i = 0; i < MAXPreserver; i++)
	    if(LifePreserver[i])
		flag++;

	for(int i = 0; i < MAXDestroyer; i++)
	    if(LifeDestroyer[i])
		flag++;

	if(flag == 1){
	    for(int i = 0; i < MAXCreator; i++)
		if(LifeCreator[i])
			Destroy(CAtom[i]);
	    for(int i = 0; i < MAXPreserver; i++)
		if(LifePreserver[i])
			Destroy(PAtom[i]);
	    for(int i = 0; i < MAXDestroyer; i++)
		if(LifeDestroyer[i])
			Destroy(DAtom[i]);
	
	}
    }
    
    
    // EFFECTS MATRIX
    
    for(int j = 0; j < n_Creator; j++){
        for(int i = 0; i < n_Preserver; i++){
            
            if(LifeCreator[j] && LifePreserver[i]  && TheEnd == 0)
                FxMatrix[j][i] = CAtom[j]->atomFxDist(PAtom[i]);
        }
    }
    
    
    // Debugging Display
    
     /*
     for(int i = 0; i < MAXCreator; i++)
	cout<<LifeCreator[i]<<" ";
	cout<<"C"<<endl;

     for(int i = 0; i < MAXPreserver; i++)
	cout<<LifePreserver[i]<<" ";
	cout<<"P"<<endl;

     for(int i = 0;i < MAXDestroyer; i++)
	cout<<LifeDestroyer[i]<<" ";
	cout<<"D"<<endl;
     */
     /*

     for(int j = 0; j < MAXCreator; j++){
     for(int i = 0; i < MAXPreserver; i++){
     cout<<FxMatrix[j][i]<<" ";
     }
     cout<< endl;
     }
     
     cout<<" Frame Khatm "<<endl;
     */
    
    //OSC codes ----------------------------------------------------------------------------------
    
    // First Msg on first frame of code
    if(ofGetFrameNum() == 1){
        PowerOn.addIntArg(1);
        sender.sendMessage(PowerOn,false);
    }
    
    // OSC for FxMatrix
    for(int i = 0; i < MAXCreator; i++){
        for(int j = 0; j < MAXPreserver; j++){
            M5[i][j].addIntArg(FxMatrix[i][j]);
            sender.sendMessage(M5[i][j],false);
            M5[i][j].clear();
        }
    }
    // OSC for LifeCreator
    for(int i = 0; i < MAXCreator; i++){
        M1[i].addIntArg(LifeCreator[i]);
        sender.sendMessage(M1[i],false);
        M1[i].clear();
    }
    // OSC for LifePreserver
    for(int i = 0; i < MAXPreserver; i++){
        M2[i].addIntArg(LifePreserver[i]);
        sender.sendMessage(M2[i],false);
        M2[i].clear();
    }
    
    // OSC for Collision Granular
    for(int j = 0; j < n_Creator; j++){
        for(int i = 0; i < n_Preserver; i++){
            if(isCollidedPreserver[j][i] != 0){
                
                M3.addIntArg(1);	// Sending a 1 for collision with preserver
                sender.sendMessage(M3,false);
            }
        }
        for(int i = 0; i < n_Destroyer; i++){
            if(isCollidedDestroyer[j][i] != 0){
                M4.addIntArg(1);	// Sending a 1 for collision with destruction
                sender.sendMessage(M4,false);
            }
        }
        if(isCollidedVirtual[j] != 0){
            M4.addIntArg(1);    // Sending a 1 for collision with destroying virtual atom
            sender.sendMessage(M4,false);
        }
        
    }
    
}

void ofApp::draw() {
    
    blur.begin();
    
    ofSetColor(255);
    ofBackground(0);
    
    //Uncomment the next line for viewing the camera input
    //cam.draw(0, 0);
    //cam.draw(cam.getWidth(),0,-cam.getWidth(),cam.getHeight());  //For laterally inverting cam video
    
    //For drawing background: the image size is 498*480
    //Comment the next line for viewing the camera input
    if(startOfEnd == 1){
        gifloader2.pages[indx2].draw(0,0,1280,720);}
    else gifloader.pages[indx].draw(391, 120);
    
    ofSetLineWidth(2);
    //contourFinder1.draw();
    //contourFinder2.draw();
    
    if(startOfEnd==1)
        ofNoFill();
    
    //Code to flip everything laterally, write all the drawing code between this and ofPopMatrix()
    ofPushMatrix(); // save the old coordinate system
    ofTranslate( ofGetWidth(), 0.0f); // move the origin to the bottom-left hand corner of the window
    ofScale(-1.0f, 1.0f); // flip the x axis horizontally
    
    
    ofSetColor(cyanPrint);
    //minAreaRect1.draw();
    ofDrawCircle(centroidmax1, 15);
    
    ofSetColor(yellowPrint);
    //minAreaRect2.draw();
    ofDrawCircle(centroidmax2, 15);
    
    ofSetColor(magentaPrint);
    //minAreaRect3.draw();
    ofDrawCircle(centroidmax3, 15);
    
    //Atoms Code
    //Loop through all existing atoms.
    for(int i = 0; i < n_Creator; i++){
        
        if(LifeCreator[i])
            CAtom[i]->draw();
    }
    for(int i = 0; i < n_Preserver; i++){
        if(LifePreserver[i])
            PAtom[i]->draw();
    }
    for(int i = 0; i < n_Destroyer; i++){
        if(LifeDestroyer[i])
            DAtom[i]->draw();
    }
    
    // DYING CREATOR ATOMS GRAPHICS
    if( TheEnd == 0){
        for(int i = 0; i < MAXCreator; i++){
            if(dyingEnvironment[i]->statusCheck() == 1){
                if(ofGetElapsedTimef() < DYINGTIME){
                    Granulate(dyingEnvironment[i]);
                    dyingEnvironment[i]->m_dyingTimeCounter++;
                }
                else{
                    dyingEnvironment[i]->statusAssign(0);
                    dyingEnvironment[i]->m_dyingTimeCounter = 0;
                }
            }
        }
    }
    // DYING GRAPHICS AT THE END
    
    if(TheEnd == 1){
        
        for(int i = 0; i < MAXAtoms; i++){
            if(dyingEnd[i]->statusCheck() == 1){
                if(ofGetElapsedTimef() < DYINGTIME){
                    
                    Granulate(dyingEnd[i]);
                    dyingEnd[i]->m_dyingTimeCounter++;
                }
                else{
                    dyingEnd[i]->statusAssign(0);
                    dyingEnd[i]->m_dyingTimeCounter = 0;
                }
            }
        }
    }
    
    ofPopMatrix(); // restore the previous coordinate system
    
    blur.end();
    blur.draw();
    
    gui.draw();
}

void ofApp::mousePressed(int x, int y, int button) {
    //targetColor = cam.getPixels().getColor(x, y);
}


void ofApp::keyPressed(int key){
    
    switch(key){
            
        case 'q':
        case 'Q':
            for(int i = 0; i < n_Creator; i++){
                if(LifeCreator[i])
                    delete [] CAtom[i];
            }
            for(int i = 0; i < MAXCreator; i++){
                delete [] dyingEnvironment[i];
            }
            for(int i = 0; i < n_Preserver; i++){
                if(LifePreserver[i])
                    delete [] PAtom[i];
            }
            for(int i = 0; i < n_Destroyer; i++){
                if(LifeDestroyer[i])
                    delete [] DAtom[i];
            }
            for(int i = 0; i < MAXAtoms; i++){
                delete [] dyingEnd[i];
            }
            //timeEndPeriod(1);
            ofExit();
            break;
        case ' ':
            TheEnd = 1;
            startOfEnd = 1;
            break;
            
    }
    
}



//------------------------------------------------------------------------
// DESTRUCTION CODE

void ofApp::Destroy(ofAtom* Atom){
    
    if(Atom->m_type == 0){
        LifeCreator[Atom->m_id] = 0;
        if(TheEnd == 0)
            dyingEnvironment[Atom->m_id]->assign(Atom->m_posX, Atom->m_posY, Atom->m_velocityX, Atom->m_velocityY, 1,0,0);
        
        if(TheEnd == 1)
            dyingEnd[Atom->m_id]->assign(Atom->m_posX,Atom->m_posY,Atom->m_velocityX,Atom->m_velocityY,1,0,0);
    }
    else if(Atom->m_type == 1){
        LifePreserver[Atom->m_id] = 0;
        dyingEnd[MAXCreator + Atom->m_id]->assign(Atom->m_posX, Atom->m_posY, Atom->m_velocityX, Atom->m_velocityY, 1, 0,1);
    }
    else if(Atom->m_type == 2){
        LifeDestroyer[Atom->m_id] = 0;
        dyingEnd[MAXCreator + MAXPreserver + Atom->m_id]->assign(Atom->m_posX, Atom->m_posY, Atom->m_velocityX, Atom->m_velocityY, 1,0,2);
    }
    // This sets a reset on the counter upto DYINGTIME	
    ofResetElapsedTimeCounter();
    
    // Free the memory assigned to Atom
    delete [] Atom;
    
}

void ofApp::Granulate(ofDyingAtom * GranulateAtom){
    
    
    int scale = GranulateAtom->m_dyingTimeCounter;
    int directionX = ofSign(GranulateAtom->m_velocityX);
    int directionY = ofSign(GranulateAtom->m_velocityY);
    int alpha = 5; // Rate at which the granules move away
    
    int opacity = 500 - (10 + (scale * 10));
    
    ofFill();
    if(GranulateAtom->m_type == 0)
        ofSetColor(225,255,255,opacity);
    else if(GranulateAtom->m_type == 1)
        ofSetColor(60,250,115,opacity);
    else
        ofSetColor(255,60,0,opacity);
    
    
    ofDrawCircle(GranulateAtom->m_posX, GranulateAtom->m_posY, RADIUS/3 );
    ofDrawCircle(GranulateAtom->m_posX + (alpha*scale*directionX), GranulateAtom->m_posY, RADIUS/3);
    ofDrawCircle(GranulateAtom->m_posX + (alpha*scale*directionX/2), GranulateAtom->m_posY, RADIUS/3);
    ofDrawCircle(GranulateAtom->m_posX, GranulateAtom->m_posY + (alpha*scale * directionY), RADIUS/3);
    ofDrawCircle(GranulateAtom->m_posX, GranulateAtom->m_posY + (alpha*scale * directionY/2), RADIUS/3);
    ofDrawCircle(GranulateAtom->m_posX + (alpha*scale*directionX), GranulateAtom->m_posY + (alpha*scale*directionY), RADIUS/3);
    ofDrawCircle(GranulateAtom->m_posX + (alpha*scale*directionX/2), GranulateAtom->m_posY + (alpha*scale*directionY/2), RADIUS/3);
    ofDrawCircle(GranulateAtom->m_posX + (alpha*scale*directionX*3/2), GranulateAtom->m_posY + (alpha*scale*directionY/2), RADIUS/3);	
    ofDrawCircle(GranulateAtom->m_posX + (alpha*scale*directionX/2), GranulateAtom->m_posY + (alpha*scale*directionY*3/2), RADIUS/3);
}
