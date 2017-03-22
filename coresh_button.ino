// Enable debug prints to serial monitor
#define MY_DEBUG

// Enable and select radio type attached
#define MY_RADIO_RFM69

#define MY_RFM69_FREQUENCY   RF69_433MHZ
//#define MY_RFM69_FREQUENCY   RF69_868MHZ
//#define MY_RFM69_FREQUENCY   RF69_315MHZ
//#define MY_RFM69_FREQUENCY   RF69_915MHZ

//#define MY_IS_RFM69HW

//#define MY_TRANSPORT_WAIT_READY_MS   (2000ul)

#define MY_SIGNING_ATSHA204
#define  MY_SIGNING_REQUEST_SIGNATURES

#define MY_OTA_FIRMWARE_FEATURE
#define MY_OTA_FLASH_JDECID 0x2020


#define MY_NODE_ID 100
//0xF3
//0xC9
//#define MY_BAUD_RATE 19200
#include <SPI.h>
#include <MySensors.h>

//#define SPIFLASH_BLOCKERASE_32K   0x52
#define SPIFLASH_BLOCKERASE_32K   0xD8
#define SPIFLASH_CHIPERASE        0x60


#define SKETCH_NAME "4 switches node "
#define SKETCH_MAJOR_VER "1"
#define SKETCH_MINOR_VER "0"

// Initialising array holding button child ID's
// NULL values used to indicate no switch attached to a connecctor and no child ID need to be added in Controller
int SWITCH_CHILD_ID[4] = {1, NULL, NULL, NULL};

#define TACT_SWITCHES

#define BUTTONS_INTERUPT_PIN 3

// Initialising array holding Arduino Digital I/O pins for button/reed switches
int SWITCH_BUTTON_PIN[4] = {4, 5, 6, 7};

int BATTERY_SENSE_PIN = A6;  // select the input pin for the battery sense point
int oldBatteryPcnt = 0;

// Initialising array holding button state messages
MyMessage msg_switch[4];


void before()
{

  //clock_prescale_set(clock_div_8); // Switch to 1Mhz for the reminder of the sketch, save power.
  // highfreq = false;


  //rfm reset pin
  pinMode(9, OUTPUT);
  digitalWrite(9, 0);
  // Setup the buttons
  // There is no need to activate internal pull-ups
  for (int i = 0; i <= 3; i++) {
    pinMode(SWITCH_BUTTON_PIN[i], INPUT);
  }
}

void presentation() {
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo(SKETCH_NAME, SKETCH_MAJOR_VER "." SKETCH_MINOR_VER);
  analogReference(INTERNAL);

  // Register binary input sensor to sensor_node (they will be created as child devices)
  // You can use S_DOOR, S_MOTION or S_LIGHT here depending on your usage.
  // If S_LIGHT is used, remember to update variable type you send in. See "msg" above.
  for (int i = 0; i <= 3; i++) {
    if (SWITCH_CHILD_ID[i] != NULL) {
      msg_switch[i] = MyMessage(SWITCH_CHILD_ID[i], V_LIGHT);
      present(SWITCH_CHILD_ID[i], S_LIGHT);
    }
  }
}

void setup() {

}

// Loop will iterate on changes on the BUTTON_PINs
void loop()
{
  // Button state value arrays
  static uint8_t  value[4] = {NULL, NULL, NULL, NULL};
  static  uint8_t last_value[4] = {NULL, NULL, NULL, NULL};

  //------------------------------------------------------- get the battery Voltage
  int sensorValue = analogRead(BATTERY_SENSE_PIN);
  /*
     #ifdef MY_DEBUG
     Serial.println(sensorValue);
     #endif
  */
  // 1M, 470K divider across battery and using internal ADC ref of 1.1V
  // Sense point is bypassed with 0.1 uF cap to reduce noise at that point
  // ((1e6+470e3)/470e3)*1.1 = Vmax = 3.44 Volts
  // 3.44/1023 = Volts per bit = 0.003363075

  int batteryPcnt = sensorValue / 10;
  /*
     #ifdef MY_DEBUG
     float batteryV  = sensorValue * 0.00312805;
     Serial.print("Battery Voltage: ");
     Serial.print(batteryV);
     Serial.println(" V");

     Serial.print("Battery percent: ");
     Serial.print(batteryPcnt);
     Serial.println(" %");
     #endif
  */
  if (oldBatteryPcnt != batteryPcnt) {
    // Power up radio after sleep
    sendBatteryLevel(batteryPcnt);
    oldBatteryPcnt = batteryPcnt;
  }
  //   Check active switches

#ifdef TACT_SWITCHES
  for (int i = 0; i <= 3; i++) {
    if (SWITCH_CHILD_ID[i] != NULL) {
      value[i] = digitalRead(SWITCH_BUTTON_PIN[i]);

      //Serial.println("CHILD_ID");
      //Serial.println(loadState(SWITCH_CHILD_ID[i]));

      if (value[i] == 1) { //last_value[i] != value[i] &&
        uint8_t loadedState = loadState(SWITCH_CHILD_ID[i]);
        if (last_value[i] == NULL) {
          last_value[i] = 0;
        }
        if ( loadedState == 255) {
          //no controller found
          send(msg_switch[i].set(!last_value[i] ? true : false), true);
          last_value[i] = !last_value[i];
        }
        else  {
          send(msg_switch[i].set(!loadedState ? true : false), true);
          last_value[i] = loadedState;
        }
      }
    }
  }
  wait(100);
#else
  for (int i = 0; i <= 3; i++) {
    if (SWITCH_CHILD_ID[i] != NULL) {
      value[i] = digitalRead(SWITCH_BUTTON_PIN[i]);
      if (last_value[i] != value[i]) {
        last_value[i] = value[i];
        Serial.println("VALUE");
        Serial.println(value[i]);
        send(msg_switch[i].set(value[i] ? false : true), true);
      }
    }
  }
  wait(100);
#endif


  /*
    if (!isTransportOK()) {
    // TNR: transport not ready
    //transportSwitchSM(stInit);
    //stInitTransition();
    Serial.println("*******TNR: interception*********");
    _begin();
    }
  */
  //rfm reset
  //digitalWrite(9,1);
  //delay(100);
  //digitalWrite(9,0);
  //wait(10000);
  // OTA wake state - keep pressing button #4
  //if (value[4] == 0) {sleep(BUTTONS_INTERUPT_PIN - 2, RISING, 0);}
  
  //if (_radio.readRSSI() < CSMA_LIMIT){Serial.print(" canSend():"); Serial.println(_radio.readRSSI());}
  sleep(BUTTONS_INTERUPT_PIN - 2, RISING, 0);
}
