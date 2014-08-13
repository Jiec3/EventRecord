
package com.syouquan.script;

import android.util.Log;

/**
 * 描述:脚本引擎
 * 
 * @author chenys
 * @since 2014-7-16 下午2:33:37
 */
public class ScriptEngine {

    /**
     * 初始化输入设备
     * 
     * @return 格式
     *         {infos:[{name:"/dev/input/event0",class:"1"},{name:"/dev/input/event4"
     *         ,class:"9"}]}
     */
    public static native String naitveInitDevice();

    /**
     * 回放
     * 
     * @param deviceName
     * @param secTime
     * @param msTime
     * @param type
     * @param code
     * @param value
     */
    public static native void nativeReplay(int deviceName, int secTime, int msTime, int type,
            int code, int value);

    /**
     * 根据类型，获取设备名称
     * 
     * @param classId
     * @return 格式{devices:[{name:"/dev/input/event3"}]}
     */
    public static native String nativeGetDeviceName(int classId);

    /**
     * 判断某个设备是否某个code
     * 
     * @param code
     * @param deviceName
     * @return 1支持，0不支持
     */
    public static native int nativeIsCodeInDevice(int code, String deviceName);

    /**
     * 保存事件
     * 
     * @param secTime
     * @param msTime
     * @param type
     * @param code
     * @param value
     */
    public void saveEventItem(String deviceName, int secTime, int msTime, int type, int code,
            int value) {
        // 保存事件，由so调用
        Log.e("test", "java log event " + deviceName + " " + secTime + "." + msTime + " " + type
                + " " + code + " " + value);
    }

    /**
     * 录制事件结束回调
     */
    public void onRecordStopCallBack() {

    }

    /**
     * 开始事件录制
     */
    public static native void nativeEventStartRecord();

    /**
     * 停止事件录制
     */
    public static native void nativeEventStopRecord();

    /**
     * 开始事件回放
     */
    public static native void nativeEventStartReplay();

    /**
     * 停止事件回放
     */
    public static native void nativeEventStopReplay();
}
