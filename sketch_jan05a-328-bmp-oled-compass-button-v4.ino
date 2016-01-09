#include <Button.h>

#include <SFE_BMP180.h>
#include <Wire.h>
#include <U8glib.h>
#include <LSM303.h>
#include <LowPower.h>

#define ALTITUDE 1655.0
#define BUTTON1_PIN 9

U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE | U8G_I2C_OPT_DEV_0); // I2C / TWI
Button button = Button(BUTTON1_PIN, true, true, 25);
LSM303 compass;
SFE_BMP180 pressure;

double baseline; // baseline pressure
double T, P, Pab, p0, a;
float heading;
int screen;

int getWidth(String theString) {
  int str_len = theString.length() + 1;
  char char_array[str_len];
  theString.toCharArray(char_array, str_len);
  int width = u8g.getStrWidth(char_array);
  return width;
}

void draw(void) {
  u8g.setFont(u8g_font_unifont);

  if ( screen == 1)  {
    String temp = "Temp: " + String(T, 0);
    String alt = "Alt: " + String(a, 1) + " m";
    String pressureAbsMb = "Pres: " + String(Pab, 0) + " mb" ;
    int baseY = 20;
    //    String pressureAbsHg = String(Pab * 0.0295333727, 2) + " Hg";

    // Print Pressure
    u8g.setPrintPos(0, baseY);
    u8g.print(pressureAbsMb);

    // Print Altitude
    u8g.setPrintPos(0, baseY * 2);
    u8g.print(alt);

    // Print Temperature
    int tempWidth = getWidth(temp);
    u8g.setPrintPos(0, baseY * 3);
    u8g.print(temp);
    u8g.drawCircle(tempWidth + 6, baseY * 3 - 8, 2);
  } else {
    int angle = (int)heading;
    String angleString = String(angle);
    int width = getWidth(angleString);
    int angleCenterX = (64 - width / 2);

    drawCompass(angle);
    u8g.drawTriangle(60, 1, 68, 1, 64, 7);
    u8g.drawLine(0, 0, 128, 0);
    u8g.drawLine(0, 26, 128, 26);

    // Print heading and degrees circle
    u8g.setPrintPos(angleCenterX, 40);
    u8g.print(angle);
    u8g.drawCircle(angleCenterX + width + 4, 32, 2);
    u8g.setPrintPos(0, 40);

    if ( angle > 348  ) {
      u8g.print("N");
    } else if ( angle < 12 ) {
      u8g.print("N");
    } else if ( angle > 11 && angle < 34 ) {
      u8g.print("NNE");
    } else if ( angle > 33 && angle < 57 ) {
      u8g.print("NE");
    } else if ( angle > 56 && angle < 79 ) {
      u8g.print("ENE");
    } else if ( angle > 78 && angle < 102 ) {
      u8g.print("E");
    } else if ( angle > 101 && angle < 124 ) {
      u8g.print("ESE");
    } else if ( angle > 123 && angle < 147 ) {
      u8g.print("SE");
    } else if ( angle > 146 && angle < 169 ) {
      u8g.print("SSE");
    } else if ( angle > 168 && angle < 192 ) {
      u8g.print("S");
    } else if ( angle > 191 && angle < 214 ) {
      u8g.print("SSW");
    } else if ( angle > 213 && angle < 237 ) {
      u8g.print("SW");
    } else if ( angle > 236 && angle < 259 ) {
      u8g.print("WSW");
    } else if ( angle > 258 && angle < 282 ) {
      u8g.print("W");
    } else if ( angle > 281 && angle < 304 ) {
      u8g.print("WNW");
    } else if ( angle > 303 && angle < 327 ) {
      u8g.print("NW");
    } else if ( angle > 326 && angle < 349 ) {
      u8g.print("NNW");
    }
  }
}

void drawCompass(int angle) {

  int start = 64 - angle / 3;
  //  if (start > 120) start += -120 ;

  int x = 0 ;
  int y = 18 ;
  for (int i = 0; i < 4; i++) {
    x = start + (i * 30) - 1;
    if (x > 120) x += -120;
    if (x < 0) x += 120;
    u8g.setPrintPos((x - 3), 18);
    String letter;
    if (i == 0) {
      letter = "N";
    }
    if (i == 1) {
      letter = "E";
    }
    if (i == 2) {
      letter = "S";
    }
    if (i == 3) {
      letter = "W";
    }
    u8g.print(letter);
  }
}

void setup(void) {
  Serial.begin(9600);

  compass.init();
  compass.enableDefault();
  compass.m_min = (LSM303::vector<int16_t>) {
    -460, -721, -419
  };
  compass.m_max = (LSM303::vector<int16_t>) {
    +737, +460, +573
  };

  if (pressure.begin())
    Serial.println("BMP180 init success");
  else
  {
    Serial.println("BMP180 init fail (disconnected?)\n\n");
    while (1); // Pause forever.
  }

  screen = 1;
}

void loop(void) {
  button.read();
  //u8g.sleepOn();
  //LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  //  u8g.sleepOff();

  if ( button.wasReleased() ) {
    if ( screen == 2 ) {
      screen--;
    } else {
      screen++;
    }

    u8g.firstPage();
    do {
    } while ( u8g.nextPage() );
  }

  if (screen == 1) {
    Serial.print("Btn 1");
    P = getPressure();
  } else {
    compass.read();
    heading = compass.heading();
  }

  delay(10);

  u8g.firstPage();
  do {
    draw();
  } while ( u8g.nextPage() );

  delay(10);
}

double getPressure()
{
  char status;

  // You must first get a temperature measurement to perform a pressure reading.

  // Start a temperature measurement:

  status = pressure.startTemperature();
  if (status != 0)
  {
    // Wait for the measurement to complete:
    delay(status);

    status = pressure.getTemperature(T);
    if (status != 0)
    {
      // Print out the measurement:
      //      Serial.print("temperature: ");
      //      Serial.print(T,2);
      //      Serial.print(" deg C, ");
      //      Serial.print((9.0/5.0)*T+32.0,2);
      //      Serial.println(" deg F");

      status = pressure.startPressure(3);
      if (status != 0)
      {
        // Wait for the measurement to complete:
        delay(status);

        // Retrieve the completed pressure measurement:
        // Note that the measurement is stored in the variable P.
        // Note also that the function requires the previous temperature measurement (T).
        // (If temperature is stable, you can do one temperature measurement for a number of pressure measurements.)
        // Function returns 1 if successful, 0 if failure.

        status = pressure.getPressure(P, T);
        if (status != 0)
        {
          Pab = P;

          // The pressure sensor returns abolute pressure, which varies with altitude.
          // To remove the effects of altitude, use the sealevel function and your current altitude.
          // This number is commonly used in weather reports.
          // Parameters: P = absolute pressure in mb, ALTITUDE = current altitude in m.
          // Result: p0 = sea-level compensated pressure in mb

          p0 = pressure.sealevel(P, ALTITUDE); // we're at 1655 meters 

          // On the other hand, if you want to determine your altitude from the pressure reading,
          // use the altitude function along with a baseline pressure (sea-level or other).
          // Parameters: P = absolute pressure in mb, p0 = baseline pressure in mb.
          // Result: a = altitude in m.

          a = pressure.altitude(P, 1013.25);
          return (P);
        }
        else Serial.println("error retrieving pressure measurement\n");
      }
      else Serial.println("error starting pressure measurement\n");
    }
    else Serial.println("error retrieving temperature measurement\n");
  }
  else Serial.println("error starting temperature measurement\n");
}

