/******************************************************************************************************
 * (C) 2017 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#include "CmdAddScale.h"
#include "Document.h"
#include "DocumentSerialize.h"
#include "EngaugeAssert.h"
#include "Logger.h"
#include "MainWindow.h"
#include "QtToString.h"
#include <QXmlStreamReader>
#include "Xml.h"

const QString CMD_DESCRIPTION ("Add scale point");

CmdAddScale::CmdAddScale (MainWindow &mainWindow,
                          Document &document,
                          const QPointF &posScreen0,
                          const QPointF &posScreen1,
                          double scaleLength,
                          double ordinal0,
                          double ordinal1) :
  CmdPointChangeBase (mainWindow,
                      document,
                      CMD_DESCRIPTION),
  m_posScreen0 (posScreen0),
  m_posScreen1 (posScreen1),
  m_scaleLength (scaleLength),
  m_ordinal0 (ordinal0),
  m_ordinal1 (ordinal1)
{
}

CmdAddScale::CmdAddScale (MainWindow &mainWindow,
                          Document &document,
                          const QString &cmdDescription,
                          QXmlStreamReader &reader) :
  CmdPointChangeBase (mainWindow,
                      document,
                      cmdDescription)
{

  QXmlStreamAttributes attributes = reader.attributes();

  QStringList requiredAttributesLeaf;
  requiredAttributesLeaf << DOCUMENT_SERIALIZE_SCREEN_X
                         << DOCUMENT_SERIALIZE_SCREEN_Y
                         << DOCUMENT_SERIALIZE_SCREEN_X1
                         << DOCUMENT_SERIALIZE_SCREEN_Y1
                         << DOCUMENT_SERIALIZE_SCALE_LENGTH
                         << DOCUMENT_SERIALIZE_IDENTIFIER
                         << DOCUMENT_SERIALIZE_IDENTIFIER1
                         << DOCUMENT_SERIALIZE_ORDINAL
                         << DOCUMENT_SERIALIZE_ORDINAL1;
  leafAndBaseAttributes (attributes,
                         requiredAttributesLeaf,
                         reader);

  m_posScreen0.setX(attributes.value(DOCUMENT_SERIALIZE_SCREEN_X).toDouble());
  m_posScreen0.setY(attributes.value(DOCUMENT_SERIALIZE_SCREEN_Y).toDouble());
  m_posScreen1.setX(attributes.value(DOCUMENT_SERIALIZE_SCREEN_X1).toDouble());
  m_posScreen1.setY(attributes.value(DOCUMENT_SERIALIZE_SCREEN_Y1).toDouble());
  m_scaleLength = attributes.value(DOCUMENT_SERIALIZE_SCALE_LENGTH).toDouble();
  m_identifierAdded0 = attributes.value(DOCUMENT_SERIALIZE_IDENTIFIER).toString();
  m_identifierAdded1 = attributes.value(DOCUMENT_SERIALIZE_IDENTIFIER1).toString();
  m_ordinal0 = attributes.value(DOCUMENT_SERIALIZE_ORDINAL).toDouble();
  m_ordinal1 = attributes.value(DOCUMENT_SERIALIZE_ORDINAL1).toDouble();
}

CmdAddScale::~CmdAddScale ()
{
}

void CmdAddScale::cmdRedo ()
{

  restoreState ();
  saveOrCheckPreCommandDocumentStateHash (document ());
  saveDocumentState (document ());
  document().addScaleWithGeneratedIdentifier (m_posScreen0,
                                              m_posScreen1,
                                              m_scaleLength,
                                              m_identifierAdded0,
                                              m_identifierAdded1,
                                              m_ordinal0,
                                              m_ordinal1);
  document().updatePointOrdinals (mainWindow().transformation());
  mainWindow().updateAfterCommand();
  saveOrCheckPostCommandDocumentStateHash (document ());
}

void CmdAddScale::cmdUndo ()
{

  restoreState ();
  saveOrCheckPostCommandDocumentStateHash (document ());
  restoreDocumentState (document ());
  mainWindow().updateAfterCommand();
  saveOrCheckPreCommandDocumentStateHash (document ());
}

void CmdAddScale::saveXml (QXmlStreamWriter &writer) const
{
  // For time coordinates, many digits of precision are needed since a typical date is 1,246,870,000 = July 6, 2009
  // and we want seconds of precision
  const char FORMAT = 'g';
  const int PRECISION = 16;

  writer.writeStartElement(DOCUMENT_SERIALIZE_CMD);
  writer.writeAttribute(DOCUMENT_SERIALIZE_CMD_TYPE, DOCUMENT_SERIALIZE_CMD_ADD_SCALE);
  writer.writeAttribute(DOCUMENT_SERIALIZE_CMD_DESCRIPTION, QUndoCommand::text ());
  writer.writeAttribute(DOCUMENT_SERIALIZE_SCREEN_X, QString::number (m_posScreen0.x()));
  writer.writeAttribute(DOCUMENT_SERIALIZE_SCREEN_Y, QString::number (m_posScreen0.y()));
  writer.writeAttribute(DOCUMENT_SERIALIZE_SCREEN_X1, QString::number (m_posScreen1.x()));
  writer.writeAttribute(DOCUMENT_SERIALIZE_SCREEN_Y1, QString::number (m_posScreen1.y()));
  writer.writeAttribute(DOCUMENT_SERIALIZE_SCALE_LENGTH, QString::number (m_scaleLength, FORMAT, PRECISION));
  writer.writeAttribute(DOCUMENT_SERIALIZE_IDENTIFIER, m_identifierAdded0);
  writer.writeAttribute(DOCUMENT_SERIALIZE_IDENTIFIER1, m_identifierAdded1);
  writer.writeAttribute(DOCUMENT_SERIALIZE_ORDINAL, QString::number (m_ordinal0));
  writer.writeAttribute(DOCUMENT_SERIALIZE_ORDINAL1, QString::number (m_ordinal1));
  baseAttributes (writer);
  writer.writeEndElement();
}
