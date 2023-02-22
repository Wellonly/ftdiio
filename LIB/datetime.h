
#ifndef _DATETIME_H_
#define _DATETIME_H_
//static_assert(0, __DATE__); result format example: "Sep 28 2016"
//static_assert(0, __TIME__); result format example: "08:06:09"

#define DATE_AS_0xYYYYMMDD(year, month, day) ((year) << 16 | (month) << 8 | (day))
#define TIME_AS_0xHHMMSS00(hour, minute, second) ((hour) << 24 | (minute) << 16 | (second) << 8)


/// PREPROCESSOR DATE TIME HANDLING...
#define HOUR_OF_BUILD ((__TIME__ [0] - '0') * 10 + (__TIME__ [1] - '0'))
#define MINUTE_OF_BUILD ((__TIME__ [3] - '0') * 10 + (__TIME__ [4] - '0'))
#define SECOND_OF_BUILD ((__TIME__ [6] - '0') * 10 + (__TIME__ [7] - '0'))

#define DAY_OF_BUILD ((__DATE__ [4] == ' ' ? 0 : __DATE__ [4] - '0') * 10 + (__DATE__ [5] - '0'))

#define YEAR_OF_BUILD (__DATE__ [7] == '2' ? ((20 + (__DATE__ [8] - '0')) * 100 + (__DATE__ [9] - '0') * 10 + (__DATE__ [10] - '0')) : 0)

/* Jan
 * Feb
 * Mar
 * Apr
 * May
 * Jun
 * Jul
 * Aug
 * Sep
 * Oct
 * Nov
 * Dec
 */
#define MONTH_OF_BUILD ( \
      __DATE__ [2] == 'n' ? (__DATE__ [1] == 'u' ? 6 : 1) \
    : __DATE__ [2] == 'b' ? 2 \
    : __DATE__ [2] == 'r' ? (__DATE__ [0] == 'M' ? 3 : 4) \
    : __DATE__ [2] == 'y' ? 5 \
    : __DATE__ [2] == 'l' ? 7 \
    : __DATE__ [2] == 'g' ? 8 \
    : __DATE__ [2] == 'p' ? 9 \
    : __DATE__ [2] == 't' ? 10 \
    : __DATE__ [2] == 'v' ? 11 \
    : __DATE__ [2] == 'c' ? 12 : 0)

#define DATE_OF_BUILD_AS_0xYYYYMMDD() (YEAR_OF_BUILD << 16 | MONTH_OF_BUILD << 8 | DAY_OF_BUILD)
#define TIME_OF_BUILD_AS_0xHHMMSS00() (HOUR_OF_BUILD << 24 | MINUTE_OF_BUILD << 16 | SECOND_OF_BUILD)
#define DATE_OF_BUILD_AS_YYYYMMDD() ((YEAR_OF_BUILD * 10000) + (MONTH_OF_BUILD * 100) + DAY_OF_BUILD)

#define MINUTE2HOUR(m) (m/60)

#endif //_DATETIME_H_
