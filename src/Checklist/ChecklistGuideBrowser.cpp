/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#include "ChecklistGuideBrowser.h"
#include "ChecklistTemplate.h"
#include "CmdMediator.h"
#include "Document.h"
#include "EngaugeAssert.h"
#include "Logger.h"
#include <QDebug>
#include <QRegularExpression>

const int MIN_WIDTH_BROWSER = 340; // Make just big enough that each "More..." appears on same line

ChecklistGuideBrowser::ChecklistGuideBrowser ()
{
  setOpenLinks (false); // Disable automatic link following
  setMinimumWidth(MIN_WIDTH_BROWSER);

  connect (this, SIGNAL (anchorClicked (const QUrl &)), this, SLOT (slotAnchorClicked (const QUrl &)));
}

QString ChecklistGuideBrowser::ahref (QString &html,
                                      const QString &name) const
{
    QString expression = QString("%1%2%3").arg(TAG_AHREF_DELIMITER_START,
                                               name,
                                               TAG_AHREF_DELIMITER_END);

    QString link;
    if (name == m_anchor) {
        // Click on this hyperlink to reload the page without details under this link, since anchor is empty
        link = QString("<a href="
                       "#"
                       ">Less ...</a>");

    } else {
        // Click on this hyperlink to reload the page with details under this link
        link = QString("<a href="
                       "#%1"
                       ">More ...</a>")
                   .arg(name);
    }

  html.replace (expression, link);

  return html;
}

void ChecklistGuideBrowser::check (QString &html,
                                   const QString &anchor,
                                   bool isChecked) const
{
    QString tag = QString("%1%2%3").arg(TAG_ANCHOR_DELIMITER_START,
                                        anchor,
                                        TAG_ANCHOR_DELIMITER_END);

    if (isChecked) {
        html.replace(tag,
                     "<img src="
                     ":/engauge/img/16-checked.png"
                     ">");
    } else {
        html.replace(tag,
                     "<img src="
                     ":/engauge/img/16-unchecked.png"
                     ">");
    }
}

void ChecklistGuideBrowser::divHide (QString &html,
                                     const QString &anchor) const
{

  // Remove everything between the start and end tags, inclusive
  QString expression = QString("\\%1%2\\%3.*\\%4%5\\%6")
                           .arg(TAG_DIV_DELIMITER_START,
                                anchor,
                                TAG_DIV_DELIMITER_END,
                                TAG_DIV_DELIMITER_START_SLASH,
                                anchor,
                                TAG_DIV_DELIMITER_END);
  QRegularExpression regExp(expression);
  html.replace(regExp, "");
}

void ChecklistGuideBrowser::divShow(QString &html, const QString &anchor) const
{

  if (!anchor.isEmpty ()) {

    // Remove the start and end tags, but leave the text in between
    QString expressionStart = QString("\\%1%2\\%3")
                                  .arg(TAG_DIV_DELIMITER_START, anchor, TAG_DIV_DELIMITER_END);
    QString expressionEnd = QString("\\%1%2\\%3")
                                .arg(TAG_DIV_DELIMITER_START_SLASH, anchor, TAG_DIV_DELIMITER_END);
    QRegularExpression regExpStart(expressionStart);
    QRegularExpression regExpEnd(expressionEnd);
    html.replace (regExpStart, "");
    html.replace (regExpEnd, "");
  }
}

QString ChecklistGuideBrowser::processAhrefs (const QString &htmlBefore)
{

  QString html = htmlBefore;

  // Background
  ahref (html, NAME_BACKGROUND);

  // Fixed axis point hrefs
  ahref (html, NAME_AXIS1);
  ahref (html, NAME_AXIS2);
  ahref (html, NAME_AXIS3);

  // Curves
  QStringList::const_iterator itr;
  for (itr = m_curveNames.begin(); itr != m_curveNames.end(); itr++) {

    QString curveName = *itr;
    ahref (html, curveName);
  }

  // Export
  ahref (html, NAME_EXPORT);

  return html;
}

QString ChecklistGuideBrowser::processCheckboxes (const QString &htmlBefore)
{

  QString html = htmlBefore;

  check (html, NAME_BACKGROUND, m_checkedTags.contains (NAME_BACKGROUND));

  check (html, NAME_AXIS1, m_checkedTags.contains (NAME_AXIS1));
  check (html, NAME_AXIS2, m_checkedTags.contains (NAME_AXIS2));
  check (html, NAME_AXIS3, m_checkedTags.contains (NAME_AXIS3));

  // Curves
  QStringList::const_iterator itr;
  for (itr = m_curveNames.begin(); itr != m_curveNames.end(); itr++) {

    QString curveName = *itr;
    check (html, curveName, m_checkedTags.contains (curveName));
  }

  check (html, NAME_EXPORT, m_checkedTags.contains (NAME_EXPORT));

  return html;
}

QString ChecklistGuideBrowser::processDivs (const QString &htmlBefore)
{

  QString html = htmlBefore;

  // Show div associated with anchor. Since this removes the tags, applying divHide below
  // will have no effect, so we apply divHide to everything
  divShow (html, m_anchor);

  // Background
  divHide (html, NAME_BACKGROUND);

  // Fixed axis point tags
  divHide (html, NAME_AXIS1);
  divHide (html, NAME_AXIS2);
  divHide (html, NAME_AXIS3);

  // Curve name tags
  QStringList::const_iterator itr;
  for (itr = m_curveNames.begin(); itr != m_curveNames.end(); itr++) {

    QString curveName = *itr;
    divHide (html, curveName);
  }

  divHide (html, NAME_EXPORT);

  return html;
}

void ChecklistGuideBrowser::refresh ()
{

  QString html = m_templateHtml;

  html = processAhrefs (html);
  html = processCheckboxes (html);
  html = processDivs (html);

  QTextBrowser::setHtml (html);
}

void ChecklistGuideBrowser::repopulateCheckedTags (const CmdMediator &cmdMediator,
                                                   bool documentIsExported)
{

  m_checkedTags.clear();

  if (cmdMediator.document().curveAxes().numPoints() > 0) {
    m_checkedTags [NAME_AXIS1] = true;
  }

  if (cmdMediator.document().curveAxes().numPoints() > 1) {
    m_checkedTags [NAME_AXIS2] = true;
  }

  if (cmdMediator.document().curveAxes().numPoints() > 2) {
    m_checkedTags [NAME_AXIS3] = true;
  }

  // Curves
  QStringList::const_iterator itr;
  for (itr = m_curveNames.begin(); itr != m_curveNames.end(); itr++) {

    QString curveName = *itr;
    if (cmdMediator.document().curvesGraphsNumPoints (curveName) > 0) {
      m_checkedTags [curveName] = true;
    }
  }

  // The export case is very tricky. The modified/dirty flag tells us when there are unsaved points,
  // but for some reason the value returned by CmdMediator.isModified (which is QUndoStack::isClean)
  // is often wrong at this point in execution. So we use a different trick - asking MainWindow if file has
  // been saved
  if (documentIsExported) {
    m_checkedTags [NAME_EXPORT] = true;
  }
}

void ChecklistGuideBrowser::setTemplateHtml (const QString &html,
                                             const QStringList &curveNames)
{
  m_templateHtml = html;
  m_curveNames = curveNames;

  refresh();
}

void ChecklistGuideBrowser::slotAnchorClicked (const QUrl &url)
{

  m_anchor = "";
  if (url.hasFragment ()) {
    m_anchor = url.fragment ();
  }

  refresh();
}

void ChecklistGuideBrowser::update (const CmdMediator &cmdMediator,
                                    bool documentIsExported)
{

  repopulateCheckedTags(cmdMediator,
                        documentIsExported);

  refresh();
}
