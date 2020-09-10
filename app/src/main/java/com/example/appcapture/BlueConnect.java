package com.example.appcapture;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.util.Log;

import java.io.IOException;
import java.io.OutputStream;
import java.util.UUID;

public class BlueConnect {

    // Atributos.
    private static String btserveraddress;
    private static UUID uuid;

    private BluetoothDevice device;
    private BluetoothAdapter btAdapter;
    private BluetoothSocket btSocket;

    private OutputStream outStream;
    private boolean conexion;

    // Constructor.
    public BlueConnect(String btserveraddress, String uuid){
        this.btserveraddress = btserveraddress;
        this.uuid = UUID.fromString(uuid);
        btAdapter = null;
        btSocket = null;
        OutputStream outStream = null;
        conexion = false;
    }

    // Método de envio texto.
    public boolean writetostream(String mensaje){
        if(outStream != null && conexion == true) {
            try {
                byte[] msgBffr = mensaje.getBytes();
                outStream.write(msgBffr);
                Log.d("Bluetooth", "OK: mensaje enviado");
                return true;
            } catch (IOException e) {
                Log.d("Bluetooth", "Error: mensaje no enviado");
            }
        }
        return false;
    }
    // Método de envio texto.
    public boolean writetostreambytes(byte[] byteSend){
        if(outStream != null && conexion == true) {
            try {
                byte[] msgBffr = byteSend;
                outStream.write(msgBffr);
                Log.d("Bluetooth", "OK: mensaje enviado");
                return true;
            } catch (IOException e) {
                Log.d("Bluetooth", "Error: mensaje no enviado");
            }
        }
        return false;
    }

    // Métodos de conexión.
    public boolean conectar(){
        btAdapter = BluetoothAdapter.getDefaultAdapter();
        device = btAdapter.getRemoteDevice(btserveraddress);
        Log.d("Bluetooth","Conectando...");

        if(createSocket()){
            if(createConnect()){
                if(createBridge()){
                    if(outStream != null) {
                        conexion = true;
                        Log.d("Bluetooth","OK: Conexión establecida. Listo para enviar datos");
                        return true;
                    }else{
                        Log.d("Bluetooth","Error: Conexión NO establecida. outStream es NULL");
                        return false;
                    }
                }else{
                    conexion = false;
                    return false;
                }
            }else{
                conexion = false;
                return false;
            }
        }else{
            conexion = false;
            return false;
        }
    }
    private boolean createSocket(){
        try {
            btSocket = device.createRfcommSocketToServiceRecord(uuid);
            Log.d("Bluetooth","OK: Socket de conexión");
            btAdapter.cancelDiscovery();
            return true;
        } catch(IOException e) {
            Log.d("Bluetooth","Error: Socket de conexión");
        }
        btAdapter.cancelDiscovery();
        return false;
    }
    private boolean createConnect(){
        try{
            btSocket.connect();
            Log.d("Bluetooth","OK: Se pudo crear conexión btSocket");
            return true;
        } catch(IOException e){
            Log.d("Bluetooth","Error: No se pudo crear conexión btSocket");
        }
        return false;
    }
    private boolean createBridge(){
        try{
            outStream = btSocket.getOutputStream();
            Log.d("Bluetooth","OK: Se puedo crear un outputstream");
            return true;
        } catch(IOException e){
            Log.d("Bluetooth","Error: No se puedo crear un outputstream");
        }
        return false;
    }

    // Método desconexión.
    public boolean closeConnection(){
        try {
            outStream.close();
            btSocket.close();
            return true;
        } catch (IOException e) {
            e.printStackTrace();
        }
        return false;
    }
}