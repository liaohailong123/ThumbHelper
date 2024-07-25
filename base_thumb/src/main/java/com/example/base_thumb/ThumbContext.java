package com.example.base_thumb;

import android.app.Application;

/**
 * Author: liaohailong
 * Date: 2024/6/14
 * Time: 14:22
 * Description:
 **/
public class ThumbContext {
    static {
        System.loadLibrary("hellogl");
    }

    private static Application sApp;

    public static void init(Application app) {
        sApp = app;
    }

    public static Application getApp() {
        return sApp;
    }
}
