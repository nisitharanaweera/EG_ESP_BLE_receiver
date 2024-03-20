#include <Arduino.h>

#include "BLEDevice.h"
//#include "BLEScan.h"
#include <BLEUUID.h>

// Define the size of the array
const int NUM_UUIDS = 5;

// Create an array of BLEUUID objects
BLEUUID serviceUUIDs[NUM_UUIDS];

int i =0;

// static BLEUUID serviceUUID("5fafc201-1fb5-459e-8fcc-c5c9c331914b");
// String serviceUUIDStr = serviceUUIDs[i];


BLEUUID serviceUUID = serviceUUIDs[i];


static BLEUUID    s1charUUID_1("ceb5483e-36e1-4688-b7f5-ea07361b26a8");
static BLEUUID    s1charUUID_2("2c95d5e3-d8f7-413a-bf3d-7a2e5d7be87e");



// Some variables to keep track on device connected
static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;

// Define pointer for the BLE connection
static BLEAdvertisedDevice* myDevice;
BLERemoteCharacteristic* pRemoteChar_1;
BLERemoteCharacteristic* pRemoteChar_2;

// Callback function for Notify function

//-------------------------------------------------Notify block -----------------------------------------


// static void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic,
//                             uint8_t* pData,
//                             size_t length,
//                             bool isNotify) {
//   if(pBLERemoteCharacteristic->getUUID().toString() == s1charUUID_1.toString()) {

//     // convert received bytes to integer
//     uint32_t counter = pData[0];
//     for(int i = 1; i<length; i++) {
//       counter = counter | (pData[i] << i*8);
//     }

//     // print to Serial
//     Serial.print("Characteristic 1 (Notify) from server: ");
//     Serial.println(counter );  
//   }
// }
//=======================================================================================================





// Callback function that is called whenever a client is connected or disconnected
class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
  }

  void onDisconnect(BLEClient* pclient) {
    connected = false;
    Serial.println("onDisconnect");
  }
};




// Function to check Characteristic
bool connectCharacteristic(BLERemoteService* pRemoteService, BLERemoteCharacteristic* l_BLERemoteChar) {
  // Obtain a reference to the characteristic in the service of the remote BLE server.
  if (l_BLERemoteChar == nullptr) {
    Serial.print("Failed to find one of the characteristics");
    Serial.print(l_BLERemoteChar->getUUID().toString().c_str());
    return false;
  }
  Serial.println(" - Found characteristic: " + String(l_BLERemoteChar->getUUID().toString().c_str()));


//-----------------------------------------notify callback------------
  // if(l_BLERemoteChar->canNotify())
  //   l_BLERemoteChar->registerForNotify(notifyCallback);
//====================================================================
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
  pRemoteChar_1 = pRemoteService->getCharacteristic(s1charUUID_1);
  pRemoteChar_2 = pRemoteService->getCharacteristic(s1charUUID_2);
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



// Scan for BLE servers 

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  //Called for each advertising BLE server.

  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("BLE Advertised Device found: ");

    Serial.println(advertisedDevice.toString().c_str());
  
    // Check for Service UUID s
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

//-----------setting up ServiceUUID s [didn't work outside setup()]-----
  i = 0;
  serviceUUIDs[0] = BLEUUID("5fafc201-1fb5-459e-8fcc-c5c9c331914b");
  serviceUUIDs[1] = BLEUUID("43721262-a0cd-433c-a1bc-36caba2b4c59");
  serviceUUIDs[2] = BLEUUID("dde8839c-2537-4d7b-bc92-0f9b7845cce8");
  serviceUUIDs[3] = BLEUUID("c433221c-eab9-433a-9967-e54e4cfa2fba");
  serviceUUIDs[4] = BLEUUID("e97a4eb1-3f80-4344-9e98-282a9998162e");
//=======================================================================


  Serial.println("Starting BLE Client application...");
  BLEDevice::init("");

  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1500);
  pBLEScan->setWindow(500);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);
  
  
} // End of setup.

void loop() {
  Serial.println(i);
  

  serviceUUID = serviceUUIDs[i];
  

  
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
    Serial.print("Characteristic 2: ");
    Serial.println(rxValue.c_str());
    
    String txValue = String("Tx from Client");
    Serial.println("Characteristic 2 : " + txValue);
    Serial.println("");

    
    // Set the characteristic's value to be the array of bytes that is actually a string.
    // pRemoteChar_2->writeValue(txValue.c_str(), txValue.length());
    
  }else if(doScan){
    BLEDevice::getScan()->start(0); 
  }

  i++;
  if (i == 5){
    i =0;
  }

  delay(2000);
}