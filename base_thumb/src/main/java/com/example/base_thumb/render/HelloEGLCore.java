package com.example.base_thumb.render;

import android.content.Context;
import android.opengl.Matrix;
import android.os.Handler;
import android.os.HandlerThread;
import android.view.Surface;

import androidx.annotation.NonNull;

/**
 * Author: liaohailong
 * Date: 2024/6/14
 * Time: 16:07
 * Description:
 **/
public class HelloEGLCore {
    @NonNull
    private final Context context;

    private final HelloGLProgram[] glPrograms;

    private final float[] projectMat = new float[4 * 4];

    private final Handler renderHandler;

    private HelloEGLContext glContext;


    public HelloEGLCore(@NonNull Context context, HelloGLProgram... programs) {
        this.context = context;
        this.glPrograms = programs;
        // 渲染线程
        HandlerThread ht = new HandlerThread("hello-gl-thread");
        ht.start();
        renderHandler = new Handler(ht.getLooper());
    }


    public void init(Runnable complete) {
        renderHandler.post(() -> {
            if (glContext == null) {
                glContext = new HelloEGLContext();

                if (complete != null) {
                    complete.run();
                }

            }
        });
    }

    /**
     * @param surface  渲染窗口
     * @param complete surface设置成功回调，失败不会回调
     */
    public void setSurface(@NonNull Surface surface,
                           @NonNull Runnable complete) {
        renderHandler.post(() -> {
            if (glContext != null) {
                glContext.setSurface(surface); // 制作EGLSurface，并使用
            }
            // 创建正交投影矩阵
            initProjectMatrix(projectMat);
            // 初始化着色器程序
            for (HelloGLProgram glProgram : glPrograms) {
                glProgram.init();
            }

            complete.run();
        });
    }

    private void initProjectMatrix(float[] projectM) {
//        // 创建正交投影：
//        float left = -1;
//        float right = 1;
//        float bottom = -1;
//        float top = 1;
//        float near = 1.0f;
//        float far = 100f;
//        Matrix.orthoM(projectM, 0, left, right, bottom, top, near, far);

        // 2D画面，初始化矩阵即可
        Matrix.setIdentityM(projectM, 0);
    }

    public void post(Runnable runnable) {
        if (runnable != null) {
            renderHandler.post(runnable);
        }
    }

    public void removeCallbacks(Runnable runnable) {
        if (runnable != null) {
            renderHandler.removeCallbacks(runnable);
        }
    }

    public void updateFrame() {
        Runnable runnable = () -> {
            if (glContext != null) {
                glContext.renderStart(); // 渲染开始之前，清空当前渲染缓冲区buffer内容

                int width = glContext.getWidth(); // EGLSurface的宽度
                int height = glContext.getHeight(); // EGLSurface的高度

                // 运行着色器程序，绘制内容
                for (HelloGLProgram glProgram : glPrograms) {
                    glProgram.begin(); // 启用着色器
                    glProgram.draw(width, height, projectMat); // 绘制内容
                    glProgram.end(); // 弃用着色器
                }

                glContext.renderEnd(); // 绘制完毕，交换前后缓冲区
            }
        };
        // 已经在渲染线程，直接执行
        if (Thread.currentThread() == renderHandler.getLooper().getThread()) {
            runnable.run();
        } else {
            // 其他线程需要调度到渲染线程执行
            renderHandler.post(runnable);
        }
    }


    public void release() {
        // EGL销毁也需要在自己的渲染线程内操作
        renderHandler.postAtFrontOfQueue(() -> {
            for (HelloGLProgram glProgram : glPrograms) {
                glProgram.destroy();
            }

            if (glContext != null) {
                glContext.destroy();
                glContext = null;
            }
            renderHandler.getLooper().quit();
        });
    }

}
