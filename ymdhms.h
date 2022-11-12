#pragma once

struct ymd {
    int year;
    int month;
    int day;

    ymd(int y, int m, int d) : year(y), month(m), day(d) {}

    friend
    bool operator==(const ymd &a, const ymd &b) {
        return a.year  == b.year
            && a.month == b.month
            && a.day   == b.day;
    }

    friend
    bool operator<(const ymd &a, const ymd &b) {
        return a.year < b.year ||
               (a.year == b.year &&
                (a.month < b.month ||
                 (a.month == b.month &&
                   a.day < b.day)));
    }

    int daynum() {
        // day index within year for each month
        static const int mdidx[12] = {
            0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

        // wonkily find leap years since epoch
        int yx = year - 1969;
        int leaps = yx / 4;
        if (yx % 4 == 3 && month > 1)
            leaps++;

        return (yx - 1) * 365 + mdidx[month] + day + leaps;
    }
};


struct hms {
    int hour;
    int min;
    int sec;

    hms(int h, int m, int s) : hour(h), min(m), sec(s) {}

    friend
    bool operator==(const hms &a, const hms &b) {
        return a.hour == b.hour
            && a.min  == b.min
            && a.sec  == b.sec;
    }

    friend
    bool operator<(const hms &a, const hms &b) {
        return a.hour < b.hour ||
               (a.hour == b.hour &&
                (a.min < b.min ||
                 (a.min == b.min &&
                   a.sec < b.sec)));
    }

    int timeofday() { return hour * (60 * 60) + min * 60 + sec; }
};

struct ymdhmsz {
    ymd dt;
    hms tm;
    int zz;

    ymdhmsz(const ymd &dt, const hms &tm, int zz) : dt(dt), tm(tm), zz(zz) {}

    long epoch() {
        return dt.daynum() * (60 * 60 * 24)
             + tm.timeofday()
             - zz * 36;   // 36 == 60 * 60 / 100
    }
};

