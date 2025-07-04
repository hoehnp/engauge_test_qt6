/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#include "Logger.h"
#include <QFileDevice>
#include <QMessageBox>
#include <QObject>
#include "QtToString.h"
#include "Xml.h"

QXmlStreamReader::TokenType loadNextFromReader (QXmlStreamReader &reader)
{
  QXmlStreamReader::TokenType tokenType = reader.readNext();

  return tokenType;
}

void xmlExitWithError (QXmlStreamReader &reader,
                           const QString &message)
{
  // Try to extract as much useful context as possible
  QString context;
  QFileDevice *fileDevice = dynamic_cast<QFileDevice*> (reader.device());
  if (fileDevice == nullptr) {

    context = QString ("%1 %2: ")
              .arg (QObject::tr ("Start at line"))
              .arg (reader.lineNumber());

  } else {

    context = QString ("%1 %2 %3 %4: ")
              .arg (QObject::tr ("File"))
              .arg (fileDevice->fileName())
              .arg (QObject::tr ("at line"))
              .arg (reader.lineNumber());

  }

  // Context plus original message gets displayed
  QString adornedMsg = QString ("%1%2. %3")
                       .arg (context)
                       .arg (message)
                       .arg (QObject::tr ("Quitting"));

  QMessageBox::critical (nullptr,
                         QObject::tr ("Error reading xml"),
                         adornedMsg);

  exit (-1);
}
