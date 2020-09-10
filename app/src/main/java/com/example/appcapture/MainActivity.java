package com.example.appcapture;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;

import android.Manifest;
import android.content.Context;
import android.content.pm.PackageManager;
import android.content.res.AssetManager;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.SurfaceView;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.CameraBridgeViewBase;
import org.opencv.android.JavaCameraView;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.OpenCVLoader;
import org.opencv.core.Mat;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public class MainActivity extends AppCompatActivity implements CameraBridgeViewBase.CvCameraViewListener2{
    private static final String TAG = "OCVSample::Activity";
    //private CameraBridgeViewBase _cameraBridgeViewBase;
    private JavaCameraView javaCameraView;
    private boolean detectionState=false, capture=false, semaforo=false;
    private int ncapturas=0;
    private static int nframesToAcceptDetection=10;
    private static int ndetectedFrames=0;
    private static String pathdetector = Environment.getExternalStorageDirectory().getAbsolutePath()+"/detector_params.yml";
    private  static String pathcamera = Environment.getExternalStorageDirectory().getAbsolutePath()+"/camera.dat";
    private File internalpath;
    // Variables para Bluetooth.
    BlueConnect conexion;
    boolean statusConection;

    private BaseLoaderCallback _baseLoaderCallback = new BaseLoaderCallback(this) {
        @Override
        public void onManagerConnected(int status) {
            switch (status) {
                case LoaderCallbackInterface.SUCCESS: {
                    Log.i(TAG, "OpenCV loaded successfully");
                    // Load ndk built module, as specified in moduleName in build.gradle
                    // after opencv initialization
                    //System.loadLibrary("native-lib2");
                    System.loadLibrary("native-lib");
                    javaCameraView.enableView();
                    //_cameraBridgeViewBase.enableView();


                    boolean result=false;
                    //result=confdetector(pathdetector, pathcamera);
                    String sPath = internalpath.toString();
                    setInternalPath(sPath);

                    result=confdetector(2);
                    //result=prueba();
                    if(!result) {
                        Toast.makeText(MainActivity.this, "Error no recuperable", Toast.LENGTH_SHORT).show();
                        finish();
                    }

                }
                break;
                default: {
                    super.onManagerConnected(status);
                }
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        // Airán - Quitar título y hacer pantalla completa.
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);

        //setContentView(R.layout.activity_main);
        // Airán. Prueba pantalla captura.
        setContentView(R.layout.activity_main1);


        final Button button = (Button) findViewById(R.id.bCapture);

        button.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                //final TextView viewCapture = (TextView) findViewById(R.id.tCapture);
                //final TextView viewInfo = (TextView) findViewById(R.id.tInfo);

                // Airán. Creamos los TextView de la nueva vista.
                final TextView viewId = (TextView) findViewById(R.id.tId);
                final TextView viewRvecs = (TextView) findViewById(R.id.tRvecs);
                final TextView viewTvecs = (TextView) findViewById(R.id.tTvecs);

                capture=true;

                /*if (!detectionState)
                    Toast.makeText(MainActivity.this, "Markers not detected", Toast.LENGTH_SHORT).show();

                else{
                    boolean result=false;
                    result=acceptCurrentDetection();
                    if(result) {
                        Toast.makeText(MainActivity.this, "Datos de detección guardados", Toast.LENGTH_SHORT).show();
                        ncapturas = ncapturas + 1;
                        //viewCapture.setText(String.valueOf(ncapturas));
                    }
                    else
                        Toast.makeText(MainActivity.this, "No se guardaron datos de detección", Toast.LENGTH_SHORT).show();
                    detectionState=false;
                    //viewInfo.setText("Esperando detección");
                }*/
                // Code here executes on main thread after user presses button
            }
        });

        /*final Button button3 = (Button) findViewById(R.id.bDiscard);

        button3.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                final TextView view = (TextView) findViewById(R.id.tInfo);
                if (!detectionState)
                    Toast.makeText(MainActivity.this, "Markers not detected", Toast.LENGTH_SHORT).show();

                else{

                    detectionState=false;
                    view.setText("Esperando detección");
                }
                // Code here executes on main thread after user presses button
            }
        });
        final Button button2 = (Button) findViewById(R.id.bCalibrate);

        button2.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                final TextView view = (TextView) findViewById(R.id.tInfo);

                boolean result=calibrate();

                if(result){
                    ncapturas=0;
                    limpiacapturas();
                    Toast.makeText(MainActivity.this, "Calibración guardada", Toast.LENGTH_SHORT).show();
                }
                else
                    Toast.makeText(MainActivity.this, "Error en calibración", Toast.LENGTH_SHORT).show();

                // Code here executes on main thread after user presses button
            }
        });*/

        // Permissions for Android 6+

        ActivityCompat.requestPermissions(MainActivity.this,
                new String[]{Manifest.permission.CAMERA, Manifest.permission.READ_EXTERNAL_STORAGE, Manifest.permission.WRITE_EXTERNAL_STORAGE},
                1);




        //_cameraBridgeViewBase = (CameraBridgeViewBase) findViewById(R.id.main_surface);
        //_cameraBridgeViewBase.setVisibility(SurfaceView.VISIBLE);
        //_cameraBridgeViewBase.setCvCameraViewListener(this);
        javaCameraView =  (JavaCameraView) findViewById(R.id.main_surface);
        javaCameraView.setVisibility(SurfaceView.VISIBLE);
        javaCameraView.setCvCameraViewListener(this);


        // Assets to Internal data
        Context context = this;
        AssetManager assetManager = getAssets();
        internalpath = context.getFilesDir();
        try {
            InputStream is_params = assetManager.open("detector_params.yml");
            File file = new File(internalpath, "detector_params.yml");
            byte[] buf = new byte[is_params.available()];
            is_params.read(buf);
            OutputStream os_params = new FileOutputStream(file);
            os_params.write(buf);
            os_params.flush();
            os_params.close();
            is_params.close();

        } catch (IOException e) {
            e.printStackTrace();
        }
        // TODO The same with camera.dat
        /*try {
            InputStream is_params = assetManager.open("camera.dat"); // creamos un input stream
            File file = new File(internalpath, "camera.dat");
            byte[] buf = new byte[is_params.available()];
            is_params.read(buf);
            OutputStream os_params = new FileOutputStream(file);
            os_params.write(buf);
            os_params.flush();
            os_params.close();
            is_params.close();

        } catch (IOException e) {
            e.printStackTrace();
        }*/


        // Copiar fichero en carpeta publica.
        /*File file = new File(getExternalFilesDir(null), "DemoFile.txt");
        String ruta = file.getAbsolutePath(); //"/storage/emulated/0/Android/data/com.example.appcapture/files/DemoFile.txt"

        try {
            // Very simple code to copy a picture from the application's
            // resource into the external file.  Note that this code does
            // no error checking, and assumes the picture is small (does not
            // try to copy it in chunks).  Note that if external storage is
            // not currently mounted this will silently fail.
            InputStream is = assetManager.open("camera.dat");
            OutputStream os = new FileOutputStream(file);
            byte[] data = new byte[is.available()];
            is.read(data);
            os.write(data);
            is.close();
            os.close();
        } catch (IOException e) {
            Log.w("ExternalStorage", "Error writing " + file, e);
        }*/

        if(traerFichero("/storage/emulated/0/Android/data/com.example.appcalibracion3/files/", "camera.dat")){ // Método para traer fichero de aplicación externa.
            Log.w("Fichero", "CORRECTO: Al taer fichero.");
        }else{
            Log.w("Fichero", "ERROR: Al taer fichero.");
        }

        conexion = new BlueConnect("B8:27:EB:F7:2E:49", "00001101-0000-1000-8000-00805f9b34fb");
        statusConection = conexion.conectar();
    }

    // Método para traer fichero de otra aplicación.
    public boolean traerFichero(String ruta, String nombre){
        boolean result = false;

        File file = new File(internalpath, nombre); // Fichero de salida.
        //File file = new File(getExternalFilesDir(null), "DemoFileCopy.txt");
        //String ruta1 = file.getAbsolutePath(); // Verificar ruta del fichero

        try {
            InputStream is = new FileInputStream(new File(ruta, nombre));
            OutputStream os = new FileOutputStream(file);
            byte[] data = new byte[is.available()];
            is.read(data);
            os.write(data);
            os.flush();
            is.close();
            os.close();
            result = true;
        } catch (IOException e) {
            Log.w("ExternalStorage", "Error writing " + file, e);
            return false;
        }
        return result;
    }


    @Override
    public void onPause() {
        super.onPause();
        disableCamera();
    }

    @Override
    public void onResume() {
        super.onResume();

        if (!OpenCVLoader.initDebug()) {
            Log.d(TAG, "Internal OpenCV library not found. Using OpenCV Manager for initialization");
            //    OpenCVLoader.initAsync(OpenCVLoader.OPENCV_VERSION_3_0_0, this, _baseLoaderCallback);
        } else {
            Log.d(TAG, "OpenCV library found inside package. Using it!");
            _baseLoaderCallback.onManagerConnected(LoaderCallbackInterface.SUCCESS);
        }

        //_baseLoaderCallback.onManagerConnected(LoaderCallbackInterface.SUCCESS);
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String permissions[], int[] grantResults) {
        switch (requestCode) {
            case 1: {
                // If request is cancelled, the result arrays are empty.
                if (grantResults.length > 0
                        && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                    // permission was granted, yay! Do the
                    // contacts-related task you need to do.
                } else {
                    // permission denied, boo! Disable the
                    // functionality that depends on this permission.
                    Toast.makeText(MainActivity.this, "Basic permissions denied", Toast.LENGTH_SHORT).show();
                    finish();
                }
                return;
            }

            // other 'case' lines to check for other
            // permissions this app might request
        }
    }

    public void onDestroy() {
        super.onDestroy();
        disableCamera();
    }

    public void disableCamera() {
        if(javaCameraView!=null)
            javaCameraView.disableView();
        // if (_cameraBridgeViewBase != null)
        //   _cameraBridgeViewBase.disableView();
    }

    @Override
    public void onCameraViewStarted(int width, int height) {
        boolean a=false;
    }

    @Override
    public void onCameraViewStopped() {
    }

    @Override
    public Mat onCameraFrame(CameraBridgeViewBase.CvCameraViewFrame inputFrame) {
        boolean result=false;
        Mat matGray = inputFrame.gray();
        Mat matRGBa = inputFrame.rgba();
        //return matRGBa;
        //Mat matRes = new Mat();
        //matGray.copyTo(matRes);

        //salt(matGray.getNativeObjAddr(), 2000);

        /*if(!detectionState) {
            result = detect(matGray.getNativeObjAddr(),matRGBa.getNativeObjAddr());
            // Airán
        }

        if(!result){
            ndetectedFrames=0;
            return matRGBa;
        }

        else{
            ndetectedFrames+=1;
            if(ndetectedFrames>=nframesToAcceptDetection) {
                ndetectedFrames = 0;
                detectionState = true;

                // onCameraFrame va en otro thread. Para tocar los elementos de la UI
                // hay que estar en el mismo thread que la UI
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        final TextView view = (TextView) findViewById(R.id.tInfo);
                        view.setText(R.string.detect);

                    }
                });
            }
            return matRGBa;
        }*/
        if(capture && !semaforo){
            semaforo=true;
            //Mat a = matGray.t(), b = matRGBa.t();
            //result=detect(a.getNativeObjAddr(),b.getNativeObjAddr());
            if(detectionState) {
                long timeCaptura = System.currentTimeMillis();
                result = detect(matGray.getNativeObjAddr(), matRGBa.getNativeObjAddr());
                detectionState=false;
                if (result) {
                    // onCameraFrame va en otro thread. Para tocar los elementos de la UI
                    // hay que estar en el mismo thread que la UI
                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            //final TextView view = (TextView) findViewById(R.id.tInfo);
                            //view.setText(R.string.detect);
                            // Airán. Creamos los TextView de la nueva vista.
                            final TextView viewId = (TextView) findViewById(R.id.tId);
                            final TextView viewRvecs = (TextView) findViewById(R.id.tRvecs);
                            final TextView viewTvecs = (TextView) findViewById(R.id.tTvecs);
                            String a = stringFromId();
                            viewId.setText("Id:" + a);
                            String b = stringFromRvecs();
                            viewRvecs.setText("Rvecs: " + b);
                            String c = stringFromTvecs();
                            viewTvecs.setText("Tvecs: " + c);
                        }
                    });
                    // Enviamos por Bluetooth.
                    if(statusConection){
                        long timeEnvio = System.currentTimeMillis();
                        String cadenaResultado = "1;-1;-1;-1;-1;-1;-1;-1;" + stringFromTvecs() + stringFromRvecs() + timeCaptura + ";" + timeEnvio + ";";
                        if (conexion.writetostream(cadenaResultado)) {
                            Log.d("Bluetooth", "OK: COMPLETADO");
                        } else {
                            Log.d("Bluetooth", "ERROR: NO COMPLETADO");
                        }
                    }
                } else {
                    // onCameraFrame va en otro thread. Para tocar los elementos de la UI
                    // hay que estar en el mismo thread que la UI
                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            //final TextView view = (TextView) findViewById(R.id.tInfo);
                            //view.setText(R.string.detect);
                            // Airán. Creamos los TextView de la nueva vista.
                            final TextView viewId = (TextView) findViewById(R.id.tId);
                            final TextView viewRvecs = (TextView) findViewById(R.id.tRvecs);
                            final TextView viewTvecs = (TextView) findViewById(R.id.tTvecs);
                            viewId.setText("Id: No detect !!");
                            viewRvecs.setText("Rvecs: No detect !!");
                            viewTvecs.setText("Tvecs: No detect !!");
                        }
                    });
                }
            }else{
                ndetectedFrames+=1;
                if(ndetectedFrames>=nframesToAcceptDetection) {
                    ndetectedFrames = 0;
                    detectionState = true;
                }
            }
            //capture=false;
            semaforo=false;
            return matRGBa;
        }
        return matRGBa;
    }


    public native void salt(long matAddrGray, int nbrElem);
    //public native boolean confdetector(String path1, String path2);
    public native boolean confdetector(int a);
    public native boolean detect(long matAddrGray, long matAddrRGBa);
    public native boolean acceptCurrentDetection();
    public native boolean calibrate();
    public native void limpiacapturas();
    public native void setInternalPath(String path);
    // Airán. Métodos para traer String de código nativo.
    public native String stringFromId();
    public native String stringFromRvecs();
    public native String stringFromTvecs();
}
