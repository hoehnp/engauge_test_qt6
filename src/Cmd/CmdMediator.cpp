/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#include "CmdAbstract.h"
#include "CmdFactory.h"
#include "CmdMediator.h"
#include "Document.h"
#include "DocumentSerialize.h"
#include "EngaugeAssert.h"
#include "Logger.h"
#include "LoggerUpload.h"
#include "MainWindow.h"
#include "Point.h"
#include <QDomNode>
#include <QImage>
#include <QUndoCommand>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include "Transformation.h"
#include "Xml.h"

CmdMediator::CmdMediator (MainWindow &mainWindow,
                          const QImage &image) :
  m_document (image)
{

  connectSignals(mainWindow);
}

CmdMediator::CmdMediator (MainWindow &mainWindow,
                          const QString &fileName) :
  m_document (fileName)
{

  connectSignals(mainWindow);
}

CmdMediator::~CmdMediator()
{
}

void CmdMediator::connectSignals (MainWindow &mainWindow)
{
  connect (this, SIGNAL (cleanChanged (bool)), &mainWindow, SLOT (slotCleanChanged (bool)));
}

const CoordSystem &CmdMediator::coordSystem() const
{
  return m_document.coordSystem();
}

const Curve &CmdMediator::curveAxes () const
{
  return m_document.curveAxes ();
}

QStringList CmdMediator::curvesGraphsNames () const
{
  return m_document.curvesGraphsNames();
}

int CmdMediator::curvesGraphsNumPoints (const QString &curveName) const
{
  return m_document.curvesGraphsNumPoints(curveName);
}

Document &CmdMediator::document()
{
  return m_document;
}

const Document &CmdMediator::document() const
{
  return m_document;
}

bool CmdMediator::isModified () const
{
  return !isClean();
}

void CmdMediator::iterateThroughCurvePointsAxes (const Functor2wRet<const QString &, const Point &, CallbackSearchReturn> &ftorWithCallback)
{
  m_document.iterateThroughCurvePointsAxes (ftorWithCallback);
}

void CmdMediator::iterateThroughCurvePointsAxes (const Functor2wRet<const QString &, const Point &, CallbackSearchReturn> &ftorWithCallback) const
{
  m_document.iterateThroughCurvePointsAxes (ftorWithCallback);
}

void CmdMediator::iterateThroughCurvesPointsGraphs (const Functor2wRet<const QString &, const Point &, CallbackSearchReturn> &ftorWithCallback)
{
  return m_document.iterateThroughCurvesPointsGraphs (ftorWithCallback);
}

QPixmap CmdMediator::pixmap () const
{
  ENGAUGE_ASSERT (m_document.successfulRead ());

  return m_document.pixmap ();
}

QString CmdMediator::reasonForUnsuccessfulRead () const
{
  return m_document.reasonForUnsuccessfulRead ();
}

void CmdMediator::saveXml(QXmlStreamWriter &writer) const
{
  writer.writeStartElement(DOCUMENT_SERIALIZE_CMD_MEDIATOR);

  for (int i = 0; i < index(); i++) {

    const CmdAbstract *cmd = dynamic_cast<const CmdAbstract *>(command(i));
    cmd->saveXml (writer);
  }

  writer.writeEndElement();
}

QString CmdMediator::selectedCurveName () const
{
  return m_document.selectedCurveName ();
}

void CmdMediator::setDocumentAxesPointsRequired (DocumentAxesPointsRequired documentAxesPointsRequired)
{
  m_document.setDocumentAxesPointsRequired (documentAxesPointsRequired);
}

void CmdMediator::setSelectedCurveName(const QString &selectedCurveName)
{
  m_document.setSelectedCurveName (selectedCurveName);
}

bool CmdMediator::successfulRead () const
{
  return m_document.successfulRead();
}
