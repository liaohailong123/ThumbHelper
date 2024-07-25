package com.example.base_thumb.render;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.opengl.GLES11Ext;
import android.opengl.GLES20;
import android.opengl.GLUtils;
import android.util.Log;

import androidx.annotation.NonNull;

import java.io.ByteArrayOutputStream;
import java.io.InputStream;
import java.nio.charset.Charset;

/**
 * Author: liaohailong
 * Date: 2021/2/12
 * Time: 10:16
 * Description: 使用openGL ES 工具类
 **/
public class GLES2Util {
    private static final String TAG = "GLES2Util";

    public static int generateTexture(Context context, int id) {
        Bitmap bitmap = BitmapFactory.decodeResource(context.getResources(), id);
        return generateTexture(bitmap);
    }

    public static int generateTexture(@NonNull Bitmap bitmap) {
        int[] texture = new int[1];
        GLES20.glGenTextures(1, texture, 0);
        GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, texture[0]);
        GLES20.glTexParameterf(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_NEAREST);
        GLES20.glTexParameterf(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);
        GLES20.glTexParameterf(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_CLAMP_TO_EDGE);
        GLES20.glTexParameterf(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_T, GLES20.GL_CLAMP_TO_EDGE);
        GLUtils.texImage2D(GLES20.GL_TEXTURE_2D, 0, bitmap, 0);
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, 0);
        return texture[0];
    }

    /**
     * 生成CPU与GPU之间共享的纹理ID，可以通过该ID创建SurfaceTexture对象
     *
     * @param textures 请注意，该方法会创建输入的数组长度数量的纹理，所以不要随意创建该数组
     */
    public static void generateOESTexture(@NonNull int[] textures) {
        GLES20.glGenTextures(textures.length, textures, 0);
        for (int texId : textures) {
            GLES20.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, texId);
            GLES20.glTexParameterf(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_NEAREST);
            GLES20.glTexParameterf(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);
            GLES20.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_CLAMP_TO_EDGE);
            GLES20.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_WRAP_T, GLES20.GL_CLAMP_TO_EDGE);
            GLES20.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, 0);
        }
    }

    public static String loadFromInputStream(@NonNull InputStream stream) {
        String result = "";
        try {
            ByteArrayOutputStream baos = new ByteArrayOutputStream();
            int ch = stream.read();
            while (ch != -1) {
                baos.write(ch);
                ch = stream.read();
            }
            byte[] buff = baos.toByteArray();
            baos.close();
            stream.close();
            result = new String(buff, Charset.defaultCharset());
            result = result.replace("\\r\\n", "\n");
        } catch (Exception e) {
            e.printStackTrace();
        }
        return result;
    }

    /**
     * 创建着色器程序
     *
     * @param vertexShaderSource   顶点着色器GLSL代码
     * @param fragmentShaderSource 片元着色器GLSL代码
     * @return 着色器程序id
     */
    public static int createProgram(String vertexShaderSource, String fragmentShaderSource) {
        // 创建顶点着色器
        int vertexShader = createShader(GLES20.GL_VERTEX_SHADER, vertexShaderSource);
        if (vertexShader == 0) {
            return 0;
        }
        // 创建片元着色器
        int fragmentShader = createShader(GLES20.GL_FRAGMENT_SHADER, fragmentShaderSource);
        if (fragmentShader == 0) {
            return 0;
        }

        // 创建着色器程序
        int program = GLES20.glCreateProgram();
        if (program != 0) {
            // 添加顶点着色器到shader程序
            GLES20.glAttachShader(program, vertexShader);
            // 添加片元着色器到shader程序
            GLES20.glAttachShader(program, fragmentShader);
            // 连接应用程序
            GLES20.glLinkProgram(program);
            // 查看着色器结果
            int[] linkStatus = new int[1];
            GLES20.glGetProgramiv(program, GLES20.GL_LINK_STATUS, linkStatus, 0);
            // 如果链接失败，则删除着色器程序
            if (linkStatus[0] != GLES20.GL_TRUE) {
                Log.e(TAG, "createProgram error: ");
                Log.e(TAG, GLES20.glGetProgramInfoLog(program));
                GLES20.glDeleteProgram(program);
                program = 0;
            }
        }
        return program;
    }

    /**
     * 创建着色器
     *
     * @param shaderType 着色器类型： GLES20.GL_VERTEX_SHADER   GLES20.GL_FRAGMENT_SHADER
     * @param sourceCode 着色器代码
     * @return shader索引
     */
    public static int createShader(int shaderType, String sourceCode) {
        int shader = GLES20.glCreateShader(shaderType);
        if (shader != 0) {
            // 加载shader源码
            GLES20.glShaderSource(shader, sourceCode);
            // 编译shader
            GLES20.glCompileShader(shader);
            // 查看编译结果
            int[] compiled = new int[1];
            GLES20.glGetShaderiv(shader, GLES20.GL_COMPILE_STATUS, compiled, 0);
            if (compiled[0] == 0) {
                Log.e(TAG, "createShader error shaderType = " + shaderType + ": ");
                Log.e(TAG, GLES20.glGetShaderInfoLog(shader));
                shader = 0;
            }
        }
        return shader;
    }
}
