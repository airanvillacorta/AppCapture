//
// Created by PRINCIPAL on 20/07/2020.
//

#include <fstream>
#include "ArucoState.h"


// Constructor and Destructor

ArucoState::ArucoState(const char *internalPath, const char* paramsFile, const char* cameraFile, int did) {
   bool result=false, result2=false;
   markersX=1;
   markersY=1;
   markerLength=600;
   markerSeparation=50;
   calibrationFlags=0;
   aspectRatio=1;
   refindStrategy=true;
   fatalError=false;
   params=Ptr<aruco::DetectorParameters>(new aruco::DetectorParameters);
   setArucoPaths(internalPath,paramsFile,cameraFile);
   result=setParametersFromFile(this->_paramsPath);
   result2=setCameraFromFileCamera(this->_cameraPath); // Airán
   if(result){
       setdictionary(did);
       setcameraFilePath(this->_cameraPath); // Revisar setcameraFilePath()

   } else{
       fatalError=true;
   }


}

ArucoState::~ArucoState(){

   delete params;

}

void ArucoState::setArucoPaths(const char *internalPath, const char* paramsFile, const char* cameraFile){

   strncpy(this->_internalPath,internalPath,ArucoState::MAXPATHLENGTH);
   strcpy(this->_paramsPath,this->_internalPath);
   strcat(this->_paramsPath, paramsFile);
   strcpy(this->_cameraPath,this->_internalPath);
   strcat(this->_cameraPath, cameraFile);
}

// Operations

void ArucoState::initializeBoard(int markersX, int markersY, float markerLength, float markerSeparation){

   this->markersX=markersX;
   this->markersY=markersY;
   this->markerLength=markerLength;
   this->markerSeparation=markerSeparation;
   this->setBoard();
}

bool ArucoState::detect(Mat &imGray, Mat &imRGBa){
   bool result=false;
   Mat imCopy;

   aruco::detectMarkers(imGray, this->dictionary,this->c_corners, this->c_ids, this->params, this->c_rejected);
   if(this->refindStrategy)
       aruco::refineDetectedMarkers(imGray, this->board, this->c_corners, this->c_ids, this->c_rejected);

    //image.copyTo(imageCopy);

   if(c_ids.size()>0) {
       imRGBa.copyTo(imCopy);

       vector<Vec3d> rvecs, tvecs; // Airán.

       aruco::estimatePoseSingleMarkers(this->c_corners, 0.1, this->c_cameraMatrix, this->c_distCoeffs, rvecs, tvecs);
       int j=0;

       this->tRvecs=rvecs;
       this->tTvecs=tvecs;
       this->Id=c_ids[0];

       for(vector< vector<Point2f> >::iterator it=c_corners.begin(); it!=c_corners.end(); ++it ){
           vector<Point2f> corners =*it;
           for (int i=0;i<4;i++){
               Point pt1 = corners[i];
               circle(imRGBa,pt1,10,Scalar(255,0,0),-1,8);
           }

           aruco::drawAxis(imRGBa, this->c_cameraMatrix, this->c_distCoeffs, rvecs[j], tvecs[j], 0.1); // Airán.
           j++;
       }

       // Airán
       /*vector<Vec3d> rvecs, tvecs;
       aruco::estimatePoseSingleMarkers(this->c_corners, 0.05, this->c_cameraMatrix, this->c_distCoeffs, rvecs, tvecs);
       for(int i=0; i<this->c_ids.size(); i++){
           aruco::drawAxis(imRGBa, this->c_cameraMatrix, this->c_distCoeffs, rvecs[i], tvecs[i], 0.1);
       }*/



       //aruco::drawDetectedMarkers(imCopy, c_corners, c_ids,Scalar(255,0,0));
       imgSize=imRGBa.size();
       //imRes=imageCopy.clone();
       result = true;
   }


   return result;
}

bool ArucoState::acceptCurrentDetection() {
   bool result=false;
   if(c_corners.size()>0) {
       c_allCorners.push_back(c_corners);
       c_allIds.push_back(c_ids);
       result = true;
   }
   return result;


}

bool ArucoState::calibrate(){
   bool result=false;
   if(c_allIds.size()<1)
       return false;
   if(calibrationFlags & CALIB_FIX_ASPECT_RATIO){
       c_cameraMatrix=Mat::eye(3,3,CV_64F);
       c_cameraMatrix.at<double>(0,0)=aspectRatio;
   }

   vector< vector<Point2f> > allCornersConcatenated;
   vector<int> allIdsConcatenated;
   vector<int> markerCounterPerFrame;
   markerCounterPerFrame.reserve(c_allCorners.size());
   for(unsigned int i=0;i<c_allCorners.size();i++){
       markerCounterPerFrame.push_back((int)c_allCorners[i].size());
       for(unsigned int j =0;j<c_allCorners[i].size();j++){
           allCornersConcatenated.push_back(c_allCorners[i][j]);
           allIdsConcatenated.push_back(c_allIds[i][j]);
       }
   }

   double repError;

   repError=aruco::calibrateCameraAruco(allCornersConcatenated,allIdsConcatenated,
       markerCounterPerFrame,board,imgSize,c_cameraMatrix,c_distCoeffs,c_rvecs,c_tvecs,calibrationFlags);

   bool saveOk = ArucoState::saveCameraParams(cameraFilePath,imgSize,aspectRatio,calibrationFlags,c_cameraMatrix,
           c_distCoeffs,repError);

   return saveOk;
}

void ArucoState::clean(){
   c_allIds.clear();
   c_allCorners.clear();

}

// Getters and Setters

bool ArucoState::setParametersFromFile(const char* df){
   FileStorage fs;

   //std::ifstream fs;
   try {
      fs.open(this->_paramsPath, FileStorage::READ);
      //fs.open(df,std::ifstream::in);
   }
   catch(cv::Exception e){
       //if(fs.fail())
         // return false;

      if(!fs.isOpened())
           return false;
   }


   fs["adaptiveThreshWinSizeMin"] >> params->adaptiveThreshWinSizeMin;
   fs["adaptiveThreshWinSizeMax"] >> params->adaptiveThreshWinSizeMax;
   fs["adaptiveThreshWinSizeStep"] >> params->adaptiveThreshWinSizeStep;
   fs["adaptiveThreshConstant"] >> params->adaptiveThreshConstant;
   fs["minMarkerPerimeterRate"] >> params->minMarkerPerimeterRate;
   fs["maxMarkerPerimeterRate"] >> params->maxMarkerPerimeterRate;
   fs["polygonalApproxAccuracyRate"] >> params->polygonalApproxAccuracyRate;
   fs["minCornerDistanceRate"] >> params->minCornerDistanceRate;
   fs["minDistanceToBorder"] >> params->minDistanceToBorder;
   fs["minMarkerDistanceRate"] >> params->minMarkerDistanceRate;
   fs["cornerRefinementMethod"] >> params->cornerRefinementMethod;
   fs["cornerRefinementWinSize"] >> params->cornerRefinementWinSize;
   fs["cornerRefinementMaxIterations"] >> params->cornerRefinementMaxIterations;
   fs["cornerRefinementMinAccuracy"] >> params->cornerRefinementMinAccuracy;
   fs["markerBorderBits"] >> params->markerBorderBits;
   fs["perspectiveRemovePixelPerCell"] >> params->perspectiveRemovePixelPerCell;
   fs["perspectiveRemoveIgnoredMarginPerCell"] >> params->perspectiveRemoveIgnoredMarginPerCell;
   fs["maxErroneousBitsInBorderRate"] >> params->maxErroneousBitsInBorderRate;
   fs["minOtsuStdDev"] >> params->minOtsuStdDev;
   fs["errorCorrectionRate"] >> params->errorCorrectionRate;

   return true;
}

// Airán
bool ArucoState::setCameraFromFileCamera(const char* df){
    FileStorage fs;

    //std::ifstream fs;
    try {
        fs.open(this->_cameraPath, FileStorage::READ);
        //fs.open(df,std::ifstream::in);
    }
    catch(cv::Exception e){
        //if(fs.fail())
        // return false;

        if(!fs.isOpened())
            return false;
    }

    fs["camera_matrix"] >> this->c_cameraMatrix;
    fs["distortion_coefficients"] >> this->c_distCoeffs;

    return true;
}

void ArucoState::setdictionary(int did){

   this->dictionaryId=did;
   this->dictionary=aruco::getPredefinedDictionary(aruco::PREDEFINED_DICTIONARY_NAME(did));

}

void ArucoState::setBoard(){

   this->gridboard=aruco::GridBoard::create(markersX,markersY,markerLength,markerSeparation,dictionary);
   this->board=gridboard.staticCast<aruco::Board>();

}



void ArucoState::setmarkersXY(int x, int y) {

   this->markersX=x;
   this->markersY=y;

}

void ArucoState::setmarkerLength(float l) {

   this->markerLength=l;
}

void ArucoState::setmarkerSeparation(float s) {

   this->markerSeparation=s;

}

void ArucoState::setcalibrationFlags(int f) {

   this->calibrationFlags=f;
}

void ArucoState::setaspectRation(float r) {

   this->aspectRatio=r;
}

void ArucoState::setrefindStrategy(bool b){

   this->refindStrategy=b;
}

void ArucoState::setcameraFilePath(const char* cf) {

   this->cameraFilePath = cf;
}

void ArucoState::setcameraMatrix(const Mat &m){
   this->c_cameraMatrix=m;
}

void ArucoState::setcameraDistCoeffs(const Mat &d){
   this->c_distCoeffs=d;
}

void ArucoState::setcameraRTvectors(const vector<Mat> &rvec, const vector<Mat> &tvec){
   this->c_rvecs=rvec;
   this->c_tvecs=tvec;
}

bool ArucoState::saveCameraParams(const string &filename, Size imageSize, float aspectRatio, int flags,
                            const Mat &cameraMatrix, const Mat &distCoeffs, double totalAvgErr) {
   FileStorage fs(filename, FileStorage::WRITE);
   if(!fs.isOpened())
       return false;

   time_t tt;
   time(&tt);
   struct tm *t2 = localtime(&tt);
   char buf[1024];
   strftime(buf, sizeof(buf) - 1, "%c", t2);

   fs << "calibration_time" << buf;

   fs << "image_width" << imageSize.width;
   fs << "image_height" << imageSize.height;

   if(flags & CALIB_FIX_ASPECT_RATIO) fs << "aspectRatio" << aspectRatio;

   if(flags != 0) {
       sprintf(buf, "flags: %s%s%s%s",
               flags & CALIB_USE_INTRINSIC_GUESS ? "+use_intrinsic_guess" : "",
               flags & CALIB_FIX_ASPECT_RATIO ? "+fix_aspectRatio" : "",
               flags & CALIB_FIX_PRINCIPAL_POINT ? "+fix_principal_point" : "",
               flags & CALIB_ZERO_TANGENT_DIST ? "+zero_tangent_dist" : "");
   }

   fs << "flags" << flags;

   fs << "camera_matrix" << cameraMatrix;
   fs << "distortion_coefficients" << distCoeffs;

   fs << "avg_reprojection_error" << totalAvgErr;

   return true;
}
