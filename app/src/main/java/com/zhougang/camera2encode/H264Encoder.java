package com.zhougang.camera2encode;

public class H264Encoder {
    static {
        System.loadLibrary("ffmpeg");
    }
    public static native int encodeInitial(int width, int height, String output);
    public static native int videoEncode(byte[] imageByte);
    public static native int flush();
}
