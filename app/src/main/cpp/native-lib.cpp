//
// Created by PRINCIPAL on 20/07/2020.
//

#include <jni.h>
#include <string>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/aruco.hpp>
#include <opencv2/calib3d.hpp>
#include "ArucoState.h"


using namespace std;
using namespace cv;


// The State
const int MAXPATHLENGTH=2048;
char internalPath[MAXPATHLENGTH];
ArucoState *aState;

static bool readDetectorParameters(string filename, Ptr<aruco::DetectorParameters> &params) {
   FileStorage fs(filename, FileStorage::READ);
   if(!fs.isOpened())
       return false;
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



/**
*/
static bool saveCameraParams(const string &filename, Size imageSize, float aspectRatio, int flags,
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


// -------------------------------

// Nuevos métodos.

extern "C"
JNIEXPORT void JNICALL
Java_com_example_appcapture_MainActivity_setInternalPath(JNIEnv *env, jobject thiz, jstring path){
   jboolean isCopy;
   strncpy(internalPath,env->GetStringUTFChars(path,&isCopy),MAXPATHLENGTH);
   //aState->setArucoPaths(env->GetStringUTFChars(path,&isCopy),"/detector_params.yml","/camera.dat");
}



extern "C"
JNIEXPORT void JNICALL
Java_com_example_appcapture_MainActivity_salt(JNIEnv *env, jobject thiz, jlong mat_addr_gray,
                                                  jint nbr_elem) {
   // TODO: implement salt()
   Mat &mGr = *(Mat *) mat_addr_gray;
   for (int k = 0; k < nbr_elem; k++) {
       int i = rand() % mGr.cols;
       int j = rand() % mGr.rows;
       mGr.at<uchar>(j, i) = 255;
   }
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_example_appcapture_MainActivity_confdetector(JNIEnv *env, jobject thiz, jint a) {
   // TODO: implement confdetector()
   jboolean iscopy=true;
   int b =a;
   //jclass jclass_MainActivity = env->FindClass("com/example/appcalibracion2/MainActivity");

   //jfieldID fid_pathdetector = env->GetStaticFieldID(jclass_MainActivity, "pathdetector" , "Ljava/lang/String;");
   //jfieldID fid_pathcamera = env->GetStaticFieldID(jclass_MainActivity, "pathcamera" , "Ljava/lang/String;");

   //jstring jstr_pathdetector = (jstring) env->GetStaticObjectField(jclass_MainActivity,fid_pathdetector);
   //jstring jstr_pathcamera = (jstring) env->GetStaticObjectField(jclass_MainActivity,fid_pathcamera);
   //env->GetStringChars(path1,&iscopy);
   //env->GetStringChars(path2,&iscopy);

   //const char* strpar = (env)->GetStringUTFChars(jstr_pathdetector,&iscopy);
   //const char* strcam = (env)->GetStringUTFChars(jstr_pathcamera,&iscopy);
   //char* strpar =

   //const char* strpar = "/storage/emulated/0/detector_params.yml";
   //const char* strcam = "/storage/emulated/0/camera.dat";

   int did=1;
   int markersX=1;
   int markersY=1;
   float markerLength=600;
   float markerSeparation=50;

   aState=new ArucoState(internalPath,"/detector_params.yml", "/camera.dat", did);
   if(!aState->fatalError)
       aState->initializeBoard(markersX,markersY,markerLength,markerSeparation);
   if(aState->fatalError)
       return false;

   return true;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_example_appcapture_MainActivity_detect(JNIEnv *env, jobject thiz, jlong mat_addr_gray,
                                                    jlong mat_addr_rgba) {
   // TODO: implement detect()
   Mat &mGr = *(Mat *) mat_addr_gray;
   Mat &mRGBa = *(Mat *) mat_addr_rgba;

   bool result=aState->detect(mGr, mRGBa);

   return result;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_example_appcapture_MainActivity_acceptCurrentDetection(JNIEnv *env, jobject thiz) {
   // TODO: implement acceptCurrentDetection()
   bool result=false;
   result=aState->acceptCurrentDetection();
   return result;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_example_appcapture_MainActivity_calibrate(JNIEnv *env, jobject thiz) {
   // TODO: implement calibrate()
   bool result=aState->calibrate();
   return result;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_appcapture_MainActivity_limpiacapturas(JNIEnv *env, jobject thiz) {
   // TODO: implement limpiacapturas()
   aState->clean();
}

// Airán. Métodos para traer String de código nativo.
extern "C" JNIEXPORT jstring JNICALL
Java_com_example_appcapture_MainActivity_stringFromId(
        JNIEnv* env,
        jobject /* this */) {
    char buf[2048]; // need a buffer for that
    int i = aState->Id;
    sprintf(buf,"%d",i);
    const char* p = buf;
    return env->NewStringUTF(p);
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_appcapture_MainActivity_stringFromRvecs(
        JNIEnv* env,
        jobject /* this */) {
    char buf1[2048]; // need a buffer for that
    memset(buf1, 0, sizeof(buf1));
    double a = aState->tRvecs[0].val[0];
    double b = aState->tRvecs[0].val[1];
    double c = aState->tRvecs[0].val[2];
    sprintf(buf1,"%lf ; %lf ; %lf",a,b,c);

    //for(int i=0;i<(aState->tRvecs).size();i++){

    const char* p = buf1;
    return env->NewStringUTF(p);
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_appcapture_MainActivity_stringFromTvecs(
        JNIEnv* env,
        jobject /* this */) {
    char buf2[2048]; // need a buffer for that
    memset(buf2, 0, sizeof(buf2));
    double a = aState->tTvecs[0].val[0];
    double b = aState->tTvecs[0].val[1];
    double c = aState->tTvecs[0].val[2];
    sprintf(buf2,"%lf ; %lf ; %lf",a,b,c);

    //for(int i=0;i<(aState->tRvecs).size();i++){

    const char* p = buf2;
    return env->NewStringUTF(p);
}