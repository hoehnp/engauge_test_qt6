/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#include "Document.h"
#include "LoadFileInfo.h"
#include "Logger.h"
#include "UrlDirty.h"

LoadFileInfo::LoadFileInfo()
{
}

LoadFileInfo::~LoadFileInfo()
{
}

bool LoadFileInfo::loadsAsDigFile (const QString &urlString) const
{

  bool success = false;

  if (urlString.length() > 0) {

    QString fileName = urlString;

    UrlDirty url (urlString);
    if (url.isLocalFile ()) {
      fileName = url.toLocalFile();
    }

    Document document (fileName);

    success = document.successfulRead();
  }

  return success;
}
