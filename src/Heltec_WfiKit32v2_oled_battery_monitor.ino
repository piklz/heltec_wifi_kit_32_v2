/*  

██████╗░██╗██╗░░██╗██╗░░░░░███████╗
██╔══██╗██║██║░██╔╝██║░░░░░╚════██║
██████╔╝██║█████═╝░██║░░░░░░░███╔═╝
██╔═══╝░██║██╔═██╗░██║░░░░░██╔══╝░░
██║░░░░░██║██║░╚██╗███████╗███████╗
╚═╝░░░░░╚═╝╚═╝░░╚═╝╚══════╝╚══════╝
                               
    heltec [esp32] wifi-kit (v2) oled built-in lipo battery monitor
    -by piklz
    -- github/piklz

      

   v001 [June 2024]
    - heltec oled battery monitor (esp3.0 fixed/updated and deprecated apis removed - adcAttachPin etc )
    
   
*/




#include "Arduino.h"
#include "heltec.h"
#include <Wire.h>
#include "HT_SSD1306Wire.h"  // legacy include: `#include "SSD1306.h"`
#include "HT_DisplayUi.h"
#include "images.h"


static SSD1306Wire display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED);  // addr , freq , i2c group , resolution , rst



#define Fbattery 3700  //The default battery is 3700mv when the battery is fully charged.(using the Makerfocus 1000mah 1A discharge battery)
#define ADC_READ_STABILIZE 10 // not sure if this is even useful yet..

int   batteryVoltage    =  0 ;
int   batteryPercentage =  0 ;
float batteryVoltFloat       ;

const int batteryVoltagePin = 37; // Adjust based on your analog input pin
float conversionFactor = 1.75542112 ;  // higher num the higher volts reads 1.84-2.2(adjust if different)
const float maxChargingVoltage = 4.2;
const float minBatteryVoltage = 3.2;

//Format the time so it looks pretty
char timeBuffer[9];
unsigned long previousMillis = 0;  // Stores the last time the counter was updated
const int updateInterval = 1000;  // Update the counter every 1 second (1000 milliseconds)




DisplayUi ui(&display);

void msOverlay(ScreenDisplay* display, DisplayUiState* state) {  //overlay keeps bits/logos in-place even when sliding ui occurs
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  display->setFont(ArialMT_Plain_10);
  //display->drawString(128, 10, String(millis()));
  // Calculate the starting X-coordinate to center the text horizontally
  int textWidth = display->getStringWidth(timeBuffer);
  int startX = (128 - textWidth) / 2;  // Center the text horizontally

  display->drawString(startX + 80, 25, "T:"+String(timeBuffer) );

  //display->drawXbm(114, 0, WIFI_width, WIFI_height, WIFI_bits);
}


void drawFrame1(ScreenDisplay* display, DisplayUiState* state, int16_t x, int16_t y) {

  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_16);
  display->drawString(x, y, "Piklz LiPo-Mon");
  display->setFont(ArialMT_Plain_10);
  
  display->drawString(x, y + 25, "volts: " + String(batteryVoltFloat/1000 ) + "V");  //float

  //display->drawString(x, y + 25, "volts: " +     String(dtostrf(valVolts/0.01/100000, 5, 2, msgBuffer) ) +"V");  //float
  display->drawString(x, y + 38, "Full%: " + String(batteryPercentage) );  //float

  if (batteryPercentage > 95) {

    display->drawXbm(x + 98, y + 40, BAT_width, BAT_height, BAT_bits);

  } else if (batteryPercentage > 50) {

    display->drawXbm(x + 106, y + 40, BATHALF_width, BATHALF_height, BATHALF_bits);

  } else {
    display->drawXbm(x + 114, y + 40, BATLOW_width, BATLOW_height, BATLOW_bits);
  }
}


/*void drawFrame1(ScreenDisplay *display, DisplayUiState* state, int16_t x, int16_t y) {
  // draw an xbm image.
  // Please note that everything that should be transitioned
  // needs to be drawn relative to x and y

  display->drawXbm(x + 34, y + 14, WiFi_Logo_width, WiFi_Logo_height, WiFi_Logo_bits);
}


void drawFrame2(ScreenDisplay *display, DisplayUiState* state, int16_t x, int16_t y) {
  // Demonstrates the 3 included default sizes. The fonts come from SSD1306Fonts.h file
  // Besides the default fonts there will be a program to convert TrueType fonts into this format
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  display->drawString(0 + x, 10 + y, "Arial 10");

  display->setFont(ArialMT_Plain_16);
  display->drawString(0 + x, 20 + y, "Arial 16");

  display->setFont(ArialMT_Plain_24);
  display->drawString(0 + x, 34 + y, "Arial 24");
}

void drawFrame3(ScreenDisplay *display, DisplayUiState* state, int16_t x, int16_t y) {
  // Text alignment demo
  display->setFont(ArialMT_Plain_10);

  // The coordinates define the left starting point of the text
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->drawString(0 + x, 11 + y, "Left aligned (0,10)");

  // The coordinates define the center of the text
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString(64 + x, 22 + y, "Center aligned (64,22)");

  // The coordinates define the right end of the text
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  display->drawString(128 + x, 33 + y, "Right aligned (128,33)");
}

void drawFrame4(ScreenDisplay *display, DisplayUiState* state, int16_t x, int16_t y) {
  // Demo for drawStringMaxWidth:
  // with the third parameter you can define the width after which words will be wrapped.
  // Currently only spaces and "-" are allowed for wrapping
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  display->drawStringMaxWidth(0 + x, 10 + y, 128, "Lorem ipsum\n dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore.");
}



void drawFrame5(ScreenDisplay *display, DisplayUiState* state, int16_t x, int16_t y) {

}
*/
/*
void VextON(void) {
  pinMode(Vext, OUTPUT);
  digitalWrite(Vext, LOW);
}

void VextOFF(void)  //Vext default OFF
{
  pinMode(Vext, OUTPUT);
  digitalWrite(Vext, HIGH);
}*/

// This array keeps function pointers to all frames
// frames are the single views that slide in
//FrameCallback frames[] = { drawFrame1, drawFrame2, drawFrame3, drawFrame4, drawFrame5 };
FrameCallback frames[] = { drawFrame1 };
// how many frames are there?
int frameCount = 1;

// Overlays are statically drawn on top of a frame eg. a clock
OverlayCallback overlays[] = { msOverlay };
int overlaysCount = 1;






void setup() {
  Serial.begin(115200);

  analogReadResolution(12);//default is 12 if not used -set the resolution to 12 bits (0-4095)

  //set the resolution to 12 bits (0-4095)
  //analogReadResolution(12);


  // The ESP is capable of rendering 60fps in 80Mhz mode
  // but that won't give you much time for anything else
  // run it in 160Mhz mode or just set it to 30 fps
  ui.setTargetFPS(60);

  // Customize the active and inactive symbol
  ui.setActiveSymbol(activeSymbol);
  ui.setInactiveSymbol(inactiveSymbol);

  // You can change this to
  // TOP, LEFT, BOTTOM, RIGHT
  ui.setIndicatorPosition(BOTTOM);

  // Defines where the first frame is located in the bar.
  ui.setIndicatorDirection(LEFT_RIGHT);

  // You can change the transition that is used
  // SLIDE_LEFT, SLIDE_RIGHT, SLIDE_UP, SLIDE_DOWN
  ui.setFrameAnimation(SLIDE_LEFT);

  // Add frames
  ui.setFrames(frames, frameCount);

  // Add overlays
  ui.setOverlays(overlays, overlaysCount);

  // Initialising the UI will init the display too.
  ui.init();

  // Flip display vertically
  //ui.display->flipScreenVertically();
  
  // diable auto transition to next frame
  ui.disableAutoTransition();
}


void loop() {

  
  int remainingTimeBudget = ui.update();
  if (remainingTimeBudget > 0) {
    // You can do some work here
    // Don't do stuff if you are below your
    // time budget.
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= updateInterval) {
      previousMillis = currentMillis;
      int elapsedSeconds = (currentMillis / 1000) % 60;
      int elapsedMinutes = ((currentMillis / 1000) / 60) % 60;
      int elapsedHours = (currentMillis / (1000 * 60 * 60));

      // Format the time string with leading zeros for hours and minutes
      //char timeBuffer[9];
      snprintf(timeBuffer, sizeof(timeBuffer), "%02d:%02d:%02d", elapsedHours, elapsedMinutes, elapsedSeconds);

      Serial.println(timeBuffer);  // Print the elapsed time to the serial monitor
      Serial.println();

      //get info for oled display here//
      battery_read();
    
  }



    delay(remainingTimeBudget);
    //ui.display->clear();

    
  }
  
}







void battery_read() {
  
  batteryVoltage = analogReadMilliVolts(batteryVoltagePin) * conversionFactor ; //output:4500
  delay(1000);
  Serial.print("Battery VoltagemV: ");
  Serial.print(batteryVoltage); // Print with 2 decimal places
  Serial.println(" V");
  //using this map(batteryVoltage, minBatteryVoltage, maxChargingVoltage, 0.0, 100.0) and constrain to 0-100 for Full%
  batteryPercentage = constrain(map(batteryVoltage, 3200, 4200, 0, 100),0,100 );
  batteryVoltFloat =(batteryVoltage) ;
  Serial.print("Battery Percentage: ");
  Serial.print(batteryPercentage, 1); // 1 decimal place
  Serial.println("%");   
  Serial.print("Battery Float for oled: ");
  Serial.print(batteryVoltFloat,2);  //  2 decimal places
  Serial.println();
  
 }