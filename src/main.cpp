/*
    BLE_MIDI Example by neilbags 
    https://github.com/neilbags/arduino-esp32-BLE-MIDI
    
    Based on BLE_notify example by Evandro Copercini.

    Creates a BLE MIDI service and characteristic.
    Once a client subscibes, send a MIDI message every 2 seconds
*/
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <JC_Button_ESP.h>

#define SERVICE_UUID "03b80e5a-ede8-4b33-a751-6ce34ec4c700"
#define CHARACTERISTIC_UUID "7772e5db-3868-4112-a1a9-f2669d106bf3"

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

const byte
    BUTTON_00(32), // connect a button switch from this pin to ground
    BUTTON_01(33),
    BUTTON_02(27); // the standard Arduino "pin 13" LED

Button btn00(BUTTON_00); // define the button
Button btn01(BUTTON_01); // define the button
Button btn02(BUTTON_02); // define the button

uint8_t ccPacket[] = {
    0x80, // header
    0x80, // timestamp, not implemented
    0xB0, // status
    0x00, // 0x3c == 60 == middle c
    0x00  // velocity
};

uint8_t pcPacket[] = {
    0x80, // header
    0x80, // timestamp, not implemented
    0xC0, // status
    0x00};

class MyServerCallbacks : public BLEServerCallbacks
{
  void onConnect(BLEServer *pServer)
  {
    deviceConnected = true;
  };

  void onDisconnect(BLEServer *pServer)
  {
    deviceConnected = false;
  }
};

void bleCCsend(uint8_t control, uint8_t value, uint8_t channel)
{
  if (value < 0)
  {
    value = 0;
  }
  if (value > 127)
  {
    value = 127;
  }
  uint8_t status = 175 + channel;
  ccPacket[2] = status;                     // note up, channel 0
  ccPacket[3] = control;                       //??
  ccPacket[4] = value;                      // velocity
  pCharacteristic->setValue(ccPacket, 5); // packet, length in bytes)
  pCharacteristic->notify();
}

void blePCsend(uint8_t program, uint8_t channel)
{

  uint8_t status = 191 + channel;
  pcPacket[2] = status;
  pcPacket[3] = program;
  pCharacteristic->setValue(pcPacket, 4);
  pCharacteristic->notify();
}

void setup()
{
  Serial.begin(115200);

  BLEDevice::init("BM1000");

  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(BLEUUID(SERVICE_UUID));

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
      BLEUUID(CHARACTERISTIC_UUID),
      BLECharacteristic::PROPERTY_READ |
          BLECharacteristic::PROPERTY_WRITE |
          BLECharacteristic::PROPERTY_NOTIFY |
          BLECharacteristic::PROPERTY_WRITE_NR);

  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor
  pCharacteristic->addDescriptor(new BLE2902());

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->addServiceUUID(pService->getUUID());
  pAdvertising->start();

  btn00.begin();
  btn01.begin();
  btn02.begin();
}

void loop()
{

  //bleCCsend(20, 127);

  //delay(500);

  //blePCsend(6, 2);

  //delay(500);

  if (deviceConnected)
  {
    btn00.read();
    btn01.read();
    btn02.read();

    if (btn00.wasPressed())
    {
      blePCsend(9, 1);
      //Serial.println("0 pressed");
    }

    if (btn01.wasPressed())
    {
      blePCsend(11, 11);
      //Serial.println("1 pressed");
    }
    if (btn02.wasPressed())
    {
      bleCCsend(45, 127, 16);
      //Serial.println("1 pressed");
    }


  }
}
