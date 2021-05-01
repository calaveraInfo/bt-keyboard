#include <bluefruit.h>
#include <Keypad.h>

#define BCK 8

// read; pullup; za diodou
const byte ROWS = 5;
// write; pred tlacitkem
const byte COLS = 7;

char hexaKeys[ROWS][COLS] = {
      {'a', 'b', 'c', 'd', 'e', 'f', 'g'},
      {'h', 'i', 'j', 'k', 'l', 'm', ' '},
  {'n','o', 'p', 'q', 'r', 's', 't'},
  {'u','v', 'w', 'x', 'y', 'z', ' '},
  {':','-', ')', '3', '4', '5', BCK},
};

/*
char hexaKeys[ROWS][COLS] = {
      {'r', 't', 'y', 'u', 'i', 'o', 'p'},
      {'f', 'g', 'h', 'j', 'k', 'l', ' '},
  {'e','d', 'c', 'v', 'b', 'n', 'm'},
  {'w','s', 'x', '1', '2', '3', '4'},
  {'q','a', 'z', '5', '6', '7', 8},
};
*/


byte rowPins[ROWS] = {A1, A2, A3, A4, A5};
byte colPins[COLS] = {13, 12, 11, 10, 9, 6, 5};

Keypad kpd = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 

unsigned long loopCount;
unsigned long startTime;
String msg;

BLEDis bledis;
BLEHidAdafruit blehid;

void setup() {
    pinMode(A0, INPUT_PULLUP);
    //Serial.begin(115200);
    //while ( !Serial ) delay(10);   // for nrf52840 with native usb
    delay(1000);
    loopCount = 0;
    startTime = millis();
    msg = "";
/*
    Serial.println("Bluefruit52 HID Keyboard Example");
    Serial.println("--------------------------------\n");
  
    Serial.println();
    Serial.println("Go to your phone's Bluetooth settings to pair your device");
    Serial.println("then open an application that accepts keyboard input");
  
    Serial.println();
    Serial.println("Enter the character(s) to send:");
    Serial.println();  
*/  
    Bluefruit.begin();
    Bluefruit.setTxPower(4);    // Check bluefruit.h for supported values
    Bluefruit.setName("Bluefruit52");
  
    // Configure and Start Device Information Service
    bledis.setManufacturer("Adafruit Industries");
    bledis.setModel("Bluefruit Feather 52");
    bledis.begin();
  
    /* Start BLE HID
     * Note: Apple requires BLE device must have min connection interval >= 20m
     * ( The smaller the connection interval the faster we could send data).
     * However for HID and MIDI device, Apple could accept min connection interval 
     * up to 11.25 ms. Therefore BLEHidAdafruit::begin() will try to set the min and max
     * connection interval to 11.25  ms and 15 ms respectively for best performance.
     */
    blehid.begin();
  
    // Set callback for set LED from central
    blehid.setKeyboardLedCallback(set_keyboard_led);
  
    /* Set connection interval (min, max) to your perferred value.
     * Note: It is already set by BLEHidAdafruit::begin() to 11.25ms - 15ms
     * min = 9*1.25=11.25 ms, max = 12*1.25= 15 ms 
     */
    /* Bluefruit.Periph.setConnInterval(9, 12); */
  
    // Set up and start advertising
    startAdv();
}

void startAdv(void)
{  
  // Advertising packet
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  Bluefruit.Advertising.addAppearance(BLE_APPEARANCE_HID_KEYBOARD);
  
  // Include BLE HID service
  Bluefruit.Advertising.addService(blehid);

  // There is enough room for the dev name in the advertising packet
  Bluefruit.Advertising.addName();
  
  /* Start Advertising
   * - Enable auto advertising if disconnected
   * - Interval:  fast mode = 20 ms, slow mode = 152.5 ms
   * - Timeout for fast mode is 30 seconds
   * - Start(timeout) with timeout = 0 will advertise forever (until connected)
   * 
   * For recommended advertising interval
   * https://developer.apple.com/library/content/qa/qa1931/_index.html   
   */
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
  Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds
}

void loop() {
    loopCount++;
    if ( (millis()-startTime)>5000 ) {
    /*
        Serial.print("Average loops per second = ");
        Serial.println(loopCount/5);
    */
        startTime = millis();
        loopCount = 0;
    }

    // Fills kpd.key[ ] array with up-to 10 active keys.
    // Returns true if there are ANY active keys.
    if (kpd.getKeys())
    {
        for (int i=0; i<LIST_MAX; i++)   // Scan the whole key list.
        {
            if ( kpd.key[i].stateChanged )   // Only find keys that have changed state.
            {
                switch (kpd.key[i].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
                    case PRESSED:
                    msg = " PRESSED.";
                    blehid.keyPress(kpd.key[i].kchar);
                break;
                    case HOLD:
                    msg = " HOLD.";
                break;
                    case RELEASED:
                    msg = " RELEASED.";
                    blehid.keyRelease();
                break;
                    case IDLE:
                    msg = " IDLE.";
                }
                /*
                Serial.print("Key ");
                Serial.print(kpd.key[i].kchar);
                Serial.println(msg);
                */
            }
        }
    }
}  // End loop

/**
 * Callback invoked when received Set LED from central.
 * Must be set previously with setKeyboardLedCallback()
 *
 * The LED bit map is as follows: (also defined by KEYBOARD_LED_* )
 *    Kana (4) | Compose (3) | ScrollLock (2) | CapsLock (1) | Numlock (0)
 */
void set_keyboard_led(uint16_t conn_handle, uint8_t led_bitmap)
{
  (void) conn_handle;
  
  // light up Red Led if any bits is set
  if ( led_bitmap )
  {
    ledOn( LED_RED );
  }
  else
  {
    ledOff( LED_RED );
  }
}
