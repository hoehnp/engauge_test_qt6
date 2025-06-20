/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#include "ChecklistGuidePage.h"
#include "ChecklistGuidePageConclusion.h"
#include "ChecklistGuidePageCurves.h"
#include "ChecklistGuidePageIntro.h"
#include "ChecklistGuideWizard.h"
#include "ChecklistTemplate.h"
#include "ColorFilterSettings.h"
#include "EngaugeAssert.h"
#include "Logger.h"
#include "MainWindow.h"
#include <QGridLayout>
#include <QHeaderView>
#include <QPushButton>
#include <QRadioButton>
#include <QTextStream>
#include <QVBoxLayout>

ChecklistGuideWizard::ChecklistGuideWizard (MainWindow &mainWindow,
                                            unsigned int numberCoordSystem) :
  QWizard (&mainWindow),
  m_mainWindow (mainWindow),
  m_dialogName (tr ("Checklist Guide"))
{
  // Note this window is positioned in showEvent

  setWindowTitle (tr ("Checklist Guide Wizard"));
  setModal (true);
  QPixmap splash (":/engauge/img/SpreadsheetsForDoc.png");
  setPixmap (QWizard::WatermarkPixmap, splash); // For ClassicStyle and ModernStyle
  setPixmap (QWizard::BackgroundPixmap, splash); // For MacStyle

  m_pageIntro = new ChecklistGuidePageIntro();
  addPage(m_pageIntro);

  for (CoordSystemIndex coordSystemIndex = 0; coordSystemIndex < numberCoordSystem; coordSystemIndex++) {
    m_pageCurves.push_back (new ChecklistGuidePageCurves(pageCurvesTitle (coordSystemIndex, numberCoordSystem)));
    addPage(m_pageCurves.last());
  }

  m_pageConclusion = new ChecklistGuidePageConclusion();
  addPage(m_pageConclusion);
}

QStringList ChecklistGuideWizard::curveNames(CoordSystemIndex coordSystemIndex) const
{
  return m_pageCurves [signed (coordSystemIndex)]->curveNames();
}

QString ChecklistGuideWizard::pageCurvesTitle (CoordSystemIndex coordSystemIndex,
                                               unsigned int numberCoordSystem) const
{

  ENGAUGE_ASSERT (coordSystemIndex < numberCoordSystem);

  if (numberCoordSystem == 1) {

    // Single curve needs no index information
    return tr ("Curves");

  } else {

    // One of multiple curves needs index information
    unsigned int indexOneBased = coordSystemIndex + 1;
    return QString ("%1 %2")
        .arg (QObject::tr ("Curves for coordinate system"))
        .arg (indexOneBased);

  }
}

void ChecklistGuideWizard::populateCurvesGraphs (CoordSystemIndex coordSystemIndex,
                                                 CurvesGraphs &curvesGraphs)
{

  QStringList curveNames = m_pageCurves [signed (coordSystemIndex)]->curveNames();
  QStringList::const_iterator itr;
  for (itr = curveNames.begin(); itr != curveNames.end(); itr++) {

    QString curveName = *itr;

    curvesGraphs.addGraphCurveAtEnd(Curve (curveName,
                                           ColorFilterSettings::defaultFilter (),
                                           CurveStyle (LineStyle::defaultGraphCurve (curvesGraphs.numCurves ()),
                                                       PointStyle::defaultGraphCurve (curvesGraphs.numCurves ()))));
  }
}

void ChecklistGuideWizard::showEvent (QShowEvent *event)
{
  // Position this window in the middle of the Engauge window so it is less likely to be obscured
  // under some other application's window for #398
  QRect screenGeometry = m_mainWindow.geometry();
  int x = screenGeometry.x() + (screenGeometry.width () - width ()) / 2;
  int y = screenGeometry.y() + (screenGeometry.height () - height ()) / 2;

  // In Mint 20 the (x,y) point must be passed through mapToGlobal at this point for this dialog to appear in the
  // middle of MainWindow (rather than at the top left of the screen). Since it is not known which Linux distribution,
  // Linux version and Qt version combinations exhibit this Qt bug, no workaround will be attempted. It helps
  // that Windows is the most popular OS and it does not exhibit this Qt bug, which conflicts with the documentation.
  
  move (x, y);

  QWizard::showEvent (event);
}

QString ChecklistGuideWizard::templateHtml (CoordSystemIndex coordSystemIndex) const
{

  QStringList curveNames = m_pageCurves [signed (coordSystemIndex)]->curveNames();
  bool withLines = m_pageCurves [signed (coordSystemIndex)]->withLines();

  QString html;
  QTextStream str (&html);

  // Insert additional space between successive list items, using stylesheet
  str << "<style type='text/css'>li { margin-top: 10px 0; }</style>";

  str << "<p>" << tr ("Follow this checklist of steps to digitize your image. Each step will show a check when it has been completed.");
  str << "<p>" << tr ("The coordinates are defined by creating axis points") << ":</p>\n";
  str << "<p>" << TAG_ANCHOR_AXIS_1 << "&nbsp;" << tr ("Add first of three axis points.") << " " << TAG_AHREF_AXIS_1 << "</p>\n";
  str << TAG_DIV_AXIS_START_1;
  str << "<ul>\n";
  str << "<li>" << tr ("Click on") << " <img src="":/engauge/img/16-DigitAxis""> " << tr ("for Axis Points mode") << "</li>\n";
  str << "<li>" << tr ("Click on an axis tick mark, or intersection of two grid lines, with labeled coordinates") << "</li>\n";
  str << "<li>" << tr ("Enter the coordinates of the axis point") << "</li>\n";
  str << "<li>" << tr ("Click on Ok") << "</li>\n";
  str << "</ul>\n";
  str << TAG_DIV_AXIS_END_1;
  str << "<p>" << TAG_ANCHOR_AXIS_2 << "&nbsp;" << tr ("Add second of three axis points.") << " " << TAG_AHREF_AXIS_2 << "</p>\n";
  str << TAG_DIV_AXIS_START_2;
  str << "<ul>\n";
  str << "<li>" << tr ("Click on") << " <img src="":/engauge/img/16-DigitAxis""> " << tr ("for Axis Points mode") << "</li>\n";
  str << "<li>" << tr ("Click on an axis tick mark, or intersection of two grid lines, with labeled coordinates, away from the other axis point") << "</li>\n";
  str << "<li>" << tr ("Enter the coordinates of the axis point") << "</li>\n";
  str << "<li>" << tr ("Click on Ok") << "</li>\n";
  str << "</ul>\n";
  str << TAG_DIV_AXIS_END_2;
  str << "<p>" << TAG_ANCHOR_AXIS_3 << "&nbsp;" << tr ("Add third of three axis points.") << " " << TAG_AHREF_AXIS_3 << "</p>\n";
  str << TAG_DIV_AXIS_START_3;
  str << "<ul>\n";
  str << "<li>" << tr ("Click on") << " <img src="":/engauge/img/16-DigitAxis""> " << tr ("for Axis Points mode") << "</li>\n";
  str << "<li>" << tr ("Click on an axis tick mark, or intersection of two grid lines, with labeled coordinates, away from the other axis points") << "</li>\n";
  str << "<li>" << tr ("Enter the coordinates of the axis point") << "</li>\n";
  str << "<li>" << tr ("Click on Ok") << "</li>\n";
  str << "</ul>\n";
  str << TAG_DIV_AXIS_END_3;

  str << "<p>&nbsp;</p>\n";
  str << "<p>" << tr ("Points are digitized along each curve") << ":</p>\n";
  QStringList::const_iterator itr;
  for (itr = curveNames.begin(); itr != curveNames.end(); itr++) {

    QString curveName = *itr;

    QString tagACurve = QString ("%1%2%3")
                        .arg (TAG_AHREF_DELIMITER_START)
                        .arg (curveName)
                        .arg (TAG_AHREF_DELIMITER_END);

    str << "<p>" << TAG_ANCHOR_DELIMITER_START << curveName << TAG_ANCHOR_DELIMITER_END
        << "&nbsp;" << tr ("Add points for curve") << " <b>" << curveName << "</b>. " << tagACurve << "</p>\n";
    str << TAG_DIV_DELIMITER_START << curveName << TAG_DIV_DELIMITER_END;
    str << "<ul>\n";

    if (withLines) {

      str << "<li>" << tr ("Click on") << " <img src="":/engauge/img/16-DigitSegment""> " << tr ("for Segment Fill mode") << "</li>\n";
      str << "<li>" << tr ("Select curve") << " <b>" << curveName << "</b> " << tr ("in the drop-down list") << "</li>\n";
      str << "<li>" << tr ("Move the cursor over the curve. If a line does not appear then adjust the Color Filter settings for this curve") << ":\n";
      str << templateHtmlToAdjustColorFilterSettings () << "</li>\n";
      str << "<li>" << tr ("Move the cursor over the curve again. When the Segment Fill line appears, click on it to generate points") << "</li>\n";
      str << "</ul>\n";

    } else {

      str << "<li>" << tr ("Click on") << " <img src="":/engauge/img/16-DigitPointMatch""> " << tr ("for Point Match mode") << "</li>\n";
      str << "<li>" << tr ("Select curve") << " <b>" << curveName << "</b> " << tr ("in the drop-down list") << "</li>\n";
      str << "<li>" << tr ("Move the cursor over a typical point in the curve. If the cursor circle does not change color then adjust "
                           "the Color Filter settings for this curve") << ":\n";
      str << templateHtmlToAdjustColorFilterSettings () << "</li>\n";
      str << "<li>" << tr ("Move the cursor over a typical point in the curve again. Click on the point to start point matching") << "</li>\n";
      str << "<li>" << tr ("Engauge will display a candidate point. To accept that candidate point, press the right arrow key") << "</li>\n";
      str << "<li>" << tr ("The previous step repeats until you select a different mode") << "</li>\n";
      str << "</ul>\n";
    }

    str << TAG_DIV_DELIMITER_START_SLASH << curveName << TAG_DIV_DELIMITER_END;
  }

  str << "<p>&nbsp;</p>\n";
  str << "<p>" << tr ("The digitized points can be exported") << ":</p>\n";
  str << "<p>" << TAG_ANCHOR_EXPORT << "&nbsp;" << tr ("Export the points to a file") << ". " << TAG_AHREF_EXPORT << "</p>\n";
  str << TAG_DIV_EXPORT_START;
  str << "<ul>\n";
  str << "<li>" << tr ("Select menu option File / Export") << "</li>\n";
  str << "<li>" << tr ("Enter the file name") << "</li>\n";
  str << "<li>" << tr ("Click on Ok") << "</li>\n";
  str << "<li>" << tr ("Congratulations!") << "</li>\n";
  str << "</ul>\n";
  str << TAG_DIV_EXPORT_END;

  str << "<p>&nbsp;</p>\n";
  str << "<p>" << tr ("Hint - The background image can be switched between the original image and filtered image.") << " " << TAG_AHREF_BACKGROUND << "</p>\n";
  str << TAG_DIV_BACKGROUND_START;
  str << "<ul>\n";
  str << "<li>" << tr ("Select menu option View / Background / Show Original Image to see the original image") << "</li>\n";
  str << "<li>" << tr ("Select menu option View / Background / Show Filtered Image to see the image from Color Filter") << "</li>\n";
  str << "</ul>\n";
  str << TAG_DIV_BACKGROUND_END;

  return html;
}

QString ChecklistGuideWizard::templateHtmlToAdjustColorFilterSettings () const
{

  QString html;
  QTextStream str (&html);

  str << "  <ul>\n";
  str << "  <li>" << tr ("Select menu option Settings / Color Filter") << "<li>\n";
  str << "  <li>" << tr ("Select the method for filtering. Hue is best if the curves have different colors") << "</li>\n";
  str << "  <li>" << tr ("Slide the green buttons back and forth until the curve is easily visible in the preview window") << "</li>\n";
  str << "  <li>" << tr ("Click on Ok") << "</li>\n";
  str << "  </ul>\n";

  return html;
}
