/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#include "CmdMediator.h"
#include "DocumentModelGuideline.h"
#include "DocumentSerialize.h"
#include "EngaugeAssert.h"
#include "Logger.h"
#include <QObject>
#include <QTextStream>
#include "QtToString.h"
#include <QXmlStreamWriter>
#include "Xml.h"

const double DEFAULT_CREATION_CIRCLE_RADIUS = 20;
const ColorPalette DEFAULT_LINE_COLOR (COLOR_PALETTE_MAGENTA); // Should be bright so it gets noticed
const double DEFAULT_LINE_WIDTH_ACTIVE = 2; // Wider to grab attention
const double DEFAULT_LINE_WIDTH_INACTIVE = 1; // Narrower for better accuracy

DocumentModelGuideline::DocumentModelGuideline() :
  m_creationCircleRadius (DEFAULT_CREATION_CIRCLE_RADIUS),
  m_lineColor (DEFAULT_LINE_COLOR),
  m_lineWidthActive (DEFAULT_LINE_WIDTH_ACTIVE),
  m_lineWidthInactive (DEFAULT_LINE_WIDTH_INACTIVE)
{
}

DocumentModelGuideline::DocumentModelGuideline(const Document &document) :
  m_valuesX (document.modelGuideline().valuesX ()),
  m_valuesY (document.modelGuideline().valuesY ()),
  m_creationCircleRadius (document.modelGuideline().creationCircleRadius ()),
  m_lineColor (document.modelGuideline().lineColor ()),
  m_lineWidthActive (document.modelGuideline().lineWidthActive ()),
  m_lineWidthInactive (document.modelGuideline().lineWidthInactive ())  
{
}

DocumentModelGuideline::DocumentModelGuideline(const DocumentModelGuideline &other) :
  m_valuesX (other.valuesX ()),
  m_valuesY (other.valuesY ()),
  m_creationCircleRadius (other.creationCircleRadius ()),
  m_lineColor (other.lineColor ()),
  m_lineWidthActive (other.lineWidthActive ()),
  m_lineWidthInactive (other.lineWidthInactive ())  
{
}

DocumentModelGuideline &DocumentModelGuideline::operator=(const DocumentModelGuideline &other)
{
  m_valuesX = other.valuesX ();
  m_valuesY = other.valuesY ();
  m_creationCircleRadius = other.creationCircleRadius ();
  m_lineColor = other.lineColor ();
  m_lineWidthActive = other.lineWidthActive ();
  m_lineWidthInactive = other.lineWidthInactive ();  
    
  return *this;
}

double DocumentModelGuideline::creationCircleRadius () const
{
  return m_creationCircleRadius;
}

ColorPalette DocumentModelGuideline::lineColor () const
{
  return m_lineColor;
}

double DocumentModelGuideline::lineWidthActive () const
{
  return m_lineWidthActive;
}

double DocumentModelGuideline::lineWidthInactive () const
{
  return m_lineWidthInactive;
}

void DocumentModelGuideline::loadXml(QXmlStreamReader &reader)
{
  LOG4CPP_INFO_S ((*mainCat)) << "DocumentModelGuideline::loadXml";

  bool success = true;

  QXmlStreamAttributes attributes = reader.attributes();

  if (attributes.hasAttribute (DOCUMENT_SERIALIZE_GUIDELINE_CREATION_CIRCLE_RADIUS) &&
      attributes.hasAttribute (DOCUMENT_SERIALIZE_GUIDELINE_LINE_COLOR) &&
      attributes.hasAttribute (DOCUMENT_SERIALIZE_GUIDELINE_LINE_WIDTH_ACTIVE) &&
      attributes.hasAttribute (DOCUMENT_SERIALIZE_GUIDELINE_LINE_WIDTH_INACTIVE)) {

    setCreationCircleRadius (attributes.value (DOCUMENT_SERIALIZE_GUIDELINE_CREATION_CIRCLE_RADIUS).toInt ());
    setLineColor (static_cast<ColorPalette> (attributes.value (DOCUMENT_SERIALIZE_GUIDELINE_LINE_COLOR).toInt ()));
    setLineWidthActive (attributes.value (DOCUMENT_SERIALIZE_GUIDELINE_LINE_WIDTH_ACTIVE).toInt ());
    setLineWidthInactive (attributes.value (DOCUMENT_SERIALIZE_GUIDELINE_LINE_WIDTH_INACTIVE).toInt ());    
  }

  // Read until end of this subtree
  while ((reader.tokenType() != QXmlStreamReader::EndElement) ||
  (reader.name() != DOCUMENT_SERIALIZE_GUIDELINES)){

    QXmlStreamReader::TokenType tokenType = loadNextFromReader(reader);

    if ((tokenType == QXmlStreamReader::StartElement) &&
        (reader.name() == DOCUMENT_SERIALIZE_GUIDELINES_X)) {

      loadXmlVector (reader,
                     DOCUMENT_SERIALIZE_GUIDELINES_X,
                     m_valuesX);
    }

    if ((tokenType == QXmlStreamReader::StartElement) &&
        (reader.name() == DOCUMENT_SERIALIZE_GUIDELINES_Y)) {

      loadXmlVector (reader,
                     DOCUMENT_SERIALIZE_GUIDELINES_Y,
                     m_valuesY);
    }

    if (reader.atEnd()) {
      success = false;
      break;
    }
  }

  if (!success) {
    reader.raiseError (QObject::tr ("Cannot read grid display data"));
  }
}

void DocumentModelGuideline::loadXmlVector (QXmlStreamReader &reader,
                                             const QString &tokenEnd,
                                             GuidelineValues &guidelineValues) const
{
  LOG4CPP_INFO_S ((*mainCat)) << "DocumentModelGuideline::loadXmlVector";

  while ((reader.tokenType() != QXmlStreamReader::EndElement) ||
  (reader.name() != tokenEnd)){

    QXmlStreamReader::TokenType tokenType = loadNextFromReader(reader);

    if (reader.atEnd()) {
      break;
    }

    if (tokenType == QXmlStreamReader::StartElement) {

      if (reader.attributes().hasAttribute (DOCUMENT_SERIALIZE_GUIDELINE_IDENTIFIER) &&
          reader.attributes().hasAttribute (DOCUMENT_SERIALIZE_GUIDELINE_VALUE)) {

        QString identifier = reader.attributes ().value (DOCUMENT_SERIALIZE_GUIDELINE_IDENTIFIER).toString ();
        double value = reader.attributes ().value (DOCUMENT_SERIALIZE_GUIDELINE_VALUE).toDouble ();

        guidelineValues [identifier] = value;
      }
    }
  }
}

void DocumentModelGuideline::printStream(QString indentation,
                                           QTextStream &str) const
{
  str << indentation << "DocumentModelGuideline\n";

  indentation += INDENTATION_DELTA;

  QString valuesX, valuesY, delimiterX, delimiterY;
  QTextStream strX (&valuesX), strY (&valuesY);
  GuidelineValues::const_iterator itr;

  for (itr = m_valuesX.constBegin(); itr != m_valuesX.constEnd(); itr++) {
    strX << delimiterX << itr.value();
    delimiterX = ", ";
  }

  for (itr = m_valuesY.constBegin(); itr != m_valuesY.constEnd(); itr++) {
    strY << delimiterY << itr.value();
    delimiterY = ", ";
  }

  str << indentation << "valuesX=" << valuesX << "\n";
  str << indentation << "valuesY=" << valuesY << "\n";
  str << indentation << "creationCircleRadius=" << m_creationCircleRadius << "\n";
  str << indentation << "lineColor=" << colorPaletteToString (m_lineColor) << "\n";
  str << indentation << "lineWidthActive=" << m_lineWidthActive << "\n";
  str << indentation << "lineWidthInactive=" << m_lineWidthInactive << "\n";  
}

void DocumentModelGuideline::saveXml(QXmlStreamWriter &writer) const
{
  LOG4CPP_INFO_S ((*mainCat)) << "DocumentModelGuideline::saveXml";

  writer.writeStartElement(DOCUMENT_SERIALIZE_GUIDELINES);
  writer.writeAttribute(DOCUMENT_SERIALIZE_GUIDELINE_CREATION_CIRCLE_RADIUS, QString::number (m_creationCircleRadius));
  writer.writeAttribute(DOCUMENT_SERIALIZE_GUIDELINE_LINE_COLOR, QString::number (m_lineColor));
  writer.writeAttribute(DOCUMENT_SERIALIZE_GUIDELINE_LINE_COLOR_STRING, colorPaletteToString (m_lineColor));
  writer.writeAttribute(DOCUMENT_SERIALIZE_GUIDELINE_LINE_WIDTH_ACTIVE, QString::number (m_lineWidthActive));
  writer.writeAttribute(DOCUMENT_SERIALIZE_GUIDELINE_LINE_WIDTH_INACTIVE, QString::number (m_lineWidthInactive));  
  saveXmlVector (writer,
                 DOCUMENT_SERIALIZE_GUIDELINES_X,
                 m_valuesX);
  saveXmlVector (writer,
                 DOCUMENT_SERIALIZE_GUIDELINES_Y,
                 m_valuesY);
  writer.writeEndElement();
}
 
void DocumentModelGuideline::saveXmlVector(QXmlStreamWriter &writer,
                                            const QString &tokenAll,
                                            const GuidelineValues &values) const
{
  writer.writeStartElement(tokenAll);

  // Loop through values
  GuidelineValues::const_iterator itr;
  for (itr = values.begin(); itr != values.end(); itr++) {
    QString identifier = itr.key();
    double value = itr.value();
    writer.writeStartElement (DOCUMENT_SERIALIZE_GUIDELINE);
    writer.writeAttribute (DOCUMENT_SERIALIZE_GUIDELINE_IDENTIFIER, identifier);
    writer.writeAttribute (DOCUMENT_SERIALIZE_GUIDELINE_VALUE, QString::number (value));
    writer.writeEndElement ();
  }

  writer.writeEndElement();
}

void DocumentModelGuideline::setCreationCircleRadius (double radius)
{
  m_creationCircleRadius = radius;
}

void DocumentModelGuideline::setLineColor (ColorPalette lineColor)
{
  m_lineColor = lineColor;
}

void DocumentModelGuideline::setLineWidthActive (double lineWidthActive)
{
  m_lineWidthActive = lineWidthActive;
}

void DocumentModelGuideline::setLineWidthInactive (double lineWidthInactive)
{
  m_lineWidthInactive = lineWidthInactive;
}

void DocumentModelGuideline::setValuesX (const GuidelineValues &valuesX)
{
  m_valuesX = valuesX;
}

void DocumentModelGuideline::setValuesY (const GuidelineValues &valuesY)
{
  m_valuesY = valuesY;
}

GuidelineValues DocumentModelGuideline::valuesX () const
{
  return m_valuesX;
}

GuidelineValues DocumentModelGuideline::valuesY () const
{
  return m_valuesY;
}
