/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#include "CmdEditPointAxis.h"
#include "Document.h"
#include "DocumentSerialize.h"
#include "EngaugeAssert.h"
#include "Logger.h"
#include "MainWindow.h"
#include <QTextStream>
#include "QtToString.h"
#include <QXmlStreamReader>
#include "Xml.h"

const QString CMD_DESCRIPTION ("Edit axis point");

CmdEditPointAxis::CmdEditPointAxis (MainWindow &mainWindow,
                                    Document &document,
                                    const QString &pointIdentifier,
                                    const QPointF &posGraphBefore,
                                    const QPointF &posGraphAfter,
                                    bool isXOnly) :
  CmdPointChangeBase (mainWindow,
                      document,
                      CMD_DESCRIPTION),
  m_pointIdentifier (pointIdentifier),
  m_posGraphBefore (posGraphBefore),
  m_posGraphAfter (posGraphAfter),
  m_isXOnly (isXOnly)
{
}

CmdEditPointAxis::CmdEditPointAxis (MainWindow &mainWindow,
                                    Document &document,
                                    const QString &cmdDescription,
                                    QXmlStreamReader &reader) :
  CmdPointChangeBase (mainWindow,
                      document,
                      cmdDescription)
{

  QXmlStreamAttributes attributes = reader.attributes();

  QStringList requiredAttributesLeaf;
  requiredAttributesLeaf << DOCUMENT_SERIALIZE_GRAPH_X_BEFORE
                         << DOCUMENT_SERIALIZE_GRAPH_Y_BEFORE
                         << DOCUMENT_SERIALIZE_GRAPH_X_AFTER
                         << DOCUMENT_SERIALIZE_GRAPH_Y_AFTER
                         << DOCUMENT_SERIALIZE_IDENTIFIER
                         << DOCUMENT_SERIALIZE_POINT_IS_X_ONLY;
  leafAndBaseAttributes (attributes,
                         requiredAttributesLeaf,
                         reader);

  // Boolean values
  QString isXOnlyValue = attributes.value(DOCUMENT_SERIALIZE_POINT_IS_X_ONLY).toString();

  m_posGraphBefore.setX(attributes.value(DOCUMENT_SERIALIZE_GRAPH_X_BEFORE).toDouble());
  m_posGraphBefore.setY(attributes.value(DOCUMENT_SERIALIZE_GRAPH_Y_BEFORE).toDouble());
  m_posGraphAfter.setX(attributes.value(DOCUMENT_SERIALIZE_GRAPH_X_AFTER).toDouble());
  m_posGraphAfter.setY(attributes.value(DOCUMENT_SERIALIZE_GRAPH_Y_AFTER).toDouble());
  m_pointIdentifier = attributes.value(DOCUMENT_SERIALIZE_IDENTIFIER).toString();
  m_isXOnly = (isXOnlyValue == DOCUMENT_SERIALIZE_BOOL_TRUE);
}

CmdEditPointAxis::~CmdEditPointAxis ()
{
}

void CmdEditPointAxis::cmdRedo ()
{

  restoreState ();
  saveOrCheckPreCommandDocumentStateHash (document ());
  saveDocumentState (document ());
  document().editPointAxis (m_posGraphAfter,
                            m_pointIdentifier);
  document().updatePointOrdinals (mainWindow().transformation());
  mainWindow().updateAfterCommand();
  saveOrCheckPostCommandDocumentStateHash (document ());
}

void CmdEditPointAxis::cmdUndo ()
{

  restoreState ();
  saveOrCheckPostCommandDocumentStateHash (document ());
  restoreDocumentState (document ());
  mainWindow().updateAfterCommand();
  saveOrCheckPreCommandDocumentStateHash (document ());
}

void CmdEditPointAxis::saveXml (QXmlStreamWriter &writer) const
{
  writer.writeStartElement(DOCUMENT_SERIALIZE_CMD);
  writer.writeAttribute(DOCUMENT_SERIALIZE_CMD_TYPE, DOCUMENT_SERIALIZE_CMD_EDIT_POINT_AXIS);
  writer.writeAttribute(DOCUMENT_SERIALIZE_CMD_DESCRIPTION, QUndoCommand::text ());
  writer.writeAttribute(DOCUMENT_SERIALIZE_IDENTIFIER, m_pointIdentifier);
  writer.writeAttribute(DOCUMENT_SERIALIZE_GRAPH_X_BEFORE, QString::number (m_posGraphBefore.x()));
  writer.writeAttribute(DOCUMENT_SERIALIZE_GRAPH_Y_BEFORE, QString::number (m_posGraphBefore.y()));
  writer.writeAttribute(DOCUMENT_SERIALIZE_GRAPH_X_AFTER, QString::number (m_posGraphAfter.x()));
  writer.writeAttribute(DOCUMENT_SERIALIZE_GRAPH_Y_AFTER, QString::number (m_posGraphAfter.y()));
  writer.writeAttribute(DOCUMENT_SERIALIZE_POINT_IS_X_ONLY, m_isXOnly ?
                          DOCUMENT_SERIALIZE_BOOL_TRUE :
                          DOCUMENT_SERIALIZE_BOOL_FALSE);
  baseAttributes (writer);
  writer.writeEndElement();
}
