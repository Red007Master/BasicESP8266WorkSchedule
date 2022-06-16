#include <ESP8266WiFi.h>
#include <time.h>
#include "credentials.h"


tm timeinfo;
time_t now;
long unsigned lastNTPtime;
unsigned long lastEntryTime;

const byte PinLed = 2;
const byte PinReley = 12;

const byte HourStart = 8;
const byte HourEnd = 20;

byte IntervalHours = 2;
byte IntervalMins = 0;
byte IntervalTotalMins = 0;

const int PressTime = 500;
const int OneCycleDeley = 10000;

byte LastCycleHour = 0;
byte LastCycleMin = 0;
byte LastCycleSec = 0;
int LastCycleTotalMins = 0;

void setup()
{
  Serial.begin(9600);

  Serial.print("Setup:Start");

  pinMode(PinReley, OUTPUT);
  pinMode(PinLed, OUTPUT);

  WifiConnect();

  IntervalTotalMins = GetTotalMins(IntervalHours, IntervalMins);

  NetTimeStart();

  Serial.print("Setup:End");
}

void loop()
{
  delay(OneCycleDeley);

  Logic();
  Output();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////Main

void Logic()
{
  time_t t = time(0); // get time now
  tm *now = localtime(&t);

  byte NowHour = now->tm_hour;
  byte NowMin = now->tm_min;
  byte NowSec = now->tm_sec;
  int NowTotalMins = GetTotalMins(NowHour, NowMin);

  if (NowHour >= HourStart && NowHour <= HourEnd)
  {
    if (NowTotalMins >= LastCycleTotalMins + IntervalTotalMins)
    {
      getNTPtime(10);

      now = localtime(&t);

      NowHour = now->tm_hour;
      NowMin = now->tm_min;
      NowSec = now->tm_sec;
      NowTotalMins = GetTotalMins(NowHour, NowMin);

      if (NowTotalMins >= LastCycleTotalMins + IntervalTotalMins)
      {
        Spray();

        LastCycleHour = NowHour;
        LastCycleMin = NowMin;
        LastCycleSec = NowSec;
        LastCycleTotalMins = GetTotalMins(LastCycleHour, LastCycleMin);
      }
    }
  }
  else
  {
    if (LastCycleTotalMins != 0)
    {
      LastCycleHour = 0;
      LastCycleMin = 0;
      LastCycleSec = 0;
      LastCycleTotalMins = 0;
    }
  }
}

void Spray()
{
  Serial.print("\nSpray\n");

  digitalWrite(PinLed, LOW);
  digitalWrite(PinReley, HIGH);
  delay(PressTime);
  digitalWrite(PinReley, LOW);
  digitalWrite(PinLed, HIGH);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////Time|Wifi

int GetTotalMins(byte hours, byte mins)
{
  return hours * 60 + mins;
}

void WifiConnect()
{
  int counter = 0;

  WiFi.begin(ssid, password);

  Serial.println("\n\nTry connect to WIFI\n\n");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(200);
    if (++counter > 100)
      ESP.restart();
    Serial.print(".");
  }
  Serial.println("\n\nWiFi connected\n\n");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////Output

void Output()
{
  Serial.print("\n");

  time_t t = time(0); // get time now
  tm *now = localtime(&t);

  int NowHour = now->tm_hour;
  int NowMin = now->tm_min;
  int NowSec = now->tm_sec;

  Serial.print("Now:" + GetTimeString(NowHour, NowMin, NowSec) + " | " + "LastCycle:" + GetTimeString(LastCycleHour, LastCycleMin, LastCycleSec));

  Serial.print("\n");
}

String GetTimeString(byte hour, byte min, byte sec)
{
  String stringHour = String(hour);
  String stringMin = String(min);
  String stringSec = String(sec);

  if (stringHour.length() == 1)
  {
    stringHour = "0" + stringHour;
  }

  if (stringMin.length() == 1)
  {
    stringMin = "0" + stringMin;
  }

  if (stringSec.length() == 1)
  {
    stringSec = "0" + stringSec;
  }

  return stringHour + ":" + stringMin + ":" + stringSec;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////Time

bool getNTPtime(int sec)
{
  {
    uint32_t start = millis();
    do
    {
      time(&now);
      localtime_r(&now, &timeinfo);
      Serial.print(".");
      delay(10);
    } while (((millis() - start) <= (1000 * sec)) && (timeinfo.tm_year < (2016 - 1900)));
    if (timeinfo.tm_year <= (2016 - 1900))
      return false; // the NTP call was not successful
    Serial.print("now ");
    Serial.println(now);
    char time_output[30];
    strftime(time_output, 30, "%a  %d-%m-%y %T", localtime(&now));
  }
  return true;
}

void NetTimeStart()
{
  configTime(0, 0, NTP_SERVER);
  setenv("TZ", TZ_INFO, 1);

  if (getNTPtime(10))
  { // wait up to 10sec to sync
  }
  else
  {
    Serial.println("Time not set");
    ESP.restart();
  }

  lastNTPtime = time(&now);
  lastEntryTime = millis();
}
