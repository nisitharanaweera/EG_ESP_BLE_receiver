#include <Arduino.h>

#include "BLEDevice.h"
//#include "BLEScan.h"

// Define UUIDs:
static BLEUUID serviceUUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
static BLEUUID    charUUID_1("beb5483e-36e1-4688-b7f5-ea07361b26a8");
static BLEUUID    charUUID_2("1c95d5e3-d8f7-413a-bf3d-7a2e5d7be87e");

// Some variables to keep track on device connected
static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;

// Define pointer for the BLE connection
static BLEAdvertisedDevice* myDevice;
BLERemoteCharacteristic* pRemoteChar_1;
BLERemoteCharacteristic* pRemoteChar_2;

// Callback function for Notify function
static void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic,
                            uint8_t* pData,
                            size_t length,
                            bool isNotify) {
  if(pBLERemoteCharacteristic->getUUID().toString() == charUUID_1.toString()) {

    // convert received bytes to integer
    uint32_t counter = pData[0];
    for(int i = 1; i<length; i++) {
      counter = counter | (pData[i] << i*8);
    }

    // print to Serial
    Serial.print("Characteristic 1 (Notify) from server: ");
    Serial.println(counter );  
  }
}

// Callback function that is called whenever a client is connected or disconnected
class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
  }

  void onDisconnect(BLEClient* pclient) {
    connected = false;
    Serial.println("onDisconnect");
  }
};
// Function to chech Characteristic
bool connectCharacteristic(BLERemoteService* pRemoteService, BLERemoteCharacteristic* l_BLERemoteChar) {
  // Obtain a reference to the characteristic in the service of the remote BLE server.
  if (l_BLERemoteChar == nullptr) {
    Serial.print("Failed to find one of the characteristics");
    Serial.print(l_BLERemoteChar->getUUID().toString().c_str());
    return false;
  }
  Serial.println(" - Found characteristic: " + String(l_BLERemoteChar->getUUID().toString().c_str()));

  if(l_BLERemoteChar->canNotify())
    l_BLERemoteChar->registerForNotify(notifyCallback);

  return true;  
}

// Function that is run if server is connected
bool connectToServer() {
  Serial.print("Forming a connection to ");
  Serial.println(myDevice->getAddress().toString().c_str());
  
  BLEClient*  pClient  = BLEDevice::createClient();
  Serial.println(" - Created client");

  pClient->setClientCallbacks(new MyClientCallback());

  // Connect to the remove BLE Server.
  pClient->connect(myDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
  Serial.println(" - Connected to server");

  // Obtain a reference to the remote BLE server.
  BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
  if (pRemoteService == nullptr) {
    Serial.print("Failed to find the service UUID: ");
    Serial.println(serviceUUID.toString().c_str());
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Found service");

  connected = true;
  pRemoteChar_1 = pRemoteService->getCharacteristic(charUUID_1);
  pRemoteChar_2 = pRemoteService->getCharacteristic(charUUID_2);
  if(connectCharacteristic(pRemoteService, pRemoteChar_1) == false)
    connected = false;
  else if(connectCharacteristic(pRemoteService, pRemoteChar_2) == false)
    connected = false;

  if(connected == false) {
    pClient-> disconnect();
    Serial.println("At least one characteristic UUID not found");
    return false;
  }
  return true;
}



// Scan for BLE servers and find the first one that advertises the service we are looking for.
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  //Called for each advertising BLE server.
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("BLE Advertised Device found: ");
    Serial.println(advertisedDevice.toString().c_str());
  
    // We have found a device, let us now see if it contains the service we are looking for.
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {
  
      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan = true;
  
    } 
  } // onResult
}; 

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE Client application...");
  BLEDevice::init("");

  
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);
} // End of setup.

void loop() {

  
  if (doConnect == true) {
    if (connectToServer()) {
      Serial.println("Connected to the BLE Server...");
    } else {
      Serial.println("Failed to connect to the server...");
    }
    doConnect = false;
  }

  if (connected) {
    std::string rxValue = pRemoteChar_2->readValue();
    Serial.print("Characteristic 2 (readValue): ");
    Serial.println(rxValue.c_str());
    
    String txValue = String(-random(1000));
    Serial.println("Characteristic 2 (writeValue): " + txValue);
    
    // Set the characteristic's value to be the array of bytes that is actually a string.
    pRemoteChar_2->writeValue(txValue.c_str(), txValue.length());
    
  }else if(doScan){
    BLEDevice::getScan()->start(0); 
  }

  
  delay(5000);
}