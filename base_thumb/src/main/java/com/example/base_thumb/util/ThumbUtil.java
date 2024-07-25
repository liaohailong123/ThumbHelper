package com.example.base_thumb.util;

import android.graphics.Rect;
import android.graphics.RectF;

import androidx.annotation.NonNull;

/**
 * Author: liaohailong
 * Time: 2021/11/15 18:13
 * Describe: ThumbHelper功能的工具类
 */
public class ThumbUtil {

    /**
     * 取出src中的范围，使dst可以以CenterCrop的形式显示src的内容
     *
     * @param src    原区域，一般是有一条边长比dst要大，另外一条和dst一致，因为前面计算视频尺寸用的是等比例缩放
     * @param dst    目标区域
     * @param result CenterCrop计算结果
     */
    public static void centerCrop(@NonNull Rect src, @NonNull RectF dst, @NonNull Rect result) {
        int sw = src.width();
        int sh = src.height();
        float dw = dst.width();
        float dh = dst.height();

        int xOffset = (int) (sw - dw) / 2;
        int yOffset = (int) (sh - dh) / 2;

        int l = src.left + xOffset;
        int t = src.top + yOffset;
        int r = (int) (l + dw);
        int b = (int) (t + dh);
        result.set(l, t, r, b);
    }
}
