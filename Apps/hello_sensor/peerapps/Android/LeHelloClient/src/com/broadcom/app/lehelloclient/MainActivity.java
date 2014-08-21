/******************************************************************************
 *
 *  Copyright (C) 2013-2014 Broadcom Corporation
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/
package com.broadcom.app.lehelloclient;

import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.UUID;

import com.broadcom.app.ledevicepicker.DeviceListFragment.Callback;
import com.broadcom.app.ledevicepicker.DevicePickerFragment;
import com.broadcom.app.lehelloclient.GattUtils.RequestQueue;

import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.os.Bundle;
import android.text.Layout;
import android.text.method.ScrollingMovementMethod;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

/**
 * Main activity for the the Hello Client application
 */
public class MainActivity extends Activity implements OnClickListener, Callback,
        OnItemSelectedListener {
    private static final String TAG = Constants.TAG_PREFIX + "MainActivity";
    private static final String FRAGMENT_DEVICE_PICKER = "DevicePickerDialog";

    // Used to format timestamp for notification entries in the notification
    // view
    private static final SimpleDateFormat mFormatter = new SimpleDateFormat(
            "yyyy-MM-dd HH:mm:ss.SSS");

    /**
     * Callback object that the LE Gatt service calls to report callback events
     * that occur
     *
     * @author fredc
     *
     */
    private class GattCallback extends BluetoothGattCallback {

        /**
         * Callback invoked by Android framework and a LE connection state
         * change occurs
         */
        @Override
        public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
            Log.d(MainActivity.TAG, "onConnectionStateChange(): address=="
                    + gatt.getDevice().getAddress() + ", status = " + status + ", state="
                    + newState);
            boolean isConnected = (newState == BluetoothAdapter.STATE_CONNECTED);

            boolean isOk = (status == 0);
            if (isConnected && isOk) {
                // Discover services, and return connection state = connected
                // after services discovered
                isOk = gatt.discoverServices();
                if (isOk) {
                    return;
                }
            }

            // If we got here, this is a disconnect with or without error
            // close gatt connection
            if (!isOk) {
                gatt.close();
            }
            processConnectionStateChanged(false, !isOk);

        }

        /**
         * Callback invoked by Android framework when LE service discovery
         * completes
         */
        @Override
        public void onServicesDiscovered(BluetoothGatt gatt, int status) {
            if (status != 0) {
                // Error occurred. close the ocnnection and return a
                // disconnected status
                gatt.close();
                try {
                    processConnectionStateChanged(false, true);
                } catch (Throwable t) {
                    Log.e(TAG, "error", t);
                }
            } else {
                try {
                    processConnectionStateChanged(true, false);
                } catch (Throwable t) {
                    Log.e(TAG, "error", t);
                }
            }
        }

        /**
         * Callback invoked by Android framework when a characteristic read
         * completes
         */
        @Override
        public void onCharacteristicRead(BluetoothGatt gatt,
                BluetoothGattCharacteristic characteristic, int status) {
            if (status == 0) {
                try {
                    processCharacteristicRead(characteristic);
                } catch (Throwable t) {
                    Log.e(TAG, "error", t);
                }
            }
            mRequestQueue.next();// Execute the next queued request, if
            // any
        }

        /**
         * Callback invoked by Android framework when a descriptor read
         * completes
         */
        @Override
        public void onDescriptorRead(BluetoothGatt gatt, BluetoothGattDescriptor descriptor,
                int status) {
            if (status == 0) {
                try {
                    processDescriptorRead(descriptor);
                } catch (Throwable t) {
                    Log.e(TAG, "error", t);
                }
            }
            mRequestQueue.next();// Execute the next queued request, if
            // any
        }

        /**
         * Callback invoked by Android framework when a characteristic
         * notification occurs
         */
        @Override
        public void onCharacteristicChanged(BluetoothGatt gatt,
                BluetoothGattCharacteristic characteristic) {
            try {
                processCharacteristicNotification(characteristic);
            } catch (Throwable t) {
                Log.e(TAG, "error", t);
            }
        }

        /**
         * Callback invoked by Android framework when a descriptor write
         * completes
         */
        @Override
        public void onDescriptorWrite(BluetoothGatt gatt, BluetoothGattDescriptor descriptor,
                int status) {
            if (status == 0) {
                try {
                    processDescriptorWrite(descriptor);
                } catch (Throwable t) {
                    Log.e(TAG, "error", t);
                }
            }

            mRequestQueue.next();// Execute the next queued request, if any

        }

        /**
         * Callback invoked by Android framework when a characteristic write
         * completes
         */
        @Override
        public void onCharacteristicWrite(BluetoothGatt gatt,
                BluetoothGattCharacteristic characteristic, int status) {
            super.onCharacteristicWrite(gatt, characteristic, status);
            if (status == 0) {
                try {
                    processCharacteristicWrite(characteristic);
                } catch (Throwable t) {
                    Log.e(TAG, "error", t);
                }
            }

            mRequestQueue.next();// Execute the next queued request, if any
        }
    }

    // UI Components
    // Device picker components
    private LinearLayout mButtonSelectDevice; // Button to start device picker
    private DevicePickerFragment mDevicePicker;
    private TextView mTextDeviceName; // Displays device's name
    private TextView mTextDeviceAddress; // Displays device's address

    // Connection components
    private Button mButtonConnect; // Button to connect to a device
    private Button mButtonDisconnect; // Button to connect from a device
    private TextView mTextConnectionState; // Displays current connection state

    // Hello service components
    private TextView mLabelHelloInput;
    private Spinner mSpinnerHelloInput;
    private Button mButtonReadHelloInput;
    private TextView mLabelHelloConfiguration;
    private EditText mEditTextHelloConfiguration;
    private Button mButtonReadHelloConfiguration;
    private Button mButtonWriteHelloConfiguration;

    // Device info service components
    private TextView mTextManufacturerName; // Displays the manufacterer name
    private TextView mTextModelNumber; // Displays the model number
    private TextView mTextSystemId; // Displays the system id
    private Button mButtonReadDeviceInfo; // Button to refresh the device
                                          // information

    // Battery service components
    private TextView mTextBatteryLevel; // Displays the battery level
    private Button mButtonReadBatteryInfo; // Button to refresh the device
                                           // information

    // Notification components
    private TextView mTextAreaNotification; // Displays notifications
    private Button mButtonClear; // Button to clear the notifications

    private final GattCallback mGattCallback = new GattCallback();
    private final RequestQueue mRequestQueue = GattUtils.createRequestQueue();
    private BluetoothAdapter mBtAdapter;
    private BluetoothDevice mPickedDevice;
    private BluetoothGatt mPickedDeviceGatt;
    private boolean mPickedDeviceIsConnected;
    private boolean mSyncNotificationSetting;

    /**
     * Helper function to show a toast notification message
     *
     * @param msg
     */
    private void showMessage(String msg) {
        Toast.makeText(this, msg, Toast.LENGTH_SHORT).show();
    }

    /**
     * Check Bluetooth is available and enabled, and initialize Bluetooth
     * adapter
     *
     * @return
     */
    private boolean checkAndInitBluetooth() {
        mBtAdapter = BluetoothAdapter.getDefaultAdapter();
        if (mBtAdapter == null || !mBtAdapter.isEnabled()) {
            return false;
        }
        return true;
    }

    /**
     * Initialize the device picker
     *
     * @return
     */
    private void initDevicePicker() {
        mDevicePicker = DevicePickerFragment.createDialog(this, null, true);
    }

    /**
     * Cleanup the device picker
     */
    private void cleanupDevicePicker() {
        if (mDevicePicker != null) {
            mDevicePicker = null;
        }
    }

    private void closeDevice() {
        if (mPickedDeviceGatt != null) {
            mPickedDeviceGatt.close();
            mPickedDeviceGatt = null;
        }
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // Check bluetooth is available. If not, exit
        if (!checkAndInitBluetooth()) {
            showMessage(getString(R.string.error_bluetooth_not_available));
            finish();
            return;
        }

        // Initialize the UI components, and register a listeners
        setContentView(R.layout.main);

        // Load device picker components
        mButtonSelectDevice = (LinearLayout) findViewById(R.id.btn_selectdevice);
        mButtonSelectDevice.setOnClickListener(this);
        mTextDeviceName = (TextView) findViewById(R.id.deviceName);
        mTextDeviceAddress = (TextView) findViewById(R.id.deviceAddress);

        // Load connection components
        mButtonConnect = (Button) findViewById(R.id.btn_connect);
        mButtonConnect.setOnClickListener(this);
        mButtonDisconnect = (Button) findViewById(R.id.btn_disconnect);
        mButtonDisconnect.setOnClickListener(this);
        mTextConnectionState = (TextView) findViewById(R.id.connectionState);

        // Load hello service components
        mLabelHelloInput = (TextView) findViewById(R.id.label_hello_input);
        mLabelHelloInput.setText(getString(R.string.label_hello_input,
                Constants.HELLO_CHARACTERISTIC_INPUT_UUID));
        mSpinnerHelloInput = (Spinner) findViewById(R.id.sp_hello_input);
        mSpinnerHelloInput.setEnabled(false);

        mButtonReadHelloInput = (Button) findViewById(R.id.btn_read_hello_input);
        mButtonReadHelloInput.setOnClickListener(this);

        mLabelHelloConfiguration = (TextView) findViewById(R.id.label_hello_configuration);
        mLabelHelloConfiguration.setText(getString(R.string.label_hello_configuration,
                Constants.HELLO_CHARACTERISTIC_CONFIGURATION_UUID));
        mEditTextHelloConfiguration = (EditText) findViewById(R.id.value_hello_configuration);
        mButtonReadHelloConfiguration = (Button) findViewById(R.id.btn_read_hello_configuration);
        mButtonReadHelloConfiguration.setOnClickListener(this);
        mButtonWriteHelloConfiguration = (Button) findViewById(R.id.btn_write_hello_configuration);
        mButtonWriteHelloConfiguration.setOnClickListener(this);

        // Load device info components
        mTextManufacturerName = (TextView) findViewById(R.id.value_manufacturer_name);
        mTextModelNumber = (TextView) findViewById(R.id.value_model_number);
        mTextSystemId = (TextView) findViewById(R.id.value_system_id);
        mButtonReadDeviceInfo = (Button) findViewById(R.id.btn_read_device_info);
        mButtonReadDeviceInfo.setOnClickListener(this);

        // Load battery info components
        mTextBatteryLevel = (TextView) findViewById(R.id.value_battery_lvel);
        mButtonReadBatteryInfo = (Button) findViewById(R.id.btn_read_battery);
        mButtonReadBatteryInfo.setOnClickListener(this);

        // Load notification components
        mTextAreaNotification = (TextView) findViewById(R.id.value_notifications);
        mTextAreaNotification.setMovementMethod(ScrollingMovementMethod.getInstance());
        mButtonClear = (Button) findViewById(R.id.btn_clear);
        mButtonClear.setOnClickListener(this);

        // Initialize the device picker UI fragment
        initDevicePicker();

        // refresh the UI component states
        updateWidgets();
    }

    /**
     * Updates the UI widgets based on the latest connection state
     */
    private void updateWidgets() {
        if (mPickedDevice == null) {
            // No devices selected: set initial state
            mButtonConnect.setEnabled(false);
            mButtonDisconnect.setEnabled(false);
            mButtonSelectDevice.setEnabled(true);
            mTextDeviceName.setText(R.string.no_device);
            mTextDeviceAddress.setText("");
        } else {
            // Device picked, always set the connect/disconnect buttons enabled
            mButtonConnect.setEnabled(true);
            mButtonDisconnect.setEnabled(true);

            if (mPickedDeviceIsConnected) {
                // Set resources when connected

                // Disable selecting new device when connected
                mButtonSelectDevice.setEnabled(false);

                // Set the connection state status
                mTextConnectionState.setText(getString(R.string.connected));

                // Update hello service components
                mSpinnerHelloInput.setEnabled(true);
                mSpinnerHelloInput.setOnItemSelectedListener(this);
                mButtonReadHelloInput.setEnabled(true);
                mEditTextHelloConfiguration.setEnabled(true);
                mButtonReadHelloConfiguration.setEnabled(true);
                mButtonWriteHelloConfiguration.setEnabled(true);

                // Update device info components
                mButtonReadDeviceInfo.setEnabled(true);

                // Update battery components
                mButtonReadBatteryInfo.setEnabled(true);

                // Update notification components
                mTextAreaNotification.setEnabled(true);
                mButtonClear.setEnabled(true);

            } else {
                // Update resources when disconnected

                // Enable selecting new device when connected
                mButtonSelectDevice.setEnabled(true);

                // Set the connection state status
                mTextConnectionState.setText(getString(R.string.disconnected));

                // Update hello service components
                mSpinnerHelloInput.setEnabled(false);
                mSpinnerHelloInput.setOnItemSelectedListener(null);
                mSpinnerHelloInput.setSelection(0);
                mButtonReadHelloInput.setEnabled(false);

                mEditTextHelloConfiguration.setEnabled(false);
                mEditTextHelloConfiguration.setText("");
                mButtonReadHelloConfiguration.setEnabled(false);
                mButtonWriteHelloConfiguration.setEnabled(false);

                // Update device info components
                mButtonReadDeviceInfo.setEnabled(false);
                mTextManufacturerName.setText(R.string.no_value);
                mTextModelNumber.setText(R.string.no_value);
                mTextSystemId.setText(R.string.no_value);

                // Update battery components
                mButtonReadBatteryInfo.setEnabled(false);
                mTextBatteryLevel.setText(R.string.no_value);

                // Update notification components
                mTextAreaNotification.setEnabled(false);
                mTextAreaNotification.setText("");
                mButtonClear.setEnabled(false);
            }
        }
    }

    @Override
    public void onDestroy() {
        closeDevice();
        cleanupDevicePicker();
        super.onDestroy();
    }

    /**
     * Callback invoked when buttons/switches clicked
     */
    @Override
    public void onClick(View v) {
        if (v == mButtonSelectDevice) {
            // Start the device selector
            mDevicePicker.show(getFragmentManager(), FRAGMENT_DEVICE_PICKER);
        } else if (v == mButtonConnect) {
            // Start device connection
            connect();
        } else if (v == mButtonDisconnect) {
            // Start device disconnect
            disconnect();
        } else if (v == mButtonReadDeviceInfo) {
            // Start reading the device information characteristics
            readDeviceInfoCharacteristics();
        } else if (v == mButtonReadBatteryInfo) {
            // Start reading the battery characteristics
            readBatteryCharacteristic();
        } else if (v == mButtonReadHelloConfiguration) {
            // Start reading the hello configuration characteristics
        } else if (v == mButtonWriteHelloConfiguration) {
            // Start writing the hello configuration characteristics
            writeHelloConfigurationCharacteristic();
        } else if (v == mButtonReadHelloInput) {
            // Start reading hello input descriptors
            readHelloInputCharacteristic();
        } else if (v == mButtonClear) {
            // Clear the notification area
            mTextAreaNotification.setText("");
        }
    }

    /**
     * Called when the hello input is selected by user
     *
     * @param view
     * @param view2
     * @param pos
     * @param id
     */
    @Override
    public void onItemSelected(AdapterView<?> view, View view2, int pos, long id) {
        if (view == mSpinnerHelloInput) {
            // Update the input characteristic on the device
            writeHelloInputCharacteristic();
        }
    }

    /**
     * Called when no hello input is selected
     */
    @Override
    public void onNothingSelected(AdapterView<?> view) {
    }

    /**
     * Callback invoked when a device was picked from the device picker
     *
     * @param device
     */
    @Override
    public void onDevicePicked(BluetoothDevice device) {
        Log.d(TAG, "onDevicePicked: " + device == null ? "" : device.getAddress());
        // Close any outstanding connections to remote devices
        closeDevice();

        // Get the remote device object
        String address = device.getAddress();
        mPickedDevice = mBtAdapter.getRemoteDevice(address);

        // Get the name
        String name = mPickedDevice.getName();
        if (name == null || name.isEmpty()) {
            name = address;
        }

        // Set UI resources
        mTextDeviceName.setText(name);
        mTextDeviceAddress.setText(address);
        // Update the connect widget
        mButtonConnect.setEnabled(true);
        mButtonDisconnect.setEnabled(true);
    }

    /**
     * Callback invoked when a devicepicker was dismissed without a device
     * picked
     */
    @Override
    public void onDevicePickError() {
        Log.d(TAG, "onDevicePickError");
    }

    /**
     * Callback invoked when a devicepicker encountered an unexpected error
     */
    @Override
    public void onDevicePickCancelled() {
        Log.d(TAG, "onDevicePickCancelled");
    }

    /**
     * Connect to the picked device
     */
    private void connect() {
        if (mPickedDevice == null) {
            showMessage(getString(R.string.error_connect, mPickedDevice.getName(),
                    mPickedDevice.getAddress()));
            return;
        }

        mPickedDeviceGatt = mPickedDevice.connectGatt(this, false, mGattCallback);

        if (mPickedDeviceGatt == null) {
            showMessage(getString(R.string.error_connect, mPickedDevice.getName(),
                    mPickedDevice.getAddress()));
        }
    }

    /**
     * Disconnects the picked device
     */
    private void disconnect() {
        if (mPickedDeviceGatt != null) {
            mPickedDeviceGatt.disconnect();
            closeDevice();
        }
    }

    /**
     * Called when a gatt connection state changes. This function updates the UI
     *
     * @param gatt
     */
    private void processConnectionStateChanged(final boolean isConnected, final boolean hasError) {
        runOnUiThread(new Runnable() {

            @Override
            public void run() {
                if (hasError) {
                    showMessage(getString(R.string.error_connect, mPickedDevice.getName(),
                            mPickedDevice.getAddress()));
                }
                mPickedDeviceIsConnected = isConnected;
                updateWidgets();

                // Refresh the device information
                if (mPickedDeviceIsConnected) {
                    mSyncNotificationSetting = true;
                    readEverything();
                }
            }
        });

    }

    /**
     * Reads the device info characteristics and updates the UI components
     */
    private void readDeviceInfoCharacteristics() {
        // Get all readable characteristics and descriptors of interest and add
        // request to a request queue

        BluetoothGattCharacteristic characteristic = null;

        // Get model number
        characteristic = GattUtils.getCharacteristic(mPickedDeviceGatt,
                Constants.DEVICE_INFO_SERVICE_UUID, Constants.MODEL_NUMBER_UUID);
        mRequestQueue.addReadCharacteristic(mPickedDeviceGatt, characteristic);

        // Get manufacturer name
        characteristic = GattUtils.getCharacteristic(mPickedDeviceGatt,
                Constants.DEVICE_INFO_SERVICE_UUID, Constants.MANUFACTURER_NAME_UUID);
        mRequestQueue.addReadCharacteristic(mPickedDeviceGatt, characteristic);

        // Get system Id
        characteristic = GattUtils.getCharacteristic(mPickedDeviceGatt,
                Constants.DEVICE_INFO_SERVICE_UUID, Constants.SYSTEM_ID_UUID);
        mRequestQueue.addReadCharacteristic(mPickedDeviceGatt, characteristic);
        mRequestQueue.execute();
    }

    /**
     * Reads the battery characteristics and updates the UI components
     */
    private void readBatteryCharacteristic() {
        // Get all readable characteristics and descriptors of interest and add
        // request to a request queue

        BluetoothGattCharacteristic characteristic = null;
        // Get battery level
        characteristic = GattUtils.getCharacteristic(mPickedDeviceGatt,
                Constants.BATTERY_SERVICE_UUID, Constants.BATTERY_LEVEL_UUID);
        mRequestQueue.addReadCharacteristic(mPickedDeviceGatt, characteristic);
        mRequestQueue.execute();
    }

    /**
     * Reads the hello input characteristic and updates the UI components
     */
    private void readHelloInputCharacteristic() {
        // Get all readable characteristics and descriptors of interest and add
        // request to a request queue
        BluetoothGattDescriptor descriptor = null;

        // Get client config descriptor: enable/disable notification
        descriptor = GattUtils.getDescriptor(mPickedDeviceGatt, Constants.HELLO_SERVICE_UUID,
                Constants.HELLO_CHARACTERISTIC_INPUT_UUID, Constants.CLIENT_CONFIG_DESCRIPTOR_UUID);
        mRequestQueue.addReadDescriptor(mPickedDeviceGatt, descriptor);
        mRequestQueue.execute();
    }

    /**
     * Reads the manufacturer name, model number, system id, battery level from
     * the device and updates the UI
     */
    private void readHelloConfigurationCharacteristic() {
        BluetoothGattCharacteristic characteristic = null;
        characteristic = GattUtils.getCharacteristic(mPickedDeviceGatt,
                Constants.HELLO_SERVICE_UUID, Constants.HELLO_CHARACTERISTIC_CONFIGURATION_UUID);
        mRequestQueue.addReadCharacteristic(mPickedDeviceGatt, characteristic);
        mRequestQueue.execute();
    }

    /**
     * Read every characteristic on the device
     */
    private void readEverything() {
        readDeviceInfoCharacteristics();
        readBatteryCharacteristic();
        readHelloConfigurationCharacteristic();
        readHelloInputCharacteristic();
    }

    /**
     * Write the hello configuration characteristic to the device
     */
    private void writeHelloConfigurationCharacteristic() {
        BluetoothGattCharacteristic characteristic = null;

        int blinks = 0;
        try {
            blinks = Integer.parseInt(mEditTextHelloConfiguration.getText().toString());
            characteristic = GattUtils
                    .getCharacteristic(mPickedDeviceGatt, Constants.HELLO_SERVICE_UUID,
                            Constants.HELLO_CHARACTERISTIC_CONFIGURATION_UUID);
            characteristic.setValue(blinks, BluetoothGattCharacteristic.FORMAT_UINT8, 0);
            mRequestQueue.addWriteCharacteristic(mPickedDeviceGatt, characteristic);
        } catch (Throwable t) {
            Log.w(TAG, "invalid number of notifications");
            mEditTextHelloConfiguration.setText("0");
        }
        mRequestQueue.addWriteCharacteristic(mPickedDeviceGatt, characteristic);
        mRequestQueue.execute();
    }

    /**
     * Write the hello input descriptor to the device
     */
    private void writeHelloInputCharacteristic() {
        // Set the enable/disable notification settings
        BluetoothGattCharacteristic notifyCharacteristic = GattUtils.getCharacteristic(
                mPickedDeviceGatt, Constants.HELLO_SERVICE_UUID,
                Constants.HELLO_CHARACTERISTIC_INPUT_UUID);
        int notificationState = mSpinnerHelloInput.getSelectedItemPosition();
        if (notificationState >= 0 && notificationState <= 2) {
            BluetoothGattDescriptor descriptor = GattUtils.getDescriptor(mPickedDeviceGatt,
                    Constants.HELLO_SERVICE_UUID, Constants.HELLO_CHARACTERISTIC_INPUT_UUID,
                    Constants.CLIENT_CONFIG_DESCRIPTOR_UUID);
            if (notificationState > 0) {
                mPickedDeviceGatt.setCharacteristicNotification(notifyCharacteristic, true);
                if (notificationState == 1) {
                    descriptor.setValue(BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);
                } else if (notificationState == 2) {
                    descriptor.setValue(BluetoothGattDescriptor.ENABLE_INDICATION_VALUE);
                }
            } else {
                mPickedDeviceGatt.setCharacteristicNotification(notifyCharacteristic, false);
                descriptor.setValue(BluetoothGattDescriptor.DISABLE_NOTIFICATION_VALUE);
            }
            mRequestQueue.addWriteDescriptor(mPickedDeviceGatt, descriptor);
            mRequestQueue.execute();
        }
    }

    /**
     * Callback invoked by the Android framework when a read characteristic
     * successfully completes
     *
     * @param characteristic
     */
    private void processCharacteristicRead(final BluetoothGattCharacteristic characteristic) {
        runOnUiThread(new Runnable() {

            @Override
            public void run() {
                UUID uuid = characteristic.getUuid();
                if (Constants.MANUFACTURER_NAME_UUID.equals(uuid)) {
                    mTextManufacturerName.setText(characteristic.getStringValue(0));
                } else if (Constants.MODEL_NUMBER_UUID.equals(uuid)) {
                    mTextModelNumber.setText(characteristic.getStringValue(0));
                } else if (Constants.SYSTEM_ID_UUID.equals(uuid)) {
                    byte[] systemIdBytes = characteristic.getValue();
                    long systemIdLong = GattUtils.unsignedBytesToLong(systemIdBytes, 8, 0);
                    long manuId = 0xFFFFFFFFFFL & systemIdLong; // 40bits
                    long orgId = 0xFFFFFFL & (systemIdLong >> 40);
                    String manuIdString = String.format("%010X", manuId);
                    String orgIdString = String.format("%06X", orgId);
                    mTextSystemId.setText(orgIdString + " " + manuIdString);
                } else if (Constants.BATTERY_LEVEL_UUID.equals(uuid)) {
                    int batteryLevel = characteristic.getIntValue(
                            BluetoothGattCharacteristic.FORMAT_UINT8, 0);
                    mTextBatteryLevel.setText(String.valueOf(batteryLevel));
                } else if (Constants.HELLO_CHARACTERISTIC_CONFIGURATION_UUID.equals(uuid)) {
                    int blinks = characteristic.getIntValue(
                            BluetoothGattCharacteristic.FORMAT_UINT8, 0);
                    mEditTextHelloConfiguration.setText(String.valueOf(blinks));
                }
            }
        });
    }

    /**
     * Callback invoked by the Android framework when a read descriptor
     * successfully completes
     *
     * @param descriptor
     */
    private void processDescriptorRead(final BluetoothGattDescriptor descriptor) {
        runOnUiThread(new Runnable() {

            @Override
            public void run() {
                UUID uuid = descriptor.getUuid();
                if (Constants.CLIENT_CONFIG_DESCRIPTOR_UUID.equals(uuid)) {
                    byte[] descriptorBytes = descriptor.getValue();
                    int notificationState = (int) GattUtils.unsignedBytesToLong(descriptorBytes, 2,
                            0);
                    Log.d(TAG, "notificationState = " + notificationState);
                    if (notificationState >= 0 && notificationState <= 2) {
                        // Temporarily disable the spinner listener when we set
                        // the selection
                        mSpinnerHelloInput.setOnItemSelectedListener(null);
                        mSpinnerHelloInput.setSelection(notificationState);
                        // Re-enable the spinner listener
                        mSpinnerHelloInput.setOnItemSelectedListener(MainActivity.this);
                    }
                    if (mSyncNotificationSetting && notificationState > 0) {
                        // On initial connection, mSyncNotificationSetting is
                        // set to indicate that we should set characteristic
                        // notification if the descriptor is set
                        mSyncNotificationSetting = false;
                        BluetoothGattCharacteristic notifyCharacteristic = GattUtils
                                .getCharacteristic(mPickedDeviceGatt, Constants.HELLO_SERVICE_UUID,
                                        Constants.HELLO_CHARACTERISTIC_INPUT_UUID);
                        mPickedDeviceGatt.setCharacteristicNotification(notifyCharacteristic, true);
                    }
                }
            }
        });

    }

    /**
     * Callback invoked by the Android framework when a write descriptor
     * successfully completes
     *
     * @param descriptor
     */
    private void processDescriptorWrite(final BluetoothGattDescriptor descriptor) {
        runOnUiThread(new Runnable() {

            @Override
            public void run() {
                readEverything();
            }
        });
    }

    /**
     * Callback invoked by the Android framework when a write characteristic
     * successfully completes
     *
     * @param characteristic
     */
    private void processCharacteristicWrite(final BluetoothGattCharacteristic characteristic) {
        runOnUiThread(new Runnable() {

            @Override
            public void run() {
                readEverything();
            }
        });
    }

    /**
     * Callback invoked by the Android framework when a characteristic
     * notification is received
     *
     * @param characteristic
     */
    private void processCharacteristicNotification(final BluetoothGattCharacteristic characteristic) {
        runOnUiThread(new Runnable() {

            @Override
            public void run() {
                String s = characteristic.getStringValue(0);

                // Write a entry to the log viewer: "timestamp value"
                mTextAreaNotification.append(mFormatter.format(Calendar.getInstance().getTime()));
                mTextAreaNotification.append(" ");
                mTextAreaNotification.append(s);
                mTextAreaNotification.append("\n");

                // Auto scroll to the bottom
                Layout l = mTextAreaNotification.getLayout();
                if (l != null) {
                    int scrollAmount = l.getLineTop(mTextAreaNotification.getLineCount())
                            - mTextAreaNotification.getHeight();
                    // if there is no need to scroll, scrollAmount will be <=0
                    if (scrollAmount > 0)
                        mTextAreaNotification.scrollTo(0, scrollAmount);
                    else
                        mTextAreaNotification.scrollTo(0, 0);
                }
            }
        });
    }

}
