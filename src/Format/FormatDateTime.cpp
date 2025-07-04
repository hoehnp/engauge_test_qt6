/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#include "EngaugeAssert.h"
#include "FormatDateTime.h"
#include "Logger.h"
#include <QDateTime>
#include <qmath.h>
#include <QTimeZone>

FormatDateTime::FormatDateTime()
{
  loadFormatsFormat();
  loadFormatsParseAcceptable();
  loadFormatsParseIncomplete();
}

bool FormatDateTime::ambiguityBetweenDateAndTime (CoordUnitsDate coordUnitsDate,
                                                  CoordUnitsTime coordUnitsTime,
                                                  const QString &string) const
{

  bool ambiguous = false;

  // There is no ambiguity if user specified either date or time as empty
  if (coordUnitsDate != COORD_UNITS_DATE_SKIP &&
      coordUnitsTime != COORD_UNITS_TIME_SKIP) {

    // See if there is just a single number
    QStringList fields = string.trimmed().split(QRegularExpression ("[/- :]"));

    if (fields.count() == 1) {

      // There is a single number. Since there are no attached delimiters to differentiate a date versus
      // a time, this means the number is ambiguous
      ambiguous = true;
    }
  }

  return ambiguous;
}

void FormatDateTime::dateTimeLookup (const FormatsDate &formatsDateAll,
                                     const FormatsTime &formatsTimeAll,
                                     CoordUnitsDate coordUnitsDate,
                                     CoordUnitsTime coordUnitsTime,
                                     const QString &string,
                                     bool useQDateTimeElseQRegExp,
                                     double &value,
                                     bool &success) const
{

  success = false;

  ENGAUGE_ASSERT (formatsDateAll.contains (coordUnitsDate));
  ENGAUGE_ASSERT (formatsTimeAll.contains (coordUnitsTime));

  QStringList formatsDate = formatsDateAll [coordUnitsDate];
  QStringList formatsTime = formatsTimeAll [coordUnitsTime];

  // Loop through the legal date/time combinations
  QStringList::const_iterator itrDate, itrTime;
  bool iterating = true;
  for (itrDate = formatsDate.begin(); itrDate != formatsDate.end() && iterating; itrDate++) {

    QString formatDate = *itrDate;

    for (itrTime = formatsTime.begin(); itrTime != formatsTime.end() && iterating; itrTime++) {

      QString formatTime = *itrTime;

      // Insert space as separator only if needed. Do not use trim around here since formatDate may or may not end in a space
      QString separator = (!formatDate.isEmpty() && !formatTime.isEmpty() ? " " : "");

      QString formatDateTime = formatDate + separator + formatTime;

      if (!formatDateTime.isEmpty()) {

        // Try parsing according to the current format
        if (useQDateTimeElseQRegExp) {

          QDateTime dt = QDateTime::fromString (string,
                                                formatDateTime);

          if (dt.isValid() && !ambiguityBetweenDateAndTime (coordUnitsDate,
                                                            coordUnitsTime,
                                                            string)) {

            success = true;
            // Convert using local time to prevent addition of utc offset. Number of seconds since 1970 epoch
            // is used with 64 bits resolution, versus time_t with only 32 bits resolution (and limites to 1970 to 2038).
            // Time value is negative for pre-epoch. Grep for fromSecsSinceEpoch in this same file
            value = toSecsSinceEpoch (dt.toLocalTime ());
            iterating = false; // Stop iterating

          }
        } else {

          QRegularExpression reg (formatDateTime);
          if (reg.match(string).hasMatch()) {

            success = true; // Note that value does not get set in QRegExp case
            iterating = false; // Stop iterating

          }
        }
      }
    }
  }
}

QString FormatDateTime::formatOutput (CoordUnitsDate coordUnitsDate,
                                      CoordUnitsTime coordUnitsTime,
                                      double value) const
{

  ENGAUGE_ASSERT (m_formatsDateFormat.contains (coordUnitsDate));
  ENGAUGE_ASSERT (m_formatsTimeFormat.contains (coordUnitsTime));

  QString format = m_formatsDateFormat [coordUnitsDate] + " " + m_formatsTimeFormat [coordUnitsTime];
  format = format.trimmed();

  // We are using 64 bits resolution on seconds from epoch rather than time_t which has 32 bits resolution (and
  // is therefore limited to 1970 to 2038). Time value is negative for pre-epoch. Grep for toSecsSinceEpoch in this same file
  QDateTime dt;
  dt = fromSecsSinceEpoch (static_cast<qint64> (value));

  return dt.toLocalTime ().toString (format); // Convert using local time to prevent addition of utc offset
}

QDateTime FormatDateTime::fromSecsSinceEpoch (qint64 secs) const
{
  return QDateTime::fromMSecsSinceEpoch (secs * 1000);
}

void FormatDateTime::loadFormatsFormat()
{
  m_formatsDateFormat [COORD_UNITS_DATE_SKIP] = "";
  m_formatsDateFormat [COORD_UNITS_DATE_MONTH_DAY_YEAR] = "MM/dd/yyyy";
  m_formatsDateFormat [COORD_UNITS_DATE_DAY_MONTH_YEAR] = "dd/MM/yyyy";
  m_formatsDateFormat [COORD_UNITS_DATE_YEAR_MONTH_DAY] = "yyyy/MM/dd";

  ENGAUGE_ASSERT (m_formatsDateFormat.count () == NUM_COORD_UNITS_DATE);

  m_formatsTimeFormat [COORD_UNITS_TIME_SKIP] = "";
  m_formatsTimeFormat [COORD_UNITS_TIME_HOUR_MINUTE] = "hh/mm";
  m_formatsTimeFormat [COORD_UNITS_TIME_HOUR_MINUTE_SECOND] = "hh:mm:ss";

  ENGAUGE_ASSERT (m_formatsTimeFormat.count () == NUM_COORD_UNITS_TIME);
}

void FormatDateTime::loadFormatsParseAcceptable()
{

  QStringList skip, dayMonth, dayMonthYear, monthDay, monthDayYear, yearMonth, yearMonthDay;

  // COORD_UNITS_DATE_SKIP and COORD_UNITS_TIME_SKIP allow date/time respectively even when skipped,
  // although there can be ambiguity with between COORD_UNITS_DATE_MONTH_DAY_YEAR and COORD_UNITS_DATE_DAY_MONTH_YEAR
  skip << "";

  dayMonth << "d/M"
           << "d-M"
           << "d/MM"
           << "d-MM"
           << "d/MMM"
           << "d-MMM"
           << "d/MMMM"
           << "d-MMMM"
           << "dd/M"
           << "dd-M"
           << "dd/M"
           << "dd-M"
           << "dd/MM"
           << "dd-MM"
           << "dd/MMM"
           << "dd-MMM"
           << "dd/MMMM"
           << "dd-MMMM";
  dayMonthYear << "d/M/yyyy"
               << "d-M-yyyy"

               << "d/MM/yyyy"
               << "d-MM-yyyy"
               << "d/MMM/yyyy"
               << "d-MMM-yyyy"
               << "d MMM yyyy"
               << "d/MMMM/yyyy"
               << "d-MMMM-yyyy"
               << "d MMMM yyyy"

               << "dd/MM/yyyy"
               << "dd-MM-yyyy"
               << "dd/MMM/yyyy"
               << "dd-MMM-yyyy"
               << "dd MMM yyyy"
               << "dd/MMMM/yyyy"
               << "dd-MMMM-yyyy"
               << "dd MMMM yyyy";
  monthDay << "M/d"
           << "M-d"
           << "M d"
           << "M/dd"
           << "M-dd"
           << "M dd"
           << "MM/d"
           << "MM-d"
           << "MM d"
           << "MM/dd"
           << "MM-dd"
           << "MM dd"
           << "MMM/d"
           << "MMM-d"
           << "MMM d"
           << "MMM/dd"
           << "MMM-dd"
           << "MMM dd"
           << "MMMM/d"
           << "MMMM-d"
           << "MMMM d"
           << "MMMM/dd"
           << "MMMM-dd"
           << "MMMM dd";
  monthDayYear << "M/d/yyyy"
               << "M-d-yyyy"
               << "M d yyyy"
               << "M/dd/yyyy"
               << "M-dd-yyyy"
               << "M dd yyyy"
               << "MM/d/yyyy"
               << "MM-d-yyyy"
               << "MM d yyyy"
               << "MM/dd/yyyy"
               << "MM-dd-yyyy"
               << "MM dd yyyy"
               << "MMM/d/yyyy"
               << "MMM-d-yyyy"
               << "MMM d yyyy"
               << "MMM/dd/yyyy"
               << "MMM-dd-yyyy"
               << "MMM dd yyyy"
               << "MMMM/d/yyyy"
               << "MMMM-d-yyyy"
               << "MMMM d"
               << "MMMM/dd"
               << "MMMM-dd"
               << "MMMM dd";
  yearMonth << "yyyy/M"
            << "yyyy-M"
            << "yyyy M"
            << "yyyy/MM"
            << "yyyy-MM"
            << "yyyy MM"
            << "yyyy/MMM"
            << "yyyy-MMM"
            << "yyyy MMM"
            << "yyyy/MMMM"
            << "yyyy-MMMM"
            << "yyyy MMMM";
  yearMonthDay << "yyyy/M/d"
               << "yyyy-M-d"
               << "yyyy M d"
               << "yyyy/M/dd"
               << "yyyy-M-dd"
               << "yyyy M dd"
               << "yyyy/MM/dd"
               << "yyyy-MM-dd"
               << "yyyy MM dd"
               << "yyyy/MMM/d"
               << "yyyy-MMM-d"
               << "yyyy MMM d"
               << "yyyy/MMM/dd"
               << "yyyy-MMM-dd"
               << "yyyy MMM dd"
               << "yyyy/MMMM/dd"
               << "yyyy-MMMM-dd"
               << "yyyy MMMM dd";

  // Potential day-month ambiguity for COORD_UNITS_DATE_SKIP gets treated as month/day
  m_formatsDateParseAcceptable [COORD_UNITS_DATE_SKIP] = skip + monthDay + monthDayYear + yearMonthDay;
  m_formatsDateParseAcceptable [COORD_UNITS_DATE_MONTH_DAY_YEAR] = skip + monthDay + monthDayYear + yearMonthDay;
  m_formatsDateParseAcceptable [COORD_UNITS_DATE_DAY_MONTH_YEAR] = skip + dayMonth + dayMonthYear + yearMonthDay;
  m_formatsDateParseAcceptable [COORD_UNITS_DATE_YEAR_MONTH_DAY] = skip + yearMonth + yearMonthDay;

  ENGAUGE_ASSERT (m_formatsDateParseAcceptable.count () == NUM_COORD_UNITS_DATE);

  QStringList hour, hourMinute, hourMinuteSecond, hourMinutePm, hourMinuteSecondPm;

  hour << "hh";
  hourMinute << "hh:mm";
  hourMinuteSecond << "hh:mm:ss";
  hourMinutePm << "hh:mmA"
               << "hh:mm A"
               << "hh:mma"
               << "hh:mm a";
  hourMinuteSecondPm << "hh:mm:ssA"
                     << "hh:mm:ss A"
                     << "hh:mm:ssa"
                     << "hh:mm:ss a";

  m_formatsTimeParseAcceptable [COORD_UNITS_TIME_SKIP] = skip + hour + hourMinute + hourMinuteSecond + hourMinutePm + hourMinuteSecondPm;
  m_formatsTimeParseAcceptable [COORD_UNITS_TIME_HOUR_MINUTE] = skip + hour + hourMinute + hourMinutePm + hourMinuteSecond + hourMinuteSecondPm;
  m_formatsTimeParseAcceptable [COORD_UNITS_TIME_HOUR_MINUTE_SECOND] = skip + hour + hourMinute + hourMinutePm + hourMinuteSecond + hourMinuteSecondPm;

  ENGAUGE_ASSERT (m_formatsTimeParseAcceptable.count () == NUM_COORD_UNITS_TIME);
}

void FormatDateTime::loadFormatsParseIncomplete()
{

  QStringList skip, day, dayMonth, dayMonthYear, month, monthDay, monthDayYear, year, yearMonth, yearMonthDay;

  // COORD_UNITS_DATE_SKIP and COORD_UNITS_TIME_SKIP allow date/time respectively even when skipped,
  // although there can be ambiguity with between COORD_UNITS_DATE_MONTH_DAY_YEAR and COORD_UNITS_DATE_DAY_MONTH_YEAR
  skip << "";

  // IMPORTANT! Be sure to include complete date values since the date, which goes before the time, will be
  //            complete when the time is getting
  day << "\\d{1,2}"
      << "\\d{1,2}/"
      << "\\d{1,2}-";
  dayMonth << "\\d{1,2}/\\d{1,2}"
           << "\\d{1,2}/\\d{1,2} "
           << "\\d{1,2}/\\d{1,2}/"
           << "\\d{1,2}-\\d{1,2}-"
           << "\\d{1,2}/[a-zA-Z]{1,12}/"
           << "\\d{1,2}-[a-zA-Z]{1,12}-"
           << "\\d{1,2} [a-zA-Z]{1,12} ";
  dayMonthYear << "\\d{1,2}/\\d{1,2}/\\d{1,4}"
               << "\\d{1,2}/\\d{1,2}/\\d{1,4} "
               << "\\d{1,2}-\\d{1,2}-\\d{1,4}"
               << "\\d{1,2}-\\d{1,2}-\\d{1,4} ";
  month << "\\d{1,2}"
        << "\\d{1,2}/"
        << "[a-zA-Z]{1,12}"
        << "[a-zA-Z]{1,12} ";
  monthDay << "\\d{1,2}/\\d{1,2}"
           << "\\d{1,2}/\\d{1,2} "
           << "\\d{1,2}/\\d{1,2}/"
           << "\\d{1,2} \\d{1,2}"
           << "\\d{1,2} \\d{1,2} "
           << "\\d{1,2}-\\d{1,2}-"
           << "[a-zA-Z]{1,12}"
           << "[a-zA-Z]{1,12} "
           << "[a-zA-Z]{1,12} \\d{1,2}"
           << "[a-zA-Z]{1,12} \\d{1,2} ";
  monthDayYear << "\\d{1,2}/\\d{1,2}/\\d{1,4}"
               << "\\d{1,2}/\\d{1,2}/\\d{1,4} "
               << "\\d{1,2}-\\d{1,2}-\\d{1,4}"
               << "\\d{1,2}-\\d{1,2}-\\d{1,4} "
               << "\\d{1,2} \\d{1,2} \\d{1,4}"
               << "\\d{1,2} \\d{1,2} \\d{1,4} ";
  year << "\\d{1,4}"
       << "\\d{1,4} "
       << "\\d{1,4}/"
       << "\\d{1,4}-";
  yearMonth << "\\d{4}/\\d{1,2}"
            << "\\d{4}/\\d{1,2} "
            << "\\d{4}/\\d{1,2}/"
            << "\\d{4}-\\d{1,2}"
            << "\\d{4}-\\d{1,2} "
            << "\\d{4}-\\d{1,2}-"
            << "\\d{4} \\d{1,2}"
            << "\\d{4} \\d{1,2} "
            << "\\d{4}/[a-zA-Z]{1,12}"
            << "\\d{4}/[a-zA-Z]{1,12} "
            << "\\d{4}/[a-zA-Z]{1,12}/"
            << "\\d{4}-[a-zA-Z]{1,12}"
            << "\\d{4}-[a-zA-Z]{1,12} "
            << "\\d{4}-[a-zA-Z]{1,12}-"
            << "\\d{4} [a-zA-Z]{1,12}"
            << "\\d{4} [a-zA-Z]{1,12} ";
  yearMonthDay << "\\d{4}/\\d{1,2}/\\d{1,2}"
               << "\\d{4}/\\d{1,2}/\\d{1,2} "    
               << "\\d{4}/\\d{1,2}-\\d{1,2}"
               << "\\d{4}/\\d{1,2}-\\d{1,2} "    
               << "\\d{4} \\d{1,2} \\d{1,2}"
               << "\\d{4}/[a-zA-Z]{1,12}/\\d{1,2}"
               << "\\d{4}/[a-zA-Z]{1,12}/\\d{1,2} "    
               << "\\d{4}-[a-zA-Z]{1,12}-\\d{1,2}"
               << "\\d{4}-[a-zA-Z]{1,12}-\\d{1,2} ";    

  // For every entry, the possible states leading up to the Acceptable states in m_formatsDateParseIncomplete are all included.
  // Potential day-month ambiguity for COORD_UNITS_DATE_SKIP gets treated as month/day.
  m_formatsDateParseIncomplete [COORD_UNITS_DATE_SKIP] = skip + month + monthDay + monthDayYear + year + yearMonth + yearMonthDay;
  m_formatsDateParseIncomplete [COORD_UNITS_DATE_MONTH_DAY_YEAR] = skip + month + monthDay + monthDayYear + year + yearMonth + yearMonthDay;
  m_formatsDateParseIncomplete [COORD_UNITS_DATE_DAY_MONTH_YEAR] = skip + day + dayMonth + dayMonthYear + year + yearMonth + yearMonthDay;
  m_formatsDateParseIncomplete [COORD_UNITS_DATE_YEAR_MONTH_DAY] = skip + year + yearMonth + yearMonthDay;

  ENGAUGE_ASSERT (m_formatsDateParseIncomplete.count () == NUM_COORD_UNITS_DATE);

  QStringList hour, hourMinute, hourMinuteAmPm, hourMinuteSecond,  hourMinuteSecondAmPm;

  hour << "\\d{1,2}"
       << "\\d{1,2}:";
  hourMinute << "\\d{1,2}:\\d{1,2}"
             << "\\d{1,2}:\\d{1,2}:"
             << "\\d{1,2}:\\d{1,2} ";
  hourMinuteAmPm << "\\d{1,2}:\\d{1,2} [aApP]";
  hourMinuteSecond << "\\d{1,2}:\\d{1,2}:\\d{1,2}"
                   << "\\d{1,2}:\\d{1,2}:\\d{1,2} ";
  hourMinuteSecondAmPm << "\\d{1,2}:\\d{1,2}:\\d{1,2} [aApP]";

  // For every entry, the possible states leading up to the Acceptable states in m_formatsTimeParseIncomplete are all included.
  m_formatsTimeParseIncomplete [COORD_UNITS_TIME_SKIP] = skip +
                                                         hour +
                                                         hourMinute + hourMinuteAmPm +
                                                         hourMinuteSecond + hourMinuteSecondAmPm;
  m_formatsTimeParseIncomplete [COORD_UNITS_TIME_HOUR_MINUTE] = skip +
                                                                hour +
                                                                hourMinute + hourMinuteAmPm +
                                                                hourMinuteSecond + hourMinuteSecondAmPm;
  m_formatsTimeParseIncomplete [COORD_UNITS_TIME_HOUR_MINUTE_SECOND] = skip +
                                                                       hour +
                                                                       hourMinute + hourMinuteAmPm +
                                                                       hourMinuteSecond + hourMinuteSecondAmPm;

  ENGAUGE_ASSERT (m_formatsTimeParseIncomplete.count () == NUM_COORD_UNITS_TIME);
}

QValidator::State FormatDateTime::parseInput (CoordUnitsDate coordUnitsDate,
                                              CoordUnitsTime coordUnitsTime,
                                              const QString &stringUntrimmed,
                                              double &value) const
{
  const bool USE_QREGEXP = true, DO_NOT_USE_QREGEXP = false;

  const QString string = stringUntrimmed.trimmed();

  QValidator::State state;
  if (string.isEmpty()) {

    state = QValidator::Intermediate;

  } else {

    state = QValidator::Invalid;

    // First see if value is acceptable
    bool success = false;
    dateTimeLookup (m_formatsDateParseAcceptable,
                    m_formatsTimeParseAcceptable,
                    coordUnitsDate,
                    coordUnitsTime,
                    string,
                    USE_QREGEXP,
                    value,
                    success);
    if (success) {
      
      state = QValidator::Acceptable;
      
    } else {
      
      // Not acceptable, but perhaps it is just incomplete
      dateTimeLookup (m_formatsDateParseIncomplete,
                      m_formatsTimeParseIncomplete,
                      coordUnitsDate,
                      coordUnitsTime,
                      string,
                      DO_NOT_USE_QREGEXP,
                      value,
                      success);
      if (success) {
        
        state = QValidator::Intermediate;
        
      }
    }
  }

  return state;
}

qint64 FormatDateTime::toSecsSinceEpoch (const QDateTime &dt) const
{
  return dt.toMSecsSinceEpoch () / static_cast<qint64> (1000);
}
