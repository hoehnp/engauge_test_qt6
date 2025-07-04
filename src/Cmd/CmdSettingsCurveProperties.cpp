/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#include "CmdSettingsCurveProperties.h"
#include "Document.h"
#include "DocumentSerialize.h"
#include "Logger.h"
#include "MainWindow.h"
#include <QDebug>
#include <QXmlStreamReader>
#include "Xml.h"

const QString CMD_DESCRIPTION ("Curve Properties settings");

CmdSettingsCurveProperties::CmdSettingsCurveProperties(MainWindow &mainWindow,
                                                       Document &document,
                                                       const CurveStyles &modelCurveStylesBefore,
                                                       const CurveStyles &modelCurveStylesAfter) :
  CmdAbstract(mainWindow,
              document,
              CMD_DESCRIPTION),
  m_modelCurveStylesBefore (modelCurveStylesBefore),
  m_modelCurveStylesAfter (modelCurveStylesAfter)
{
}

CmdSettingsCurveProperties::CmdSettingsCurveProperties (MainWindow &mainWindow,
                                                        Document &document,
                                                        const QString &cmdDescription,
                                                        QXmlStreamReader &reader) :
  CmdAbstract (mainWindow,
               document,
               cmdDescription)
{
  
  bool success = true;

  // Read until end of this subtree
  bool isBefore = true;
  while ((reader.tokenType() != QXmlStreamReader::EndElement) ||
  (reader.name() != DOCUMENT_SERIALIZE_CMD)){
    loadNextFromReader(reader);
    if (reader.atEnd()) {
      xmlExitWithError (reader,
                        QString ("%1 %2")
                        .arg (QObject::tr ("Reached end of file before finding end element for"))
                        .arg (DOCUMENT_SERIALIZE_CMD));
      success = false;
      break;
    }

    if ((reader.tokenType() == QXmlStreamReader::StartElement) &&
        (reader.name() == DOCUMENT_SERIALIZE_CURVE_STYLES)) {

      if (isBefore) {

        m_modelCurveStylesBefore.loadXml (reader);
        isBefore = false;

      } else {

        m_modelCurveStylesAfter.loadXml (reader);

      }
    }
  }

  if (!success) {
    reader.raiseError ("Cannot read curve properties settings");
  }
}

CmdSettingsCurveProperties::~CmdSettingsCurveProperties ()
{
}

void CmdSettingsCurveProperties::cmdRedo ()
{

  restoreState ();
  saveOrCheckPreCommandDocumentStateHash (document ());
  document().updatePointOrdinals (mainWindow().transformation());
  mainWindow().updateSettingsCurveStyles(m_modelCurveStylesAfter);
  mainWindow().updateAfterCommand();
  saveOrCheckPostCommandDocumentStateHash (document ());
}

void CmdSettingsCurveProperties::cmdUndo ()
{

  restoreState ();
  saveOrCheckPostCommandDocumentStateHash (document ());
  document().updatePointOrdinals (mainWindow().transformation());
  mainWindow().updateSettingsCurveStyles(m_modelCurveStylesBefore);
  mainWindow().updateAfterCommand();
  saveOrCheckPreCommandDocumentStateHash (document ());
}

void CmdSettingsCurveProperties::saveXml (QXmlStreamWriter &writer) const
{
  writer.writeStartElement(DOCUMENT_SERIALIZE_CMD);
  writer.writeAttribute(DOCUMENT_SERIALIZE_CMD_TYPE, DOCUMENT_SERIALIZE_CMD_SETTINGS_CURVE_PROPERTIES);
  writer.writeAttribute(DOCUMENT_SERIALIZE_CMD_DESCRIPTION, QUndoCommand::text ());
  baseAttributes (writer);
  m_modelCurveStylesBefore.saveXml(writer);
  m_modelCurveStylesAfter.saveXml(writer);
  writer.writeEndElement();
}
