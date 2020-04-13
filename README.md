# arduino-tiny_rtc
TinyRTC library. Picked from DS1307RTC and Time lib and created a very tiny code to control the RTC DS1307 and DS3231 module.


### Type Def
```
> unsigned long time_t
> tmElements_t {
    uint8_t Second;
    uint8_t Minute;
    uint8_t Hour;
    uint8_t Wday; // day of week, sunday is day 1
    uint8_t Day;
    uint8_t Month;
    uint16_t Year; // offset from 1970;
}
```

### Available functions
```
> tmElements_t getTM();
> time_t get();
> bool set(time_t t);
> bool set(int Hour, int Minute, int Second, int Day, int Month, int Year);
> bool chipPresent();
> unsigned char isRunning();
```

## See examples
