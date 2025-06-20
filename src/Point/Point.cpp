/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#include "DocumentSerialize.h"
#include "EngaugeAssert.h"
#include "Logger.h"
#include "Point.h"
#include <QObject>
#include <QStringList>
#include <QTextStream>
#include "QtToString.h"
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include "Xml.h"

unsigned int Point::m_identifierIndex = 0;

extern const QString AXIS_CURVE_NAME;
extern const QString DUMMY_CURVE_NAME;
const QString POINT_IDENTIFIER_DELIMITER_SAFE ("\t"); // Character that could never be entered when editing curve names
const QString POINT_IDENTIFIER_DELIMITER_XML ("_"); // From incoming xml that does not like tabs

const double MISSING_ORDINAL_VALUE = 0;
const double MISSING_POSGRAPH_VALUE = 0;

Point::Point ()
{
}

Point::Point(const QString &curveName,
             const QPointF &posScreen) :
  m_isAxisPoint (curveName == AXIS_CURVE_NAME),
  m_identifier (uniqueIdentifierGenerator(curveName)),
  m_posScreen (posScreen),
  m_hasPosGraph (false),
  m_posGraph (MISSING_POSGRAPH_VALUE, MISSING_POSGRAPH_VALUE),
  m_hasOrdinal (false),
  m_ordinal (MISSING_ORDINAL_VALUE),
  m_isXOnly (false)
{
  ENGAUGE_ASSERT (!curveName.isEmpty ());
}

Point::Point(const QString &curveName,
             const QPointF &posScreen,
             const QPointF &posGraph,
             bool isXOnly) :
  m_isAxisPoint (true),
  m_identifier (uniqueIdentifierGenerator(curveName)),
  m_posScreen (posScreen),
  m_hasPosGraph (true),
  m_posGraph (posGraph),
  m_hasOrdinal (false),
  m_ordinal (MISSING_ORDINAL_VALUE),
  m_isXOnly (isXOnly)
{
  ENGAUGE_ASSERT (curveName == AXIS_CURVE_NAME ||
                  curveName == DUMMY_CURVE_NAME);

  ENGAUGE_ASSERT (!curveName.isEmpty ());
}

Point::Point(const QString &curveName,
             const QString &identifier,
             const QPointF &posScreen,
             const QPointF &posGraph,
             double ordinal,
             bool isXOnly) :
  m_isAxisPoint (true),
  m_identifier (identifier),
  m_posScreen (posScreen),
  m_hasPosGraph (true),
  m_posGraph (posGraph),
  m_hasOrdinal (true),
  m_ordinal (ordinal),
  m_isXOnly (isXOnly)
{
  ENGAUGE_ASSERT (curveName == AXIS_CURVE_NAME);

  ENGAUGE_ASSERT (!curveName.isEmpty ());
}

Point::Point(const QString &curveName,
             const QPointF &posScreen,
             const QPointF &posGraph,
             double ordinal,
             bool isXOnly) :
  m_isAxisPoint (true),
  m_identifier (uniqueIdentifierGenerator(curveName)),
  m_posScreen (posScreen),
  m_hasPosGraph (true),
  m_posGraph (posGraph),
  m_hasOrdinal (true),
  m_ordinal (ordinal),
  m_isXOnly (isXOnly)
{
  ENGAUGE_ASSERT (curveName == AXIS_CURVE_NAME);

  ENGAUGE_ASSERT (!curveName.isEmpty ());
}

Point::Point(const QString &curveName,
             const QString &identifier,
             const QPointF &posScreen,
             double ordinal) :
  m_isAxisPoint (false),
  m_identifier (identifier),
  m_posScreen (posScreen),
  m_hasPosGraph (false),
  m_posGraph (MISSING_POSGRAPH_VALUE, MISSING_POSGRAPH_VALUE),
  m_hasOrdinal (true),
  m_ordinal (ordinal),
  m_isXOnly (false)
{
  ENGAUGE_ASSERT (curveName != AXIS_CURVE_NAME);

  ENGAUGE_ASSERT (!curveName.isEmpty ());
}

Point::Point (const QString &curveName,
              const QPointF &posScreen,
              double ordinal) :
  m_isAxisPoint (false),
  m_identifier (uniqueIdentifierGenerator(curveName)),
  m_posScreen (posScreen),
  m_hasPosGraph (false),
  m_posGraph (MISSING_POSGRAPH_VALUE, MISSING_POSGRAPH_VALUE),
  m_hasOrdinal (true),
  m_ordinal (ordinal),
  m_isXOnly (false)
{
  ENGAUGE_ASSERT (curveName != AXIS_CURVE_NAME);

}

Point::Point (QXmlStreamReader &reader)
{
  loadXml(reader);
}

Point::Point (const Point &other)
{
  m_isAxisPoint = other.isAxisPoint ();
  m_identifier = other.identifier ();
  m_posScreen = other.posScreen ();
  m_hasPosGraph = other.hasPosGraph ();
  m_posGraph = other.posGraph (SKIP_HAS_CHECK);
  m_hasOrdinal = other.hasOrdinal ();
  m_ordinal = other.ordinal (SKIP_HAS_CHECK);
  m_isXOnly = other.isXOnly ();
}

Point &Point::operator=(const Point &point)
{
  m_isAxisPoint = point.isAxisPoint ();
  m_identifier = point.identifier ();
  m_posScreen = point.posScreen ();
  m_hasPosGraph = point.hasPosGraph ();
  m_posGraph = point.posGraph (SKIP_HAS_CHECK);
  m_hasOrdinal = point.hasOrdinal ();
  m_ordinal = point.ordinal (SKIP_HAS_CHECK);
  m_isXOnly = point.isXOnly ();

  return *this;
}

QString Point::curveNameFromPointIdentifier (const QString &pointIdentifier)
{
  QStringList tokens;

  if (pointIdentifier.contains (POINT_IDENTIFIER_DELIMITER_SAFE)) {

    tokens = pointIdentifier.split (POINT_IDENTIFIER_DELIMITER_SAFE);

  } else {

    // Yes, this is a hack - underscores could have been inserted by user (in the curve name) and/or this source code,
    // but there are many dig files laying around that have underscores so we need to support them
    tokens = pointIdentifier.split (POINT_IDENTIFIER_DELIMITER_XML);

  }

  return tokens.value (0);
}

QString Point::fixUnderscores (const QString &identifier) const
{
  QString rtn = identifier;

  if (!identifier.contains (POINT_IDENTIFIER_DELIMITER_SAFE)) {
    QString mutableIdentifier = identifier;
    rtn = mutableIdentifier.replace ("_", POINT_IDENTIFIER_DELIMITER_SAFE);
  }

  return rtn;
}

bool Point::hasOrdinal () const
{
  return m_hasOrdinal;
}

bool Point::hasPosGraph () const
{
  return m_hasPosGraph;
}

QString Point::identifier() const
{
  return m_identifier;
}

unsigned int Point::identifierIndex ()
{

  return m_identifierIndex;
}

bool Point::isAxisPoint() const
{
  return m_isAxisPoint;
}

bool Point::isXOnly() const
{
  return m_isXOnly;
}

void Point::loadXml(QXmlStreamReader &reader)
{

  bool success = true;

  QXmlStreamAttributes attributes = reader.attributes();

  // Note that DOCUMENT_SERIALIZE_POINT_IS_X_ONLY is optional since it is not used in Version 6
  // but is used in Version 7
  if (attributes.hasAttribute(DOCUMENT_SERIALIZE_POINT_IDENTIFIER) &&
      attributes.hasAttribute(DOCUMENT_SERIALIZE_POINT_IDENTIFIER_INDEX) &&
      attributes.hasAttribute(DOCUMENT_SERIALIZE_POINT_IS_AXIS_POINT)) {

    m_hasOrdinal = attributes.hasAttribute(DOCUMENT_SERIALIZE_POINT_ORDINAL);
    if (m_hasOrdinal) {
      m_ordinal = attributes.value(DOCUMENT_SERIALIZE_POINT_ORDINAL).toDouble();
    } else {
      m_ordinal = MISSING_ORDINAL_VALUE;
    }

    QString isAxisPoint = attributes.value(DOCUMENT_SERIALIZE_POINT_IS_AXIS_POINT).toString();
    QString isXOnly; // Default is anything but DOCUMENT_SERIALIZE_BOOL_TRUE
    if (attributes.hasAttribute (DOCUMENT_SERIALIZE_POINT_IS_X_ONLY)) {
      isXOnly = attributes.value(DOCUMENT_SERIALIZE_POINT_IS_X_ONLY).toString();
    }

    m_identifier = fixUnderscores (attributes.value(DOCUMENT_SERIALIZE_POINT_IDENTIFIER).toString());
    m_identifierIndex = attributes.value(DOCUMENT_SERIALIZE_POINT_IDENTIFIER_INDEX).toUInt();
    m_isAxisPoint = (isAxisPoint == DOCUMENT_SERIALIZE_BOOL_TRUE);
    m_hasPosGraph = false;
    m_posGraph.setX (MISSING_POSGRAPH_VALUE);
    m_posGraph.setY (MISSING_POSGRAPH_VALUE);
    m_isXOnly = (isXOnly == DOCUMENT_SERIALIZE_BOOL_TRUE);

    while ((reader.tokenType() != QXmlStreamReader::EndElement) ||
           (reader.name () != DOCUMENT_SERIALIZE_POINT)) {

      loadNextFromReader(reader);
      if (reader.atEnd()) {
        success = false;
        break;
      }

      if (reader.tokenType () == QXmlStreamReader::StartElement) {

        if (reader.name() == DOCUMENT_SERIALIZE_POINT_POSITION_SCREEN) {

          attributes = reader.attributes();

          if (attributes.hasAttribute(DOCUMENT_SERIALIZE_POINT_X) &&
              attributes.hasAttribute(DOCUMENT_SERIALIZE_POINT_Y)) {

            m_posScreen.setX (attributes.value(DOCUMENT_SERIALIZE_POINT_X).toDouble());
            m_posScreen.setY (attributes.value(DOCUMENT_SERIALIZE_POINT_Y).toDouble());

          } else {
            success = false;
            break;
          }
        } else if (reader.name() == DOCUMENT_SERIALIZE_POINT_POSITION_GRAPH) {

          m_hasPosGraph = true;
          attributes = reader.attributes();

          if (attributes.hasAttribute(DOCUMENT_SERIALIZE_POINT_X) &&
              attributes.hasAttribute(DOCUMENT_SERIALIZE_POINT_Y)) {

            m_posGraph.setX (attributes.value(DOCUMENT_SERIALIZE_POINT_X).toDouble());
            m_posGraph.setY (attributes.value(DOCUMENT_SERIALIZE_POINT_Y).toDouble());

          } else {
            success = false;
            break;
          }
        }
      }
    }

  } else {
    success = false;
  }

  if (!success) {
    reader.raiseError(QObject::tr ("Cannot read point data"));
  }
}

double Point::ordinal (ApplyHasCheck applyHasCheck) const
{
  if (applyHasCheck == KEEP_HAS_CHECK) {
    ENGAUGE_ASSERT (m_hasOrdinal);
  }

  return m_ordinal;
}

QPointF Point::posGraph (ApplyHasCheck applyHasCheck) const
{
  if (applyHasCheck == KEEP_HAS_CHECK) {
    ENGAUGE_ASSERT (m_hasPosGraph);
  }

  return m_posGraph;
}

QPointF Point::posScreen () const
{
  return m_posScreen;
}

void Point::printStream(QString indentation,
                        QTextStream &str) const
{
  const QString UNDEFINED ("undefined");

  str << indentation << "Point\n";

  indentation += INDENTATION_DELTA;

  str << indentation << "identifier=" << m_identifier << "\n";
  str << indentation << "posScreen=" << QPointFToString (m_posScreen) << "\n";
  if (m_hasPosGraph) {
    str << indentation << "posGraph=" << QPointFToString (m_posGraph) << "\n";
  } else {
    str << indentation << "posGraph=" << UNDEFINED << "\n";
  }
  if (m_hasOrdinal) {
    str << indentation << "ordinal=" << m_ordinal << "\n";
  } else {
    str << indentation << "ordinal=" << UNDEFINED << "\n";
  }
}

void Point::saveXml(QXmlStreamWriter &writer) const
{

  writer.writeStartElement(DOCUMENT_SERIALIZE_POINT);
  writer.writeAttribute(DOCUMENT_SERIALIZE_POINT_IDENTIFIER, m_identifier);
  if (m_hasOrdinal) {
    writer.writeAttribute(DOCUMENT_SERIALIZE_POINT_ORDINAL, QString::number (m_ordinal));
  }
  writer.writeAttribute(DOCUMENT_SERIALIZE_POINT_IS_AXIS_POINT,
                        m_isAxisPoint ? DOCUMENT_SERIALIZE_BOOL_TRUE : DOCUMENT_SERIALIZE_BOOL_FALSE);
  writer.writeAttribute(DOCUMENT_SERIALIZE_POINT_IS_X_ONLY,
                        m_isXOnly ? DOCUMENT_SERIALIZE_BOOL_TRUE : DOCUMENT_SERIALIZE_BOOL_FALSE);

  // Variable m_identifierIndex is static, but for simplicity this is handled like other values. Those values are all
  // the same, but simplicity wins over a few extra bytes of storage
  writer.writeAttribute(DOCUMENT_SERIALIZE_POINT_IDENTIFIER_INDEX, QString::number (m_identifierIndex));

  writer.writeStartElement(DOCUMENT_SERIALIZE_POINT_POSITION_SCREEN);
  writer.writeAttribute(DOCUMENT_SERIALIZE_POINT_X, QString::number (m_posScreen.x()));
  writer.writeAttribute(DOCUMENT_SERIALIZE_POINT_Y, QString::number (m_posScreen.y()));
  writer.writeEndElement();

  if (m_hasPosGraph) {

    // For time coordinates, many digits of precision are needed since a typical date is 1,246,870,000 = July 6, 2009
    // and we want seconds of precision
    const char FORMAT = 'g';
    const int PRECISION = 16;

    writer.writeStartElement(DOCUMENT_SERIALIZE_POINT_POSITION_GRAPH);
    writer.writeAttribute(DOCUMENT_SERIALIZE_POINT_X, QString::number (m_posGraph.x(), FORMAT, PRECISION));
    writer.writeAttribute(DOCUMENT_SERIALIZE_POINT_Y, QString::number (m_posGraph.y(), FORMAT, PRECISION));
    writer.writeEndElement();
  }

  writer.writeEndElement();
}

void Point::setCurveName(const QString &curveNameNew)
{
  // Replace the old curve name at the start of the string
  QString curveNameOld = Point::curveNameFromPointIdentifier (m_identifier);
  m_identifier = curveNameNew  + m_identifier.mid (curveNameOld.length());
}

void Point::setIdentifierIndex (unsigned int identifierIndex)
{
  m_identifierIndex = identifierIndex;
}

void Point::setOrdinal(double ordinal)
{
  m_hasOrdinal = true;
  m_ordinal = ordinal;
}

void Point::setPosGraph (const QPointF &posGraph)
{
  // Curve point graph coordinates should always be computed on the fly versus stored in this class, to reduce the
  // chances for stale information
  ENGAUGE_ASSERT (m_isAxisPoint);

  m_hasPosGraph = true;
  m_posGraph = posGraph;
}

void Point::setPosScreen (const QPointF &posScreen)
{
  m_posScreen = posScreen;
}

QString Point::temporaryPointIdentifier ()
{
  return QString ("%1%2%3")
      .arg (AXIS_CURVE_NAME)
      .arg (POINT_IDENTIFIER_DELIMITER_SAFE)
      .arg (0);
}

QString Point::uniqueIdentifierGenerator (const QString &curveName)
{
  return QString ("%1%2point%3%4")
      .arg (curveName)
      .arg (POINT_IDENTIFIER_DELIMITER_SAFE)
      .arg (POINT_IDENTIFIER_DELIMITER_SAFE)
      .arg (m_identifierIndex++);
}
