/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#include "CmdGuidelineAddXT.h"
#include "Document.h"
#include "DocumentSerialize.h"
#include "GuidelineIdentifierGenerator.h"
#include "Logger.h"
#include "MainWindow.h"
#include "Xml.h"

const QString CMD_DESCRIPTION ("GuidelineAddXT");

CmdGuidelineAddXT::CmdGuidelineAddXT(MainWindow &mainWindow,
                                     Document &document,
                                     double value) :
  CmdAbstract(mainWindow,
              document,
              CMD_DESCRIPTION),
  m_identifier (GuidelineIdentifierGenerator::next ()),
  m_value (value)
{
  LOG4CPP_INFO_S ((*mainCat)) << "CmdGuidelineAddXT::CmdGuidelineAddXT";
}

CmdGuidelineAddXT::CmdGuidelineAddXT (MainWindow &mainWindow,
                                      Document &document,
                                      const QString &cmdDescription,
                                      QXmlStreamReader &reader) :
  CmdAbstract (mainWindow,
               document,
               cmdDescription)
{
  LOG4CPP_INFO_S ((*mainCat)) << "CmdGuidelineAddXT::CmdGuidelineAddXT";

  QXmlStreamAttributes attributes = reader.attributes();

  QStringList requiredAttributesLeaf;
  requiredAttributesLeaf << DOCUMENT_SERIALIZE_IDENTIFIER
                         << DOCUMENT_SERIALIZE_GRAPH_AFTER;
  leafAndBaseAttributes (attributes,
                         requiredAttributesLeaf,
                         reader);

  m_identifier = attributes.value(DOCUMENT_SERIALIZE_IDENTIFIER).toString();
  m_value = attributes.value(DOCUMENT_SERIALIZE_GRAPH_AFTER).toDouble();
}

CmdGuidelineAddXT::~CmdGuidelineAddXT ()
{
}

void CmdGuidelineAddXT::cmdRedo ()
{
  LOG4CPP_INFO_S ((*mainCat)) << "CmdGuidelineAddXT::cmdRedo"
                              << " identifier=" << m_identifier.toLatin1().data()
                              << " value=" << m_value;

  restoreState ();
  mainWindow().guidelineAddXT (m_identifier,
                               m_value);
}

void CmdGuidelineAddXT::cmdUndo ()
{
  LOG4CPP_INFO_S ((*mainCat)) << "CmdGuidelineAddXT::cmdUndo"
                              << " identifier=" << m_identifier.toLatin1().data();

  restoreState ();
  mainWindow().guidelineRemove (m_identifier);
}

void CmdGuidelineAddXT::saveXml (QXmlStreamWriter &writer) const
{
  writer.writeStartElement(DOCUMENT_SERIALIZE_CMD);
  writer.writeAttribute(DOCUMENT_SERIALIZE_CMD_TYPE, DOCUMENT_SERIALIZE_CMD_GUIDELINE_ADD_X_T);
  writer.writeAttribute(DOCUMENT_SERIALIZE_CMD_DESCRIPTION, QUndoCommand::text ());
  writer.writeAttribute(DOCUMENT_SERIALIZE_IDENTIFIER, m_identifier);
  writer.writeAttribute(DOCUMENT_SERIALIZE_GRAPH_AFTER, QString::number (m_value));
  baseAttributes (writer);
  writer.writeEndElement();
}
