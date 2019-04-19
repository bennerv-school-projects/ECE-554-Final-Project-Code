package com.bennerv.ebike_dashboard;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothServerSocket;
import android.bluetooth.BluetoothSocket;
import android.content.Intent;
import android.os.ParcelUuid;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.Switch;
import android.widget.Toast;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Set;
import java.util.UUID;

public class MainActivity extends AppCompatActivity {

    private final String TAG = MainActivity.class.getSimpleName();
    private static final String DESIRED_DEVICE_ADDRESS = "00:14:03:06:55:43";
    private static final int REQUEST_ENABLE_BT = 69;

    private BluetoothSocket bluetoothSocket = null;
    private OutputStream outputStream = null;
    private InputStream inputStream = null;
    private BluetoothAdapter mBluetoothAdapter;

    private Switch HAZARDS_SWITCH = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        HAZARDS_SWITCH = findViewById(R.id.hazardsSwitch);

        HAZARDS_SWITCH.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                sendHazardsViaBluetooth(isChecked);
            }
        });
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (bluetoothSocket == null || inputStream == null || outputStream == null) {
            initializeBluetooth();
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);

        // Bluetooth was enabled so start the pairing process
        if (requestCode == REQUEST_ENABLE_BT && resultCode == RESULT_OK) {
            initializeBluetooth();
        }
    }

    private void initializeBluetooth() {
        mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();

        if (mBluetoothAdapter == null) {
            Toast.makeText(this, "Device doesn't support bluetooth", Toast.LENGTH_SHORT).show();
            return;
        }

        if (!mBluetoothAdapter.isEnabled()) {
            // Start bluetooth as it's disabled right now
            Intent enableBluetooth = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(enableBluetooth, REQUEST_ENABLE_BT);
            return;
        }

        Set<BluetoothDevice> pairedDevices = mBluetoothAdapter.getBondedDevices();

        if (pairedDevices.size() == 0) {
            Toast.makeText(this, "No paired devices on your phone", Toast.LENGTH_SHORT).show();
            return;
        }


        // Connect as a client
        for (BluetoothDevice device : pairedDevices) {
            if (device.getAddress().equalsIgnoreCase(DESIRED_DEVICE_ADDRESS)) {
                Toast.makeText(this, "Found the device we're looking for", Toast.LENGTH_SHORT).show();

                // Get device UUID
                ParcelUuid[] uuids = device.getUuids();
                Log.i(TAG, "Trying to get on uuid " + uuids[0]);

                // Try to open a bluetooth socket
                try {
                    bluetoothSocket = device.createRfcommSocketToServiceRecord(uuids[0].getUuid());
                    bluetoothSocket.connect();
                    outputStream = bluetoothSocket.getOutputStream();
                    inputStream = bluetoothSocket.getInputStream();
                } catch (IOException e) {
                    Log.e(TAG, "Failed to create bluetooth socket", e);
                }
            }
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        try {
            if (inputStream != null) {
                inputStream.close();
                inputStream = null;
            }
        } catch (IOException e) {
            Log.i(TAG, "Failed to close input stream");
        }

        try {
            if (outputStream != null) {
                outputStream.close();
                outputStream = null;
            }
        } catch (IOException e) {
            Log.i(TAG, "Failed to output stream");
        }

        try {
            if (bluetoothSocket != null) {
                bluetoothSocket.close();
                bluetoothSocket = null;
            }
        } catch (IOException e) {
            Log.i(TAG, "Failed to close bluetooth socket");
        }
    }

    public void sendViaBluetooth(String message) {

        if (outputStream == null) {
            Toast.makeText(this, "No bluetooth device connected", Toast.LENGTH_SHORT).show();
            return;
        }

        WriteThread writeThread = new WriteThread(message);
        writeThread.start();
    }

    public void sendHazardsViaBluetooth(boolean isChecked) {
        sendViaBluetooth(isChecked ? "1" : "0");
    }

    private class WriteThread extends Thread {
        private String message;

        public WriteThread(String message) {
            this.message = message;
        }

        public void run() {
            try {
                outputStream.write(message.getBytes());
            } catch (IOException e) {
                Log.e(TAG, "failed to write", e);
            }
        }
    }
}



