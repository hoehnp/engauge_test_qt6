/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#include "CmdSettingsColorFilter.h"
#include "Document.h"
#include "DocumentSerialize.h"
#include "Logger.h"
#include "MainWindow.h"
#include <QXmlStreamReader>
#include "Xml.h"

const QString CMD_DESCRIPTION ("Filter settings");

CmdSettingsColorFilter::CmdSettingsColorFilter(MainWindow &mainWindow,
                                               Document &document,
                                               const DocumentModelColorFilter &modelColorFilterBefore,
                                               const DocumentModelColorFilter &modelColorFilterAfter) :
  CmdAbstract(mainWindow,
              document,
              CMD_DESCRIPTION),
  m_modelColorFilterBefore (modelColorFilterBefore),
  m_modelColorFilterAfter (modelColorFilterAfter)
{
}

CmdSettingsColorFilter::CmdSettingsColorFilter (MainWindow &mainWindow,
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
        (reader.name() == DOCUMENT_SERIALIZE_FILTER)) {

      if (isBefore) {

        m_modelColorFilterBefore.loadXml (reader);
        isBefore = false;

      } else {

        m_modelColorFilterAfter.loadXml (reader);

      }
    }
  }

  if (!success) {
    reader.raiseError ("Cannot read color filter settings");
  }
}

CmdSettingsColorFilter::~CmdSettingsColorFilter ()
{
}

void CmdSettingsColorFilter::cmdRedo ()
{

  restoreState ();
  saveOrCheckPreCommandDocumentStateHash (document ());
  mainWindow().updateSettingsColorFilter(m_modelColorFilterAfter);
  mainWindow().updateAfterCommand();
  saveOrCheckPostCommandDocumentStateHash (document ());
}

void CmdSettingsColorFilter::cmdUndo ()
{

  restoreState ();
  saveOrCheckPostCommandDocumentStateHash (document ());
  mainWindow().updateSettingsColorFilter(m_modelColorFilterBefore);
  mainWindow().updateAfterCommand();
  saveOrCheckPreCommandDocumentStateHash (document ());
}

void CmdSettingsColorFilter::saveXml (QXmlStreamWriter &writer) const
{
  writer.writeStartElement(DOCUMENT_SERIALIZE_CMD);
  writer.writeAttribute(DOCUMENT_SERIALIZE_CMD_TYPE, DOCUMENT_SERIALIZE_CMD_SETTINGS_COLOR_FILTER);
  writer.writeAttribute(DOCUMENT_SERIALIZE_CMD_DESCRIPTION, QUndoCommand::text ());
  baseAttributes (writer);  
  m_modelColorFilterBefore.saveXml (writer);
  m_modelColorFilterAfter.saveXml(writer);
  writer.writeEndElement();
}
