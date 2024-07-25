package com.example.base_thumb.render;

import android.app.Application;
import android.content.res.Resources;

import androidx.annotation.Keep;

import com.example.base_thumb.ThumbContext;

import java.io.InputStream;

/**
 * create by liaohailong
 * 2024/6/14 20:39
 * desc: 协助加载 着色器代码
 */
public class HelloGLSL {

    /**
     * called by native code
     *
     * @param name the name of raw source
     * @return read the source text
     */
    @Keep
    public static String getRawSourceByName(String name) {
        Application app = ThumbContext.getApp();
        if (app == null) {
            return "";
        }

        String packageName = app.getPackageName();
        Resources resources = app.getResources();
        int id = resources.getIdentifier(name, "raw", packageName);
        if (id == 0) {
            return "";
        }

        InputStream is = resources.openRawResource(id);
        return GLES2Util.loadFromInputStream(is);
    }

    public static ClassLoader getClassLoader() {
        return HelloGLSL.class.getClassLoader();
    }
}
