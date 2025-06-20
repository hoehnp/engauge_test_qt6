/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#include "CmdCut.h"
#include "DataKey.h"
#include "Document.h"
#include "DocumentSerialize.h"
#include "EngaugeAssert.h"
#include "ExportToClipboard.h"
#include "GraphicsItemType.h"
#include "GraphicsView.h"
#include "Logger.h"
#include "MainWindow.h"
#include "MimePointsExport.h"
#include <QApplication>
#include <QClipboard>
#include <QTextStream>
#include "QtToString.h"
#include <QXmlStreamReader>
#include "Xml.h"

const QString CMD_DESCRIPTION ("Cut");

CmdCut::CmdCut(MainWindow &mainWindow,
               Document &document,
               const QStringList &selectedPointIdentifiers) :
  CmdPointChangeBase (mainWindow,
                      document,
                      CMD_DESCRIPTION),
  m_transformIsDefined (mainWindow.transformIsDefined())
{

  // Export to clipboard
  ExportToClipboard exportStrategy;
  QTextStream strCsv (&m_csv), strHtml (&m_html);
  exportStrategy.exportToClipboard (selectedPointIdentifiers,
                                    mainWindow.transformation(),
                                    strCsv,
                                    strHtml,
                                    document.curveAxes(),
                                    document.curvesGraphs(),
                                    m_curvesGraphsRemoved);
}

CmdCut::CmdCut (MainWindow &mainWindow,
                Document &document,
                const QString &cmdDescription,
                QXmlStreamReader &reader) :
  CmdPointChangeBase (mainWindow,
                      document,
                      cmdDescription)
{

  QXmlStreamAttributes attributes = reader.attributes();

  QStringList requiredAttributesLeaf;
  requiredAttributesLeaf << DOCUMENT_SERIALIZE_TRANSFORM_DEFINED
                         << DOCUMENT_SERIALIZE_CSV
                         << DOCUMENT_SERIALIZE_HTML;
  leafAndBaseAttributes (attributes,
                         requiredAttributesLeaf,
                         reader);

  // Boolean values
  QString defined = attributes.value(DOCUMENT_SERIALIZE_TRANSFORM_DEFINED).toString();

  m_transformIsDefined = (defined == DOCUMENT_SERIALIZE_BOOL_TRUE);
  m_csv = attributes.value(DOCUMENT_SERIALIZE_CSV).toString();
  m_html = attributes.value(DOCUMENT_SERIALIZE_HTML).toString();
  m_curvesGraphsRemoved.loadXml(reader);
}

CmdCut::~CmdCut ()
{
}

void CmdCut::cmdRedo ()
{

  MimePointsExport *mimePointsExport;
  if (m_transformIsDefined) {
    mimePointsExport = new MimePointsExport (m_csv,
                                             m_html);
  } else {
    mimePointsExport = new MimePointsExport (m_csv);
  }

  QClipboard *clipboard = QApplication::clipboard();
  clipboard->setMimeData (mimePointsExport, QClipboard::Clipboard);

  restoreState ();
  saveOrCheckPreCommandDocumentStateHash (document ());
  saveDocumentState (document ());
  document().removePointsInCurvesGraphs (m_curvesGraphsRemoved);

  document().updatePointOrdinals (mainWindow().transformation());
  mainWindow().updateAfterCommand();
  saveOrCheckPostCommandDocumentStateHash (document ());
}

void CmdCut::cmdUndo ()
{

  restoreState ();
  saveOrCheckPostCommandDocumentStateHash (document ());
  restoreDocumentState (document ());
  mainWindow().updateAfterCommand();
  saveOrCheckPreCommandDocumentStateHash (document ());
}

void CmdCut::saveXml (QXmlStreamWriter &writer) const
{
  writer.writeStartElement(DOCUMENT_SERIALIZE_CMD);
  writer.writeAttribute(DOCUMENT_SERIALIZE_CMD_TYPE, DOCUMENT_SERIALIZE_CMD_CUT);
  writer.writeAttribute(DOCUMENT_SERIALIZE_CMD_DESCRIPTION, QUndoCommand::text ());
  writer.writeAttribute(DOCUMENT_SERIALIZE_TRANSFORM_DEFINED,
                        m_transformIsDefined ? DOCUMENT_SERIALIZE_BOOL_TRUE: DOCUMENT_SERIALIZE_BOOL_FALSE);
  writer.writeAttribute(DOCUMENT_SERIALIZE_CSV, m_csv);
  writer.writeAttribute(DOCUMENT_SERIALIZE_HTML, m_html);
  m_curvesGraphsRemoved.saveXml(writer);
  baseAttributes (writer);
  writer.writeEndElement();
}
