#pragma once
#include "string/string.hpp"
#include "string/string_functions.h"
#include <chrono>
#include <ctime>
#include <thread>

namespace svrlib {
class time {
    using time_point = std::chrono::time_point<std::chrono::steady_clock>;
    inline static time_point start_time_point_ = std::chrono::steady_clock::now();
    inline static int64_t start_millsecond = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    inline static int64_t offset_ = 0;

public:
    static int64_t second()
    {
        return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    }

    static int64_t millisecond()
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    }

    static int64_t microsecond()
    {
        return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    }

    static void offset(int64_t v)
    {
        offset_ = v;
    }

    static int64_t now()
    {
        auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_time_point_);
        return start_millsecond + diff.count() + offset_;
    }

    //e. 2017-11-11 16:03:11.635
    static size_t milltimestamp(int64_t t, char* buf, size_t len)
    {
        if (len < 23) {
            return 0;
        }

        auto mill = t;
        time_t now = mill / 1000;
        std::tm m;
        localtime(&now, &m);
        uint64_t ymd = (m.tm_year + 1900ULL) * svrlib::pow10(15)
            + (m.tm_mon + 1ULL) * svrlib::pow10(12)
            + m.tm_mday * svrlib::pow10(9)
            + m.tm_hour * svrlib::pow10(6)
            + m.tm_min * svrlib::pow10(3)
            + m.tm_sec;

        size_t n = uint64_to_str(ymd, buf);
        buf[4] = '-';
        buf[7] = '-';
        buf[10] = ' ';
        buf[13] = ':';
        buf[16] = ':';
        n += uint64_to_str(1000 + mill % 1000, buf + n);
        buf[n - 4] = '.';
        return n;
    }

    static time_t make_time(int year, int month, int day, int hour, int min, int sec)
    {
        assert(year >= 1990);
        assert(month > 0 && month <= 12);
        assert(day > 0 && day <= 31);
        assert(hour >= 0 && hour < 24);
        assert(min >= 0 && min < 60);
        assert(sec >= 0 && sec < 60);

        tm _tm;
        _tm.tm_year = (year - 1900);
        _tm.tm_mon = (month - 1);
        _tm.tm_mday = day;
        _tm.tm_hour = hour;
        _tm.tm_min = min;
        _tm.tm_sec = sec;
        _tm.tm_isdst = 0;

        return mktime(&_tm);
    }

    static bool get_time_from_str(const std::string& strTime, time_t& tTime)
    {
        struct tm tm;
        if (strptime(strTime.c_str(), "%Y-%m-%d %H:%M:%S", &tm) == NULL) {
            tTime = 0;
            return false;
        }
        tTime = mktime(&tm);
        return true;
    }

    static void sleep(uint32_t msec)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(msec));
    }

    //判断一个月有多少天
    static int GetMonthDays(int year, int month)
    {
        int flag = 0;
        if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0) {
            flag = 1; //是闰年
        }
        static int const month_normal[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
        static int const month_ruinian[12] = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

        return flag ? month_ruinian[month] : month_normal[month];
    }

    static std::tm* localtime(std::time_t* t, std::tm* result)
    {
#ifdef WIN32
        localtime_s(result, t);
#else
        localtime_r(t, result);
#endif
        return result;
    }
    static std::tm* localtime(std::time_t _t, struct tm* _Tm)
    {
        return localtime(&_t, _Tm);
    }

    static std::tm gmtime(const std::time_t& time_tt)
    {

#ifdef WIN32
        std::tm tm;
        gmtime_s(&tm, &time_tt);
#else
        std::tm tm;
        gmtime_r(&time_tt, &tm);
#endif
        return tm;
    }

    //------------------------------ 时间格式化(YYYY-MM-DD HH:MM:SS)
    static const char* time_format(uint64_t _time)
    {
        tm tmTime;
        memset(&tmTime, 0, sizeof(tmTime));
        time::localtime(_time, &tmTime);
        static char szDate[32] = { 0 };
        dSprintf(szDate, sizeof(szDate), "%.4d-%.2d-%.2d %.2d:%.2d:%.2d", tmTime.tm_year + 1900, tmTime.tm_mon + 1,
            tmTime.tm_mday, tmTime.tm_hour, tmTime.tm_min, tmTime.tm_sec);
        return szDate;
    }

    //--- 时间日期(YYYYMMDD)
    static const char* date_format(uint64_t _time)
    {
        tm tmTime;
        memset(&tmTime, 0, sizeof(tmTime));
        time::localtime(_time, &tmTime);
        static char szDate[32] = { 0 };
        dSprintf(szDate, sizeof(szDate), "%.4d%.2d%.2d", tmTime.tm_year + 1900, tmTime.tm_mon + 1, tmTime.tm_mday);
        return szDate;
    }
    //--- 获得今日开始时间
    static uint64_t getDayBeginTime()
    {
        static tm _tm;
        memset(&_tm, 0, sizeof(_tm));
        time::localtime(time::second(), &_tm);
        _tm.tm_hour = 0;
        _tm.tm_min = 0;
        _tm.tm_sec = 0;

        return mktime(&_tm);
    }

    //--- 获得时间相差天数
    static int32_t diffTimeDay(uint64_t _early, uint64_t _late)
    {
        if (_early == 0 || _late == 0)
            return 0;

        static tm tm_early;
        static tm tm_late;
        memset(&tm_early, 0, sizeof(tm_early));
        memset(&tm_late, 0, sizeof(tm_late));

        time::localtime(_early, &tm_early);
        time::localtime(_late, &tm_late);

        if (tm_early.tm_year > tm_late.tm_year)
            return 0;

        //同年同日
        if (tm_early.tm_year == tm_late.tm_year && tm_early.tm_yday == tm_late.tm_yday)
            return 0;

        //同年判断
        if (tm_early.tm_year == tm_late.tm_year) {
            if (tm_early.tm_yday >= tm_late.tm_yday)
                return 0;

            return (tm_late.tm_yday - tm_early.tm_yday);
        }

        int32_t iDay = 0;
        //不同年时
        if (tm_early.tm_year != tm_late.tm_year) {
            tm tm_temp = tm_early;

            //获取12月31日时间
            tm_temp.tm_mon = 11;
            tm_temp.tm_mday = 31;
            tm_temp.tm_yday = 0;
            uint64_t _temp = mktime(&tm_temp);

            time::localtime(_temp, &tm_temp);
            iDay = tm_temp.tm_yday - tm_early.tm_yday;

            iDay += 1; //跨年+1

            //获得相差年天数
            for (int32_t i = tm_early.tm_year + 1; i < tm_late.tm_year; i++) {
                tm_temp.tm_year++;
                tm_temp.tm_yday = 0;

                _temp = mktime(&tm_temp);
                time::localtime(_temp, &tm_temp);

                iDay += tm_temp.tm_yday;
                iDay += 1; //跨年+1
            }
        }

        return (iDay + tm_late.tm_yday);
    }

    //------------------------------ 获得时间相差周数
    static int32_t diffTimeWeek(uint64_t _early, uint64_t _late)
    {
        if (_early == 0 || _late == 0)
            return 0;

        static tm tm_early;
        static tm tm_late;
        memset(&tm_early, 0, sizeof(tm_early));
        memset(&tm_late, 0, sizeof(tm_late));

        time::localtime(_early, &tm_early);
        time::localtime(_late, &tm_late);

        if (tm_early.tm_year > tm_late.tm_year)
            return 0;

        //同年同日
        if (tm_early.tm_year == tm_late.tm_year && tm_early.tm_yday == tm_late.tm_yday)
            return 0;

        //计算两个日期的每一个周六相差多少天
        if (tm_early.tm_wday != 6)
            tm_early.tm_mday += (6 - tm_early.tm_wday);
        if (tm_late.tm_wday != 6)
            tm_late.tm_mday += (6 - tm_late.tm_wday);

        int32_t iDay = diffTimeDay(mktime(&tm_early), mktime(&tm_late));

        int32_t iWeek = 0;
        if (iDay > 0)
            iWeek = iDay / 7; //肯定相差都是7的倍数因为都是周六

        return iWeek;
    }
    //------------------------------ 获得时间相差月数
    static int32_t diffTimeMonth(uint64_t _early, uint64_t _late)
    {
        if (_early == 0 || _late == 0)
            return 0;

        static tm tm_early;
        static tm tm_late;
        memset(&tm_early, 0, sizeof(tm_early));
        memset(&tm_late, 0, sizeof(tm_late));

        time::localtime(_early, &tm_early);
        time::localtime(_late, &tm_late);

        if (tm_early.tm_year > tm_late.tm_year)
            return 0;

        //同年同月
        if (tm_early.tm_year == tm_late.tm_year && tm_early.tm_mon == tm_late.tm_mon)
            return 0;

        //同年判断
        if (tm_early.tm_year == tm_late.tm_year)
            return (tm_late.tm_mon - tm_early.tm_mon);

        int32_t iMon = 0;
        //不同年时
        if (tm_early.tm_year != tm_late.tm_year) {
            //计算相差年数
            iMon = (tm_late.tm_year - tm_early.tm_year) * 12;
            //再计算相差月数
            iMon += tm_late.tm_mon;
            if (iMon >= tm_early.tm_mon)
                iMon -= tm_early.tm_mon;
            else
                iMon = 0;
        }

        return iMon;
    }

    //---- 获得系统启动毫秒(64位)【需要手动更新】
    static uint64_t getSystemTick64(bool flush = false)
    {
        static uint64_t g_uSystemTick64 = 0;
        if (!g_uSystemTick64 || flush) {
            g_uSystemTick64 = time::millisecond();
        }
        return g_uSystemTick64;
    }

    //---- 获得系统时间
    static uint64_t getSysTime(bool flush = false)
    {
        static uint64_t g_uSystemTime = 0;
        if (!g_uSystemTime || flush) {
            g_uSystemTime = time::second();
        }
        return g_uSystemTime;
    }
};

inline bool operator==(const std::tm& tm1, const std::tm& tm2)
{
    return (tm1.tm_sec == tm2.tm_sec && tm1.tm_min == tm2.tm_min && tm1.tm_hour == tm2.tm_hour && tm1.tm_mday == tm2.tm_mday && tm1.tm_mon == tm2.tm_mon && tm1.tm_year == tm2.tm_year && tm1.tm_isdst == tm2.tm_isdst);
}

inline bool operator!=(const std::tm& tm1, const std::tm& tm2)
{
    return !(tm1 == tm2);
}
};
