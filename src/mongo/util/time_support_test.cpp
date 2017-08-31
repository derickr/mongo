/*    Copyright 2013 10gen Inc.
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

#define MONGO_LOG_DEFAULT_COMPONENT ::mongo::logger::LogComponent::kDefault

#include <cstdlib>
#include <ctime>
#include <string>
#include <sstream>

#include "mongo/base/init.h"
#include "mongo/platform/basic.h"
#include "mongo/unittest/unittest.h"
#include "mongo/util/log.h"
#include "mongo/util/time_support.h"
#include "mongo/util/date_time_support.h"

namespace mongo {
namespace {

const TimeZoneDatabase kDefaultTimeZoneDatabase{};

/**
 * To make this test deterministic, we set the time zone to America/New_York.
 */
#ifdef _WIN32
char tzEnvString[] = "TZ=EST+5EDT";
#else
char tzEnvString[] = "TZ=America/New_York";
#endif
MONGO_INITIALIZER(SetTimeZoneToEasternForTest)(InitializerContext*) {
    if (-1 == putenv(tzEnvString)) {
        return Status(ErrorCodes::BadValue, errnoWithDescription());
    }
    tzset();
    return Status::OK();
}

TEST(TimeFormatting, DateAsISO8601UTCString) {
    ASSERT_EQUALS(std::string("1902-01-30T01:06:40.981Z"),
                  dateToISOStringUTC(Date_t::fromMillisSinceEpoch(-2143407199019LL)));
    ASSERT_EQUALS(std::string("1970-01-01T00:00:00.000Z"), dateToISOStringUTC(Date_t()));
    ASSERT_EQUALS(std::string("1970-06-30T01:06:40.981Z"),
                  dateToISOStringUTC(Date_t::fromMillisSinceEpoch(15556000981LL)));
    ASSERT_EQUALS(std::string("2058-02-20T18:29:11.100Z"),
                  dateToISOStringUTC(Date_t::fromMillisSinceEpoch(2781455351100LL)));
    ASSERT_EQUALS(std::string("2013-02-20T18:29:11.100Z"),
                  dateToISOStringUTC(Date_t::fromMillisSinceEpoch(1361384951100LL)));
}

TEST(TimeFormatting, DateAsISO8601LocalString) {
    ASSERT_EQUALS(std::string("1969-12-31T19:00:00.000-0500"), dateToISOStringLocal(Date_t()));
    ASSERT_EQUALS(std::string("1970-06-29T21:06:40.981-0400"),
                  dateToISOStringLocal(Date_t::fromMillisSinceEpoch(15556000981LL)));
    ASSERT_EQUALS(std::string("2058-02-20T13:29:11.100-0500"),
                  dateToISOStringLocal(Date_t::fromMillisSinceEpoch(2781455351100LL)));
    ASSERT_EQUALS(std::string("2013-02-20T13:29:11.100-0500"),
                  dateToISOStringLocal(Date_t::fromMillisSinceEpoch(1361384951100LL)));
}

TEST(TimeFormatting, DateAsCtimeString) {
    ASSERT_EQUALS(std::string("Wed Dec 31 19:00:00.000"), dateToCtimeString(Date_t()));
    ASSERT_EQUALS(std::string("Mon Jun 29 21:06:40.981"),
                  dateToCtimeString(Date_t::fromMillisSinceEpoch(15556000981LL)));
    ASSERT_EQUALS(std::string("Wed Feb 20 13:29:11.100"),
                  dateToCtimeString(Date_t::fromMillisSinceEpoch(2781455351100LL)));
    ASSERT_EQUALS(std::string("Wed Feb 20 13:29:11.100"),
                  dateToCtimeString(Date_t::fromMillisSinceEpoch(1361384951100LL)));
}

TEST(TimeFormatting, DateAsCtimeStringWithAllDaysOfWeek) {
    ASSERT_EQUALS(std::string("Sun Jul 30 14:18:11.331"),
                  dateToCtimeString(Date_t::fromMillisSinceEpoch(1501438691331)));
    ASSERT_EQUALS(std::string("Mon Jul 31 14:18:11.331"),
                  dateToCtimeString(Date_t::fromMillisSinceEpoch(1501525091331)));
    ASSERT_EQUALS(std::string("Tue Aug  1 14:18:11.331"),
                  dateToCtimeString(Date_t::fromMillisSinceEpoch(1501611491331)));
    ASSERT_EQUALS(std::string("Wed Aug  2 14:18:11.331"),
                  dateToCtimeString(Date_t::fromMillisSinceEpoch(1501697891331)));
    ASSERT_EQUALS(std::string("Thu Aug  3 14:18:11.331"),
                  dateToCtimeString(Date_t::fromMillisSinceEpoch(1501784291331)));
    ASSERT_EQUALS(std::string("Fri Aug  4 14:18:11.331"),
                  dateToCtimeString(Date_t::fromMillisSinceEpoch(1501870691331)));
    ASSERT_EQUALS(std::string("Sat Aug  5 14:18:11.331"),
                  dateToCtimeString(Date_t::fromMillisSinceEpoch(1501957091331)));
}

TEST(TimeFormatting, DateAsTerseColons) {
    ASSERT_EQUALS(std::string("1970-06-30T01:06:40"),
                  dateToString(Date_t::fromMillisSinceEpoch(15556000981LL),
                               false,
                               Date_t::kTerseCurrentTimeColon));
    ASSERT_EQUALS(std::string("2058-02-20T18:29:11"),
                  dateToString(Date_t::fromMillisSinceEpoch(2781455351100LL),
                               false,
                               Date_t::kTerseCurrentTimeColon));
}

TEST(TimeFormatting, DateAsTerseHyphens) {
    ASSERT_EQUALS(std::string("1970-06-30T01-06-40"),
                  dateToString(Date_t::fromMillisSinceEpoch(15556000981LL),
                               false,
                               Date_t::kTerseCurrentTimeHyphen));
    ASSERT_EQUALS(std::string("2058-02-20T18-29-11"),
                  dateToString(Date_t::fromMillisSinceEpoch(2781455351100LL),
                               false,
                               Date_t::kTerseCurrentTimeHyphen));
}

static std::string stringstreamDate(void (*formatter)(std::ostream&, Date_t), Date_t date) {
    std::ostringstream os;
    formatter(os, date);
    return os.str();
}

TEST(TimeFormatting, DateAsISO8601UTCStream) {
    ASSERT_EQUALS(std::string("1970-01-01T00:00:00.000Z"),
                  stringstreamDate(outputDateAsISOStringUTC, Date_t()));
    ASSERT_EQUALS(
        std::string("1970-06-30T01:06:40.981Z"),
        stringstreamDate(outputDateAsISOStringUTC, Date_t::fromMillisSinceEpoch(15556000981LL)));
    ASSERT_EQUALS(
        std::string("2058-02-20T18:29:11.100Z"),
        stringstreamDate(outputDateAsISOStringUTC, Date_t::fromMillisSinceEpoch(2781455351100LL)));
    ASSERT_EQUALS(
        std::string("2013-02-20T18:29:11.100Z"),
        stringstreamDate(outputDateAsISOStringUTC, Date_t::fromMillisSinceEpoch(1361384951100LL)));
}

TEST(TimeFormatting, DateAsISO8601LocalStream) {
    ASSERT_EQUALS(std::string("1969-12-31T19:00:00.000-0500"),
                  stringstreamDate(outputDateAsISOStringLocal, Date_t()));
    ASSERT_EQUALS(
        std::string("1970-06-29T21:06:40.981-0400"),
        stringstreamDate(outputDateAsISOStringLocal, Date_t::fromMillisSinceEpoch(15556000981LL)));
    ASSERT_EQUALS(std::string("2058-02-20T13:29:11.100-0500"),
                  stringstreamDate(outputDateAsISOStringLocal,
                                   Date_t::fromMillisSinceEpoch(2781455351100LL)));
    ASSERT_EQUALS(std::string("2013-02-20T13:29:11.100-0500"),
                  stringstreamDate(outputDateAsISOStringLocal,
                                   Date_t::fromMillisSinceEpoch(1361384951100LL)));
}

TEST(TimeFormatting, DateAsCtimeStream) {
    ASSERT_EQUALS(std::string("Wed Dec 31 19:00:00.000"),
                  stringstreamDate(outputDateAsCtime, Date_t::fromMillisSinceEpoch(0)));
    ASSERT_EQUALS(std::string("Mon Jun 29 21:06:40.981"),
                  stringstreamDate(outputDateAsCtime, Date_t::fromMillisSinceEpoch(15556000981LL)));
    ASSERT_EQUALS(
        std::string("Wed Feb 20 13:29:11.100"),
        stringstreamDate(outputDateAsCtime, Date_t::fromMillisSinceEpoch(2781455351100LL)));
    ASSERT_EQUALS(
        std::string("Wed Feb 20 13:29:11.100"),
        stringstreamDate(outputDateAsCtime, Date_t::fromMillisSinceEpoch(1361384951100LL)));
}

TEST(TimeParsing, DateAsISO8601UTC) {
    // Allowed date format:
    // YYYY-MM-DDTHH:MM[:SS[.m[m[m]]]]Z
    // Year, month, day, hour, and minute are required, while the seconds component and one to
    // three milliseconds are optional.

    StatusWith<Date_t> swull = dateFromISOString("1971-02-03T04:05:06.789Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 34401906789LL);

    swull = dateFromISOString("1971-02-03T04:05:06.78Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 34401906780LL);

    swull = dateFromISOString("1971-02-03T04:05:06.7Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 34401906700LL);

    swull = dateFromISOString("1971-02-03T04:05:06Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 34401906000LL);

    swull = dateFromISOString("1971-02-03T04:05Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 34401900000LL);

    swull = dateFromISOString("1970-01-01T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 0LL);

    swull = dateFromISOString("1970-06-30T01:06:40.981Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 15556000981LL);

    swull = dateFromISOString("2058-02-20T18:29:11.100Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 2781455351100LL);

    swull = dateFromISOString("3001-01-01T08:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 32535244800000LL);

    swull = dateFromISOString("2013-02-20T18:29:11.100Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 1361384951100LL);
}

TEST(TimeParsing, DateAsISO8601Local) {
    // Allowed date format:
    // YYYY-MM-DDTHH:MM[:SS[.m[m[m]]]]+HHMM
    // Year, month, day, hour, and minute are required, while the seconds component and one to
    // three milliseconds are optional.  The time zone offset must be four digits.

    StatusWith<Date_t> swull = dateFromISOString("1971-02-03T09:16:06.789+0511");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 34401906789LL);

    swull = dateFromISOString("1971-02-03T09:16:06.78+0511");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 34401906780LL);

    swull = dateFromISOString("1971-02-03T09:16:06.7+0511");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 34401906700LL);

    swull = dateFromISOString("1971-02-03T09:16:06+0511");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 34401906000LL);

    swull = dateFromISOString("1971-02-03T09:16+0511");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 34401900000LL);

    swull = dateFromISOString("1970-01-01T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 0LL);

    swull = dateFromISOString("1970-06-30T01:06:40.981Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 15556000981LL);

    // Local times not supported
    // swull = dateFromISOString("1970-01-01T00:00:00.001");
    // ASSERT_OK(swull.getStatus());
    // ASSERT_EQUALS(swull.getValue().asInt64(), 18000001LL);

    // swull = dateFromISOString("1970-01-01T00:00:00.01");
    // ASSERT_OK(swull.getStatus());
    // ASSERT_EQUALS(swull.getValue().asInt64(), 18000010LL);

    // swull = dateFromISOString("1970-01-01T00:00:00.1");
    // ASSERT_OK(swull.getStatus());
    // ASSERT_EQUALS(swull.getValue().asInt64(), 18000100LL);

    // swull = dateFromISOString("1970-01-01T00:00:01");
    // ASSERT_OK(swull.getStatus());
    // ASSERT_EQUALS(swull.getValue().asInt64(), 18001000LL);

    // swull = dateFromISOString("1970-01-01T00:01");
    // ASSERT_OK(swull.getStatus());
    // ASSERT_EQUALS(swull.getValue().asInt64(), 18060000LL);

    swull = dateFromISOString("1970-06-29T21:06:40.981-0400");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 15556000981LL);

    swull = dateFromISOString("2058-02-20T13:29:11.100-0500");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 2781455351100LL);

    swull = dateFromISOString("3000-12-31T23:59:59Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 32535215999000LL);

    swull = dateFromISOString("2038-01-19T03:14:07Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 2147483647000LL);

    swull = dateFromISOString("2013-02-20T13:29:11.100-0500");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 1361384951100LL);

    swull = dateFromISOString("2013-02-20T13:29:11.100-0501");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 1361385011100LL);
}

TEST(TimeParsing, InvalidDates) {
    // Invalid decimal
    ASSERT_NOT_OK(dateFromISOString("1970-01-01T00:00:00.0.0Z").getStatus());
    ASSERT_NOT_OK(dateFromISOString("1970-01-01T00:00:.0.000Z").getStatus());
    ASSERT_NOT_OK(dateFromISOString("1970-01-01T00:.0:00.000Z").getStatus());
    ASSERT_NOT_OK(dateFromISOString("1970-01-01T.0:00:00.000Z").getStatus());
    ASSERT_NOT_OK(dateFromISOString("1970-01-.1T00:00:00.000Z").getStatus());
    ASSERT_NOT_OK(dateFromISOString("1970-.1-01T00:00:00.000Z").getStatus());

    // Extra sign characters
    ASSERT_NOT_OK(dateFromISOString("1970-01-01T00:00:00.+00Z").getStatus());
    ASSERT_NOT_OK(dateFromISOString("1970-01-01T00:00:+0.000Z").getStatus());
    ASSERT_NOT_OK(dateFromISOString("1970-01-01T00:+0:00.000Z").getStatus());
    ASSERT_NOT_OK(dateFromISOString("1970-01-01T+0:00:00.000Z").getStatus());
    ASSERT_NOT_OK(dateFromISOString("1970-01-+1T00:00:00.000Z").getStatus());
    ASSERT_NOT_OK(dateFromISOString("1970-+1-01T00:00:00.000Z").getStatus());
    ASSERT_NOT_OK(dateFromISOString("+970-01-01T00:00:00.000Z").getStatus());

    ASSERT_NOT_OK(dateFromISOString("1970-01-01T00:00:00.-00Z").getStatus());
    ASSERT_NOT_OK(dateFromISOString("1970-01-01T00:00:-0.000Z").getStatus());
    ASSERT_NOT_OK(dateFromISOString("1970-01-01T00:-0:00.000Z").getStatus());
    ASSERT_NOT_OK(dateFromISOString("1970-01-01T-0:00:00.000Z").getStatus());
    ASSERT_NOT_OK(dateFromISOString("1970-01--1T00:00:00.000Z").getStatus());
    ASSERT_NOT_OK(dateFromISOString("1970--1-01T00:00:00.000Z").getStatus());
    ASSERT_NOT_OK(dateFromISOString("-970-01-01T00:00:00.000Z").getStatus());

    // Out of range
    ASSERT_NOT_OK(dateFromISOString("1970-01-01T00:00:60.000Z").getStatus());
    ASSERT_NOT_OK(dateFromISOString("1970-01-01T00:60:00.000Z").getStatus());
    ASSERT_NOT_OK(dateFromISOString("1970-01-01T24:00:00.000Z").getStatus());
    ASSERT_NOT_OK(dateFromISOString("1970-01-32T00:00:00.000Z").getStatus());
    ASSERT_NOT_OK(dateFromISOString("1970-01-00T00:00:00.000Z").getStatus());
    ASSERT_NOT_OK(dateFromISOString("1970-13-01T00:00:00.000Z").getStatus());
    ASSERT_NOT_OK(dateFromISOString("1970-00-01T00:00:00.000Z").getStatus());
    // ASSERT_NOT_OK(dateFromISOString("1969-01-01T00:00:00.000Z").getStatus());

    // Invalid lengths
    ASSERT_NOT_OK(dateFromISOString("1970-001-01T00:00:00.000Z").getStatus());
    ASSERT_NOT_OK(dateFromISOString("1970-01-001T00:00:00.000Z").getStatus());
    ASSERT_NOT_OK(dateFromISOString("1970-01-01T000:00:00.000Z").getStatus());
    ASSERT_NOT_OK(dateFromISOString("1970-01-01T00:000:00.000Z").getStatus());
    ASSERT_NOT_OK(dateFromISOString("1970-01-01T00:00:000.000Z").getStatus());
    ASSERT_NOT_OK(dateFromISOString("1970-1-01T00:00:00.000Z").getStatus());
    ASSERT_NOT_OK(dateFromISOString("1970-01-1T00:00:00.000Z").getStatus());
    ASSERT_NOT_OK(dateFromISOString("1970-01-01T0:00:00.000Z").getStatus());
    ASSERT_NOT_OK(dateFromISOString("1970-01-01T00:0:00.000Z").getStatus());
    ASSERT_NOT_OK(dateFromISOString("1970-01-01T00:00:0.000Z").getStatus());

    // Invalid delimiters
    ASSERT_NOT_OK(dateFromISOString("1970+01-01T00:00:00.000Z").getStatus());
    ASSERT_NOT_OK(dateFromISOString("1970-01+01T00:00:00.000Z").getStatus());
    ASSERT_NOT_OK(dateFromISOString("1970-01-01Q00:00:00.000Z").getStatus());
    ASSERT_NOT_OK(dateFromISOString("1970-01-01T00-00:00.000Z").getStatus());
    ASSERT_NOT_OK(dateFromISOString("1970-01-01T00:00-00.000Z").getStatus());
    ASSERT_NOT_OK(dateFromISOString("1970-01-01T00:00:00-000Z").getStatus());

    // Missing numbers
    ASSERT_NOT_OK(dateFromISOString("1970--01T00:00:00.000Z").getStatus());
    ASSERT_NOT_OK(dateFromISOString("1970-01-T00:00:00.000Z").getStatus());
    ASSERT_NOT_OK(dateFromISOString("1970-01-01T:00:00.000Z").getStatus());
    ASSERT_NOT_OK(dateFromISOString("1970-01-01T00::00.000Z").getStatus());
    ASSERT_NOT_OK(dateFromISOString("1970-01-01T00:00:.000Z").getStatus());

    // Bad time offset field
    ASSERT_NOT_OK(dateFromISOString("1970-01-01T05:00:01ZZ").getStatus());
    ASSERT_NOT_OK(dateFromISOString("1970-01-01T05:00:01+").getStatus());
    ASSERT_NOT_OK(dateFromISOString("1970-01-01T05:00:01-").getStatus());
    ASSERT_NOT_OK(dateFromISOString("1970-01-01T05:00:01-11111").getStatus());
    ASSERT_NOT_OK(dateFromISOString("1970-01-01T05:00:01+1160").getStatus());
    ASSERT_NOT_OK(dateFromISOString("1970-01-01T05:00:01+00+0").getStatus());

    // Bad prefixes
    ASSERT_NOT_OK(dateFromISOString("1970-01-01T05:00:").getStatus());
    ASSERT_NOT_OK(dateFromISOString("1970-01-01T05:").getStatus());
    ASSERT_NOT_OK(dateFromISOString("1970-01-").getStatus());
    ASSERT_NOT_OK(dateFromISOString("1970-").getStatus());
    ASSERT_NOT_OK(dateFromISOString("1970-01-01T05+0500").getStatus());
    ASSERT_NOT_OK(dateFromISOString("1970-01-01T01Z").getStatus());

    // No local time
    ASSERT_NOT_OK(dateFromISOString("1970-01-01T00:00:00.000").getStatus());
    ASSERT_NOT_OK(dateFromISOString("1970-01-01T00:00:00").getStatus());
    ASSERT_NOT_OK(dateFromISOString("1970-01-01T00:00").getStatus());
    ASSERT_NOT_OK(dateFromISOString("1970-01-01T00").getStatus());
    ASSERT_NOT_OK(dateFromISOString("1970-01-01").getStatus());
    ASSERT_NOT_OK(dateFromISOString("1970-01").getStatus());
    ASSERT_NOT_OK(dateFromISOString("1970").getStatus());

    // Invalid hex base specifiers
    ASSERT_NOT_OK(dateFromISOString("x970-01-01T00:00:00.000Z").getStatus());
    ASSERT_NOT_OK(dateFromISOString("1970-x1-01T00:00:00.000Z").getStatus());
    ASSERT_NOT_OK(dateFromISOString("1970-01-x1T00:00:00.000Z").getStatus());
    ASSERT_NOT_OK(dateFromISOString("1970-01-01Tx0:00:00.000Z").getStatus());
    ASSERT_NOT_OK(dateFromISOString("1970-01-01T00:x0:00.000Z").getStatus());
    ASSERT_NOT_OK(dateFromISOString("1970-01-01T00:00:x0.000Z").getStatus());
    ASSERT_NOT_OK(dateFromISOString("1970-01-01T00:00:00.x00Z").getStatus());
}

TEST(TimeParsing, LeapYears) {
    StatusWith<Date_t> swull = dateFromISOString("1972-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 68169600000LL);

    swull = dateFromISOString("1976-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 194400000000LL);

    swull = dateFromISOString("1980-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 320630400000LL);

    swull = dateFromISOString("1984-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 446860800000LL);

    swull = dateFromISOString("1988-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 573091200000LL);

    swull = dateFromISOString("1992-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 699321600000LL);

    swull = dateFromISOString("1996-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 825552000000LL);

    swull = dateFromISOString("2000-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 951782400000LL);

    swull = dateFromISOString("2004-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 1078012800000LL);

    swull = dateFromISOString("2008-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 1204243200000LL);

    swull = dateFromISOString("2012-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 1330473600000LL);

    swull = dateFromISOString("2016-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 1456704000000LL);

    swull = dateFromISOString("2020-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 1582934400000LL);

    swull = dateFromISOString("2024-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 1709164800000LL);

    swull = dateFromISOString("2028-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 1835395200000LL);

    swull = dateFromISOString("2032-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 1961625600000LL);

    swull = dateFromISOString("2036-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 2087856000000LL);

    swull = dateFromISOString("2040-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 2214086400000LL);

    swull = dateFromISOString("2044-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 2340316800000LL);

    swull = dateFromISOString("2048-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 2466547200000LL);

    swull = dateFromISOString("2052-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 2592777600000LL);

    swull = dateFromISOString("2056-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 2719008000000LL);

    swull = dateFromISOString("2060-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 2845238400000LL);

    swull = dateFromISOString("2064-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 2971468800000LL);

    swull = dateFromISOString("2068-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 3097699200000LL);

    swull = dateFromISOString("2072-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 3223929600000LL);

    swull = dateFromISOString("2076-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 3350160000000LL);

    swull = dateFromISOString("2080-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 3476390400000LL);

    swull = dateFromISOString("2084-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 3602620800000LL);

    swull = dateFromISOString("2088-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 3728851200000LL);

    swull = dateFromISOString("2092-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 3855081600000LL);

    swull = dateFromISOString("2096-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 3981312000000LL);

    swull = dateFromISOString("2104-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 4233686400000LL);

    swull = dateFromISOString("2108-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 4359916800000LL);

    swull = dateFromISOString("2112-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 4486147200000LL);

    swull = dateFromISOString("2116-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 4612377600000LL);

    swull = dateFromISOString("2120-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 4738608000000LL);

    swull = dateFromISOString("2124-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 4864838400000LL);

    swull = dateFromISOString("2128-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 4991068800000LL);

    swull = dateFromISOString("2132-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 5117299200000LL);

    swull = dateFromISOString("2136-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 5243529600000LL);

    swull = dateFromISOString("2140-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 5369760000000LL);

    swull = dateFromISOString("2144-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 5495990400000LL);

    swull = dateFromISOString("2148-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 5622220800000LL);

    swull = dateFromISOString("2152-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 5748451200000LL);

    swull = dateFromISOString("2156-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 5874681600000LL);

    swull = dateFromISOString("2160-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 6000912000000LL);

    swull = dateFromISOString("2164-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 6127142400000LL);

    swull = dateFromISOString("2168-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 6253372800000LL);

    swull = dateFromISOString("2172-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 6379603200000LL);

    swull = dateFromISOString("2176-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 6505833600000LL);

    swull = dateFromISOString("2180-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 6632064000000LL);

    swull = dateFromISOString("2184-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 6758294400000LL);

    swull = dateFromISOString("2188-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 6884524800000LL);

    swull = dateFromISOString("2192-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 7010755200000LL);

    swull = dateFromISOString("2196-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 7136985600000LL);

    swull = dateFromISOString("2204-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 7389360000000LL);

    swull = dateFromISOString("2208-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 7515590400000LL);

    swull = dateFromISOString("2212-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 7641820800000LL);

    swull = dateFromISOString("2216-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 7768051200000LL);

    swull = dateFromISOString("2220-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 7894281600000LL);

    swull = dateFromISOString("2224-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 8020512000000LL);

    swull = dateFromISOString("2228-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 8146742400000LL);

    swull = dateFromISOString("2232-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 8272972800000LL);

    swull = dateFromISOString("2236-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 8399203200000LL);

    swull = dateFromISOString("2240-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 8525433600000LL);

    swull = dateFromISOString("2244-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 8651664000000LL);

    swull = dateFromISOString("2248-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 8777894400000LL);

    swull = dateFromISOString("2252-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 8904124800000LL);

    swull = dateFromISOString("2256-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 9030355200000LL);

    swull = dateFromISOString("2260-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 9156585600000LL);

    swull = dateFromISOString("2264-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 9282816000000LL);

    swull = dateFromISOString("2268-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 9409046400000LL);

    swull = dateFromISOString("2272-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 9535276800000LL);

    swull = dateFromISOString("2276-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 9661507200000LL);

    swull = dateFromISOString("2280-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 9787737600000LL);

    swull = dateFromISOString("2284-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 9913968000000LL);

    swull = dateFromISOString("2288-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 10040198400000LL);

    swull = dateFromISOString("2292-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 10166428800000LL);

    swull = dateFromISOString("2296-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 10292659200000LL);

    swull = dateFromISOString("2304-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 10545033600000LL);

    swull = dateFromISOString("2308-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 10671264000000LL);

    swull = dateFromISOString("2312-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 10797494400000LL);

    swull = dateFromISOString("2316-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 10923724800000LL);

    swull = dateFromISOString("2320-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 11049955200000LL);

    swull = dateFromISOString("2324-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 11176185600000LL);

    swull = dateFromISOString("2328-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 11302416000000LL);

    swull = dateFromISOString("2332-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 11428646400000LL);

    swull = dateFromISOString("2336-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 11554876800000LL);

    swull = dateFromISOString("2340-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 11681107200000LL);

    swull = dateFromISOString("2344-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 11807337600000LL);

    swull = dateFromISOString("2348-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 11933568000000LL);

    swull = dateFromISOString("2352-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 12059798400000LL);

    swull = dateFromISOString("2356-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 12186028800000LL);

    swull = dateFromISOString("2360-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 12312259200000LL);

    swull = dateFromISOString("2364-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 12438489600000LL);

    swull = dateFromISOString("2368-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 12564720000000LL);

    swull = dateFromISOString("2372-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 12690950400000LL);

    swull = dateFromISOString("2376-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 12817180800000LL);

    swull = dateFromISOString("2380-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 12943411200000LL);

    swull = dateFromISOString("2384-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 13069641600000LL);

    swull = dateFromISOString("2388-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 13195872000000LL);

    swull = dateFromISOString("2392-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 13322102400000LL);

    swull = dateFromISOString("2396-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 13448332800000LL);

    swull = dateFromISOString("2400-02-29T00:00:00.000Z");
    ASSERT_OK(swull.getStatus());
    ASSERT_EQUALS(swull.getValue().asInt64(), 13574563200000LL);
}

TEST(TimeFormatting, DurationFormatting) {
    ASSERT_EQUALS("52\xce\xbcs", static_cast<std::string>(str::stream() << Microseconds(52)));
    ASSERT_EQUALS("52ms", static_cast<std::string>(str::stream() << Milliseconds(52)));
    ASSERT_EQUALS("52s", static_cast<std::string>(str::stream() << Seconds(52)));

    std::ostringstream os;
    os << Milliseconds(52) << Microseconds(52) << Seconds(52);
    ASSERT_EQUALS("52ms52\xce\xbcs52s", os.str());
}

TEST(TimeFormatting, WriteToStream) {
    const std::vector<std::string> dateStrings = {
        "1996-04-07T00:00:00.000Z",
        "1996-05-02T00:00:00.000Z",
        "1997-06-23T07:55:00.000Z",
        "2015-05-14T17:28:33.123Z",
        "2036-02-29T00:00:00.000Z",
    };

    for (const std::string& isoTimeString : dateStrings) {
        const Date_t aDate = unittest::assertGet(dateFromISOString(isoTimeString));
        std::ostringstream testStream;
        testStream << aDate;
        std::string streamOut = testStream.str();
        ASSERT_EQUALS(aDate.toString(), streamOut);
    }
}

TEST(SystemTime, ConvertDateToSystemTime) {
    const std::string isoTimeString = "2015-05-14T17:28:33.123Z";
    const Date_t aDate = unittest::assertGet(dateFromISOString(isoTimeString));
    const auto aTimePoint = aDate.toSystemTimePoint();
    const auto actual = aTimePoint - stdx::chrono::system_clock::from_time_t(0);
    ASSERT(aDate.toDurationSinceEpoch().toSystemDuration() == actual)
        << "Expected " << aDate << "; but found " << Date_t::fromDurationSinceEpoch(actual);
    ASSERT_EQUALS(aDate, Date_t(aTimePoint));
}

TEST(GetTimeZone, DoesReturnKnownTimeZone) {
    // Just asserting that these do not throw exceptions.
    kDefaultTimeZoneDatabase.getTimeZone("UTC");
    kDefaultTimeZoneDatabase.getTimeZone("America/New_York");
    kDefaultTimeZoneDatabase.getTimeZone("Australia/Sydney");
}

TEST(GetTimeZone, DoesParseHourOnlyOffset) {
    auto date = Date_t::fromMillisSinceEpoch(1500371861000LL);

    auto zone = kDefaultTimeZoneDatabase.getTimeZone("+02");
    ASSERT_EQ(durationCount<Hours>(zone.utcOffset(date)), 2);

    zone = kDefaultTimeZoneDatabase.getTimeZone("-02");
    ASSERT_EQ(durationCount<Hours>(zone.utcOffset(date)), -2);

    zone = kDefaultTimeZoneDatabase.getTimeZone("+00");
    ASSERT_EQ(durationCount<Seconds>(zone.utcOffset(date)), 0);

    zone = kDefaultTimeZoneDatabase.getTimeZone("-00");
    ASSERT_EQ(durationCount<Seconds>(zone.utcOffset(date)), 0);
}

TEST(GetTimeZone, DoesParseHourMinuteOffset) {
    auto date = Date_t::fromMillisSinceEpoch(1500371861000LL);

    auto zone = kDefaultTimeZoneDatabase.getTimeZone("+0200");
    ASSERT_EQ(durationCount<Hours>(zone.utcOffset(date)), 2);

    zone = kDefaultTimeZoneDatabase.getTimeZone("-0200");
    ASSERT_EQ(durationCount<Hours>(zone.utcOffset(date)), -2);

    zone = kDefaultTimeZoneDatabase.getTimeZone("+0245");
    ASSERT_EQ(durationCount<Minutes>(zone.utcOffset(date)), 165);

    zone = kDefaultTimeZoneDatabase.getTimeZone("-0245");
    ASSERT_EQ(durationCount<Minutes>(zone.utcOffset(date)), -165);
}

TEST(GetTimeZone, DoesParseHourMinuteOffsetWithColon) {
    auto date = Date_t::fromMillisSinceEpoch(1500371861000LL);

    auto zone = kDefaultTimeZoneDatabase.getTimeZone("+12:00");
    ASSERT_EQ(durationCount<Hours>(zone.utcOffset(date)), 12);

    zone = kDefaultTimeZoneDatabase.getTimeZone("-11:00");
    ASSERT_EQ(durationCount<Hours>(zone.utcOffset(date)), -11);

    zone = kDefaultTimeZoneDatabase.getTimeZone("+09:27");
    ASSERT_EQ(durationCount<Minutes>(zone.utcOffset(date)), 567);

    zone = kDefaultTimeZoneDatabase.getTimeZone("-00:37");
    ASSERT_EQ(durationCount<Minutes>(zone.utcOffset(date)), -37);
}

TEST(GetTimeZone, DoesNotReturnUnknownTimeZone) {
    ASSERT_THROWS_CODE(kDefaultTimeZoneDatabase.getTimeZone("The moon"), UserException, 40485);
    ASSERT_THROWS_CODE(kDefaultTimeZoneDatabase.getTimeZone("xyz"), UserException, 40485);
    ASSERT_THROWS_CODE(kDefaultTimeZoneDatabase.getTimeZone("Jupiter"), UserException, 40485);
}

TEST(GetTimeZone, ThrowsUserExceptionIfGivenUnparsableUtcOffset) {
    ASSERT_THROWS_CODE(kDefaultTimeZoneDatabase.getTimeZone("123"), UserException, 40485);
    ASSERT_THROWS_CODE(kDefaultTimeZoneDatabase.getTimeZone("1234"), UserException, 40485);
    ASSERT_THROWS_CODE(kDefaultTimeZoneDatabase.getTimeZone("12345"), UserException, 40485);
    ASSERT_THROWS_CODE(kDefaultTimeZoneDatabase.getTimeZone("-123"), UserException, 40485);
    ASSERT_THROWS_CODE(kDefaultTimeZoneDatabase.getTimeZone("-12*34"), UserException, 40485);
    ASSERT_THROWS_CODE(kDefaultTimeZoneDatabase.getTimeZone("-1:23"), UserException, 40485);
    ASSERT_THROWS_CODE(kDefaultTimeZoneDatabase.getTimeZone("-12:3"), UserException, 40485);
    ASSERT_THROWS_CODE(kDefaultTimeZoneDatabase.getTimeZone("+123"), UserException, 40485);
    ASSERT_THROWS_CODE(kDefaultTimeZoneDatabase.getTimeZone("+12*34"), UserException, 40485);
    ASSERT_THROWS_CODE(kDefaultTimeZoneDatabase.getTimeZone("+1:23"), UserException, 40485);
    ASSERT_THROWS_CODE(kDefaultTimeZoneDatabase.getTimeZone("+12:3"), UserException, 40485);
    ASSERT_THROWS_CODE(kDefaultTimeZoneDatabase.getTimeZone("+0x4"), UserException, 40485);
    ASSERT_THROWS_CODE(kDefaultTimeZoneDatabase.getTimeZone("-0xa0"), UserException, 40485);
}

TEST(UTCTimeBeforeEpoch, DoesExtractDateParts) {
    // Dec 30, 1969 13:42:23:211
    auto date = Date_t::fromMillisSinceEpoch(-123456789LL);
    auto dateParts = TimeZoneDatabase::utcZone().dateParts(date);
    ASSERT_EQ(dateParts.year, 1969);
    ASSERT_EQ(dateParts.month, 12);
    ASSERT_EQ(dateParts.dayOfMonth, 30);
    ASSERT_EQ(dateParts.hour, 13);
    ASSERT_EQ(dateParts.minute, 42);
    ASSERT_EQ(dateParts.second, 23);
    ASSERT_EQ(dateParts.millisecond, 211);
}

TEST(NewYorkTimeBeforeEpoch, DoesExtractDateParts) {
    auto newYorkZone = kDefaultTimeZoneDatabase.getTimeZone("America/New_York");

    // 1969-12-30T13:42:23.211Z.
    auto date = Date_t::fromMillisSinceEpoch(-123456789LL);
    auto dateParts = newYorkZone.dateParts(date);
    ASSERT_EQ(dateParts.year, 1969);
    ASSERT_EQ(dateParts.month, 12);
    ASSERT_EQ(dateParts.dayOfMonth, 30);
    ASSERT_EQ(dateParts.hour, 8);
    ASSERT_EQ(dateParts.minute, 42);
    ASSERT_EQ(dateParts.second, 23);
    ASSERT_EQ(dateParts.millisecond, 211);
}

TEST(UtcOffsetBeforeEpoch, DoesExtractDateParts) {
    auto utcOffsetZone = kDefaultTimeZoneDatabase.getTimeZone("-05:00");

    // 1969-12-30T13:42:23.211Z.
    auto date = Date_t::fromMillisSinceEpoch(-123456789LL);
    auto dateParts = utcOffsetZone.dateParts(date);
    ASSERT_EQ(dateParts.year, 1969);
    ASSERT_EQ(dateParts.month, 12);
    ASSERT_EQ(dateParts.dayOfMonth, 30);
    ASSERT_EQ(dateParts.hour, 8);
    ASSERT_EQ(dateParts.minute, 42);
    ASSERT_EQ(dateParts.second, 23);
    ASSERT_EQ(dateParts.millisecond, 211);
}

TEST(UTCTimeBeforeEpoch, DoesComputeISOYear) {
    // Sunday, December 28, 1969.
    auto date = Date_t::fromMillisSinceEpoch(-345600000LL);
    ASSERT_EQ(TimeZoneDatabase::utcZone().isoYear(date), 1969);
    // Tue, December 30, 1969, part of the following year.
    date = Date_t::fromMillisSinceEpoch(-123456000LL);
    ASSERT_EQ(TimeZoneDatabase::utcZone().isoYear(date), 1970);
    // Saturday, January 1, 1966, part of the previous year.
    date = Date_t::fromMillisSinceEpoch(-126230400000LL);
    ASSERT_EQ(TimeZoneDatabase::utcZone().isoYear(date), 1965);
}

TEST(NewYorkTimeBeforeEpoch, DoesComputeISOYear) {
    auto newYorkZone = kDefaultTimeZoneDatabase.getTimeZone("America/New_York");

    // Sunday, December 28, 1969.
    auto date = Date_t::fromMillisSinceEpoch(-345600000LL);
    ASSERT_EQ(newYorkZone.isoYear(date), 1969);

    // 1969-12-30T13:42:23.211Z (Tuesday), part of the following year.
    date = Date_t::fromMillisSinceEpoch(-123456000LL);
    ASSERT_EQ(newYorkZone.isoYear(date), 1970);

    // 1966-01-01T00:00:00.000Z (Saturday), part of the previous year.
    date = Date_t::fromMillisSinceEpoch(-126230400000LL);
    ASSERT_EQ(newYorkZone.isoYear(date), 1965);
}

TEST(UTCTimeBeforeEpoch, DoesComputeDayOfWeek) {
    // Sunday, December 28, 1969.
    auto date = Date_t::fromMillisSinceEpoch(-345600000LL);
    ASSERT_EQ(TimeZoneDatabase::utcZone().dayOfWeek(date), 1);
    // Tuesday, December 30, 1969.
    date = Date_t::fromMillisSinceEpoch(-123456000LL);
    ASSERT_EQ(TimeZoneDatabase::utcZone().dayOfWeek(date), 3);
    // Saturday, January 1, 1966.
    date = Date_t::fromMillisSinceEpoch(-126230400000LL);
    ASSERT_EQ(TimeZoneDatabase::utcZone().dayOfWeek(date), 7);
}

TEST(NewYorkTimeBeforeEpoch, DoesComputeDayOfWeek) {
    auto newYorkZone = kDefaultTimeZoneDatabase.getTimeZone("America/New_York");

    // 1969-12-28T00:00:00.000Z (Sunday).
    auto date = Date_t::fromMillisSinceEpoch(-345600000LL);
    // Part of the previous day (Saturday) in New York.
    ASSERT_EQ(newYorkZone.dayOfWeek(date), 7);

    // 1969-12-30T13:42:23.211Z (Tuesday).
    date = Date_t::fromMillisSinceEpoch(-123456000LL);
    ASSERT_EQ(newYorkZone.dayOfWeek(date), 3);
}

TEST(UTCTimeBeforeEpoch, DoesComputeISODayOfWeek) {
    // Sunday, December 28, 1969.
    auto date = Date_t::fromMillisSinceEpoch(-345600000LL);
    ASSERT_EQ(TimeZoneDatabase::utcZone().isoDayOfWeek(date), 7);
    // Tue, December 30, 1969.
    date = Date_t::fromMillisSinceEpoch(-123456000LL);
    ASSERT_EQ(TimeZoneDatabase::utcZone().isoDayOfWeek(date), 2);
    // Saturday, January 1, 1966.
    date = Date_t::fromMillisSinceEpoch(-126230400000LL);
    ASSERT_EQ(TimeZoneDatabase::utcZone().isoDayOfWeek(date), 6);
}

TEST(NewYorkTimeBeforeEpoch, DoesComputeISODayOfWeek) {
    auto newYorkZone = kDefaultTimeZoneDatabase.getTimeZone("America/New_York");

    // 1969-12-28T00:00:00.000Z (Sunday).
    auto date = Date_t::fromMillisSinceEpoch(-345600000LL);
    // Part of the previous day (Saturday) in New York.
    ASSERT_EQ(newYorkZone.isoDayOfWeek(date), 6);

    // 1969-12-30T13:42:23.211Z (Tuesday).
    date = Date_t::fromMillisSinceEpoch(-123456000LL);
    ASSERT_EQ(newYorkZone.isoDayOfWeek(date), 2);
}

TEST(UTCTimeBeforeEpoch, DoesComputeDayOfYear) {
    // December 30, 1969.
    auto date = Date_t::fromMillisSinceEpoch(-123456000LL);
    ASSERT_EQ(TimeZoneDatabase::utcZone().dayOfYear(date), 364);
    // January 1, 1966.
    date = Date_t::fromMillisSinceEpoch(-126230400000LL);
    ASSERT_EQ(TimeZoneDatabase::utcZone().dayOfYear(date), 1);

    // Feb 28, 1960 (leap year).
    date = Date_t::fromMillisSinceEpoch(-310608000000LL);
    ASSERT_EQ(TimeZoneDatabase::utcZone().dayOfYear(date), 59);
    // Feb 29, 1960 (leap year).
    date = Date_t::fromMillisSinceEpoch(-310521600000LL);
    ASSERT_EQ(TimeZoneDatabase::utcZone().dayOfYear(date), 60);
    // Mar 1, 1960 (leap year).
    date = Date_t::fromMillisSinceEpoch(-310435200000LL);
    ASSERT_EQ(TimeZoneDatabase::utcZone().dayOfYear(date), 61);
    // December 31, 1960 (leap year).
    date = Date_t::fromMillisSinceEpoch(-284083200000LL);
    ASSERT_EQ(TimeZoneDatabase::utcZone().dayOfYear(date), 366);

    // Feb 28, 1900 (not leap year).
    date = Date_t::fromMillisSinceEpoch(-2203977600000LL);
    ASSERT_EQ(TimeZoneDatabase::utcZone().dayOfYear(date), 59);
    // Mar 1, 1900 (not leap year).
    date = Date_t::fromMillisSinceEpoch(-2203891200000LL);
    ASSERT_EQ(TimeZoneDatabase::utcZone().dayOfYear(date), 60);
    // December 31, 1900 (not leap year).
    date = Date_t::fromMillisSinceEpoch(-2177539200000LL);
    ASSERT_EQ(TimeZoneDatabase::utcZone().dayOfYear(date), 365);
}

TEST(NewYorkTimeBeforeEpoch, DoesComputeDayOfYear) {
    auto newYorkZone = kDefaultTimeZoneDatabase.getTimeZone("America/New_York");

    // 1969-12-28T13:42:24.000Z (Sunday).
    auto date = Date_t::fromMillisSinceEpoch(-123456000LL);
    ASSERT_EQ(newYorkZone.dayOfYear(date), 364);

    // 1966-01-01T00:00:00.000Z (Saturday).
    date = Date_t::fromMillisSinceEpoch(-126230400000LL);
    ASSERT_EQ(newYorkZone.dayOfYear(date), 365);

    // 1960-02-28T00:00:00.000Z (leap year).
    date = Date_t::fromMillisSinceEpoch(-310608000000LL);
    ASSERT_EQ(newYorkZone.dayOfYear(date), 58);
    // 1960-02-29T00:00:00.000Z.
    date = Date_t::fromMillisSinceEpoch(-310521600000LL);
    ASSERT_EQ(newYorkZone.dayOfYear(date), 59);
    // 1960-01-01T00:00:00.000Z.
    date = Date_t::fromMillisSinceEpoch(-310435200000LL);
    ASSERT_EQ(newYorkZone.dayOfYear(date), 60);
    // 1960-12-31T00:00:00.000Z.
    date = Date_t::fromMillisSinceEpoch(-284083200000LL);
    ASSERT_EQ(newYorkZone.dayOfYear(date), 365);

    // 1900-02-28T00:00:00.000Z (not leap year).
    date = Date_t::fromMillisSinceEpoch(-2203977600000LL);
    ASSERT_EQ(newYorkZone.dayOfYear(date), 58);
    // 1900-03-01T00:00:00.000Z.
    date = Date_t::fromMillisSinceEpoch(-2203891200000LL);
    ASSERT_EQ(newYorkZone.dayOfYear(date), 59);
    // 1900-12-31T00:00:00.000Z.
    date = Date_t::fromMillisSinceEpoch(-2177539200000LL);
    ASSERT_EQ(newYorkZone.dayOfYear(date), 364);
}

TEST(UTCTimeBeforeEpoch, DoesComputeWeek) {
    // Sunday, December 28, 1969.
    auto date = Date_t::fromMillisSinceEpoch(-345600000LL);
    ASSERT_EQ(TimeZoneDatabase::utcZone().week(date), 52);
    // Saturday, January 1, 1966.
    date = Date_t::fromMillisSinceEpoch(-126230400000LL);
    ASSERT_EQ(TimeZoneDatabase::utcZone().week(date), 0);
}

TEST(NewYorkTimeBeforeEpoch, DoesComputeWeek) {
    auto newYorkZone = kDefaultTimeZoneDatabase.getTimeZone("America/New_York");

    // 1969-12-28T00:00:00.000Z (Sunday).
    auto date = Date_t::fromMillisSinceEpoch(-345600000LL);
    ASSERT_EQ(newYorkZone.week(date), 51);

    // 1966-01-01T00:00:00.000Z (Saturday).
    date = Date_t::fromMillisSinceEpoch(-126230400000LL);
    ASSERT_EQ(newYorkZone.week(date), 52);
}

TEST(UTCTimeBeforeEpoch, DoesComputeUtcOffset) {
    // Sunday, December 28, 1969.
    auto date = Date_t::fromMillisSinceEpoch(-345600000LL);
    ASSERT_EQ(durationCount<Seconds>(TimeZoneDatabase::utcZone().utcOffset(date)), 0);
}

TEST(NewYorkTimeBeforeEpoch, DoesComputeUtcOffset) {
    auto newYorkZone = kDefaultTimeZoneDatabase.getTimeZone("America/New_York");

    // 1969-06-29T00:00:00.000Z (Sunday).
    auto date = Date_t::fromMillisSinceEpoch(-16070400000LL);
    ASSERT_EQ(durationCount<Hours>(newYorkZone.utcOffset(date)), -4);

    // 1966-01-01T00:00:00.000Z (Saturday).
    date = Date_t::fromMillisSinceEpoch(-126230400000LL);
    ASSERT_EQ(durationCount<Hours>(newYorkZone.utcOffset(date)), -5);
}

TEST(UTCTimeBeforeEpoch, DoesComputeISOWeek) {
    // Sunday, December 28, 1969.
    auto date = Date_t::fromMillisSinceEpoch(-345600000LL);
    ASSERT_EQ(TimeZoneDatabase::utcZone().isoWeek(date), 52);
    // Tuesday, December 30, 1969.
    date = Date_t::fromMillisSinceEpoch(-123456000LL);
    ASSERT_EQ(TimeZoneDatabase::utcZone().isoWeek(date), 1);
    // Saturday, January 1, 1966, part of previous year.
    date = Date_t::fromMillisSinceEpoch(-126230400000LL);
    ASSERT_EQ(TimeZoneDatabase::utcZone().isoWeek(date), 52);
    // Tuesday, December 29, 1959.
    date = Date_t::fromMillisSinceEpoch(-315878400000LL);
    ASSERT_EQ(TimeZoneDatabase::utcZone().isoWeek(date), 53);
    // Saturday, January 2, 1960, part of previous ISO year.
    date = Date_t::fromMillisSinceEpoch(-315532800000LL);
    ASSERT_EQ(TimeZoneDatabase::utcZone().isoWeek(date), 53);
}

TEST(NewYorkTimeBeforeEpoch, DoesComputeISOWeek) {
    auto newYorkZone = kDefaultTimeZoneDatabase.getTimeZone("America/New_York");

    // 1969-12-28T00:00:00.000Z (Sunday).
    auto date = Date_t::fromMillisSinceEpoch(-345600000LL);
    ASSERT_EQ(newYorkZone.isoWeek(date), 52);

    // 1969-12-30T00:00:00.000Z (Tuesday).
    date = Date_t::fromMillisSinceEpoch(-123456000LL);
    ASSERT_EQ(newYorkZone.isoWeek(date), 1);

    // 1966-01-01T00:00:00.000Z (Saturday), part of previous year.
    date = Date_t::fromMillisSinceEpoch(-126230400000LL);
    ASSERT_EQ(newYorkZone.isoWeek(date), 52);
    // 1959-12-29T00:00:00.000Z (Tuesday).
    date = Date_t::fromMillisSinceEpoch(-315878400000LL);
    ASSERT_EQ(newYorkZone.isoWeek(date), 53);
    // 1960-01-02T00:00:00.000Z (Saturday), part of previous year.
    date = Date_t::fromMillisSinceEpoch(-315532800000LL);
    ASSERT_EQ(newYorkZone.isoWeek(date), 53);
}

TEST(UTCTimeBeforeEpoch, DoesFormatDate) {
    // Tuesday, Dec 30, 1969 13:42:23:211
    auto date = Date_t::fromMillisSinceEpoch(-123456789LL);
    ASSERT_EQ(TimeZoneDatabase::utcZone().formatDate("%Y/%m/%d %H:%M:%S:%L, dayOfYear: %j, "
                                                     "dayOfWeek: %w, week: %U, isoYear: %G, "
                                                     "isoWeek: %V, isoDayOfWeek: %u, percent: %%",
                                                     date),
              "1969/12/30 13:42:23:211, dayOfYear: 364, dayOfWeek: 3, week: 52, isoYear: 1970, "
              "isoWeek: 01, isoDayOfWeek: 2, percent: %");
}

TEST(NewYorkTimeBeforeEpoch, DoesFormatDate) {
    auto newYorkZone = kDefaultTimeZoneDatabase.getTimeZone("America/New_York");

    // Tuesday, Dec 30, 1969 13:42:23:211
    auto date = Date_t::fromMillisSinceEpoch(-123456789LL);
    ASSERT_EQ(newYorkZone.formatDate("%Y/%m/%d %H:%M:%S:%L, dayOfYear: %j, "
                                     "dayOfWeek: %w, week: %U, isoYear: %G, "
                                     "isoWeek: %V, isoDayOfWeek: %u, percent: %%",
                                     date),
              "1969/12/30 08:42:23:211, dayOfYear: 364, dayOfWeek: 3, week: 52, isoYear: 1970, "
              "isoWeek: 01, isoDayOfWeek: 2, percent: %");
}

TEST(UTCTimeBeforeEpoch, DoesOutputFormatDate) {
    // Tuesday, Dec 30, 1969 13:42:23:211
    auto date = Date_t::fromMillisSinceEpoch(-123456789LL);
    std::ostringstream os;
    TimeZoneDatabase::utcZone().outputDateWithFormat(os,
                                                     "%Y/%m/%d %H:%M:%S:%L, dayOfYear: %j, "
                                                     "dayOfWeek: %w, week: %U, isoYear: %G, "
                                                     "isoWeek: %V, isoDayOfWeek: %u, percent: %%",
                                                     date);
    ASSERT_EQ(os.str(),
              "1969/12/30 13:42:23:211, dayOfYear: 364, dayOfWeek: 3, week: 52, isoYear: 1970, "
              "isoWeek: 01, isoDayOfWeek: 2, percent: %");
}

TEST(NewYorkTimeBeforeEpoch, DoesOutputFormatDate) {
    auto newYorkZone = kDefaultTimeZoneDatabase.getTimeZone("America/New_York");

    // Tuesday, Dec 30, 1969 13:42:23:211
    auto date = Date_t::fromMillisSinceEpoch(-123456789LL);
    std::ostringstream os;
    newYorkZone.outputDateWithFormat(os,
                                     "%Y/%m/%d %H:%M:%S:%L, dayOfYear: %j, "
                                     "dayOfWeek: %w, week: %U, isoYear: %G, "
                                     "isoWeek: %V, isoDayOfWeek: %u, percent: %%",
                                     date);
    ASSERT_EQ(os.str(),
              "1969/12/30 08:42:23:211, dayOfYear: 364, dayOfWeek: 3, week: 52, isoYear: 1970, "
              "isoWeek: 01, isoDayOfWeek: 2, percent: %");
}

TEST(UTCTimeAtEpoch, DoesExtractDateParts) {
    // Jan 1, 1970 00:00:00:000
    auto date = Date_t::fromMillisSinceEpoch(0);
    auto dateParts = TimeZoneDatabase::utcZone().dateParts(date);
    ASSERT_EQ(dateParts.year, 1970);
    ASSERT_EQ(dateParts.month, 1);
    ASSERT_EQ(dateParts.dayOfMonth, 1);
    ASSERT_EQ(dateParts.hour, 0);
    ASSERT_EQ(dateParts.minute, 0);
    ASSERT_EQ(dateParts.second, 0);
    ASSERT_EQ(dateParts.millisecond, 0);
}

TEST(NewYorkTimeAtEpoch, DoesExtractDateParts) {
    auto newYorkZone = kDefaultTimeZoneDatabase.getTimeZone("America/New_York");

    // Jan 1, 1970 00:00:00:000
    auto date = Date_t::fromMillisSinceEpoch(0);
    auto dateParts = newYorkZone.dateParts(date);
    ASSERT_EQ(dateParts.year, 1969);
    ASSERT_EQ(dateParts.month, 12);
    ASSERT_EQ(dateParts.dayOfMonth, 31);
    ASSERT_EQ(dateParts.hour, 19);
    ASSERT_EQ(dateParts.minute, 0);
    ASSERT_EQ(dateParts.second, 0);
    ASSERT_EQ(dateParts.millisecond, 0);
}

TEST(UTCTimeAtEpoch, DoesComputeISOYear) {
    // Thursday, January 1, 1970.
    auto date = Date_t::fromMillisSinceEpoch(0);
    ASSERT_EQ(TimeZoneDatabase::utcZone().isoYear(date), 1970);
}

TEST(NewYorkTimeAtEpoch, DoesComputeISOYear) {
    auto newYorkZone = kDefaultTimeZoneDatabase.getTimeZone("America/New_York");

    // 1970-1-1T00:00:00.000Z (Thursday)
    auto date = Date_t::fromMillisSinceEpoch(0);
    ASSERT_EQ(newYorkZone.isoYear(date), 1970);  // The Wednesday is still considered part of 1970.
}

TEST(UTCTimeAtEpoch, DoesComputeDayOfWeek) {
    // Thursday, January 1, 1970.
    auto date = Date_t::fromMillisSinceEpoch(0);
    ASSERT_EQ(TimeZoneDatabase::utcZone().dayOfWeek(date), 5);
}

TEST(NewYorkTimeAtEpoch, DoesComputeDayOfWeek) {
    auto newYorkZone = kDefaultTimeZoneDatabase.getTimeZone("America/New_York");

    // 1970-1-1T00:00:00.000Z (Thursday)
    auto date = Date_t::fromMillisSinceEpoch(0);
    ASSERT_EQ(newYorkZone.dayOfWeek(date), 4);  // The Wednesday before.
}

TEST(UTCTimeAtEpoch, DoesComputeISODayOfWeek) {
    // Thursday, January 1, 1970.
    auto date = Date_t::fromMillisSinceEpoch(0);
    ASSERT_EQ(TimeZoneDatabase::utcZone().isoDayOfWeek(date), 4);
}

TEST(NewYorkTimeAtEpoch, DoesComputeISODayOfWeek) {
    // 1970-1-1T00:00:00.000Z (Thursday)
    auto date = Date_t::fromMillisSinceEpoch(0);
    auto newYorkZone = kDefaultTimeZoneDatabase.getTimeZone("America/New_York");
    ASSERT_EQ(newYorkZone.isoDayOfWeek(date), 3);  // The Wednesday before.
}

TEST(UTCTimeAtEpoch, DoesComputeDayOfYear) {
    // Thursday, January 1, 1970.
    auto date = Date_t::fromMillisSinceEpoch(0);
    ASSERT_EQ(TimeZoneDatabase::utcZone().dayOfYear(date), 1);
}

TEST(NewYorkTimeAtEpoch, DoesComputeDayOfYear) {
    auto newYorkZone = kDefaultTimeZoneDatabase.getTimeZone("America/New_York");

    // 1970-1-1T00:00:00.000Z
    auto date = Date_t::fromMillisSinceEpoch(0);
    ASSERT_EQ(newYorkZone.dayOfYear(date), 365);
}

TEST(UTCTimeAtEpoch, DoesComputeWeek) {
    // Thursday, January 1, 1970.
    auto date = Date_t::fromMillisSinceEpoch(0);
    ASSERT_EQ(TimeZoneDatabase::utcZone().week(date), 0);
}

TEST(NewYorkTimeAtEpoch, DoesComputeWeek) {
    auto newYorkZone = kDefaultTimeZoneDatabase.getTimeZone("America/New_York");

    // 1970-1-1T00:00:00.000Z
    auto date = Date_t::fromMillisSinceEpoch(0);
    ASSERT_EQ(newYorkZone.week(date), 52);
}

TEST(UTCTimeAtEpoch, DoesComputeUtcOffset) {
    // Thursday, January 1, 1970.
    auto date = Date_t::fromMillisSinceEpoch(0);
    ASSERT_EQ(durationCount<Seconds>(TimeZoneDatabase::utcZone().utcOffset(date)), 0);
}

TEST(NewYorkTimeAtEpoch, DoesComputeUtcOffset) {
    auto newYorkZone = kDefaultTimeZoneDatabase.getTimeZone("America/New_York");

    // 1970-1-1T00:00:00.000Z
    auto date = Date_t::fromMillisSinceEpoch(0);
    ASSERT_EQ(durationCount<Hours>(newYorkZone.utcOffset(date)), -5);
}

TEST(UTCTimeAtEpoch, DoesComputeISOWeek) {
    // Thursday, January 1, 1970.
    auto date = Date_t::fromMillisSinceEpoch(0);
    ASSERT_EQ(TimeZoneDatabase::utcZone().isoWeek(date), 1);
}

TEST(NewYorkTimeAtEpoch, DoesComputeISOWeek) {
    auto newYorkZone = kDefaultTimeZoneDatabase.getTimeZone("America/New_York");

    // Thu, Jan 1, 1970 00:00:00:000Z
    auto date = Date_t::fromMillisSinceEpoch(0);
    // This is Wednesday in New York, but that is still part of the first week.
    ASSERT_EQ(newYorkZone.isoWeek(date), 1);
}

TEST(UTCTimeAtEpoch, DoesFormatDate) {
    // Thu, Jan 1, 1970 00:00:00:000
    auto date = Date_t::fromMillisSinceEpoch(0);
    ASSERT_EQ(TimeZoneDatabase::utcZone().formatDate("%Y/%m/%d %H:%M:%S:%L, dayOfYear: %j, "
                                                     "dayOfWeek: %w, week: %U, isoYear: %G, "
                                                     "isoWeek: %V, isoDayOfWeek: %u, percent: %%",
                                                     date),
              "1970/01/01 00:00:00:000, dayOfYear: 001, dayOfWeek: 5, week: 00, isoYear: 1970, "
              "isoWeek: 01, isoDayOfWeek: 4, percent: %");
}

TEST(NewYorkTimeAtEpoch, DoesFormatDate) {
    auto newYorkZone = kDefaultTimeZoneDatabase.getTimeZone("America/New_York");

    // Thu, Jan 1, 1970 00:00:00:000Z
    auto date = Date_t::fromMillisSinceEpoch(0);
    ASSERT_EQ(newYorkZone.formatDate("%Y/%m/%d %H:%M:%S:%L, dayOfYear: %j, "
                                     "dayOfWeek: %w, week: %U, isoYear: %G, "
                                     "isoWeek: %V, isoDayOfWeek: %u, percent: %%",
                                     date),
              "1969/12/31 19:00:00:000, dayOfYear: 365, dayOfWeek: 4, week: 52, isoYear: 1970, "
              "isoWeek: 01, isoDayOfWeek: 3, percent: %");
}

TEST(UTCTimeAtEpoch, DoesOutputFormatDate) {
    auto date = Date_t::fromMillisSinceEpoch(0);
    std::ostringstream os;
    TimeZoneDatabase::utcZone().outputDateWithFormat(os,
                                                     "%Y/%m/%d %H:%M:%S:%L, dayOfYear: %j, "
                                                     "dayOfWeek: %w, week: %U, isoYear: %G, "
                                                     "isoWeek: %V, isoDayOfWeek: %u, percent: %%",
                                                     date);
    ASSERT_EQ(os.str(),
              "1970/01/01 00:00:00:000, dayOfYear: 001, dayOfWeek: 5, week: 00, isoYear: 1970, "
              "isoWeek: 01, isoDayOfWeek: 4, percent: %");
}

TEST(NewYorkTimeAtEpoch, DoesOutputFormatDate) {
    auto newYorkZone = kDefaultTimeZoneDatabase.getTimeZone("America/New_York");
    auto date = Date_t::fromMillisSinceEpoch(0);
    std::ostringstream os;
    newYorkZone.outputDateWithFormat(os,
                                     "%Y/%m/%d %H:%M:%S:%L, dayOfYear: %j, "
                                     "dayOfWeek: %w, week: %U, isoYear: %G, "
                                     "isoWeek: %V, isoDayOfWeek: %u, percent: %%",
                                     date);
    ASSERT_EQ(os.str(),
              "1969/12/31 19:00:00:000, dayOfYear: 365, dayOfWeek: 4, week: 52, isoYear: 1970, "
              "isoWeek: 01, isoDayOfWeek: 3, percent: %");
}

TEST(UTCTimeAfterEpoch, DoesExtractDateParts) {
    // Jun 6, 2017 19:38:43:123.
    auto date = Date_t::fromMillisSinceEpoch(1496777923123LL);
    auto dateParts = TimeZoneDatabase::utcZone().dateParts(date);
    ASSERT_EQ(dateParts.year, 2017);
    ASSERT_EQ(dateParts.month, 6);
    ASSERT_EQ(dateParts.dayOfMonth, 6);
    ASSERT_EQ(dateParts.hour, 19);
    ASSERT_EQ(dateParts.minute, 38);
    ASSERT_EQ(dateParts.second, 43);
    ASSERT_EQ(dateParts.millisecond, 123);
}

TEST(NewYorkTimeAfterEpoch, DoesExtractDateParts) {
    auto newYorkZone = kDefaultTimeZoneDatabase.getTimeZone("America/New_York");

    // Jun 6, 2017 19:38:43:123Z.
    auto date = Date_t::fromMillisSinceEpoch(1496777923123LL);
    auto dateParts = newYorkZone.dateParts(date);
    ASSERT_EQ(dateParts.year, 2017);
    ASSERT_EQ(dateParts.month, 6);
    ASSERT_EQ(dateParts.dayOfMonth, 6);
    ASSERT_EQ(dateParts.hour, 15);
    ASSERT_EQ(dateParts.minute, 38);
    ASSERT_EQ(dateParts.second, 43);
    ASSERT_EQ(dateParts.millisecond, 123);
}

TEST(UTCTimeAfterEpoch, DoesComputeISOYear) {
    // Tuesday, June 6, 2017.
    auto date = Date_t::fromMillisSinceEpoch(1496777923000LL);
    ASSERT_EQ(TimeZoneDatabase::utcZone().isoYear(date), 2017);
    // Saturday, January 1, 2005, part of the previous year.
    date = Date_t::fromMillisSinceEpoch(1104537600000LL);
    ASSERT_EQ(TimeZoneDatabase::utcZone().isoYear(date), 2004);
    // Monday, January 1, 2007.
    date = Date_t::fromMillisSinceEpoch(1167609600000LL);
    ASSERT_EQ(TimeZoneDatabase::utcZone().isoYear(date), 2007);
    // Monday, December 31, 2007, part of the next year.
    date = Date_t::fromMillisSinceEpoch(1199059200000LL);
    ASSERT_EQ(TimeZoneDatabase::utcZone().isoYear(date), 2008);
}

TEST(NewYorkTimeAfterEpoch, DoesComputeISOYear) {
    auto newYorkZone = kDefaultTimeZoneDatabase.getTimeZone("America/New_York");

    // 2017-06-06T19:38:43:123Z (Tuesday).
    auto date = Date_t::fromMillisSinceEpoch(1496777923000LL);
    ASSERT_EQ(newYorkZone.isoYear(date), 2017);

    // 2005-01-01T00:00:00.000Z (Saturday), part of 2004.
    date = Date_t::fromMillisSinceEpoch(1104537600000LL);
    ASSERT_EQ(newYorkZone.isoYear(date), 2004);

    // 2007-01-01T00:00:00.000Z (Monday), part of 2007.
    date = Date_t::fromMillisSinceEpoch(1167609600000LL);
    // ISO weeks are Mon-Sun, so this is part of the previous week in New York, so part of the
    // previous year, 2006.
    ASSERT_EQ(newYorkZone.isoYear(date), 2006);

    // 2007-12-31T00:00:00.000Z (Monday), part of 2007.
    date = Date_t::fromMillisSinceEpoch(1199059200000LL);
    // ISO weeks are Mon-Sun, so this is part of the previous week in New York, so part of the
    // previous year, 2007.
    ASSERT_EQ(newYorkZone.isoYear(date), 2007);
}

TEST(UTCTimeAfterEpoch, DoesComputeDayOfWeek) {
    // Tuesday, June 6, 2017.
    auto date = Date_t::fromMillisSinceEpoch(1496777923000LL);
    ASSERT_EQ(TimeZoneDatabase::utcZone().dayOfWeek(date), 3);
    // Saturday, January 1, 2005.
    date = Date_t::fromMillisSinceEpoch(1104537600000LL);
    ASSERT_EQ(TimeZoneDatabase::utcZone().dayOfWeek(date), 7);
    // Monday, January 1, 2007.
    date = Date_t::fromMillisSinceEpoch(1167609600000LL);
    ASSERT_EQ(TimeZoneDatabase::utcZone().dayOfWeek(date), 2);
}

TEST(NewYorkTimeAfterEpoch, DoesComputeDayOfWeek) {
    auto newYorkZone = kDefaultTimeZoneDatabase.getTimeZone("America/New_York");

    // 2017-06-06T19:38:43.123Z (Tuesday).
    auto date = Date_t::fromMillisSinceEpoch(1496777923000LL);
    ASSERT_EQ(newYorkZone.dayOfWeek(date), 3);

    // 2005-01-01T00:00:00.000Z (Saturday).
    date = Date_t::fromMillisSinceEpoch(1104537600000LL);
    ASSERT_EQ(newYorkZone.dayOfWeek(date), 6);  // Part of the previous day in New York.
}

TEST(UTCTimeAfterEpoch, DoesComputeISODayOfWeek) {
    // Tuesday, June 6, 2017.
    auto date = Date_t::fromMillisSinceEpoch(1496777923000LL);
    ASSERT_EQ(TimeZoneDatabase::utcZone().isoDayOfWeek(date), 2);
    // Saturday, January 1, 2005.
    date = Date_t::fromMillisSinceEpoch(1104537600000LL);
    ASSERT_EQ(TimeZoneDatabase::utcZone().isoDayOfWeek(date), 6);
    // Monday, January 1, 2007.
    date = Date_t::fromMillisSinceEpoch(1167609600000LL);
    ASSERT_EQ(TimeZoneDatabase::utcZone().isoDayOfWeek(date), 1);
}

TEST(NewYorkTimeAfterEpoch, DoesComputeISODayOfWeek) {
    auto newYorkZone = kDefaultTimeZoneDatabase.getTimeZone("America/New_York");

    // 2017-06-06T19:38:43.123Z (Tuesday).
    auto date = Date_t::fromMillisSinceEpoch(1496777923000LL);
    ASSERT_EQ(newYorkZone.isoDayOfWeek(date), 2);

    // 2005-01-01T00:00:00.000Z (Saturday).
    date = Date_t::fromMillisSinceEpoch(1104537600000LL);
    ASSERT_EQ(newYorkZone.isoDayOfWeek(date), 5);  // Part of the previous day in New York.
}

TEST(UTCTimeAfterEpoch, DoesComputeDayOfYear) {
    // June 6, 2017.
    auto date = Date_t::fromMillisSinceEpoch(1496777923000LL);
    ASSERT_EQ(TimeZoneDatabase::utcZone().dayOfYear(date), 157);
    // January 1, 2005.
    date = Date_t::fromMillisSinceEpoch(1104537600000LL);
    ASSERT_EQ(TimeZoneDatabase::utcZone().dayOfYear(date), 1);
    // Feb 28, 2008 (leap year).
    date = Date_t::fromMillisSinceEpoch(1204156800000LL);
    ASSERT_EQ(TimeZoneDatabase::utcZone().dayOfYear(date), 59);
    // Feb 29, 2008 (leap year).
    date = Date_t::fromMillisSinceEpoch(1204243200000LL);
    ASSERT_EQ(TimeZoneDatabase::utcZone().dayOfYear(date), 60);
    // Mar 1, 2008 (leap year).
    date = Date_t::fromMillisSinceEpoch(1204329600000LL);
    ASSERT_EQ(TimeZoneDatabase::utcZone().dayOfYear(date), 61);
    // December 31, 2008 (leap year).
    date = Date_t::fromMillisSinceEpoch(1230681600000LL);
    ASSERT_EQ(TimeZoneDatabase::utcZone().dayOfYear(date), 366);

    // Feb 28, 2001 (not leap year).
    date = Date_t::fromMillisSinceEpoch(983318400000LL);
    ASSERT_EQ(TimeZoneDatabase::utcZone().dayOfYear(date), 59);
    // Mar 1, 2001 (not leap year).
    date = Date_t::fromMillisSinceEpoch(983404800000LL);
    ASSERT_EQ(TimeZoneDatabase::utcZone().dayOfYear(date), 60);
    // December 31, 2001 (not leap year).
    date = Date_t::fromMillisSinceEpoch(1009756800000LL);
    ASSERT_EQ(TimeZoneDatabase::utcZone().dayOfYear(date), 365);
}

TEST(NewYorkTimeAfterEpoch, DoesComputeDayOfYear) {
    auto newYorkZone = kDefaultTimeZoneDatabase.getTimeZone("America/New_York");

    // 2017-06-06T19:38:43.123Z.
    auto date = Date_t::fromMillisSinceEpoch(1496777923000LL);
    ASSERT_EQ(newYorkZone.dayOfYear(date), 157);

    // 2005-01-01T00:00:00.000Z, part of 2004 in New York, which was a leap year.
    date = Date_t::fromMillisSinceEpoch(1104537600000LL);
    ASSERT_EQ(newYorkZone.dayOfYear(date), 366);

    // 2008-02-28T00:00:00.000Z (leap year).
    date = Date_t::fromMillisSinceEpoch(1204156800000LL);
    ASSERT_EQ(newYorkZone.dayOfYear(date), 58);
    // 2008-02-29T00:00:00.000Z.
    date = Date_t::fromMillisSinceEpoch(1204243200000LL);
    ASSERT_EQ(newYorkZone.dayOfYear(date), 59);
    // 2008-03-01T00:00:00.000Z.
    date = Date_t::fromMillisSinceEpoch(1204329600000LL);
    ASSERT_EQ(newYorkZone.dayOfYear(date), 60);
    // 2008-12-31T00:00:00.000Z.
    date = Date_t::fromMillisSinceEpoch(1230681600000LL);
    ASSERT_EQ(newYorkZone.dayOfYear(date), 365);
    // 2009-01-01T00:00:00.000Z, part of the previous year in New York.
    date = Date_t::fromMillisSinceEpoch(1230768000000LL);
    ASSERT_EQ(newYorkZone.dayOfYear(date), 366);

    // 2001-02-28T00:00:00.000Z (not leap year).
    date = Date_t::fromMillisSinceEpoch(983318400000LL);
    ASSERT_EQ(newYorkZone.dayOfYear(date), 58);
    // 2001-03-01T00:00:00.000Z.
    date = Date_t::fromMillisSinceEpoch(983404800000LL);
    ASSERT_EQ(newYorkZone.dayOfYear(date), 59);
    // 2001-12-31T00:00:00.000Z.
    date = Date_t::fromMillisSinceEpoch(1009756800000LL);
    ASSERT_EQ(newYorkZone.dayOfYear(date), 364);
    // 2002-01-01T00:00:00.000Z, part of the previous year in New York.
    date = Date_t::fromMillisSinceEpoch(1009843200000LL);
    ASSERT_EQ(newYorkZone.dayOfYear(date), 365);
}

TEST(UTCTimeAfterEpoch, DoesComputeWeek) {
    // Tuesday, June 6, 2017.
    auto date = Date_t::fromMillisSinceEpoch(1496777923000LL);
    ASSERT_EQ(TimeZoneDatabase::utcZone().week(date), 23);
    // Saturday, January 1, 2005, before first Sunday.
    date = Date_t::fromMillisSinceEpoch(1104537600000LL);
    ASSERT_EQ(TimeZoneDatabase::utcZone().week(date), 0);
    // Monday, January 1, 2007, before first Sunday.
    date = Date_t::fromMillisSinceEpoch(1167609600000LL);
    ASSERT_EQ(TimeZoneDatabase::utcZone().week(date), 0);
}

TEST(NewYorkTimeAfterEpoch, DoesComputeWeek) {
    auto newYorkZone = kDefaultTimeZoneDatabase.getTimeZone("America/New_York");

    // Tuesday, June 6, 2017.
    auto date = Date_t::fromMillisSinceEpoch(1496777923000LL);
    ASSERT_EQ(newYorkZone.week(date), 23);

    // 2005-01-01T00:00:00.00Z (Saturday).
    date = Date_t::fromMillisSinceEpoch(1104537600000LL);
    ASSERT_EQ(newYorkZone.week(date), 52);

    // 2007-01-01T00:00:00.00Z (Monday), the last Sunday of 2006 in New York.
    date = Date_t::fromMillisSinceEpoch(1167609600000LL);
    ASSERT_EQ(newYorkZone.week(date), 53);
}

TEST(UTCTimeAfterEpoch, DoesComputeUtcOffset) {
    // Tuesday, June 6, 2017.
    auto date = Date_t::fromMillisSinceEpoch(1496777923000LL);
    ASSERT_EQ(durationCount<Seconds>(TimeZoneDatabase::utcZone().utcOffset(date)), 0);
}

TEST(NewYorkTimeAfterEpoch, DoesComputeUtcOffset) {
    auto newYorkZone = kDefaultTimeZoneDatabase.getTimeZone("America/New_York");

    // Tuesday, June 6, 2017.
    auto date = Date_t::fromMillisSinceEpoch(1496777923000LL);
    ASSERT_EQ(durationCount<Hours>(newYorkZone.utcOffset(date)), -4);

    // 2005-01-01T00:00:00.00Z (Saturday).
    date = Date_t::fromMillisSinceEpoch(1104537600000LL);
    ASSERT_EQ(durationCount<Hours>(newYorkZone.utcOffset(date)), -5);
}

TEST(UTCTimeAfterEpoch, DoesComputeISOWeek) {
    // Tuesday, June 6, 2017.
    auto date = Date_t::fromMillisSinceEpoch(1496777923000LL);
    ASSERT_EQ(TimeZoneDatabase::utcZone().isoWeek(date), 23);
    // Saturday, January 1, 2005, considered part of 2004, which was a leap year.
    date = Date_t::fromMillisSinceEpoch(1104537600000LL);
    ASSERT_EQ(TimeZoneDatabase::utcZone().isoWeek(date), 53);
    // Monday, January 1, 2007.
    date = Date_t::fromMillisSinceEpoch(1167609600000LL);
    ASSERT_EQ(TimeZoneDatabase::utcZone().isoWeek(date), 1);
}

TEST(NewYorkTimeAfterEpoch, DoesComputeISOWeek) {
    auto newYorkZone = kDefaultTimeZoneDatabase.getTimeZone("America/New_York");

    // Tuesday, June 6, 2017.
    auto date = Date_t::fromMillisSinceEpoch(1496777923000LL);
    ASSERT_EQ(newYorkZone.isoWeek(date), 23);

    // 2005-01-01T00:00:00.000Z (Saturday), part of 2004.
    date = Date_t::fromMillisSinceEpoch(1104537600000LL);
    ASSERT_EQ(newYorkZone.isoWeek(date), 53);

    // 2007-01-01T00:00:00.000Z (Monday), part of 2006 in New York.
    date = Date_t::fromMillisSinceEpoch(1167609600000LL);
    ASSERT_EQ(newYorkZone.isoWeek(date), 52);
}

TEST(UTCTimeAfterEpoch, DoesFormatDate) {
    // Tue, Jun 6, 2017 19:38:43:234.
    auto date = Date_t::fromMillisSinceEpoch(1496777923234LL);
    ASSERT_EQ(TimeZoneDatabase::utcZone().formatDate("%Y/%m/%d %H:%M:%S:%L, dayOfYear: %j, "
                                                     "dayOfWeek: %w, week: %U, isoYear: %G, "
                                                     "isoWeek: %V, isoDayOfWeek: %u, percent: %%",
                                                     date),
              "2017/06/06 19:38:43:234, dayOfYear: 157, dayOfWeek: 3, week: 23, isoYear: 2017, "
              "isoWeek: 23, isoDayOfWeek: 2, percent: %");
}

TEST(NewYorkTimeAfterEpoch, DoesFormatDate) {
    // 2017-06-06T19:38:43:234Z (Tuesday).
    auto date = Date_t::fromMillisSinceEpoch(1496777923234LL);
    auto newYorkZone = kDefaultTimeZoneDatabase.getTimeZone("America/New_York");
    ASSERT_EQ(newYorkZone.formatDate("%Y/%m/%d %H:%M:%S:%L, dayOfYear: %j, "
                                     "dayOfWeek: %w, week: %U, isoYear: %G, "
                                     "isoWeek: %V, isoDayOfWeek: %u, percent: %%",
                                     date),
              "2017/06/06 15:38:43:234, dayOfYear: 157, dayOfWeek: 3, week: 23, isoYear: 2017, "
              "isoWeek: 23, isoDayOfWeek: 2, percent: %");
}

TEST(UTCOffsetAfterEpoch, DoesFormatDate) {
    // 2017-06-06T19:38:43:234Z (Tuesday).
    auto date = Date_t::fromMillisSinceEpoch(1496777923234LL);
    auto offsetSpec = kDefaultTimeZoneDatabase.getTimeZone("+02:30");
    ASSERT_EQ(offsetSpec.formatDate("%Y/%m/%d %H:%M:%S:%L, dayOfYear: %j, "
                                    "dayOfWeek: %w, week: %U, isoYear: %G, "
                                    "isoWeek: %V, isoDayOfWeek: %u, percent: %%",
                                    date),
              "2017/06/06 22:08:43:234, dayOfYear: 157, dayOfWeek: 3, week: 23, isoYear: 2017, "
              "isoWeek: 23, isoDayOfWeek: 2, percent: %");
}

TEST(UTCTimeAfterEpoch, DoesOutputFormatDate) {
    // Tue, Jun 6, 2017 19:38:43:234.
    auto date = Date_t::fromMillisSinceEpoch(1496777923234LL);
    std::ostringstream os;
    TimeZoneDatabase::utcZone().outputDateWithFormat(os,
                                                     "%Y/%m/%d %H:%M:%S:%L, dayOfYear: %j, "
                                                     "dayOfWeek: %w, week: %U, isoYear: %G, "
                                                     "isoWeek: %V, isoDayOfWeek: %u, percent: %%",
                                                     date);
    ASSERT_EQ(os.str(),
              "2017/06/06 19:38:43:234, dayOfYear: 157, dayOfWeek: 3, week: 23, isoYear: 2017, "
              "isoWeek: 23, isoDayOfWeek: 2, percent: %");
}

TEST(NewYorkTimeAfterEpoch, DoesOutputFormatDate) {
    // 2017-06-06T19:38:43:234Z (Tuesday).
    auto date = Date_t::fromMillisSinceEpoch(1496777923234LL);
    std::ostringstream os;
    auto newYorkZone = kDefaultTimeZoneDatabase.getTimeZone("America/New_York");
    newYorkZone.outputDateWithFormat(os,
                                     "%Y/%m/%d %H:%M:%S:%L, dayOfYear: %j, "
                                     "dayOfWeek: %w, week: %U, isoYear: %G, "
                                     "isoWeek: %V, isoDayOfWeek: %u, percent: %%",
                                     date);
    ASSERT_EQ(os.str(),
              "2017/06/06 15:38:43:234, dayOfYear: 157, dayOfWeek: 3, week: 23, isoYear: 2017, "
              "isoWeek: 23, isoDayOfWeek: 2, percent: %");
}

TEST(DateFormat, ThrowsUserExceptionIfGivenUnrecognizedFormatter) {
    ASSERT_THROWS_CODE(TimeZoneDatabase::utcZone().validateFormat("%x"), UserException, 18536);
}

TEST(DateFormat, ThrowsUserExceptionIfGivenUnmatchedPercent) {
    ASSERT_THROWS_CODE(TimeZoneDatabase::utcZone().validateFormat("%"), UserException, 18535);
    ASSERT_THROWS_CODE(TimeZoneDatabase::utcZone().validateFormat("%%%"), UserException, 18535);
    ASSERT_THROWS_CODE(
        TimeZoneDatabase::utcZone().validateFormat("blahblah%"), UserException, 18535);
}

TEST(DateFormat, ThrowsUserExceptionIfGivenDateBeforeYear0) {
    const long long kMillisPerYear = 31556926000;
    ASSERT_THROWS_CODE(TimeZoneDatabase::utcZone().formatDate(
                           "%Y", Date_t::fromMillisSinceEpoch(-(kMillisPerYear * 1971))),
                       UserException,
                       18537);
    ASSERT_EQ("0000",
              TimeZoneDatabase::utcZone().formatDate(
                  "%Y", Date_t::fromMillisSinceEpoch(-(kMillisPerYear * 1970))));
}

TEST(DateFormat, ThrowsUserExceptionIfGivenDateAfterYear9999) {
    ASSERT_THROWS_CODE(
        TimeZoneDatabase::utcZone().formatDate("%Y", Date_t::max()), UserException, 18537);
}

}  // namespace
}  // namespace mongo
