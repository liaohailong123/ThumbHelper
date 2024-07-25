package com.example.base_thumb.util;

import androidx.annotation.WorkerThread;

/**
 * Author: liaohailong
 * Date: 2024/7/24
 * Time: 15:27
 * Description: 使用FFmpeg库，获取多媒体文件信息
 **/
public class FFMediaHelper {

    /**
     * 获取多媒体文件的基本信息，IO操作，建议子线程调用
     *
     * @param uri 本地资源 或 线上资源
     * @return MetaData对象为null或者里面的内容都是默认值，表示解析不出来
     */
    @WorkerThread
    public MetaData getMetaData(String uri) {
        return nativeGetMetaData(uri);
    }

    protected native MetaData nativeGetMetaData(String uri);

}
