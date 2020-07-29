//
// Created by PRINCIPAL on 20/07/2020.
//

#ifndef APPCALIBRACION3_ARUCOSTATE_H
#define APPCALIBRACION3_ARUCOSTATE_H

#include <opencv2/core.hpp>
#include <opencv2/core/persistence.hpp>
#include <opencv2/aruco.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <vector>


using namespace std;
using namespace cv;



class ArucoState {

public:
   // Constructor and Destructor
   ArucoState(const char *internalPath, const char* df, const char* cf, int did);
   ~ArucoState();

   //Operations

   void initializeBoard(int markersX, int markersY, float markerLength, float markerSeparation);
   bool detect(Mat &image1, Mat &image2);
   bool acceptCurrentDetection();
   bool calibrate();
   void clean();


   // Getters and setter
   bool setParametersFromFile(const char* df);
   bool setCameraFromFileCamera(const char* df); // Airán
   void setdictionary(int did);
   void setmarkersXY(int x, int y);
   void setmarkerLength(float l);
   void setmarkerSeparation(float s);
   void setBoard();
   void setcalibrationFlags(int f);
   void setaspectRation(float r);
   void setrefindStrategy(bool b);
   void setcameraFilePath(const char*  cf);
   void setcameraMatrix (const Mat &m);
   void setcameraDistCoeffs(const Mat &d);
   void setcameraRTvectors(const vector<Mat> &rvec, const vector<Mat> &tvec);
   void setArucoPaths(const char *internalPath, const char* paramsFile, const char* cameraFile);

   bool fatalError;
   static const int MAXPATHLENGTH=2048;

    // Airán. variables para paso de parámetros a Java, los ponemos públicos.
    vector< Vec3d > tRvecs, tTvecs;
    int Id;

private:

   static bool saveCameraParams(const string &filename, Size imageSize, float aspectRatio, int flags,
                                const Mat &cameraMatrix, const Mat &distCoeffs, double totalAvgErr);

   Ptr<aruco::DetectorParameters> params;

   int dictionaryId;
   int markersX;
   int markersY;
   float markerLength;
   float markerSeparation;
   int calibrationFlags;
   float aspectRatio;
   bool refindStrategy;


   Ptr<aruco::Dictionary> dictionary;
   Ptr<aruco::GridBoard> gridboard;
   Ptr<aruco::Board> board;


   // Calibration Data
   string cameraFilePath;
   vector< vector<int > > c_allIds;
   vector <int> c_ids;
   vector< vector < vector <Point2f> > > c_allCorners;
   vector< vector< Point2f> > c_corners, c_rejected;
   Mat c_cameraMatrix;
   Mat c_distCoeffs;
   vector< Mat > c_rvecs, c_tvecs;
   Mat imageCopy;
   Size imgSize;
   char _internalPath[MAXPATHLENGTH];
   char _paramsPath[MAXPATHLENGTH];
   char _cameraPath[MAXPATHLENGTH];



};

#endif //APPCALIBRACION3_ARUCOSTATE_H