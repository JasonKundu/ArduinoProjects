#include <ArduinoBLE.h>

BLEService ledService("19B10000-E8F2-537E-4F6C-D104768A1214"); // Bluetooth® Low Energy LED Service

// Bluetooth® Low Energy characteristics
BLEByteCharacteristic switchCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite);
BLEStringCharacteristic colorCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1215", BLERead | BLEWrite, 11);

const int PIN_RED = 3;
const int PIN_GREEN = 5;
const int PIN_BLUE = 6;


void setLED(String rgbString) {
  int sep_1 = rgbString.indexOf(',');
  int sep_2 = rgbString.indexOf(',', (sep_1 + 1));
  int led_r = 255 - rgbString.substring(0, sep_1).toInt();    // Invert the color values for anode LED
  int led_g = 255 - rgbString.substring(sep_1 + 1, sep_2).toInt();
  int led_b = 255 - rgbString.substring(sep_2 + 1).toInt();

  analogWrite(PIN_RED, led_r);
  analogWrite(PIN_GREEN, led_g);
  analogWrite(PIN_BLUE, led_b);
}

void setup() {
  Serial.begin(9600);
  while (!Serial);

  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_BLUE, OUTPUT);

  if (!BLE.begin()) {
    Serial.println("Starting Bluetooth® Low Energy module failed!");
    while (1);
  }

  BLE.setLocalName("Arduino LED");
  BLE.setAdvertisedService(ledService);

  ledService.addCharacteristic(switchCharacteristic);
  ledService.addCharacteristic(colorCharacteristic);

  BLE.addService(ledService);

  switchCharacteristic.writeValue(0); // Set initial state to OFF
  colorCharacteristic.writeValue("0,0,0"); // Set initial color to off (black)
  setLED("0,0,0");
  BLE.advertise();

  Serial.println("BLE LED Peripheral");
}

void loop() {
  BLEDevice central = BLE.central();

  if (central) {
    Serial.print("Connected to central: ");
    Serial.println(central.address());

    // flash led 3 colors when connected
    setLED("255,0,0");
    delay(500);
    setLED("0,255,0");
    delay(500);
    setLED("0,0,255");
    delay(500);
    setLED("0,0,0");

    
    bool ledIsOn = false;              // Track if the LED is on or off
    String previousColor = "0,0,0";    // Track previous color
    String currentColor = "0,0,0";
    
    // While the devices are connected together
    while (central.connected()) 
    {
      // if i get a request from the switch characteristic
      if (switchCharacteristic.written())
      {
        
        switch(switchCharacteristic.value())
        {
          case 0:
          if (!ledIsOn){
            Serial.println("LED is already off");
          }
          else{
            setLED("0,0,0");
            ledIsOn = false;
            Serial.println("LED has been turned off.");
          }
          break;

          case 1:
            if (ledIsOn){
              Serial.println("LED is already on");
            }
            else{
              currentColor = "255,255,255";

              Serial.println("LED has been turned on! RBG value: ");
              ledIsOn = true;
              setLED(currentColor);
            }
          break;
          default:
          Serial.println("Something went wrong couldn't change LED state");
          break;
        }
      }


      // if i get a request from the color characteristic
      if (colorCharacteristic.written())
      {
        switch(ledIsOn)
        {
          case true:
          
            if (colorCharacteristic.value() == "0,0,0"){
              Serial.println("LED has been turned off, RGB combination received: " + colorCharacteristic.value());
              setLED("0,0,0");
              ledIsOn = false;
            }

            else if (colorCharacteristic.value() == previousColor){
              Serial.println(" New color combination is the same as before. Color hasn't been changed");
            }

            else{
              Serial.println("New LED value: " + colorCharacteristic.value());
              setLED(colorCharacteristic.value());
              currentColor = colorCharacteristic.value();
              ledIsOn = true;
            }
          break;
          
          case false:
            Serial.println("Please turn on LED to set a color");
          break;

        }
      }

      previousColor = currentColor;
    }

    Serial.print("Disconnected from central: ");
    Serial.println(central.address());
  }
}
