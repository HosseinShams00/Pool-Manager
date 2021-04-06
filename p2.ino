#include <EEPROM.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Sodaq_DS3231.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define KeyPadPin A2

#define Sensor_1_pin 8

#define Relay_1_pin 2
#define Relay_2_pin 3

#define Sensor_1_Temp_Low_Address 1
#define Sensor_1_Temp_HIGH_Address 2

int LastTime = 0, KeyPadReader = 0, Sensor_1_Temp = 0, Sensor_1_Temp_Low = 10, Sensor_1_Temp_HIGH = 20;

bool Sensor_1_On = false, _1Hour[24], _30Min[48], HourCheck = true;

// One Wire Input Is Pin
OneWire oneWire(Sensor_1_pin);
DallasTemperature SensorDall(&oneWire);

LiquidCrystal_I2C Lcd(0x20, 16, 2);

#define _1HourCheckArrayAddress 6                   // 6 = Temp_sensore 1 , 2 High And Low Addres +  HourCheck + 1
#define _30MinCheckArrayAddress sizeof(&_1Hour) + 7 // 7 = _1HourCheckArrayAddress + 1

void Menu();
void SetSensor();
int SetSensorValue(bool IsOff, bool IsLow);

void CheckTimersArray();
void TimerMenu();
void SetLocalTime();
int getTime();
void SetTimerValue(bool Data[], bool IsHour);
bool GetKeypad();
void TimeChecker(bool HourCh, int Hour, int Minut);

// KeyPad Values
// Right : 0 - 60
// Up : 60 - 200
// Down : 200 - 400
// Left : 400 - 600
// Select : 600 - 800

void setup()
{
  Wire.begin();
  rtc.begin();

  /// LCD Config
  Lcd.begin();
  Serial.begin(9600);

  // Sensor Config For DS18B20
  SensorDall.begin();
  // KeyPad SetUP
  pinMode(KeyPadPin, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(9, OUTPUT);

  // Relay Config
  pinMode(Relay_1_pin, OUTPUT);
  pinMode(Relay_2_pin, OUTPUT);

  // Sensor Config
  pinMode(Sensor_1_pin, OUTPUT);

  // Sensor Value Config
  Sensor_1_Temp_Low = EEPROM.read(Sensor_1_Temp_Low_Address);
  Sensor_1_Temp_HIGH = EEPROM.read(Sensor_1_Temp_HIGH_Address);

  // Check Timer
  CheckTimersArray();
  Serial.println("------ Hours Array ------");
  for (int i = 0; i < 24; i++)
  {
    Serial.println(EEPROM.read(i + _1HourCheckArrayAddress));
  }
  Serial.println("------ min Array ------");
  for (int i = 0; i < 48; i++)
  {
    Serial.println(EEPROM.read(i + _30MinCheckArrayAddress));
  }

  Lcd.home();
  Lcd.setCursor(0, 0);
  Lcd.print("    Wellcome    ");
  Lcd.setCursor(0, 1);
  Lcd.print("  Please  Wait  ");
}

void loop()
{

  Serial.print(rtc.now().hour());
  Serial.print(":");
  Serial.print(rtc.now().minute());
  Serial.print(":");
  Serial.println(rtc.now().second());
  delay(150);
  //// Check 1 minute has passed or not
  if (LastTime != rtc.now().minute())
  {
    LastTime = rtc.now().minute();
    Lcd.clear();
    Lcd.home();
    Lcd.print("Please Wait");
    Lcd.setCursor(0, 1);
    Lcd.print("Update Temp");
    while (true)
    {
      SensorDall.requestTemperatures();
      delay(300);
      Serial.println("Request Done ! ");
      Sensor_1_Temp = SensorDall.getTempCByIndex(0);

      // If Device Connected Set Sensor Temp and Break Loop
      if (Sensor_1_Temp != DEVICE_DISCONNECTED_C)
      {
        Serial.print("Device Is connected Temp Is : ");
        Serial.println(Sensor_1_Temp);
        break;
      }
      else
      {
        Serial.println("Device 0 Is Disconnected ");
      }
    }
  }

  Lcd.clear();
  Lcd.home();
  Lcd.print("Sensor : ");
  Lcd.setCursor(9, 0);

  // Read Digital Sensor
  Lcd.print(Sensor_1_Temp);

  Lcd.print(" C");
  Lcd.setCursor(0, 1);
  Lcd.print("Time : ");
  Lcd.print(rtc.now().hour());
  Lcd.print(":");
  Lcd.print(rtc.now().minute());
  Lcd.print(":");
  Lcd.print(rtc.now().second());

  if (digitalRead(13) == HIGH)
  {
    delay(300);
    Menu();
  }

  if (Sensor_1_Temp <= Sensor_1_Temp_Low)
  {
    Sensor_1_On = true;
    digitalWrite(Relay_1_pin, HIGH);
  }
  else if (Sensor_1_Temp >= Sensor_1_Temp_HIGH)
  {
    Sensor_1_On = false;
    digitalWrite(Relay_1_pin, LOW);
  }

  TimeChecker(HourCheck, rtc.now().hour(), rtc.now().minute());
}

void Menu()
{
  bool Sensors = true;
  Lcd.clear();
  Lcd.home();
  Lcd.print("* Sensor");
  Lcd.setCursor(0, 1);
  Lcd.print("  Timer");
  while (true)
  {
    /// if Press Submit Button Goto This Method
    //  else Press Up Button Break While
    // For This Option Don't Check Down And Home Key

    KeyPadReader = analogRead(KeyPadPin);

    //if (  KeyPadReader > 60 && KeyPadReader < 200 )     // Check Up Button
    if (digitalRead(11) == HIGH)
    {
      Lcd.setCursor(0, 0);
      Lcd.write('*');
      Lcd.setCursor(0, 1);
      Lcd.write(' ');
      delay(400);
      Sensors = true;
    }
    //else if ( KeyPadReader > 400 && KeyPadReader < 600 )  // Check Down Button
    else if (digitalRead(12) == HIGH)
    {
      Lcd.setCursor(0, 0);
      Lcd.write(' ');
      Lcd.setCursor(0, 1);
      Lcd.write('*');
      delay(400);
      Sensors = false;
    }
    else if (KeyPadReader > 0 && KeyPadReader < 60) // Check Right Button
    {
    }
    //else if (  KeyPadReader > 600 && KeyPadReader < 800 )    // Check Submit Button
    else if (digitalRead(13) == HIGH)
    {
      delay(400);
      if (Sensors)
      {
        delay(300);
        SetSensor();
      }
      else
      {
        delay(300);
        TimerMenu();
      }
      break;
    }
  }
  Lcd.clear();
}

void SetSensor()
{
  delay(500);
  Sensor_1_Temp_Low = SetSensorValue(true, true);
  Sensor_1_Temp_HIGH = SetSensorValue(false, false);

  EEPROM.update(Sensor_1_Temp_Low_Address, Sensor_1_Temp_Low);
  EEPROM.update(Sensor_1_Temp_HIGH_Address, Sensor_1_Temp_HIGH);

  Lcd.clear();
}

int SetSensorValue(bool IsOff, bool IsLow)
{
  int Outp = 0;
  if (IsLow)
  {
    Outp = Sensor_1_Temp_Low;
  }
  else
  {
    Outp = Sensor_1_Temp_HIGH;
  }

  Lcd.clear();
  Lcd.setCursor(0, 0);
  if (IsOff)
  {
    Lcd.print("Your Temp: On");
  }
  else
  {
    Lcd.print("Your Temp: OFF");
  }
  Lcd.setCursor(0, 1);
  Lcd.print(Outp);
  Lcd.print(" C");
  while (true)
  {

    if (digitalRead(11) == HIGH)
    {
      delay(300);
      if (Outp == 255 || Outp == 0)
      {
        Outp = 0;
        Lcd.setCursor(0, 1);
        Lcd.print("     ");
        Lcd.setCursor(0, 1);
        Lcd.print(Outp);
        Lcd.print(" C");
      }
      else if (Outp <= 150)
      {
        Outp++;
        Lcd.setCursor(0, 1);
        Lcd.print("     ");
        Lcd.setCursor(0, 1);
        Lcd.print(Outp);
        Lcd.print(" C");
      }
    }
    else if (digitalRead(12) == HIGH)
    {
      delay(300);
      if (Outp >= -50)
      {
        Outp--;
        Lcd.setCursor(0, 1);
        Lcd.print("     ");
        Lcd.setCursor(0, 1);
        Lcd.print(Outp);
        Lcd.print(" C");
      }
      else if (Outp == -50)
      {
        Outp = 0;
        Lcd.setCursor(0, 1);
        Lcd.print("     ");
        Lcd.setCursor(0, 1);
        Lcd.print(Outp);
        Lcd.print(" C");
      }
    }
    else if (digitalRead(13) == HIGH)
    {
      delay(1000);
      Lcd.clear();
      Lcd.home();

      if (IsOff)
      {
        Lcd.print("Your Temp: On");
      }
      else
      {
        Lcd.print("Your Temp: OFF");
      }

      Lcd.setCursor(0, 1);
      Lcd.print(Outp);
      Lcd.print(" C");
      delay(300);
      break;
    }
  }
  return Outp;
}

void CheckTimersArray()
{
  for (int j = 0; j < 24; j++)
  {
    if (EEPROM.read(j + _1HourCheckArrayAddress) != 255)
    {
      _1Hour[j] = EEPROM.read(j + _1HourCheckArrayAddress);
    }
    else
    {
      EEPROM.write(j + _1HourCheckArrayAddress, false);
      _1Hour[j] = false;
    }
  }

  for (int j = 0; j < 48; j++)
  {
    if (EEPROM.read(j + _30MinCheckArrayAddress) != 255)
    {
      _30Min[j] = EEPROM.read(j + _30MinCheckArrayAddress);
    }
    else
    {
      EEPROM.write(j + _30MinCheckArrayAddress, false);
      _30Min[j] = false;
    }
  }
}

void TimerMenu()
{
  bool LocalTime = false;
  Lcd.clear();
  Lcd.home();
  Lcd.print("* Set Timer");
  Lcd.setCursor(0, 1);
  Lcd.print("  Set Local Time");
  while (true)
  {
    /// if Press Submit Button Goto This Method
    //  else Press Up Button Break While
    // For This Option Don't Check Down And Home Key

    KeyPadReader = analogRead(KeyPadPin);

    //if (  KeyPadReader > 60 && KeyPadReader < 200 )     // Check Up Button
    if (digitalRead(11) == HIGH)
    {
      delay(400);
      Lcd.setCursor(0, 0);
      Lcd.write('*');
      Lcd.setCursor(0, 1);
      Lcd.write(' ');
      LocalTime = false;
    }
    //else if ( KeyPadReader > 400 && KeyPadReader < 600 )  // Check Down Button
    else if (digitalRead(12) == HIGH)
    {
      delay(400);
      Lcd.setCursor(0, 0);
      Lcd.write(' ');
      Lcd.setCursor(0, 1);
      Lcd.write('*');
      LocalTime = true;
    }
    else if (KeyPadReader > 0 && KeyPadReader < 60) // Check Right Button
    {
    }
    else if (digitalRead(13) == HIGH)
    {
      delay(300);
      if (LocalTime)
      {
        SetLocalTime();
      }
      else
      {
        int Time_Returned = 0;
        Time_Returned = getTime();
        if (Time_Returned == 60)
        {
          SetTimerValue(_1Hour, true);
        }
        else if (Time_Returned == 30)
        {
          SetTimerValue(_30Min, false);
        }
      }
      break;
    }
  }
}

bool GetKeypad()
{
  int i = 1;
  bool IsOn = true;
  Lcd.setCursor(0, 1);
  Lcd.print("* ON        OFF ");
  while (i)
  {
    if (digitalRead(9) == HIGH) // Right Button
    {
      delay(300);
      Lcd.setCursor(0, 1);
      Lcd.print(" ");
      Lcd.setCursor(11, 1);
      Lcd.print("*");
      IsOn = false;
    }
    else if (digitalRead(10) == HIGH) // Left Button
    {
      delay(300);
      Lcd.setCursor(0, 1);
      Lcd.print("*");
      Lcd.setCursor(11, 1);
      Lcd.print(" ");
      IsOn = true;
    }
    else if (digitalRead(13) == HIGH) // Submit Button
    {
      delay(300);
      break;
    }
  }
  return IsOn;
}

void SetLocalTime()
{

  int Hour = rtc.now().hour(), Min = rtc.now().minute();
  bool LeftCursore = true;
  Lcd.clear();
  Lcd.home();
  Lcd.print("Set Your Times");
  Lcd.setCursor(0, 1);
  Lcd.print("-->  ");

  if (Hour <= 9)
    Lcd.print("0");

  Lcd.print(Hour);
  Lcd.print(":");

  if (Min <= 9)
    Lcd.print("0");
  Lcd.print(Min);

  while (true)
  {

    if (digitalRead(9) == HIGH) // Right Button
    {
      delay(300);
      Lcd.setCursor(0, 1);
      Lcd.print("     ");
      Lcd.setCursor(12, 1);
      Lcd.print("<-- ");
      LeftCursore = false;
    }

    else if (digitalRead(10) == HIGH) // Left Button
    {
      delay(300);
      Lcd.setCursor(11, 1);
      Lcd.print("     ");
      Lcd.setCursor(0, 1);
      Lcd.print("-->  ");
      LeftCursore = true;
    }

    else if (digitalRead(11) == HIGH) // Up Button
    {
      delay(300);
      if (LeftCursore)
      {
        if (Hour == 23)
        {
          Hour = 0;
          Lcd.setCursor(5, 1);
          Lcd.print(Hour);
          Lcd.setCursor(6, 1);
          Lcd.print(Hour);
        }
        else if (Hour >= 0 && Hour < 9)
        {
          Hour++;
          Lcd.setCursor(6, 1);
          Lcd.print(Hour);
        }
        else if (Hour >= 9 && Hour < 23)
        {
          Hour++;
          Lcd.setCursor(5, 1);
          Lcd.print(Hour);
        }
      }
      else
      {
        if (Min == 59)
        {
          Min = 0;
          Lcd.setCursor(8, 1);
          Lcd.print(Min);
          Lcd.setCursor(9, 1);
          Lcd.print(Min);
        }
        else if (Min >= 0 && Min < 9)
        {
          Min++;
          Lcd.setCursor(9, 1);
          Lcd.print(Min);
        }
        else if (Min >= 9 && Min < 59)
        {
          Min++;
          Lcd.setCursor(8, 1);
          Lcd.print(Min);
        }
      }
    }

    else if (digitalRead(12) == HIGH) // Down Button
    {
      delay(300);
      if (LeftCursore)
      {
        if (Hour == 0)
        {
          Hour = 23;
          Lcd.setCursor(5, 1);
          Lcd.print(Hour);
        }
        else if (Hour > 0 && Hour <= 9)
        {
          Hour--;
          Lcd.setCursor(6, 1);
          Lcd.print(Hour);
        }
        else if (Hour >= 10 && Hour <= 23)
        {
          Hour--;
          Lcd.setCursor(5, 1);
          Lcd.print(Hour);
        }
      }
      else
      {
        if (Min == 0)
        {
          Min = 59;
          Lcd.setCursor(8, 1);
          Lcd.print(Min);
        }
        else if (Min >= 1 && Min <= 9)
        {
          Min--;
          Lcd.setCursor(9, 1);
          Lcd.print(Min);
        }
        else if (Min >= 10 && Min <= 59)
        {
          Min--;
          Lcd.setCursor(8, 1);
          Lcd.print(Min);
        }
      }
    }

    else if (digitalRead(13) == HIGH) // Submit Button
    {
      break;
    }
  }

  delay(300);
  DateTime dt(2021, 1, 1, Hour, Min, 0, 0);
  rtc.setDateTime(dt);
}

int getTime()
{
  bool IsHours = false;
  Lcd.clear();
  Lcd.home();
  Lcd.print("Set Your Period");
  Lcd.setCursor(0, 1);
  Lcd.print("*30 Min   60 Min");
  while (true)
  {
    if (digitalRead(9) == HIGH) // Right Button
    {
      delay(300);
      Lcd.setCursor(0, 1);
      Lcd.print(" ");
      Lcd.setCursor(9, 1);
      Lcd.print("*");
      IsHours = true;
    }
    else if (digitalRead(10) == HIGH) // Left Button
    {
      delay(300);
      Lcd.setCursor(0, 1);
      Lcd.print("*");
      Lcd.setCursor(9, 1);
      Lcd.print(" ");
      IsHours = false;
    }
    else if (digitalRead(13) == HIGH) // Submit Button
    {
      delay(300);
      break;
    }
  }

  if (IsHours)
    return 60;
  else
    return 30;
}

void SetTimerValue(bool Data[], bool IsHour)
{
  if (IsHour)
  {
    HourCheck = true;
    EEPROM.update(5, HourCheck);

    for (int i = 0; i < 24; i++)
    {
      Lcd.clear();
      Lcd.home();
      if (i <= 9)
      {
        Lcd.print("0");
      }
      Lcd.print(i);
      Lcd.print(":00 --> ");
      if (i < 9)
      {
        Lcd.print("0");
      }
      Lcd.print(i + 1);
      Lcd.print(":00");
      if (GetKeypad())
      {
        Data[i] = true;
        EEPROM.update(i + _1HourCheckArrayAddress, true);
      }
      else
      {
        Data[i] = false;
        EEPROM.update(i + _1HourCheckArrayAddress, false);
      }
    }
  }
  else
  {
    HourCheck = false;
    EEPROM.update(5, HourCheck);
    int Counter = -1;
    for (int i = 1; i <= 24; i++)
    {
      Counter++;
      Lcd.clear();
      Lcd.home();
      Lcd.print(i - 1);
      Lcd.print(":00 --> ");
      Lcd.print(i - 1);
      Lcd.print(":30");

      if (GetKeypad())
      {
        _30Min[Counter] = true;
        EEPROM.update(Counter + _30MinCheckArrayAddress, true);
      }
      else
      {
        _30Min[Counter] = false;
        EEPROM.update(Counter + _30MinCheckArrayAddress, false);
      }

      Lcd.clear();
      Lcd.home();
      Lcd.print(i - 1);
      Lcd.print(":30 --> ");
      Lcd.print(i);
      Lcd.print(":00");
      Counter++;
      if (GetKeypad())
      {
        _30Min[Counter] = true;
        EEPROM.update(Counter + _30MinCheckArrayAddress, true);
      }
      else
      {
        _30Min[Counter] = false;
        EEPROM.update(Counter + _30MinCheckArrayAddress, false);
      }
    }
  }
}

void TimeChecker(bool HourCh, int Hour, int Minut)
{
  if (HourCh)
  {
    if (_1Hour[Hour])
    {
      digitalWrite(Relay_2_pin, HIGH);
    }
    else
    {
      digitalWrite(Relay_2_pin, LOW);
    }
  }
  else
  {
    if (Minut == 30 || Minut == 0)
    {
      if (Hour == 0)
      {
        if (Minut == 0)
        {
          if (_30Min[0])
          {
            digitalWrite(Relay_2_pin, HIGH);
          }
          else
          {
            digitalWrite(Relay_2_pin, LOW);
          }
        }
        else
        {
          if (_30Min[1])
          {
            digitalWrite(Relay_2_pin, HIGH);
          }
          else
          {
            digitalWrite(Relay_2_pin, LOW);
          }
        }
      }
      else
      {
        if (_30Min[Hour * 2])
        {
          digitalWrite(Relay_2_pin, HIGH);
        }
        else
        {
          digitalWrite(Relay_2_pin, LOW);
        }
      }
    }
  }
}
