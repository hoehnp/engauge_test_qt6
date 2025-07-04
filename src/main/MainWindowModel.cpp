/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#include "CmdMediator.h"
#include "DocumentSerialize.h"
#include "GraphicsPoint.h"
#include "GridLineLimiter.h"
#include "ImportCroppingUtilBase.h"
#include "Logger.h"
#include "MainWindowModel.h"
#include "PdfResolution.h"
#include <QLocale>
#include <QObject>
#include <QTextStream>
#include "QtToString.h"
#include <QXmlStreamWriter>
#include "Xml.h"
#include "ZoomFactorInitial.h"

// Prevent comma ambiguity with group separator commas and field delimiting commas
const QLocale::NumberOption HIDE_GROUP_SEPARATOR = QLocale::OmitGroupSeparator;

const bool DEFAULT_DRAG_DROP_EXPORT = false; // False value allows intuitive copy-and-drag to select a rectangular set of table cells
const int DEFAULT_SIGNIFICANT_DIGITS = 7;
const bool DEFAULT_SMALL_DIALOGS = false;
const bool DEFAULT_IMAGE_REPLACE_RENAMES_DOCUMENT = true; // Pre-version 11.3 behavior
const int DEFAULT_MAXIMUM_EXPORTED_POINTS_PER_CURVE = 5000; // Moved 1/2020 from Export classes
const LoadViews DEFAULT_LOAD_VIEWS = LOAD_VIEWS_KEEP_CURRENT; // Pre-version 13 behavior
  
MainWindowModel::MainWindowModel() :
  m_zoomControl (ZOOM_CONTROL_MENU_WHEEL_PLUSMINUS),
  m_zoomFactorInitial (DEFAULT_ZOOM_FACTOR_INITIAL),
  m_mainTitleBarFormat (MAIN_TITLE_BAR_FORMAT_PATH),
  m_pdfResolution (DEFAULT_IMPORT_PDF_RESOLUTION),
  m_importCropping (DEFAULT_IMPORT_CROPPING),
  m_loadViews (DEFAULT_LOAD_VIEWS),
  m_maximumGridLines (DEFAULT_MAXIMUM_GRID_LINES),
  m_highlightOpacity (DEFAULT_HIGHLIGHT_OPACITY),
  m_smallDialogs (DEFAULT_SMALL_DIALOGS),
  m_dragDropExport (DEFAULT_DRAG_DROP_EXPORT),
  m_significantDigits (DEFAULT_SIGNIFICANT_DIGITS),
  m_imageReplaceRenamesDocument (DEFAULT_IMAGE_REPLACE_RENAMES_DOCUMENT),
  m_maximumExportedPointsPerCurve (DEFAULT_MAXIMUM_EXPORTED_POINTS_PER_CURVE)
{
  // Locale member variable m_locale is initialized to default locale when default constructor is called
}

MainWindowModel::MainWindowModel(const MainWindowModel &other) :
  m_locale (other.locale()),
  m_zoomControl (other.zoomControl()),
  m_zoomFactorInitial (other.zoomFactorInitial()),
  m_mainTitleBarFormat (other.mainTitleBarFormat()),
  m_pdfResolution (other.pdfResolution()),
  m_importCropping (other.importCropping()),
  m_loadViews (other.loadViews()),
  m_maximumGridLines (other.maximumGridLines()),
  m_highlightOpacity (other.highlightOpacity()),
  m_smallDialogs (other.smallDialogs()),
  m_dragDropExport (other.dragDropExport()),
  m_significantDigits (other.significantDigits()),
  m_imageReplaceRenamesDocument (other.imageReplaceRenamesDocument()),
  m_maximumExportedPointsPerCurve (other.maximumExportedPointsPerCurve ())
{
}

MainWindowModel &MainWindowModel::operator=(const MainWindowModel &other)
{
  m_locale = other.locale();
  m_zoomControl = other.zoomControl();
  m_zoomFactorInitial = other.zoomFactorInitial();
  m_mainTitleBarFormat = other.mainTitleBarFormat();
  m_pdfResolution = other.pdfResolution();
  m_importCropping = other.importCropping();
  m_loadViews = other.loadViews();
  m_maximumGridLines = other.maximumGridLines();
  m_highlightOpacity = other.highlightOpacity();
  m_smallDialogs = other.smallDialogs();
  m_dragDropExport = other.dragDropExport();
  m_significantDigits = other.significantDigits();
  m_imageReplaceRenamesDocument = other.imageReplaceRenamesDocument();
  m_maximumExportedPointsPerCurve = other.maximumExportedPointsPerCurve();
  
  return *this;
}

bool MainWindowModel::dragDropExport() const
{
  return m_dragDropExport;
}

double MainWindowModel::highlightOpacity() const
{
  return m_highlightOpacity;
}

bool MainWindowModel::imageReplaceRenamesDocument() const
{
  return m_imageReplaceRenamesDocument;
}

ImportCropping MainWindowModel::importCropping() const
{
  return m_importCropping;
}

LoadViews MainWindowModel::loadViews () const
{
  return m_loadViews;
}

void MainWindowModel::loadXml(QXmlStreamReader & /* reader */)
{
  // This class is  never serialized
}

QLocale MainWindowModel::locale () const
{
  return m_locale;
}

MainTitleBarFormat MainWindowModel::mainTitleBarFormat() const
{
  return m_mainTitleBarFormat;
}

int MainWindowModel::maximumExportedPointsPerCurve() const
{
  return m_maximumExportedPointsPerCurve;
}

int MainWindowModel::maximumGridLines() const
{
  return m_maximumGridLines;
}

int MainWindowModel::pdfResolution() const
{
  return m_pdfResolution;
}

void MainWindowModel::printStream(QString indentation,
                                     QTextStream &str) const
{
  str << indentation << "MainWindowModel\n";

  indentation += INDENTATION_DELTA;

  str << indentation << "locale=" << m_locale.name() << "\n";
  str << indentation << "zoomControl=" << m_zoomControl << "\n";
  str << indentation << "zoomFactorInitial=" << m_zoomFactorInitial << "\n";
  str << indentation << "mainWindowTitleBarFormat=" << (m_mainTitleBarFormat == MAIN_TITLE_BAR_FORMAT_NO_PATH ?
                                                        "NoPath" :
                                                        "Path") << "\n";
  str << indentation << "pdfResolution=" << m_pdfResolution << "\n";
  str << indentation << "importCropping=" << ImportCroppingUtilBase::importCroppingToString (m_importCropping).toLatin1().data() << "\n";
  str << indentation << "loadViews=" << (m_loadViews == LOAD_VIEWS_KEEP_CURRENT ?

                                         "KeepCurrent" :
                                         "UseDocument") << "\n";
  str << indentation << "maximumGridLines=" << m_maximumGridLines << "\n";
  str << indentation << "highlightOpacity=" << m_highlightOpacity << "\n";
  str << indentation << "smallDialogs=" << (m_smallDialogs ? "yes" : "no") << "\n";
  str << indentation << "dragDropExport=" << (m_dragDropExport ? "yes" : "no") << "\n";
  str << indentation << "significantDigits=" << m_significantDigits << "\n";
  str << indentation << "imageReplaceRenamesDocument=" << (m_imageReplaceRenamesDocument ? "yes" : "no") << "\n";
  str << indentation << "maximumExportedPointsPerCurve=" << m_maximumExportedPointsPerCurve << "\n";
}

void MainWindowModel::saveXml(QXmlStreamWriter & /* writer */) const
{
  // This class is  never serialized
}

void MainWindowModel::setDragDropExport(bool dragDropExport)
{
  m_dragDropExport = dragDropExport;
}

void MainWindowModel::setHighlightOpacity(double highlightOpacity)
{
  m_highlightOpacity = highlightOpacity;
}

void MainWindowModel::setImageReplaceRenamesDocument(bool imageReplaceRenamesDocument)
{
  m_imageReplaceRenamesDocument = imageReplaceRenamesDocument;
}

void MainWindowModel::setImportCropping (ImportCropping importCropping)
{
  m_importCropping = importCropping;
}

void MainWindowModel::setLoadViews (LoadViews loadViews)
{
  m_loadViews = loadViews;
}

void MainWindowModel::setLocale (QLocale::Language language,
                                 QLocale::Country country)
{
  QLocale locale (language,
                  country);
  locale.setNumberOptions(HIDE_GROUP_SEPARATOR);

  m_locale = locale;
}

void MainWindowModel::setLocale (const QLocale &locale)
{
  m_locale = locale;
  m_locale.setNumberOptions(HIDE_GROUP_SEPARATOR);
}

void MainWindowModel::setMainTitleBarFormat(MainTitleBarFormat mainTitleBarFormat)
{
  m_mainTitleBarFormat = mainTitleBarFormat;
}

void MainWindowModel::setMaximumExportedPointsPerCurve(int maximumExportedPointsPerCurve)
{
  m_maximumExportedPointsPerCurve = maximumExportedPointsPerCurve;
}

void MainWindowModel::setMaximumGridLines(int maximumGridLines)
{
  m_maximumGridLines = maximumGridLines;
}

void MainWindowModel::setPdfResolution(int resolution)
{
  m_pdfResolution = resolution;
}

void MainWindowModel::setSignificantDigits (int significantDigits)
{
  m_significantDigits = significantDigits;
}

void MainWindowModel::setSmallDialogs(bool smallDialogs)
{
  m_smallDialogs = smallDialogs;
}

void MainWindowModel::setZoomControl (ZoomControl zoomControl)
{
  m_zoomControl = zoomControl;
}

void MainWindowModel::setZoomFactorInitial(ZoomFactorInitial zoomFactorInitial)
{
  m_zoomFactorInitial = zoomFactorInitial;
}

int MainWindowModel::significantDigits() const
{
  return m_significantDigits;
}

bool MainWindowModel::smallDialogs () const
{
  return m_smallDialogs;
}

ZoomControl MainWindowModel::zoomControl () const
{
  return m_zoomControl;
}

ZoomFactorInitial MainWindowModel::zoomFactorInitial() const
{
  return m_zoomFactorInitial;
}
