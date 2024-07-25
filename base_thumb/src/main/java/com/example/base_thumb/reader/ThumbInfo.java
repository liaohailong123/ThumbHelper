package com.example.base_thumb.reader;

import android.graphics.Bitmap;

import androidx.annotation.NonNull;

/**
 * Author: liaohailong
 * Date: 2021/8/14
 * Time: 9:00
 * Description:
 **/
public class ThumbInfo {
    @NonNull
    private final Bitmap frame;
    /**
     * 缩略图显示时间戳，单位：us
     */
    private final long timestampsUs;
    private final int index;

    public ThumbInfo(@NonNull Bitmap frame, long timestampsUs, int index) {
        this.frame = frame;
        this.timestampsUs = timestampsUs;
        this.index = index;
    }

    @NonNull
    public Bitmap getFrame() {
        return frame;
    }

    public long getTimestampsUs() {
        return timestampsUs;
    }

    public long getTimestampsMs() {
        return timestampsUs / 1000L;
    }

    public int getIndex() {
        return index;
    }
}
