/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#include "CallbackAddPointsInCurvesGraphs.h"
#include "CallbackBoundingRects.h"
#include "CallbackCheckAddPointAxis.h"
#include "CallbackCheckEditPointAxis.h"
#include "CallbackNextOrdinal.h"
#include "CallbackRemovePointsInCurvesGraphs.h"
#include "Curve.h"
#include "CurvesGraphs.h"
#include "CurveStyle.h"
#include "CurveStyles.h"
#include "Document.h"
#include "DocumentSerialize.h"
#include "EngaugeAssert.h"
#include "EnumsToQt.h"
#include "GridInitializer.h"
#include <iostream>
#include "Logger.h"
#include "OrdinalGenerator.h"
#include "Point.h"
#include "PointStyle.h"
#include <QByteArray>
#include <QDataStream>
#include <QDebug>
#include <QDomDocument>
#include <QFile>
#include <QImage>
#include <qmath.h>
#include <QObject>
#include <QtToString.h>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include "SettingsForGraph.h"
#include "Transformation.h"
#include "Version.h"
#include "Xml.h"

const int FOUR_BYTES = 4;
const int NOMINAL_COORD_SYSTEM_COUNT = 1;
const int VERSION_6 = 6;
const int VERSION_7 = 7;
const int VERSION_8 = 8;
const int VERSION_9 = 9;
const int VERSION_10 = 10;
const int VERSION_11 = 11;
const int VERSION_12 = 12;
const int VERSION_13 = 13;

Document::Document (const QImage &image) :
  m_name ("untitled"),
  m_documentAxesPointsRequired (DOCUMENT_AXES_POINTS_REQUIRED_3)
{

  m_coordSystemContext.addCoordSystems(NOMINAL_COORD_SYSTEM_COUNT);

  m_successfulRead = true; // Reading from QImage always succeeds, resulting in empty Document

  m_pixmap.convertFromImage (image);
}

Document::Document (const QString &fileName) :
  m_name (fileName),
  m_documentAxesPointsRequired (DOCUMENT_AXES_POINTS_REQUIRED_3)
{

  m_successfulRead = true;

  // Grab first few bytes to determine the version number
  QFile fileVersion (fileName);
  if (fileVersion.open(QIODevice::ReadOnly)) {

    QByteArray bytesStart = fileVersion.read (FOUR_BYTES);
    fileVersion.close ();

    if (bytesIndicatePreVersion6 (bytesStart)) {

      QFile *file = new QFile (fileName);
      if (file->open (QIODevice::ReadOnly)) {
        QDataStream str (file);

        m_coordSystemContext.addCoordSystems(NOMINAL_COORD_SYSTEM_COUNT);
        loadPreVersion6 (str);

      } else {

        m_successfulRead = false;
        m_reasonForUnsuccessfulRead = QObject::tr ("Operating system says file is not readable");

      }
    } else {

      QFile *file = new QFile (fileName);
      if (file->open (QIODevice::ReadOnly | QIODevice::Text)) {

        int version = versionFromFile (file);
        switch (version)
        {
          case VERSION_6:
            loadVersion6 (file);
            break;

          case VERSION_7:
          case VERSION_8:
          case VERSION_9:
          case VERSION_10:
          case VERSION_11:
          case VERSION_12:
          case VERSION_13:
            loadVersions7AndUp (file);
            break;

          default:
            m_successfulRead = false;
            m_reasonForUnsuccessfulRead = QString ("Engauge %1 %2 %3 %4 Engauge")
                                          .arg (VERSION_NUMBER)
                                          .arg (QObject::tr ("cannot read newer files from version"))
                                          .arg (version)
                                          .arg (QObject::tr ("of"));\
            break;
        }

        // Close and deactivate
        file->close ();
        delete file;
        file = nullptr;

      } else {

        m_successfulRead = false;
        m_reasonForUnsuccessfulRead = QObject::tr ("Operating system says file is not readable");
      }
    }
  } else {
    fileVersion.close ();
    m_successfulRead = false;
    m_reasonForUnsuccessfulRead = QString ("%1 '%2' %3")
                                  .arg (QObject::tr ("File"))
                                  .arg (fileName)
                                  .arg (QObject::tr ("was not found"));
  }
}

void Document::addCoordSystems(unsigned int numberCoordSystemToAdd)
{

  m_coordSystemContext.addCoordSystems(numberCoordSystemToAdd);
}

void Document::addGraphCurveAtEnd (const QString &curveName)
{

  m_coordSystemContext.addGraphCurveAtEnd  (curveName);
}

void Document::addPointAxisWithGeneratedIdentifier (const QPointF &posScreen,
                                                    const QPointF &posGraph,
                                                    QString &identifier,
                                                    double ordinal,
                                                    bool isXOnly)
{

  m_coordSystemContext.addPointAxisWithGeneratedIdentifier(posScreen,
                                                           posGraph,
                                                           identifier,
                                                           ordinal,
                                                           isXOnly);
}

void Document::addPointAxisWithSpecifiedIdentifier (const QPointF &posScreen,
                                                    const QPointF &posGraph,
                                                    const QString &identifier,
                                                    double ordinal,
                                                    bool isXOnly)
{

  m_coordSystemContext.addPointAxisWithSpecifiedIdentifier(posScreen,
                                                           posGraph,
                                                           identifier,
                                                           ordinal,
                                                           isXOnly);
}

void Document::addPointGraphWithGeneratedIdentifier (const QString &curveName,
                                                     const QPointF &posScreen,
                                                     QString &identifier,
                                                     double ordinal)
{

  m_coordSystemContext.addPointGraphWithGeneratedIdentifier(curveName,
                                                            posScreen,
                                                            identifier,
                                                            ordinal);
}

void Document::addPointGraphWithSpecifiedIdentifier (const QString &curveName,
                                                     const QPointF &posScreen,
                                                     const QString &identifier,
                                                     double ordinal)
{

  m_coordSystemContext.addPointGraphWithSpecifiedIdentifier(curveName,
                                                            posScreen,
                                                            identifier,
                                                            ordinal);
}

void Document::addPointsInCurvesGraphs (CurvesGraphs &curvesGraphs)
{

  m_coordSystemContext.addPointsInCurvesGraphs(curvesGraphs);
}

void Document::addScaleWithGeneratedIdentifier (const QPointF &posScreen0,
                                                const QPointF &posScreen1,
                                                double scaleLength,
                                                QString &identifier0,
                                                QString &identifier1,
                                                double ordinal0,
                                                double ordinal1)
{

  const bool IS_X_ONLY = false;

  m_coordSystemContext.addPointAxisWithGeneratedIdentifier(posScreen0,
                                                           QPointF (0, 0),
                                                           identifier0,
                                                           ordinal0,
                                                           IS_X_ONLY);
  m_coordSystemContext.addPointAxisWithGeneratedIdentifier(posScreen1,
                                                           QPointF (scaleLength, 0),
                                                           identifier1,
                                                           ordinal1,
                                                           IS_X_ONLY);
}

bool Document::bytesIndicatePreVersion6 (const QByteArray &bytes) const
{

  QByteArray preVersion6MagicNumber;
  preVersion6MagicNumber.resize (FOUR_BYTES);

  // Windows compiler gives warning if 0x## is used instead of '\x##' below
  preVersion6MagicNumber[0] = '\x00';
  preVersion6MagicNumber[1] = '\x00';
  preVersion6MagicNumber[2] = '\xCA';
  preVersion6MagicNumber[3] = '\xFE';

  return (bytes == preVersion6MagicNumber);
}

void Document::checkAddPointAxis (const QPointF &posScreen,
                                  const QPointF &posGraph,
                                  bool &isError,
                                  QString &errorMessage,
                                  bool isXOnly)
{

  m_coordSystemContext.checkAddPointAxis(posScreen,
                                         posGraph,
                                         isError,
                                         errorMessage,
                                         isXOnly,
                                         m_documentAxesPointsRequired);
}

void Document::checkEditPointAxis (const QString &pointIdentifier,
                                   const QPointF &posScreen,
                                   const QPointF &posGraph,
                                   bool &isError,
                                   QString &errorMessage)
{

  m_coordSystemContext.checkEditPointAxis(pointIdentifier,
                                          posScreen,
                                          posGraph,
                                          isError,
                                          errorMessage,
                                          m_documentAxesPointsRequired);
}

const CoordSystem &Document::coordSystem() const
{

  return m_coordSystemContext.coordSystem();
}

unsigned int Document::coordSystemCount () const
{

  return m_coordSystemContext.coordSystemCount();
}

CoordSystemIndex Document::coordSystemIndex() const
{

  return m_coordSystemContext.coordSystemIndex();
}

const Curve &Document::curveAxes () const
{

  return m_coordSystemContext.curveAxes();
}

Curve *Document::curveForCurveName (const QString &curveName)
{

  return m_coordSystemContext.curveForCurveName(curveName);
}

const Curve *Document::curveForCurveName (const QString &curveName) const
{

  return m_coordSystemContext.curveForCurveName (curveName);
}

const CurvesGraphs &Document::curvesGraphs () const
{

  return m_coordSystemContext.curvesGraphs();
}

QStringList Document::curvesGraphsNames() const
{

  return m_coordSystemContext.curvesGraphsNames();
}

int Document::curvesGraphsNumPoints(const QString &curveName) const
{

  return m_coordSystemContext.curvesGraphsNumPoints(curveName);
}

DocumentAxesPointsRequired Document::documentAxesPointsRequired () const
{
  return m_documentAxesPointsRequired;
}

void Document::editPointAxis (const QPointF &posGraph,
                              const QString &identifier)
{

  m_coordSystemContext.editPointAxis(posGraph,
                                     identifier);
}

void Document::editPointGraph (bool isX,
                               bool isY,
                               double x,
                               double y,
                               const QStringList &identifiers,
                               const Transformation &transformation)
{

  m_coordSystemContext.editPointGraph (isX,
                                       isY,
                                       x,
                                       y,
                                       identifiers,
                                       transformation);
}

void Document::generateEmptyPixmap(const QXmlStreamAttributes &attributes)
{

  int width = 800, height = 500; // Defaults

  if (attributes.hasAttribute (DOCUMENT_SERIALIZE_IMAGE_WIDTH) &&
      attributes.hasAttribute (DOCUMENT_SERIALIZE_IMAGE_HEIGHT)) {

    width = attributes.value (DOCUMENT_SERIALIZE_IMAGE_WIDTH).toInt();
    height = attributes.value (DOCUMENT_SERIALIZE_IMAGE_HEIGHT).toInt();

  }

  m_pixmap = QPixmap (width, height);
}

void Document::initializeGridDisplay (const Transformation &transformation)
{

  ENGAUGE_ASSERT (!m_coordSystemContext.modelGridDisplay().stable());

  // Get graph coordinate bounds
  CallbackBoundingRects ftor (m_documentAxesPointsRequired,
                              transformation);

  Functor2wRet<const QString &, const Point &, CallbackSearchReturn> ftorWithCallback = functor_ret (ftor,
                                                                                                     &CallbackBoundingRects::callback);

  iterateThroughCurvePointsAxes (ftorWithCallback);

  // Initialize. Note that if there are no graph points then these next steps have no effect
  bool isEmpty;
  QPointF boundingRectGraphMin = ftor.boundingRectGraphMin (isEmpty);
  QPointF boundingRectGraphMax = ftor.boundingRectGraphMax (isEmpty);
  if (!isEmpty) {

    GridInitializer gridInitializer;

    DocumentModelGridDisplay modelGridDisplay = gridInitializer.initializeWithWidePolarCoverage (boundingRectGraphMin,
                                                                                                 boundingRectGraphMax,
                                                                                                 modelCoords(),
                                                                                                 transformation,
                                                                                                 m_pixmap.size ());

    m_coordSystemContext.setModelGridDisplay (modelGridDisplay);
  }
}

bool Document::isXOnly (const QString &pointIdentifier) const
{
  return m_coordSystemContext.isXOnly (pointIdentifier);
}

void Document::iterateThroughCurvePointsAxes (const Functor2wRet<const QString &, const Point &, CallbackSearchReturn> &ftorWithCallback)
{

  m_coordSystemContext.iterateThroughCurvePointsAxes(ftorWithCallback);
}

void Document::iterateThroughCurvePointsAxes (const Functor2wRet<const QString &, const Point &, CallbackSearchReturn> &ftorWithCallback) const
{

  m_coordSystemContext.iterateThroughCurvePointsAxes(ftorWithCallback);
}

void Document::iterateThroughCurveSegments (const QString &curveName,
                                            const Functor2wRet<const Point &, const Point &, CallbackSearchReturn> &ftorWithCallback) const
{

  m_coordSystemContext.iterateThroughCurveSegments(curveName,
                                                   ftorWithCallback);
}

void Document::iterateThroughCurvesPointsGraphs (const Functor2wRet<const QString &, const Point &, CallbackSearchReturn> &ftorWithCallback)
{

  m_coordSystemContext.iterateThroughCurvesPointsGraphs(ftorWithCallback);
}

void Document::iterateThroughCurvesPointsGraphs (const Functor2wRet<const QString &, const Point &, CallbackSearchReturn> &ftorWithCallback) const
{

  m_coordSystemContext.iterateThroughCurvesPointsGraphs(ftorWithCallback);
}

void Document::loadImage(QXmlStreamReader &reader)
{

  loadNextFromReader(reader); // Read to CDATA
  if (reader.isCDATA ()) {

    // Get base64 array
    QByteArray array64 = reader.text().toString().toUtf8();

    // Decoded array
    QByteArray array;
    array = QByteArray::fromBase64(array64);

    // Read decoded array into image
    QDataStream str (&array, QIODevice::ReadOnly);
    QImage img = m_pixmap.toImage ();
    str >> img;
    m_pixmap = QPixmap::fromImage (img);

    // Read until end of this subtree
    while ((reader.tokenType() != QXmlStreamReader::EndElement) ||
           (reader.name() != DOCUMENT_SERIALIZE_IMAGE)){
      loadNextFromReader(reader);
    }

  } else {

    // This point can be reached if:
    // 1) File is broken
    // 2) Bad character is in text, and NetworkClient::cleanXml did not do its job
    reader.raiseError (QObject::tr ("Cannot read image data"));
  }
}

void Document::loadPreVersion6 (QDataStream &str)
{

  qint32 int32;
  double version;
  QString st;

  str >> int32; // Magic number
  str >> version;
  str >> st; // Version string
  str >> int32; // Background
  str >> m_pixmap;
  str >> m_name;

  m_coordSystemContext.loadPreVersion6 (str,
                                        version,
                                        m_documentAxesPointsRequired); // Returns axes points required
}

void Document::loadVersion6 (QFile *file)
{

  QXmlStreamReader reader (file);

  m_documentAxesPointsRequired = DOCUMENT_AXES_POINTS_REQUIRED_3;

  // Create the single CoordSystem used in versions before version 7
  m_coordSystemContext.addCoordSystems(NOMINAL_COORD_SYSTEM_COUNT);

  // If this is purely a serialized Document then we process every node under the root. However, if this is an error report file
  // then we need to skip the non-Document stuff. The common solution is to skip nodes outside the Document subtree using this flag
  bool inDocumentSubtree = false;

  // Import from xml. Loop to end of data or error condition occurs, whichever is first
  while (!reader.atEnd() &&
         !reader.hasError()) {
    QXmlStreamReader::TokenType tokenType = loadNextFromReader(reader);

    // Special processing of DOCUMENT_SERIALIZE_IMAGE outside DOCUMENT_SERIALIZE_DOCUMENT, for an error report file
    if ((reader.name() == DOCUMENT_SERIALIZE_IMAGE) &&
        (tokenType == QXmlStreamReader::StartElement)) {

      generateEmptyPixmap (reader.attributes());
    }

    // Branching to skip non-Document nodes, with the exception of any DOCUMENT_SERIALIZE_IMAGE outside DOCUMENT_SERIALIZE_DOCUMENT
    if ((reader.name() == DOCUMENT_SERIALIZE_DOCUMENT) &&
        (tokenType == QXmlStreamReader::StartElement)) {

      inDocumentSubtree = true;

    } else if ((reader.name() == DOCUMENT_SERIALIZE_DOCUMENT) &&
               (tokenType == QXmlStreamReader::EndElement)) {

      // Exit out of loop immediately
      break;
    }

    if (inDocumentSubtree) {

      // Iterate to next StartElement
      if (tokenType == QXmlStreamReader::StartElement) {

        // This is a StartElement, so process it up to the corresonding EndElement
        QString tag = reader.name().toString();
        if (tag == DOCUMENT_SERIALIZE_IMAGE) {
          // A standard Document file has DOCUMENT_SERIALIZE_IMAGE inside DOCUMENT_SERIALIZE_DOCUMENT, versus an error report file
          loadImage(reader);

          // Now that we have the image at the DOCUMENT_SERIALIZE_DOCUMENT level, we read the rest at this level into CoordSystem
          m_coordSystemContext.loadVersion6 (reader,
                                             m_documentAxesPointsRequired); // Returns axes points required

          // Reading of DOCUMENT_SERIALIZE_DOCUMENT has just finished, so the reading has finished
          break;
        }
      }
    }
  }

  if (reader.hasError ()) {

    m_successfulRead = false;
    m_reasonForUnsuccessfulRead = reader.errorString();
  }

  // There are already one axes curve and at least one graph curve so we do not need to add any more graph curves
}

void Document::loadVersions7AndUp (QFile *file)
{

  const int ONE_COORDINATE_SYSTEM = 1;

  QXmlStreamReader reader (file);

  // If this is purely a serialized Document then we process every node under the root. However, if this is an error report file
  // then we need to skip the non-Document stuff. The common solution is to skip nodes outside the Document subtree using this flag
  bool inDocumentSubtree = false;

  // Import from xml. Loop to end of data or error condition occurs, whichever is first
  while (!reader.atEnd() &&
         !reader.hasError()) {
    QXmlStreamReader::TokenType tokenType = loadNextFromReader(reader);

    // Special processing of DOCUMENT_SERIALIZE_IMAGE outside DOCUMENT_SERIALIZE_DOCUMENT, for an error report file
    if ((reader.name() == DOCUMENT_SERIALIZE_IMAGE) &&
        (tokenType == QXmlStreamReader::StartElement)) {

      generateEmptyPixmap (reader.attributes());
    }

    // Branching to skip non-Document nodes, with the exception of any DOCUMENT_SERIALIZE_IMAGE outside DOCUMENT_SERIALIZE_DOCUMENT
    if ((reader.name() == DOCUMENT_SERIALIZE_DOCUMENT) &&
        (tokenType == QXmlStreamReader::StartElement)) {

      inDocumentSubtree = true;

      QXmlStreamAttributes attributes = reader.attributes();
      if (attributes.hasAttribute (DOCUMENT_SERIALIZE_AXES_POINTS_REQUIRED)) {
        m_documentAxesPointsRequired = static_cast<DocumentAxesPointsRequired> (attributes.value (DOCUMENT_SERIALIZE_AXES_POINTS_REQUIRED).toInt());
      } else {
        m_documentAxesPointsRequired = DOCUMENT_AXES_POINTS_REQUIRED_3;
      }

    } else if ((reader.name() == DOCUMENT_SERIALIZE_DOCUMENT) &&
               (tokenType == QXmlStreamReader::EndElement)) {

      // Exit out of loop immediately
      break;
    }

    if (inDocumentSubtree) {

      // Iterate to next StartElement
      if (tokenType == QXmlStreamReader::StartElement) {

        // This is a StartElement, so process it
        QString tag = reader.name().toString();
        if (tag == DOCUMENT_SERIALIZE_COORD_SYSTEM) {
          m_coordSystemContext.addCoordSystems (ONE_COORDINATE_SYSTEM);
          m_coordSystemContext.loadVersions7AndUp (reader);
        } else if (tag == DOCUMENT_SERIALIZE_IMAGE) {
          // A standard Document file has DOCUMENT_SERIALIZE_IMAGE inside DOCUMENT_SERIALIZE_DOCUMENT, versus an error report file
          loadImage(reader);
        } else if (tag == DOCUMENT_SERIALIZE_LOAD_VIEWS) {
          m_modelLoadViews.loadXml (reader);
        }
      }
    }
  }

  if (reader.hasError ()) {

    m_successfulRead = false;
    m_reasonForUnsuccessfulRead = reader.errorString();\
  }

  // There are already one axes curve and at least one graph curve so we do not need to add any more graph curves
}

DocumentModelAxesChecker Document::modelAxesChecker() const
{
  return m_coordSystemContext.modelAxesChecker();
}

DocumentModelColorFilter Document::modelColorFilter() const
{
  return m_coordSystemContext.modelColorFilter();
}

DocumentModelCoords Document::modelCoords() const
{
  return m_coordSystemContext.modelCoords();
}

CurveStyles Document::modelCurveStyles() const
{
  return m_coordSystemContext.modelCurveStyles();
}

DocumentModelDigitizeCurve Document::modelDigitizeCurve() const
{
  return m_coordSystemContext.modelDigitizeCurve();
}

DocumentModelExportFormat Document::modelExport() const
{
  return m_coordSystemContext.modelExport();
}

DocumentModelGeneral Document::modelGeneral() const
{
  return m_coordSystemContext.modelGeneral();
}

DocumentModelGridDisplay Document::modelGridDisplay() const
{
  return m_coordSystemContext.modelGridDisplay();
}

DocumentModelGridRemoval Document::modelGridRemoval() const
{
  return m_coordSystemContext.modelGridRemoval();
}


DocumentModelGuideline Document::modelGuideline() const
{
  return m_coordSystemContext.modelGuideline();
}

DocumentModelLoadViews Document::modelLoadViews() const
{
  return m_modelLoadViews;
}

DocumentModelPointMatch Document::modelPointMatch() const
{
  return m_coordSystemContext.modelPointMatch();
}

DocumentModelSegments Document::modelSegments() const
{
  return m_coordSystemContext.modelSegments();
}

void Document::movePoint (const QString &pointIdentifier,
                          const QPointF &deltaScreen)
{
  m_coordSystemContext.movePoint (pointIdentifier,
                     deltaScreen);
}

int Document::nextOrdinalForCurve (const QString &curveName) const
{

  return m_coordSystemContext.nextOrdinalForCurve(curveName);
}

void Document::overrideGraphDefaultsWithMapDefaults ()
{
  const int DEFAULT_WIDTH = 1;

  // Axis style for scale bar is better with transparent-filled circles so user can more accurately place them,
  // and a visible line between the points also helps to make the points more visible
  CurveStyles curveStyles = modelCurveStyles ();

  CurveStyle curveStyle  = curveStyles.curveStyle (AXIS_CURVE_NAME);

  PointStyle pointStyle = curveStyle.pointStyle ();
  pointStyle.setShape (POINT_SHAPE_CIRCLE);
  pointStyle.setPaletteColor (COLOR_PALETTE_RED);

  LineStyle lineStyle = curveStyle.lineStyle ();
  lineStyle.setCurveConnectAs (CONNECT_AS_RELATION_STRAIGHT);
  lineStyle.setWidth (DEFAULT_WIDTH);
  lineStyle.setPaletteColor (COLOR_PALETTE_RED);

  curveStyle.setPointStyle (pointStyle);
  curveStyle.setLineStyle (lineStyle);

  curveStyles.setCurveStyle (AXIS_CURVE_NAME,
                             curveStyle);

  // Change all graph curves from functions to relations
  QStringList curveNames = curvesGraphsNames ();
  QStringList::const_iterator itr;
  for (itr = curveNames.begin(); itr != curveNames.end(); itr++) {
    QString curveName = *itr;
    CurveStyle curveStyle = curveStyles.curveStyle (curveName);

    LineStyle lineStyle = curveStyle.lineStyle ();
    lineStyle.setCurveConnectAs (CONNECT_AS_RELATION_STRAIGHT);

    curveStyle.setLineStyle (lineStyle);

    curveStyles.setCurveStyle (curveName,
                               curveStyle);
  }

  // Save modified curve styles into Document
  setModelCurveStyles (curveStyles);
}

QPixmap Document::pixmap () const
{                               
  return m_pixmap;
}

QPointF Document::positionGraph (const QString &pointIdentifier) const
{
  return m_coordSystemContext.positionGraph(pointIdentifier);
}

QPointF Document::positionScreen (const QString &pointIdentifier) const
{                          
  return m_coordSystemContext.positionScreen(pointIdentifier);
}

void Document::print () const
{                                 
  QString text;
  QTextStream str (&text);

  printStream ("",
               str);
  std::cerr << text.toLatin1().data();
}

void Document::printStream (QString indentation,
                            QTextStream &str) const
{
  str << indentation << "Document\n";

  indentation += INDENTATION_DELTA;

  str << indentation << "name=" << m_name << "\n";
  str << indentation << "pixmap=" << m_pixmap.width() << "x" <<  m_pixmap.height() << "\n";

  m_coordSystemContext.printStream(indentation,
                      str);
}

QString Document::reasonForUnsuccessfulRead () const
{
  ENGAUGE_ASSERT (!m_successfulRead);

  return m_reasonForUnsuccessfulRead;
}

void Document::removePointAxis (const QString &identifier)
{

  m_coordSystemContext.removePointAxis(identifier);
}

void Document::removePointGraph (const QString &identifier)
{

  m_coordSystemContext.removePointGraph(identifier);
}

void Document::removePointsInCurvesGraphs (CurvesGraphs &curvesGraphs)
{

  m_coordSystemContext.removePointsInCurvesGraphs(curvesGraphs);
}

void Document::saveXml (QXmlStreamWriter &writer) const
{
  writer.writeStartElement(DOCUMENT_SERIALIZE_DOCUMENT);

  // Version number is tacked onto DOCUMENT_SERIALIZE_DOCUMENT since the alternative  (creating a new start element)
  // causes the code to complain during loading
  writer.writeAttribute(DOCUMENT_SERIALIZE_APPLICATION_VERSION_NUMBER, VERSION_NUMBER);

  // Number of axes points required
  writer.writeAttribute(DOCUMENT_SERIALIZE_AXES_POINTS_REQUIRED, QString::number (m_documentAxesPointsRequired));

  // Serialize the Document image. That binary data is encoded as base64
  QByteArray array;
  QDataStream str (&array, QIODevice::WriteOnly);
  QImage img = m_pixmap.toImage ();
  str << img;
  writer.writeStartElement(DOCUMENT_SERIALIZE_IMAGE);

  // Image width and height are explicitly inserted for error reports, since the CDATA is removed
  // but we still want the image size for reconstructing the error(s)
  writer.writeAttribute(DOCUMENT_SERIALIZE_IMAGE_WIDTH, QString::number (img.width()));
  writer.writeAttribute(DOCUMENT_SERIALIZE_IMAGE_HEIGHT, QString::number (img.height()));

  writer.writeCDATA (array.toBase64 ());
  writer.writeEndElement();

  m_coordSystemContext.saveXml (writer);
  m_modelLoadViews.saveXml (writer);
}

QString Document::selectedCurveName() const
{
  return m_coordSystemContext.selectedCurveName();
}

void Document::setCoordSystemIndex(CoordSystemIndex coordSystemIndex)
{

  m_coordSystemContext.setCoordSystemIndex (coordSystemIndex);
}

void Document::setCurveAxes (const Curve &curveAxes)
{

  m_coordSystemContext.setCurveAxes (curveAxes);
}

void Document::setCurvesGraphs (const CurvesGraphs &curvesGraphs)
{

  m_coordSystemContext.setCurvesGraphs(curvesGraphs);
}

void Document::setDocumentAxesPointsRequired(DocumentAxesPointsRequired documentAxesPointsRequired)
{

  m_documentAxesPointsRequired = documentAxesPointsRequired;

  if (documentAxesPointsRequired == DOCUMENT_AXES_POINTS_REQUIRED_2) {

    overrideGraphDefaultsWithMapDefaults ();
  }
}

void Document::setModelAxesChecker(const DocumentModelAxesChecker &modelAxesChecker)
{

  m_coordSystemContext.setModelAxesChecker(modelAxesChecker);
}

void Document::setModelColorFilter(const DocumentModelColorFilter &modelColorFilter)
{

  // Save the CurveFilter for each Curve
  ColorFilterSettingsList::const_iterator itr;
  for (itr = modelColorFilter.colorFilterSettingsList().constBegin ();
       itr != modelColorFilter.colorFilterSettingsList().constEnd();
       itr++) {

    QString curveName = itr.key();
    const ColorFilterSettings &colorFilterSettings = itr.value();

    Curve *curve = curveForCurveName (curveName);
    curve->setColorFilterSettings (colorFilterSettings);
  }
}

void Document::setModelCoords (const DocumentModelCoords &modelCoords)
{

  m_coordSystemContext.setModelCoords(modelCoords);
}

void Document::setModelCurveStyles(const CurveStyles &modelCurveStyles)
{

  // Save the LineStyle and PointStyle for each Curve
  QStringList curveNames = modelCurveStyles.curveNames();
  QStringList::iterator itr;
  for (itr = curveNames.begin(); itr != curveNames.end(); itr++) {

    QString curveName = *itr;
    const CurveStyle &curveStyle = modelCurveStyles.curveStyle (curveName);

    Curve *curve = curveForCurveName (curveName);
    curve->setCurveStyle (curveStyle);
  }
}

void Document::setModelDigitizeCurve (const DocumentModelDigitizeCurve &modelDigitizeCurve)
{

  m_coordSystemContext.setModelDigitizeCurve(modelDigitizeCurve);
}

void Document::setModelExport(const DocumentModelExportFormat &modelExport)
{

  m_coordSystemContext.setModelExport (modelExport);
}

void Document::setModelGeneral (const DocumentModelGeneral &modelGeneral)
{

  m_coordSystemContext.setModelGeneral(modelGeneral);
}

void Document::setModelGridDisplay(const DocumentModelGridDisplay &modelGridDisplay)
{

  m_coordSystemContext.setModelGridDisplay(modelGridDisplay);
}

void Document::setModelGridRemoval(const DocumentModelGridRemoval &modelGridRemoval)
{

  m_coordSystemContext.setModelGridRemoval(modelGridRemoval);
}

void Document::setModelGuideline(const DocumentModelGuideline &modelGuideline)
{

  m_coordSystemContext.setModelGuideline(modelGuideline);
}  

void Document::setModelLoadViews(const DocumentModelLoadViews &modelLoadViews)
{

  m_modelLoadViews = modelLoadViews;
}

void Document::setModelPointMatch(const DocumentModelPointMatch &modelPointMatch)
{

  m_coordSystemContext.setModelPointMatch(modelPointMatch);
}

void Document::setModelSegments(const DocumentModelSegments &modelSegments)
{

  m_coordSystemContext.setModelSegments (modelSegments);
}

void Document::setPixmap(const QImage &image)
{

  m_pixmap = QPixmap::fromImage (image);
}

void Document::setSelectedCurveName(const QString &selectedCurveName)
{
  m_coordSystemContext.setSelectedCurveName (selectedCurveName);
}

bool Document::successfulRead () const
{                                 
  return m_successfulRead;
}

void Document::updatePointOrdinals (const Transformation &transformation)
{

  m_coordSystemContext.updatePointOrdinals(transformation);
}

int Document::versionFromFile (QFile *file) const
{

  int version = VERSION_6; // Use default if tag is missing

  QDomDocument doc;
  if (doc.setContent (file)) {

    QDomNodeList nodes = doc.elementsByTagName (DOCUMENT_SERIALIZE_DOCUMENT);
    if (nodes.count() > 0) {
      QDomNode node = nodes.at (0);

      QDomNamedNodeMap attributes = node.attributes();

      if (attributes.contains (DOCUMENT_SERIALIZE_APPLICATION_VERSION_NUMBER)) {

        QDomElement elem = node.toElement();
        version = qFloor (elem.attribute (DOCUMENT_SERIALIZE_APPLICATION_VERSION_NUMBER).toDouble());
      }
    }
  }

  file->seek (0); // Go back to beginning

  return version;
}
