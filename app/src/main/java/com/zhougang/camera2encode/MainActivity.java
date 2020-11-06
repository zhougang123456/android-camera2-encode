package com.zhougang.camera2encode;

import android.Manifest;
import android.annotation.TargetApi;
import android.content.Context;
import android.content.pm.PackageManager;
import android.graphics.ImageFormat;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraDevice;
import android.hardware.camera2.CameraManager;
import android.hardware.camera2.CaptureRequest;
import android.media.Image;
import android.media.ImageReader;
import android.os.Build;
import android.os.Environment;
import android.os.Handler;
import android.os.HandlerThread;
import android.support.annotation.NonNull;
import android.support.annotation.RequiresApi;
import android.support.v4.app.ActivityCompat;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.widget.Toast;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.Arrays;

@RequiresApi(api = Build.VERSION_CODES.LOLLIPOP)
public class MainActivity extends AppCompatActivity {
    private SurfaceView surfaceView;
    private SurfaceHolder surfaceHolder;
    private Handler childHandler,mainHandler;
    private CameraManager cameraManager;
    private CameraDevice cameraDevice;
    private String cameraId;
    private ImageReader imageReader;
    private CameraCaptureSession cameraCaptureSession;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        initView();
    }
    @Override
    protected void onPause(){
        super.onPause();
        Log.e("Activity ","pause!");
        H264Encoder.flush();
    }
    public void initView(){
        surfaceView = findViewById(R.id.surface_view);
        surfaceHolder = surfaceView.getHolder();
        surfaceHolder.setKeepScreenOn(true);
        surfaceHolder.addCallback(new SurfaceHolder.Callback() {
            @Override
            public void surfaceCreated(SurfaceHolder holder) {
                initCamera2();
            }
            @Override
            public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
            }
            @RequiresApi(api = Build.VERSION_CODES.LOLLIPOP)
            @Override
            public void surfaceDestroyed(SurfaceHolder holder) {
                if (null == cameraDevice){
                    cameraDevice.close();
                    cameraDevice = null;
                }
            }
        });
    }
    private void initCamera2() {
        HandlerThread handlerThread = new HandlerThread("Camera2");
        handlerThread.start();
        childHandler = new Handler(handlerThread.getLooper());
        mainHandler = new Handler(getMainLooper());
        cameraId = "" + CameraCharacteristics.LENS_FACING_BACK;//前摄像头
        String output = Environment.getExternalStorageDirectory().getAbsolutePath() + "/cameraEncode.h264";
        H264Encoder.encodeInitial(640,480,output);
        imageReader = ImageReader.newInstance(640, 480, ImageFormat.YUV_420_888,2);
        imageReader.setOnImageAvailableListener(new ImageReader.OnImageAvailableListener() { //可以在这里处理拍照得到的临时照片 例如，写入本地
            @Override
            public void onImageAvailable(ImageReader reader) {
                Image image = reader.acquireNextImage();
                ByteBuffer buffer = image.getPlanes()[0].getBuffer();
                byte[] bytes = new byte[buffer.remaining()];
                buffer.get(bytes);//由缓冲区存入字节数组
                H264Encoder.videoEncode(bytes);
                image.close();
            }
        }, mainHandler);
        //获取摄像头管理

        cameraManager = (CameraManager) getSystemService(Context.CAMERA_SERVICE);
        try {
            if (ActivityCompat.checkSelfPermission(this, Manifest.permission.CAMERA) != PackageManager.PERMISSION_GRANTED) {
                return;
            }
            //打开摄像头
            cameraManager.openCamera(cameraId, new CameraDevice.StateCallback() {
                @Override
                public void onOpened(@NonNull CameraDevice camera) {
                    cameraDevice = camera;
                    takePreview();
                }

                @Override
                public void onDisconnected(@NonNull CameraDevice camera) {
                    if (null != cameraDevice) {
                        cameraDevice.close();
                        cameraDevice = null;
                    }
                }

                @Override
                public void onError(@NonNull CameraDevice camera, int error) {
                    Toast.makeText(MainActivity.this,"Open camera failed!",Toast.LENGTH_SHORT).show();
                }
            }, mainHandler);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }


    private void takePreview(){
        final CaptureRequest.Builder previewRequestBuilder;
        try {
            previewRequestBuilder = cameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_PREVIEW);
            previewRequestBuilder.addTarget(surfaceHolder.getSurface());
            previewRequestBuilder.addTarget(imageReader.getSurface());
            cameraDevice.createCaptureSession(Arrays.asList(surfaceHolder.getSurface(),
                    imageReader.getSurface()), new CameraCaptureSession.StateCallback() {
                @Override
                public void onConfigured(@NonNull CameraCaptureSession session) {
                    if (null == cameraDevice){
                        return;
                    }
                    cameraCaptureSession = session;
                    CaptureRequest captureRequest = previewRequestBuilder.build();
                    try {
                        cameraCaptureSession.setRepeatingRequest(captureRequest,null, childHandler);
                    } catch (CameraAccessException e) {
                        e.printStackTrace();
                    }

                }
                @Override
                public void onConfigureFailed(@NonNull CameraCaptureSession session) {
                    Toast.makeText(MainActivity.this,"Config failed!",Toast.LENGTH_SHORT).show();
                }
            },childHandler);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }
}
