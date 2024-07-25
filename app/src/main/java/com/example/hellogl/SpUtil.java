package com.example.hellogl;

import android.content.Context;
import android.content.SharedPreferences;

import androidx.annotation.NonNull;

/**
 * create by liaohailong
 * time 2022/9/26 9:21
 * desc: key-value 形式的本地少量数据持久化工具
 */
public class SpUtil {
    private static SharedPreferences sInstance = null;

    /**
     * @param context 必须在 application 中初始化，否则功能无法使用
     */
    public static void init(@NonNull Context context) {
        String name = context.getApplicationInfo().name;
        sInstance = context.getSharedPreferences(name, Context.MODE_PRIVATE);
    }

    /**
     * @param key   缓存数据的key
     * @param value 缓存数据值
     */
    public static void put(@NonNull String key, @NonNull String value) {
        if (sInstance == null) return;
        sInstance.edit().putString(key, value).apply();
    }

    /**
     * @param key   缓存数据的key
     * @param value 缓存数据值
     */
    public static void put(@NonNull String key, boolean value) {
        if (sInstance == null) return;
        sInstance.edit().putBoolean(key, value).apply();
    }

    /**
     * @param key   缓存数据的key
     * @param value 缓存数据值
     */
    public static void put(@NonNull String key, int value) {
        if (sInstance == null) return;
        sInstance.edit().putInt(key, value).apply();
    }

    /**
     * @param key   缓存数据的key
     * @param value 缓存数据值
     */
    public static void put(@NonNull String key, long value) {
        if (sInstance == null) return;
        sInstance.edit().putLong(key, value).apply();
    }

    /**
     * @param key   缓存数据的key
     * @param value 缓存数据值
     */
    public static void put(@NonNull String key, float value) {
        if (sInstance == null) return;
        sInstance.edit().putFloat(key, value).apply();
    }

    /**
     * @param key          获取缓存数据的key
     * @param defaultValue 当没有发现缓存数据是，返回的默认数据
     * @return 缓存数据or默认数据
     */
    public static String getString(@NonNull String key, @NonNull String defaultValue) {
        if (sInstance == null) return defaultValue;
        return sInstance.getString(key, defaultValue);
    }

    /**
     * @param key          获取缓存数据的key
     * @param defaultValue 当没有发现缓存数据是，返回的默认数据
     * @return 缓存数据or默认数据
     */
    public static boolean getBoolean(@NonNull String key, boolean defaultValue) {
        if (sInstance == null) return defaultValue;
        return sInstance.getBoolean(key, defaultValue);
    }

    /**
     * @param key          获取缓存数据的key
     * @param defaultValue 当没有发现缓存数据是，返回的默认数据
     * @return 缓存数据or默认数据
     */
    public static int getInt(@NonNull String key, int defaultValue) {
        if (sInstance == null) return defaultValue;
        return sInstance.getInt(key, defaultValue);
    }

    /**
     * @param key          获取缓存数据的key
     * @param defaultValue 当没有发现缓存数据是，返回的默认数据
     * @return 缓存数据or默认数据
     */
    public static long getLong(@NonNull String key, long defaultValue) {
        if (sInstance == null) return defaultValue;
        return sInstance.getLong(key, defaultValue);
    }

    /**
     * @param key          获取缓存数据的key
     * @param defaultValue 当没有发现缓存数据是，返回的默认数据
     * @return 缓存数据or默认数据
     */
    public static float getFloat(@NonNull String key, float defaultValue) {
        if (sInstance == null) return defaultValue;
        return sInstance.getFloat(key, defaultValue);
    }


}
