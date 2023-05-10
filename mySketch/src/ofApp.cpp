#include "ofApp.h"

//Author Millieon Hu 33689667
//--------------------------------------------
//Code reference

using namespace ofxCv;
using namespace cv;

void ofApp::debugCameraDevices()
{
    // create a collection holding information about all available camera sources
    vector< ofVideoDevice > devices = cameraInput.listDevices();
    
    // iterate through camera source collection and print info for each item
    for(int i = 0; i < devices.size(); i++) {
        cout <<
        "Device ID: " << devices[i].id <<
        " Device Name: " << devices[i].deviceName <<
        " Hardware Name: " << devices[i].hardwareName <<
        " Is Available: " << devices[i].bAvailable <<
        endl;
    }
}

//load the data of the iris position with time of the video file
void ofApp::loadFile()
{
    string fileName = "framePos_ready";

    // Get the filepath to save to the data directory
    string filePath = ofToDataPath(fileName);

    // If the data directory doesn't already exist, create it
    if (!ofDirectory::doesDirectoryExist("")) {
        ofDirectory::createDirectory("");
    }

    // Open text file and read data from it
    ifstream infile(filePath);

    if (!infile.is_open()) {
        std::cout << "Error: couldn't open file " << fileName << " for reading\n";
        return;
    }
        
    for(int i=0;i<2000;i++)
    { infile>>pos_ready[i];
        
       }
    
    infile.close();
    
}
void ofApp::setup(){
  
    
    loadFile();//load Frame and pos info
    contour.setMinAreaRadius(10);
    contour.setMaxAreaRadius(100);
    fixedPos=0;
    
    speed = 4.5;
    vidPlayer.load("eyeMovement.mp4");
    //vidPlayer.setLoopState(OF_LOOP_NONE);//don't loop
    vidPlayer.setLoopState(OF_LOOP_NORMAL);//the frame restart from 0 as the video restarts
    
    //vidPlayer.setSpeed(speed);
    vidPlayer.play();
    totalFrames=vidPlayer.getTotalNumFrames();
    color=(12,12,12);//set original color to be the iris color for tracking
    colorImg.allocate(320,240);
    

    VidWidth=vidPlayer.getWidth();
    VidHeight=vidPlayer.getHeight();
    
    originalInputImg.allocate(VidWidth, VidHeight);
    leftBound=500;
    rightBound=0;
    originalCentroid=0;
    
    //get the x range of the i
    for(int i=0;i<2000;i++)
    {
        if(pos_ready[i]<leftBound)
        {
            if(pos_ready[i]>0)
            {leftBound=pos_ready[i];}
        }
        
        if(pos_ready[i]>rightBound)
        {
            rightBound=pos_ready[i];
        }
    }

    //calculated result lb169,rb474
    
    //frames for clips
    frames[0]=171;
    frames[1]=834;
    frames[2]=964;
    frames[3]=1104;
    frames[4]=1218;
    frames[5]=1772;
    frames[6]=1875;
    frames[7]=1942;


    
    // initialize image properties
    imgWidth  = 640;
    imgHeight = 480;
    
    // initialize object detection properties
    detectionThreshold = 70;    // very high contrast
    detectedObjectMax  = 1;    // maximum of 10 detected object at a time
    contourMinArea     = 40;    // detect a wide range of different sized objects
    contourMaxArea     = (imgWidth * imgHeight) / 3;
    
    // initialize OpenCV image instances
    // (manual memory allocation required)
    originalCamInputImg.allocate(imgWidth, imgHeight);
    hsvImg.allocate(imgWidth, imgHeight);
    hueImg.allocate(imgWidth, imgHeight);
    saturationImg.allocate(imgWidth, imgHeight);
    valueImg.allocate(imgWidth, imgHeight);
    backgroundImg.allocate(imgWidth, imgHeight);
    bckgrndSatDiffImg.allocate(imgWidth, imgHeight);
    
    // initialize camera instance
    debugCameraDevices();   // print information about available camera sources
    cameraInput.setDeviceID(0
                            );     // 0 -> default if at least once camera is available (get device id of other camera sources from running debugCameraDevices() )
    // cameraInput.initGrabber(imgWidth, imgHeight, true); // OF version 0.8.4; enable bTexture flag for setting up a texture and displaying the video on screen
   // grabber.setGrabber(std::make_shared<ofxPS3EyeGrabber>());
    cameraInput.initGrabber(imgWidth, imgHeight); // OF version 0.9.0
    
    // initialize helper values
    labelPosDelta     = 14;
    blobOverlayRadius = 10;
   
    
}
//
void ofApp::FindNearestPos(int targetPos)
{
    nearestTargetPos=targetPos;
    int dist=500;
    
    for(int i=0;i<2000;i++)
    {
        int diff=abs(pos_ready[i]-targetPos);
        if(dist>diff)
        {
            dist=diff;
            nearestTargetPos=pos_ready[i];
        }
    }
    
   
}

void ofApp::EyesFocus(float objectPos)
{
    //objectPos is the fixedPos: average pos calculated in the first 50 frames
    int currentFrame=vidPlayer.getCurrentFrame();
    

    int targetPos=int(objectPos);
    
    
    vector<int> kPos;//to find the nearest neighbor
    for(int i=0;i<2000;i++)
    {
        
        if(pos_ready[i]==targetPos)
        {
            kPos.push_back(i);
        }
        
    }
    
    //didn't find matching pos
    if(kPos.size()==0)
    {
        for(int i=0;i<2000;i++)
        {
            if(pos_ready[i]==nearestTargetPos)
            {
                kPos.push_back(i);
            }
            
        }
    }
    
    int dist=500;
    int marker;
    
    for(int i=0;i<kPos.size();i++)
    {
       if(abs(kPos[i]-currentFrame)<dist)
       {   dist=abs(kPos[i]-currentFrame);
           marker=kPos[i];//which one is the nearest frame
       }
    }

   
    
    if(vidPlayer.getCurrentFrame()!=marker)
    {
        
        vidPlayer.play();
        

        if(vidPlayer.isPlaying()==false)
        {
            vidPlayer.setPaused(false);
        }
        found=false;
    }
    else{
        vidPlayer.setPaused(true);
        ofSetColor(255, 255, 255);
        ofDrawBitmapString("Target found", VidWidth/2, VidHeight/2);
       
        found=true;
        bufferSize=movingPos.size();


        
        
    }
    
}



//--------------------------------------------------------------
void ofApp::update(){
   

    // Ask the video player to update itself.
   
   
    vidPlayer.update();
    
    
    if (vidPlayer.isFrameNew()){ // If there is fresh data...
        colorImg.setFromPixels(vidPlayer.getPixels());
 
        contour.setTargetColor(color);
          contour.setThreshold(10);
          contour.findContours(vidPlayer);
        
        int n=contour.size();
        for(int i=0;i<n;i++)
        { ofVec2f centroid = toOf(contour.getCentroid(i));
             ofVec2f average = toOf(contour.getAverage(0));
            int currentFrame=vidPlayer.getCurrentFrame();
            
            pos[currentFrame]=centroid.x;
            


        }
        }
    
    if(vidPlayer.isPlaying()==false)
    {
       
    
            //save the data to the file
            // Get the fileName from the GUI parameter
            string fileName = "framePos";

            // Get the filepath to save to the data directory
            string filePath = ofToDataPath(fileName);

            // If the data directory doesn't already exist, create it
            if (!ofDirectory::doesDirectoryExist("")) {
                ofDirectory::createDirectory("");
            }

            // Open text file and write data to it
            ofstream outfile(filePath);

            if (!outfile.is_open()) {
                std::cout << "Error: couldn't open file " << fileName << " for writing\n";
                return;
            }
        for(int i=0;i<2000;i++)
        { outfile << pos[i] << "\n";
           }
        outfile.close();
        
    }
    
    // update (read) input from camera feed
    cameraInput.update();
    
    // check if a new frame from the camera source was received
    if (cameraInput.isFrameNew())
    {
        // read (new) pixels from camera input and write them to original input image instance
        //originalInputImg.setFromPixels(cameraInput.getPixels(), imgWidth, imgHeight);    // OF version 0.8.4
        originalCamInputImg.setFromPixels(cameraInput.getPixels());    // OF version 0.9.0
        
        // create HCV color space image based on original (RGB) received camera input image
        hsvImg = originalCamInputImg;
        hsvImg.convertRgbToHsv();
        
        // extract HSV color space channels into separate image instances
        hsvImg.convertToGrayscalePlanarImages(hueImg, saturationImg, valueImg);
        
        // take the absolute value of the difference between the registered background image and the updated saturation color channel image in order to determine the image parts that have changed
        bckgrndSatDiffImg.absDiff(backgroundImg, saturationImg);
        
        // increase the contrast of the image
        bckgrndSatDiffImg.threshold(detectionThreshold);
        
        // apply object detection via OpenCV contour finder class
        contourFinder.findContours(bckgrndSatDiffImg, contourMinArea, contourMaxArea, detectedObjectMax, false);
        
       

        EyesFocus(fixedPos);

    }


}

void ofApp::buffer()
{
    
    movingPos.push_back(CalculatedObjectPos);
        //fixedPos=movingPos[10];
        if(!found)
        {
            if(movingPos.size()>=100 &&movingPos.size()<1000)
            {
                ofSetColor(255, 255, 255);
                ofDrawBitmapString("Potential target in vision", VidWidth/2, VidHeight/2);
                ofDrawBitmapString("Stay still", VidWidth/2, 3*VidHeight/2);
                for(int i=0;i<100;i++)
                {fixedPos+=movingPos[i];}
                fixedPos/=100;
                
            }
            
        }
        else{
            //fixedPos=0;
            if(movingPos.size()>=100 &&movingPos.size()<1000)
                {
                    
                    ofSetColor(255, 255, 255);
                    ofDrawBitmapString("Potential target in vision", VidWidth/2, VidHeight/2);
                    ofDrawBitmapString("Stay still", VidWidth/2, 3*VidHeight/2);
                    for(int i=0;i<100;i++)
                    {fixedPos+=movingPos[i];}
                    fixedPos/=100;
                }
            
        }
}

//--------------------------------------------------------------
void ofApp::draw(){
    

  ofSetColor(255);
  vidPlayer.draw(0*VidWidth, 0*VidHeight,VidWidth*1,VidHeight*1);
  contour.draw();//contour of the Vid, row 0
    
    //originalCamInputImg.draw(0 * VidWidth, 1 * VidHeight,imgWidth,imgHeight); // draw row 1
    
   
  ofFill();
  ofSetColor(color);
//the rectangle to select color
  //ofDrawRectangle(VidWidth+10, 0, 128, 128);
    
    ofSetHexColor(0xffffffff);  // set color "white" in hexadecimal representation
    
    for (int i = 0; i < contourFinder.nBlobs; i++) {
        // access current blob
        contourFinder.blobs[i].draw(0 * imgWidth, 1 * imgHeight);   // draw current blob in bottom right image grid
        
        // extract RGB color from the center of the current blob based on original input image
        //
        
        // get pixel reference of original input image
        //ofPixels originalInputImagePxls = originalInputImg.getPixelsRef();    // OF version 0.8.4
        ofPixels originalInputImagePxls = originalCamInputImg.getPixels(); // OF version 0.9.0
        
        // get point reference to the center of the current detected blob
        ofPoint blobCenterPnt = contourFinder.blobs[i].centroid;
        
        // get color of pixel in the center of the detected blob
        //ofColor detectedBlobClr = originalInputImagePxls.getColor(blobCenterPnt.x, blobCenterPnt.y);          // OF version 0.9.0
        ofColor detectedBlobClr = originalInputImagePxls.getColor(blobCenterPnt.x / 3, blobCenterPnt.y / 3);    // OF version 0.9.6

        // apply detected color for drawing circle overlay
        ofSetColor(detectedBlobClr);
        ofFill();
        
        // draw circle overlay in bottom left image of the grid (ontop of a copy of the saturation image)
        // OF version 0.9.0
        ofDrawCircle(blobCenterPnt.x + 0 * imgWidth,
                     blobCenterPnt.y + 2 * imgHeight,
                     blobOverlayRadius);
        
       
        CalculatedObjectPos=(imgWidth-blobCenterPnt.x);
       
       
        CalculatedObjectPos = ofMap(CalculatedObjectPos, 0, imgWidth, leftBound, rightBound); // newx = 21.5 a value [21, 22).


        
        
        
           
       
        buffer();
        
        
        //motion tracking
    }
}


//--------------------------------------------------------------
void ofApp::keyPressed(int key){
//for installation performance
    if(key=='l')
    {
    
        int n=contour.size();
        for(int i=0;i<n;i++)
        { ofVec2f centroid = toOf(contour.getCentroid(i));

            std::cout<<"current frame:"<<vidPlayer.getCurrentFrame()<<"; pos:"<<centroid.x<<endl;

        }
        
    }
    //make the vid pause
    if(key=='p')
    {
        vidPlayer.setPaused(true);
    }
    //make the vid continue after it found the target
    if(key=='c')
    {   movingPos.clear();
        fixedPos=0;
        vidPlayer.setPaused(false);}
    //rewind the video
    if(key == 'r') {speed *= -1.0; vidPlayer.setSpeed(speed);}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
  color = vidPlayer.getPixels().getColor(x, y);
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    color = vidPlayer.getPixels().getColor(x, y);
    movingPos.clear();
    fixedPos=0;
    vidPlayer.setPaused(false);
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
