/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#include "LoadImageFromUrl.h"
#include "Logger.h"
#include "MainWindow.h"
#include <QFileInfo>
#include <QMessageBox>
#include <QTextStream>
#ifdef NETWORKING
#include <QtNetwork/QNetworkReply>
#endif
#include "UrlDirty.h"
#include "Version.h"

LoadImageFromUrl::LoadImageFromUrl (MainWindow &mainWindow) :
  m_mainWindow (mainWindow),
#ifdef NETWORKING
  m_http (this),
  m_reply (nullptr),
#endif
  m_buffer (nullptr)
{
  connect (this, SIGNAL (signalImportImage (QString, QImage)), &m_mainWindow, SLOT (slotFileImportImage (QString, QImage)));
}

LoadImageFromUrl::~LoadImageFromUrl ()
{
  deallocate ();
}

void LoadImageFromUrl::deallocate ()
{
#ifdef NETWORKING
  delete m_reply;
  delete m_buffer;
  
  m_reply = nullptr;
  m_buffer = nullptr;
#endif
}

void LoadImageFromUrl::slotFinished ()
{
  // Download has just finished

  QString urlWithoutScheme = m_url.toString (QUrl::RemoveScheme);

  // Import
  QImage image;
  if (image.loadFromData (*m_buffer)) {

    emit signalImportImage (urlWithoutScheme,
                            image);
  } else {

    // Images embedded in web pages produce html in m_buffer. No easy way to fix that. Even
    // gimp fails in the same situations so we just show an error

    QString message;
    QTextStream str (&message);

    str << tr ("Unable to download image from") << " " << urlWithoutScheme;

    QMessageBox::critical (&m_mainWindow,
                           engaugeWindowTitle(),
                           message,
                           QMessageBox::Ok);
  }
}

void LoadImageFromUrl::startLoadImage (const UrlDirty &url)
{

  m_url = url;
  if (url.isLocalFile ()) {

    QFileInfo fileInfo (url.toLocalFile ());

    // Load local file. This is done synchronously
    QImage image;
    if (image.load (url.toLocalFile ())) {

      emit signalImportImage (fileInfo.fileName (),
                              image);

    } else {

      // Probably a bad file type

      QString message;
      QTextStream str (&message);

      str << tr ("Unable to load image from") << " " << url.toLocalFile ();

      QMessageBox::critical (&m_mainWindow,
                             engaugeWindowTitle(),
                             message,
                             QMessageBox::Ok);
    }

  } else {

    // Drop on the floor if networking is not enabled
#ifdef NETWORKING
    // Asynchronous read from url
    deallocate ();
    m_buffer = new QByteArray;
    QNetworkRequest request (url);
    m_reply = m_http.get (request);

    connect (m_reply, SIGNAL (readyRead()), this, SLOT (slotReadData()));
    connect (m_reply, SIGNAL (finished ()), this, SLOT (slotFinished ()));
#endif
  }
}

void LoadImageFromUrl::slotReadData ()
{
#ifdef NETWORKING
  *m_buffer += m_reply->readAll ();
#endif
}
