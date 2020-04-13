/*
 * TinyRTC.h - library for DS1307 RTC
 * This library is made to reduce DS1307RTC library code size
 * Author: Alauddin Ansari
 * 2020-04-05
 * https://github.com/AlauddinTheWonder/arduino-tiny_ds1307.git
 */

#ifndef TinyRTC_h
#define TinyRTC_h

#define tmYearToCalendar(Y) ((Y) + 1970) // full four digit year
#define CalendarYrToTm(Y) ((Y)-1970)
#define y2kYearToTm(Y) ((Y) + 30)
#define tmYearToY2k(Y) ((Y)-30) // offset is from 2000
#define LEAP_YEAR(Y) (((1970 + (Y)) > 0) && !((1970 + (Y)) % 4) && (((1970 + (Y)) % 100) || !((1970 + (Y)) % 400)))

/* Useful Constants */
#define SECS_PER_MIN ((time_t)(60UL))
#define SECS_PER_HOUR ((time_t)(3600UL))
#define SECS_PER_DAY ((time_t)(SECS_PER_HOUR * 24UL))

static const uint8_t monthDays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}; // API starts months from 1, this array starts from 0

typedef unsigned long time_t;

typedef struct
{
    uint8_t Second;
    uint8_t Minute;
    uint8_t Hour;
    uint8_t Wday; // day of week, sunday is day 1
    uint8_t Day;
    uint8_t Month;
    uint16_t Year; // offset from 1970;
} tmElements_t;



// library interface description
class TinyRTC
{
    public:
        TinyRTC();
        static tmElements_t getTM();
        static time_t get();
        static bool set(time_t t);
        static bool set(int hr, int min, int sec, int day, int month, int yr);

        static bool chipPresent() { return exists; }
        static unsigned char isRunning();

    private:
        static bool exists;
        static bool read(tmElements_t &tm);
        static bool write(tmElements_t &tm);
        static void breakTime(time_t time, tmElements_t &tm); // break time_t into elements
        static time_t makeTime(const tmElements_t &tm);       // convert time elements into time_t
        static uint8_t dec2bcd(uint8_t num);
        static uint8_t bcd2dec(uint8_t num);
};

#ifdef RTC
    #undef RTC // workaround for Arduino Due, which defines "RTC"...
#endif

extern TinyRTC RTC;

#endif
