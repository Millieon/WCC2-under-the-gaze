#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "ofxKinect.h"
#include "ofxOpenCv.h"  // make functionalities of OpenCV addon available

class ofApp : public ofBaseApp{
    
private:
    void debugCameraDevices();  // helper method to print information about available camera sources to the console

public:
  void setup();
  void update();
  void draw();
  


  void keyPressed(int key);
  void keyReleased(int key);
  void mouseMoved(int x, int y );
  void mouseDragged(int x, int y, int button);
  void mousePressed(int x, int y, int button);
  void mouseReleased(int x, int y, int button);
  void mouseEntered(int x, int y);
  void mouseExited(int x, int y);
  void windowResized(int w, int h);
  void dragEvent(ofDragInfo dragInfo);
  void gotMessage(ofMessage msg);
    void loadFile();
    void EyesFocus(float objectPos);
    void FindNearestPos(int targetPos);
    void buffer();
    
    // image properties
    int imgWidth;
    int imgHeight;
    
    int VidWidth;
    int VidHeight;

  

  ofxCv::ContourFinder contour;//handling Vid color tracking
  ofColor color;//selet color from Vid
    ofVideoPlayer            vidPlayer;
    float speed; //video playspeed;

    ofxCvColorImage            colorImg,originalInputImg;
    float leftBound;//how left the iris can go
    float rightBound;//how right the iris can go
    float originalCentroid;
    int frames[8];//splited into 8 clips
    int totalFrames;
    int pos[5000];
    int pos_ready[5000];
    float fixedPos;
    vector<int> movingPos;
    
    // object detection properties
    int detectionThreshold;     // representing the contrast increase
    int detectedObjectMax;      // representing the maximum amount of detected objects
    int contourMinArea;         // presenting the minimum amount of adjacent pixels in order to detect an object
    int contourMaxArea;         // presenting the maximum amount of adjacent pixels in order to detect an object

    // image instances (managed by OpenCV)
    ofxCvColorImage originalCamInputImg;   // original image as received from camera source in RGB color space
    ofxCvColorImage hsvImg; // representing the original input image in HSV color space
    ofxCvGrayscaleImage hueImg;         // representing the hue channel of the HSV image
    ofxCvGrayscaleImage saturationImg;  // representing the saturation channel of the HSV image
    ofxCvGrayscaleImage valueImg;       // representing the value channel of the HSV image
    ofxCvGrayscaleImage backgroundImg;      // registred background image in order to assist object detection
    ofxCvGrayscaleImage bckgrndSatDiffImg;    // image instance representing the difference between the registered background image and the current saturation color channel image in order to run the object (contour) detection on

    // OpenCV contour finder instance for handling object detection
    ofxCvContourFinder contourFinder;

    // camera instance
    ofVideoGrabber cameraInput;
    
    // helper values
    int labelPosDelta;
    int blobOverlayRadius;
    
    float CalculatedObjectPos;
    int nearestTargetPos;
    bool found;
    int bufferSize;
        
};
