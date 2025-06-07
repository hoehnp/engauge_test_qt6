/******************************************************************************************************
 * (C) 2020 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#include "Compatibility.h"
#include "Logger.h"
#include <QTextStream>
#include <QtGlobal>

using namespace std;

Compatibility::Compatibility ()
{
}

QTextStream &Compatibility::endl (QTextStream &str)
{
  LOG4CPP_INFO_S ((*mainCat)) <<  "Compatibility::endl";
  
  // Comments:
  // 1) QTextStream in text mode uses \n\r for carriage return
  // 2) \n by itself does not flush
  // 3) \n by itself could be safely used wherever this method is used, but this method has nice comments
  //    and the performance gain from skipping the flush that end() adds is insignificant
  
#if QT_VERSION < QT_VERSION_CHECK (5, 14, 0)
  str << "\n"; // std::endl will not compile
#else
  str << Qt::endl; // Enum
#endif

  return str;
}

QTextStream &Compatibility::flush (QTextStream &str)
{
  
#if QT_VERSION < QT_VERSION_CHECK (5, 14, 0)
  // std::flush will not compile so we skip flushing
#else
  str << Qt::flush; // Enum
#endif

  return str;
}

#if QT_VERSION < QT_VERSION_CHECK (5, 14, 0)
QString::SplitBehavior Compatibility::SkipEmptyParts ()
{
  return QString::SkipEmptyParts;  
}
#else
Qt::SplitBehavior Compatibility::SkipEmptyParts ()
{
  return Qt::SkipEmptyParts;
}
#endif
