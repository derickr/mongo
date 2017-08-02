/*    Copyright 2010 10gen Inc.
 *
 *    This program is free software: you can redistribute it and/or  modify
 *    it under the terms of the GNU Affero General Public License, version 3,
 *    as published by the Free Software Foundation.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Affero General Public License for more details.
 *
 *    You should have received a copy of the GNU Affero General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *    As a special exception, the copyright holders give permission to link the
 *    code of portions of this program with the OpenSSL library under certain
 *    conditions as described in each individual source file and distribute
 *    linked combinations including the program with the OpenSSL library. You
 *    must comply with the GNU Affero General Public License in all respects
 *    for all of the code used other than as permitted herein. If you modify
 *    file(s) with this exception, you may extend this exception to your
 *    version of the file(s), but you are not obligated to do so. If you do not
 *    wish to do so, delete this exception statement from your version. If you
 *    delete this exception statement from all source files in the program,
 *    then also delete it in the license file.
 */

#include "mongo/platform/basic.h"

#include "mongo/util/time_support.h"

#include <cstdint>
#include <cstdio>
#include <iostream>
#include <string>
#include <timelib.h>

#include "mongo/base/init.h"
#include "mongo/base/parse_number.h"
#include "mongo/bson/util/builder.h"
#include "mongo/stdx/thread.h"
#include "mongo/util/assert_util.h"
#include "mongo/util/date_time_support.h"
#include "mongo/util/mongoutils/str.h"

#ifdef _WIN32
#include "mongo/util/concurrency/mutex.h"
#include "mongo/util/system_tick_source.h"
#include "mongo/util/timer.h"
#include <boost/date_time/filetime_functions.hpp>
#endif

namespace mongo {

Date_t Date_t::now() {
    return fromMillisSinceEpoch(curTimeMillis64());
}

Date_t::Date_t(stdx::chrono::system_clock::time_point tp)
    : millis(durationCount<Milliseconds>(tp - stdx::chrono::system_clock::from_time_t(0))) {}

stdx::chrono::system_clock::time_point Date_t::toSystemTimePoint() const {
    return stdx::chrono::system_clock::from_time_t(0) + toDurationSinceEpoch().toSystemDuration();
}

bool Date_t::isFormattable() const {
    if (millis < -2145916800000LL) {  // "1902-01-01T00:00:00Z"
        return false;
    }
    if (sizeof(time_t) == sizeof(int32_t)) {
        return millis < 2147483647000LL;  // "2038-01-19T03:14:07Z"
    } else {
        return millis < 32535215999000LL;  // "3000-12-31T23:59:59Z"
    }
}


// jsTime_virtual_skew is just for testing. a test command manipulates it.
long long jsTime_virtual_skew = 0;
thread_local long long jsTime_virtual_thread_skew = 0;

using std::string;

string dateToString(Date_t date, bool local, string format) {
    invariant(date.isFormattable());

    if (local) {
        TimeZone zone;
#ifdef _WIN32
        // NOTE(schwerin): The value stored by _get_timezone is the value one adds to local time
        // to get UTC.  This is opposite of the ISO-8601 meaning of the timezone offset.
        // NOTE(schwerin): Microsoft's timezone code always assumes US rules for daylight
        // savings time.  We can do no better without completely reimplementing localtime_s and
        // related time library functions.
        long msTimeZone;
        int  dayLightHours;
        _get_timezone(&msTimeZone);
        _get_daylight(&dayLightHours);
        if (dayLightHours) {
            msTimeZone -= 3600 * dayLightHours;
        }

        zone = mongo::TimeZone(Seconds(-msTimeZone));
#else
        time_t t = date.toTimeT();
        struct tm lt = {0};
        localtime_r(&t, &lt);

        zone = mongo::TimeZone(Seconds(lt.tm_gmtoff));
#endif
        return zone.formatDate(format, date);
    } else {
        return mongo::TimeZoneDatabase::utcZone().formatDate(format, date);
    }
}

string time_t_to_String_short(time_t time) {
    Date_t date = Date_t::fromTimeT(time);

    return dateToString(date, true, kCTimeFormatWithoutDayName);
}

// uses ISO 8601 dates without trailing Z
// colonsOk should be false when creating filenames
string terseCurrentTime(bool colonsOk) {
    return dateToString(
        Date_t::now(), false, colonsOk ? kTerseCurrentTimeColon : kTerseCurrentTimeHyphen);
}

string terseUTCCurrentTime() {
    return dateToString(Date_t::now(), false, kTerseCurrentTimeHyphenUTC);
}

string dateToISOStringUTC(Date_t date) {
    return dateToString(date, false, kISODateFormatUTC);
}

string dateToISOStringLocal(Date_t date) {
    return dateToString(date, true, kISODateFormatLocal);
}

string dateToCtimeString(Date_t date) {
    return dateToString(date, true, kCTimeFormat);
}

void outputDateAsISOStringUTC(std::ostream& os, Date_t date) {
    os << dateToString(date, false, kISODateFormatUTC);
}

void outputDateAsISOStringLocal(std::ostream& os, Date_t date) {
    os << dateToString(date, true, kISODateFormatLocal);
}

void outputDateAsCtime(std::ostream& os, Date_t date) {
    os << dateToString(date, true, kCTimeFormat);
}

static timelib_tzinfo* fromisostring_gettzinfowrapper(char* tz_id,
                                                      const _timelib_tzdb* db,
                                                      int* error) {
    return nullptr;
}

namespace timeSupportHelpers {

/**
 * A custom-deleter which destructs a timelib_time* when it goes out of scope.
 */
struct TimelibTimeDeleter {
    TimelibTimeDeleter() = default;
    void operator()(timelib_time* time);
};

/**
 * A custom-deleter which destructs a timelib_error_container* when it goes out of scope.
 */
struct TimelibErrorContainerDeleter {
    TimelibErrorContainerDeleter() = default;
    void operator()(timelib_error_container* errorContainer);
};

void TimelibTimeDeleter::operator()(timelib_time* time) {
    timelib_time_dtor(time);
}

void TimelibErrorContainerDeleter::operator()(timelib_error_container* errorContainer) {
    timelib_error_container_dtor(errorContainer);
}

}  // namespace

StatusWith<Date_t> dateFromISOString(StringData dateString) {
    std::unique_ptr<timelib_error_container,
                    mongo::timeSupportHelpers::TimelibErrorContainerDeleter>
        errors{};
    timelib_error_container* rawErrors;

    std::unique_ptr<timelib_time, mongo::timeSupportHelpers::TimelibTimeDeleter> parsedTime(
        timelib_strtotime(const_cast<char*>(dateString.toString().c_str()),
                          dateString.size(),
                          &rawErrors,
                          nullptr,
                          fromisostring_gettzinfowrapper));
    errors.reset(rawErrors);

    // If the parsed string has a warning or error, throw an error.
    if (errors->warning_count || errors->error_count) {
        StringBuilder sb;

        sb << "Error parsing date string '" << dateString << "'";

        for (int i = 0; i < errors->error_count; ++i) {
            auto error = errors->error_messages[i];

            sb << "; " << error.position << ": ";
            // We need to override the error message for unknown time zone identifiers, as we never
            // make them available. We also change the error code to signal this is a different
            // error than a normal parse error.
            if (error.error_code == TIMELIB_ERR_TZID_NOT_FOUND) {
                sb << "passing a time zone identifier as part of the string is not allowed";
            } else {
                sb << error.message;
            }
            sb << " '" << error.character << "'";
        }

        for (int i = 0; i < errors->warning_count; ++i) {
            sb << "; " << errors->warning_messages[i].position << ": "
               << errors->warning_messages[i].message << " '"
               << errors->warning_messages[i].character << "'";
        }

        return StatusWith<Date_t>(ErrorCodes::BadValue, sb.str());
    }
    timelib_time* tmp = parsedTime.get();
    if (!(tmp->zone_type == 1 || (tmp->zone_type == 2 && tmp->tz_abbr && tmp->tz_abbr[0] == 'Z'))) {
        return StatusWith<Date_t>(ErrorCodes::BadValue, "Not local time");
    }

    timelib_update_ts(parsedTime.get(), nullptr);

    return Date_t::fromMillisSinceEpoch(
        durationCount<Milliseconds>(Seconds(parsedTime->sse) + Microseconds(parsedTime->us)));
}

std::string Date_t::toString() const {
    if (isFormattable()) {
        return dateToISOStringLocal(*this);
    } else {
        return str::stream() << "Date(" << millis << ")";
    }
}

time_t Date_t::toTimeT() const {
    const auto secs = millis / 1000;
    verify(secs >= std::numeric_limits<time_t>::min());
    verify(secs <= std::numeric_limits<time_t>::max());
    return secs;
}

boost::gregorian::date currentDate() {
    boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
    return now.date();
}

// parses time of day in "hh:mm" format assuming 'hh' is 00-23
bool toPointInTime(const string& str, boost::posix_time::ptime* timeOfDay) {
    int hh = 0;
    int mm = 0;
    if (2 != sscanf(str.c_str(), "%d:%d", &hh, &mm)) {
        return false;
    }

    // verify that time is well formed
    if ((hh / 24) || (mm / 60)) {
        return false;
    }

    boost::posix_time::ptime res(currentDate(),
                                 boost::posix_time::hours(hh) + boost::posix_time::minutes(mm));
    *timeOfDay = res;
    return true;
}

void sleepsecs(int s) {
    stdx::this_thread::sleep_for(Seconds(s).toSystemDuration());
}

void sleepmillis(long long s) {
    stdx::this_thread::sleep_for(Milliseconds(s).toSystemDuration());
}
void sleepmicros(long long s) {
    stdx::this_thread::sleep_for(Microseconds(s).toSystemDuration());
}

void Backoff::nextSleepMillis() {
    // Get the current time
    unsigned long long currTimeMillis = curTimeMillis64();

    int lastSleepMillis = _lastSleepMillis;

    if (_lastErrorTimeMillis == 0 || _lastErrorTimeMillis > currTimeMillis /* VM bugs exist */)
        _lastErrorTimeMillis = currTimeMillis;
    unsigned long long lastErrorTimeMillis = _lastErrorTimeMillis;
    _lastErrorTimeMillis = currTimeMillis;

    lastSleepMillis = getNextSleepMillis(lastSleepMillis, currTimeMillis, lastErrorTimeMillis);

    // Store the last slept time
    _lastSleepMillis = lastSleepMillis;
    sleepmillis(lastSleepMillis);
}

int Backoff::getNextSleepMillis(int lastSleepMillis,
                                unsigned long long currTimeMillis,
                                unsigned long long lastErrorTimeMillis) const {
    // Backoff logic

    // Get the time since the last error
    unsigned long long timeSinceLastErrorMillis = currTimeMillis - lastErrorTimeMillis;

    // Makes the cast below safe
    verify(_resetAfterMillis >= 0);

    // If we haven't seen another error recently (3x the max wait time), reset our
    // wait counter.
    if (timeSinceLastErrorMillis > (unsigned)(_resetAfterMillis))
        lastSleepMillis = 0;

    // Makes the test below sane
    verify(_maxSleepMillis > 0);

    // Wait a power of two millis
    if (lastSleepMillis == 0)
        lastSleepMillis = 1;
    else
        lastSleepMillis = std::min(lastSleepMillis * 2, _maxSleepMillis);

    return lastSleepMillis;
}

// DO NOT TOUCH except for testing
void jsTimeVirtualSkew(long long skew) {
    jsTime_virtual_skew = skew;
}
long long getJSTimeVirtualSkew() {
    return jsTime_virtual_skew;
}

void jsTimeVirtualThreadSkew(long long skew) {
    jsTime_virtual_thread_skew = skew;
}

long long getJSTimeVirtualThreadSkew() {
    return jsTime_virtual_thread_skew;
}

/** Date_t is milliseconds since epoch */
Date_t jsTime() {
    return Date_t::now() + Milliseconds(getJSTimeVirtualThreadSkew()) +
        Milliseconds(getJSTimeVirtualSkew());
}

#ifdef _WIN32  // no gettimeofday on windows
unsigned long long curTimeMillis64() {
    using stdx::chrono::system_clock;
    return static_cast<unsigned long long>(
        durationCount<Milliseconds>(system_clock::now() - system_clock::from_time_t(0)));
}

static unsigned long long getFiletime() {
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    return *reinterpret_cast<unsigned long long*>(&ft);
}

static unsigned long long getPerfCounter() {
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return li.QuadPart;
}

static unsigned long long baseFiletime = 0;
static unsigned long long basePerfCounter = 0;
static unsigned long long resyncInterval = 0;
static SimpleMutex _curTimeMicros64ReadMutex;
static SimpleMutex _curTimeMicros64ResyncMutex;

typedef WINBASEAPI VOID(WINAPI* pGetSystemTimePreciseAsFileTime)(
    _Out_ LPFILETIME lpSystemTimeAsFileTime);

static pGetSystemTimePreciseAsFileTime GetSystemTimePreciseAsFileTimeFunc;

MONGO_INITIALIZER(Init32TimeSupport)(InitializerContext*) {
    HINSTANCE kernelLib = LoadLibraryA("kernel32.dll");
    if (kernelLib) {
        GetSystemTimePreciseAsFileTimeFunc = reinterpret_cast<pGetSystemTimePreciseAsFileTime>(
            GetProcAddress(kernelLib, "GetSystemTimePreciseAsFileTime"));
    }

    return Status::OK();
}

static unsigned long long resyncTime() {
    stdx::lock_guard<SimpleMutex> lkResync(_curTimeMicros64ResyncMutex);
    unsigned long long ftOld;
    unsigned long long ftNew;
    ftOld = ftNew = getFiletime();
    do {
        ftNew = getFiletime();
    } while (ftOld == ftNew);  // wait for filetime to change

    unsigned long long newPerfCounter = getPerfCounter();

    // Make sure that we use consistent values for baseFiletime and basePerfCounter.
    //
    stdx::lock_guard<SimpleMutex> lkRead(_curTimeMicros64ReadMutex);
    baseFiletime = ftNew;
    basePerfCounter = newPerfCounter;
    resyncInterval = 60 * SystemTickSource::get()->getTicksPerSecond();
    return newPerfCounter;
}

unsigned long long curTimeMicros64() {
    // Windows 8/2012 & later support a <1us time function
    if (GetSystemTimePreciseAsFileTimeFunc != NULL) {
        FILETIME time;
        GetSystemTimePreciseAsFileTimeFunc(&time);
        return boost::date_time::winapi::file_time_to_microseconds(time);
    }

    // Get a current value for QueryPerformanceCounter; if it is not time to resync we will
    // use this value.
    //
    unsigned long long perfCounter = getPerfCounter();

    // Periodically resync the timer so that we don't let timer drift accumulate.  Testing
    // suggests that we drift by about one microsecond per minute, so resynching once per
    // minute should keep drift to no more than one microsecond.
    //
    if ((perfCounter - basePerfCounter) > resyncInterval) {
        perfCounter = resyncTime();
    }

    // Make sure that we use consistent values for baseFiletime and basePerfCounter.
    //
    stdx::lock_guard<SimpleMutex> lkRead(_curTimeMicros64ReadMutex);

    // Compute the current time in FILETIME format by adding our base FILETIME and an offset
    // from that time based on QueryPerformanceCounter.  The math is (logically) to compute the
    // fraction of a second elapsed since 'baseFiletime' by taking the difference in ticks
    // and dividing by the tick frequency, then scaling this fraction up to units of 100
    // nanoseconds to match the FILETIME format.  We do the multiplication first to avoid
    // truncation while using only integer instructions.
    //
    unsigned long long computedTime = baseFiletime +
        ((perfCounter - basePerfCounter) * 10 * 1000 * 1000) /
            SystemTickSource::get()->getTicksPerSecond();

    // Convert the computed FILETIME into microseconds since the Unix epoch (1/1/1970).
    //
    return boost::date_time::winapi::file_time_to_microseconds(computedTime);
}

#else
#include <sys/time.h>
unsigned long long curTimeMillis64() {
    timeval tv;
    gettimeofday(&tv, NULL);
    return ((unsigned long long)tv.tv_sec) * 1000 + tv.tv_usec / 1000;
}

unsigned long long curTimeMicros64() {
    timeval tv;
    gettimeofday(&tv, NULL);
    return (((unsigned long long)tv.tv_sec) * 1000 * 1000) + tv.tv_usec;
}
#endif

}  // namespace mongo
