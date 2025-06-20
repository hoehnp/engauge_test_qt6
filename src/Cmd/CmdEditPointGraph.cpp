/******************************************************************************************************
 * (C) 2016 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#include "CmdEditPointGraph.h"
#include "Document.h"
#include "DocumentSerialize.h"
#include "EngaugeAssert.h"
#include "Logger.h"
#include "MainWindow.h"
#include <QTextStream>
#include "QtToString.h"
#include <QXmlStreamReader>
#include "Xml.h"

const QString CMD_DESCRIPTION ("Edit curve points");

CmdEditPointGraph::CmdEditPointGraph (MainWindow &mainWindow,
                                      Document &document,
                                      const QStringList &pointIdentifiers,
                                      bool isX,
                                      bool isY,
                                      double x,
                                      double y) :
  CmdPointChangeBase (mainWindow,
                      document,
                      CMD_DESCRIPTION),
  m_pointIdentifiers (pointIdentifiers),
  m_isX (isX),
  m_isY (isY),
  m_x (x),
  m_y (y)
{
}

CmdEditPointGraph::CmdEditPointGraph (MainWindow &mainWindow,
                                      Document &document,
                                      const QString &cmdDescription,
                                      QXmlStreamReader &reader) :
  CmdPointChangeBase (mainWindow,
                      document,
                      cmdDescription)
{

  QXmlStreamAttributes attributes = reader.attributes();

  QStringList requiredAttributesLeaf;
  requiredAttributesLeaf << DOCUMENT_SERIALIZE_EDIT_GRAPH_IS_X
                         << DOCUMENT_SERIALIZE_EDIT_GRAPH_IS_Y
                         << DOCUMENT_SERIALIZE_EDIT_GRAPH_X
                         << DOCUMENT_SERIALIZE_EDIT_GRAPH_Y;
  leafAndBaseAttributes (attributes,
                         requiredAttributesLeaf,
                         reader);

  // Boolean attributes
  QString isX = attributes.value(DOCUMENT_SERIALIZE_EDIT_GRAPH_IS_X).toString();
  QString isY = attributes.value(DOCUMENT_SERIALIZE_EDIT_GRAPH_IS_Y).toString();

  m_isX = (isX == DOCUMENT_SERIALIZE_BOOL_TRUE);
  m_isY = (isY == DOCUMENT_SERIALIZE_BOOL_TRUE);
  m_x = attributes.value(DOCUMENT_SERIALIZE_EDIT_GRAPH_X).toDouble();
  m_y = attributes.value(DOCUMENT_SERIALIZE_EDIT_GRAPH_Y).toDouble();

  bool success = true;
  while (loadNextFromReader (reader)) {

    if (reader.atEnd() || reader.hasError ()) {
      success = false;
      break;
    }

    if ((reader.tokenType() == QXmlStreamReader::EndElement) &
        (reader.name() == DOCUMENT_SERIALIZE_CMD)) {
      break;
    }

    // Not done yet
    if ((reader.tokenType() == QXmlStreamReader::StartElement) &&
        (reader.name() == DOCUMENT_SERIALIZE_POINT)) {

      // This is an entry that we need to add
      QXmlStreamAttributes attributes = reader.attributes ();

      if (attributes.hasAttribute(DOCUMENT_SERIALIZE_IDENTIFIER)) {

        m_pointIdentifiers << attributes.value(DOCUMENT_SERIALIZE_IDENTIFIER).toString();
      }
    }

    if (!success) {
      reader.raiseError (QObject::tr ("Cannot read graph points"));
    }
  }
}

CmdEditPointGraph::~CmdEditPointGraph ()
{
}

void CmdEditPointGraph::cmdRedo ()
{

  restoreState ();
  saveOrCheckPreCommandDocumentStateHash (document ());
  saveDocumentState (document ());
  document().editPointGraph (m_isX,
                             m_isY,
                             m_x,
                             m_y,
                             m_pointIdentifiers,
                             mainWindow().transformation());
  document().updatePointOrdinals (mainWindow().transformation());
  mainWindow().updateAfterCommand();
  saveOrCheckPostCommandDocumentStateHash (document ());
}

void CmdEditPointGraph::cmdUndo ()
{

  restoreState ();
  saveOrCheckPostCommandDocumentStateHash (document ());
  restoreDocumentState (document ());
  mainWindow().updateAfterCommand();
  saveOrCheckPreCommandDocumentStateHash (document ());
}

void CmdEditPointGraph::saveXml (QXmlStreamWriter &writer) const
{
  writer.writeStartElement(DOCUMENT_SERIALIZE_CMD);
  writer.writeAttribute(DOCUMENT_SERIALIZE_CMD_TYPE, DOCUMENT_SERIALIZE_CMD_EDIT_POINT_GRAPH);
  writer.writeAttribute(DOCUMENT_SERIALIZE_CMD_DESCRIPTION, QUndoCommand::text ());
  writer.writeAttribute(DOCUMENT_SERIALIZE_EDIT_GRAPH_IS_X, m_isX ?
                          DOCUMENT_SERIALIZE_BOOL_TRUE :
                          DOCUMENT_SERIALIZE_BOOL_FALSE);
  writer.writeAttribute(DOCUMENT_SERIALIZE_EDIT_GRAPH_IS_Y, m_isY ?
                          DOCUMENT_SERIALIZE_BOOL_TRUE :
                          DOCUMENT_SERIALIZE_BOOL_FALSE);
  writer.writeAttribute(DOCUMENT_SERIALIZE_EDIT_GRAPH_X, QString::number (m_x));
  writer.writeAttribute(DOCUMENT_SERIALIZE_EDIT_GRAPH_Y, QString::number (m_y));

  for (int index = 0; index < m_pointIdentifiers.count(); index++) {

    writer.writeStartElement (DOCUMENT_SERIALIZE_POINT);
    writer.writeAttribute(DOCUMENT_SERIALIZE_IDENTIFIER, m_pointIdentifiers.at (index));
    writer.writeEndElement();
  }
  baseAttributes (writer);
  writer.writeEndElement();
}
