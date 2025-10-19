package com.example.ledcontrollerv2;

import android.Manifest;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.widget.Button;
import android.widget.SeekBar;
import android.widget.Switch;
import android.widget.Toast;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import java.io.IOException;
import java.io.OutputStream;
import java.util.UUID;

public class MainActivity extends AppCompatActivity {
    private BluetoothAdapter bluetoothAdapter;
    private BluetoothSocket bluetoothSocket;
    private OutputStream outputStream;
    private static final String DEVICE_ADDRESS = "98:D3:31:FB:48:E2";
    private static final UUID MY_UUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        
        Button connectBtn = findViewById(R.id.connectBtn);
        Switch soundSwitch = findViewById(R.id.soundSwitch);
        SeekBar brightnessSeek = findViewById(R.id.brightnessSeek);
        Button allBtn = findViewById(R.id.allBtn);
        Button leftBtn = findViewById(R.id.leftBtn);
        Button rightBtn = findViewById(R.id.rightBtn);
        Button centerBtn = findViewById(R.id.centerBtn);

        connectBtn.setOnClickListener(v -> connectBluetooth());
        
        soundSwitch.setOnCheckedChangeListener((buttonView, isChecked) -> {
            sendCommand(isChecked ? "SOUND_ON" : "SOUND_OFF");
        });
        
        brightnessSeek.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                if (fromUser) {
                    sendCommand("BRIGHTNESS:" + progress);
                }
            }
            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {}
            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {}
        });

        allBtn.setOnClickListener(v -> sendCommand("DIRECTION:0"));
        leftBtn.setOnClickListener(v -> sendCommand("DIRECTION:1"));
        rightBtn.setOnClickListener(v -> sendCommand("DIRECTION:2"));
        centerBtn.setOnClickListener(v -> sendCommand("DIRECTION:3"));
    }

    private void connectBluetooth() {
        if (ActivityCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.BLUETOOTH_CONNECT}, 1);
            return;
        }
        
        BluetoothDevice device = bluetoothAdapter.getRemoteDevice(DEVICE_ADDRESS);
        try {
            bluetoothSocket = device.createRfcommSocketToServiceRecord(MY_UUID);
            bluetoothSocket.connect();
            outputStream = bluetoothSocket.getOutputStream();
            Toast.makeText(this, "Conectado", Toast.LENGTH_SHORT).show();
        } catch (IOException e) {
            Toast.makeText(this, "Error de conexión", Toast.LENGTH_SHORT).show();
        }
    }

    private void sendCommand(String command) {
        if (outputStream != null) {
            try {
                outputStream.write(command.getBytes());
                outputStream.flush();
            } catch (IOException e) {
                Toast.makeText(this, "Error enviando comando", Toast.LENGTH_SHORT).show();
            }
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        try {
            if (bluetoothSocket != null) bluetoothSocket.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
