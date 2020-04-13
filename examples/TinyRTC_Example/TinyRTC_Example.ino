#include <TinyRTC.h>


int hh, mm, ss, dd, ww, mmm, yy;

void setup() {
  Serial.begin(9600);

  Serial.println("Serial started");
  delay(500);

  connectDS1307();

  delay(1000);
}

void loop() {
  tmElements_t tm = RTC.getTM();

//  unsigned long tms = RTC.get();

  if (!RTC.chipPresent()) {
    Serial.println("Chip not present");
    delay(2000);
    return;
  }
  
  hh = tm.Hour;
  mm = tm.Minute;
  ss = tm.Second;
  dd = tm.Day;
  ww = tm.Wday;
  mmm = tm.Month;
  yy = tm.Year;

//  Serial.println(tms);

  Serial.print(yy);
  Serial.print("-");
  Serial.print(mmm);
  Serial.print("-");
  Serial.print(dd);
  Serial.print(" ");
  Serial.print(hh);
  Serial.print(":");
  Serial.print(mm);
  Serial.print(":");
  Serial.print(ss);
  Serial.println();
  
  delay(1000);
}

void connectDS1307() {
  Serial.println("Syncing time with DS1307...");
  
  while (!RTC.isRunning()) {
    Serial.println("RTC not running");
    Serial.println("Retring...");
    delay(2000);
  }

  Serial.println("DS1307 RTC is connected");

//  RTC.set(21, 27, 10, 4, 4, 2020); // (Hour, Min, Sec, Date, Month, Year)
//  RTC.set(1586016902);
}
