#if defined(__AVR_ATtiny84__) || defined(__AVR_ATtiny85__) || (__AVR_ATtiny2313__)
#include <TinyWireM.h>
#define Wire TinyWireM
#else
#include <Wire.h>
#endif
#include "TinyRTC.h"

#define RTC_CHIP_ADDR 0x68
#define TM_FIELDS 7

TinyRTC::TinyRTC()
{
    Wire.begin();
}

// PUBLIC FUNCTIONS
tmElements_t TinyRTC::getTM()
{
    tmElements_t tm;
    read(tm);
    tm.Year = tmYearToCalendar(tm.Year);
    return tm;
}

time_t TinyRTC::get() // Aquire data from buffer and convert to time_t
{
    tmElements_t tm;
    if (read(tm) == false)
        return 0;
    return (makeTime(tm));
}

bool TinyRTC::set(time_t t)
{
    tmElements_t tm;
    breakTime(t, tm);
    return write(tm);
}

bool TinyRTC::set(tmElements_t tm)
{
    return write(tm);
}

bool TinyRTC::set(int hr, int min, int sec, int dy, int mnth, int yr)
{
    if (yr > 99)
        yr = yr - 1970;
    else
        yr += 30;

    tmElements_t tm;

    tm.Year = yr;
    tm.Month = mnth;
    tm.Day = dy;
    tm.Hour = hr;
    tm.Minute = min;
    tm.Second = sec;
    set(makeTime(tm));
}

unsigned char TinyRTC::isRunning()
{
    Wire.beginTransmission(RTC_CHIP_ADDR);
    Wire.write((uint8_t)0x00);
    Wire.endTransmission();

    // Just fetch the seconds register and check the top bit
    Wire.requestFrom(RTC_CHIP_ADDR, 1);
    return !(Wire.read() & 0x80);
}

// PRIVATE FUNCTIONS

// Aquire data from the RTC chip in BCD format
bool TinyRTC::read(tmElements_t &tm)
{
    uint8_t sec;
    Wire.beginTransmission(RTC_CHIP_ADDR);
    Wire.write((uint8_t)0x00);

    if (Wire.endTransmission() != 0)
    {
        exists = false;
        return false;
    }
    exists = true;

    // request the 7 data fields   (secs, min, hr, dow, date, mth, yr)
    Wire.requestFrom(RTC_CHIP_ADDR, TM_FIELDS);
    if (Wire.available() < TM_FIELDS)
        return false;

    tm.Second = bcd2dec(Wire.read() & 0x7f);
    tm.Minute = bcd2dec(Wire.read());
    tm.Hour = bcd2dec(Wire.read() & 0x3f); // mask assumes 24hr clock
    tm.Wday = bcd2dec(Wire.read());
    tm.Day = bcd2dec(Wire.read());
    tm.Month = bcd2dec(Wire.read());
    tm.Year = y2kYearToTm((bcd2dec(Wire.read())));

    if (sec & 0x80)
        return false; // clock is halted
    return true;
}

bool TinyRTC::write(tmElements_t &tm)
{
    // To eliminate any potential race conditions,
    // stop the clock before writing the values,
    // then restart it after.
    Wire.beginTransmission(RTC_CHIP_ADDR);
    Wire.write((uint8_t)0x00); // reset register pointer
    Wire.write((uint8_t)0x80); // Stop the clock. The seconds will be written last
    Wire.write(dec2bcd(tm.Minute));
    Wire.write(dec2bcd(tm.Hour)); // sets 24 hour format
    Wire.write(dec2bcd(tm.Wday));
    Wire.write(dec2bcd(tm.Day));
    Wire.write(dec2bcd(tm.Month));
    Wire.write(dec2bcd(tmYearToY2k(tm.Year)));

    if (Wire.endTransmission() != 0)
    {
        exists = false;
        return false;
    }
    exists = true;

    // Now go back and set the seconds, starting the clock back up as a side effect
    Wire.beginTransmission(RTC_CHIP_ADDR);
    Wire.write((uint8_t)0x00);      // reset register pointer
    Wire.write(dec2bcd(tm.Second)); // write the seconds, with the stop bit clear to restart

    if (Wire.endTransmission() != 0)
    {
        exists = false;
        return false;
    }
    exists = true;
    return true;
}

void TinyRTC::breakTime(time_t timeInput, tmElements_t &tm)
{
    // break the given time_t into time components
    // this is a more compact version of the C library localtime function
    // note that year is offset from 1970 !!!

    uint8_t year;
    uint8_t month, monthLength;
    uint32_t time;
    unsigned long days;

    time = (uint32_t)timeInput;
    tm.Second = time % 60;
    time /= 60; // now it is minutes
    tm.Minute = time % 60;
    time /= 60; // now it is hours
    tm.Hour = time % 24;
    time /= 24;                     // now it is days
    tm.Wday = ((time + 4) % 7) + 1; // Sunday is day 1

    year = 0;
    days = 0;
    while ((unsigned)(days += (LEAP_YEAR(year) ? 366 : 365)) <= time)
    {
        year++;
    }
    tm.Year = year; // year is offset from 1970

    days -= LEAP_YEAR(year) ? 366 : 365;
    time -= days; // now it is days in this year, starting at 0

    days = 0;
    month = 0;
    monthLength = 0;
    for (month = 0; month < 12; month++)
    {
        if (month == 1)
        { // february
            if (LEAP_YEAR(year))
            {
                monthLength = 29;
            }
            else
            {
                monthLength = 28;
            }
        }
        else
        {
            monthLength = monthDays[month];
        }

        if (time >= monthLength)
        {
            time -= monthLength;
        }
        else
        {
            break;
        }
    }
    tm.Month = month + 1; // jan is month 1
    tm.Day = time + 1;    // day of month
}

time_t TinyRTC::makeTime(const tmElements_t &tm)
{
    // assemble time elements into time_t
    // note year argument is offset from 1970 (see macros in time.h to convert to other formats)
    // previous version used full four digit year (or digits since 2000),i.e. 2009 was 2009 or 9

    int i;
    uint32_t seconds;

    // seconds from 1970 till 1 jan 00:00:00 of the given year
    seconds = tm.Year * (SECS_PER_DAY * 365);
    for (i = 0; i < tm.Year; i++)
    {
        if (LEAP_YEAR(i))
        {
            seconds += SECS_PER_DAY; // add extra days for leap years
        }
    }

    // add days for this year, months start from 1
    for (i = 1; i < tm.Month; i++)
    {
        if ((i == 2) && LEAP_YEAR(tm.Year))
        {
            seconds += SECS_PER_DAY * 29;
        }
        else
        {
            seconds += SECS_PER_DAY * monthDays[i - 1]; //monthDay array starts from 0
        }
    }
    seconds += (tm.Day - 1) * SECS_PER_DAY;
    seconds += tm.Hour * SECS_PER_HOUR;
    seconds += tm.Minute * SECS_PER_MIN;
    seconds += tm.Second;
    return (time_t)seconds;
}

// Convert Decimal to Binary Coded Decimal (BCD)
uint8_t TinyRTC::dec2bcd(uint8_t num)
{
    return ((num / 10 * 16) + (num % 10));
}

// Convert Binary Coded Decimal (BCD) to Decimal
uint8_t TinyRTC::bcd2dec(uint8_t num)
{
    return ((num / 16 * 10) + (num % 16));
}

bool TinyRTC::exists = false;

TinyRTC RTC = TinyRTC();
