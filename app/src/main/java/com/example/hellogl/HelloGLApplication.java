package com.example.hellogl;

import android.app.Application;

import com.example.base_thumb.ThumbContext;
import com.example.hellogl.SpUtil;

/**
 * Author: liaohailong
 * Date: 2024/6/14
 * Time: 14:23
 * Description:
 **/
public class HelloGLApplication extends Application {
    @Override
    public void onCreate() {
        super.onCreate();

        SpUtil.init(this);
        ThumbContext.init(this);
    }
}
