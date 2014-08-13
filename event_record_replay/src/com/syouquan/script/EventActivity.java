
package com.syouquan.script;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

import com.example.demo_native_event.R;

public class EventActivity extends Activity {

    private static Activity sActivity;

    private EditText mCodeET = null;

    private EditText mDeviceET = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        sActivity = this;

        ShellUtils.execCommand("chmod 777 /dev/input/event*", true);

        Log.e("test", "device info = " + ScriptEngine.naitveInitDevice());

        initBtn();
        initEditText();

    }

    private void initBtn() {
        final Button btn1 = (Button) findViewById(R.id.button1);
        btn1.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View arg0) {
                new Thread(new Runnable() {

                    @Override
                    public void run() {
                        ScriptEngine.nativeEventStartRecord();
                    }
                }).start();
            }
        });

        final Button btn2 = (Button) findViewById(R.id.button2);
        btn2.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View arg0) {
                ScriptEngine.nativeEventStopRecord();
            }
        });

        Button btn3 = (Button) findViewById(R.id.button3);
        btn3.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View arg0) {
                new Thread(new Runnable() {
                    public void run() {
                        ScriptEngine.nativeEventStartReplay();
                    }
                }).start();
            }
        });

        Button btn4 = (Button) findViewById(R.id.button4);
        btn4.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View arg0) {
                ScriptEngine.nativeEventStopReplay();
            }
        });

        Button btn5 = (Button) findViewById(R.id.button5);
        btn5.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View arg0) {
            }
        });

        Button btn6 = (Button) findViewById(R.id.button6);
        btn6.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View arg0) {
                checkCode();
            }
        });
    }

    void checkCode() {
        int code = Integer.valueOf(mCodeET.getText().toString());
        String device = "/dev/input/event" + Integer.valueOf(mDeviceET.getText().toString());
        if (ScriptEngine.nativeIsCodeInDevice(code, device) == 1) {
            Toast.makeText(this, "device /dev/input/event" + device + " have code " + code,
                    Toast.LENGTH_LONG).show();
        } else {
            Toast.makeText(this, "device /dev/input/event" + device + " have not code " + code,
                    Toast.LENGTH_LONG).show();
        }
    }

    void initEditText() {
        mCodeET = (EditText) findViewById(R.id.editText1);
        mDeviceET = (EditText) findViewById(R.id.editText2);
    }

    public static Activity theActivity() {
        return sActivity;
    }

    static {
        System.loadLibrary("event");
    }

}
