/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#include "ChecklistGuide.h"
#include "ChecklistGuideBrowser.h"
#include "CmdMediator.h"
#include "EngaugeAssert.h"
#include "Logger.h"
#include <QTextBrowser>

ChecklistGuide::ChecklistGuide (QWidget *parent) :
  QDockWidget (parent),
  m_browser (nullptr)
{
  setVisible (false);
  setAllowedAreas (Qt::AllDockWidgetAreas);
  setWindowTitle (tr ("Checklist Guide")); // Appears in title bar when undocked
  setStatusTip (tr ("Checklist Guide"));
  setWhatsThis (tr ("Checklist Guide\n\n"
                    "This box contains a checklist of steps suggested by the Checklist Guide Wizard. Following "
                    "these steps should produce a set of digitized points in an output file.\n\n"
                    "To run the Checklist Guide Wizard when an image file is imported, select the "
                    "Help / Checklist Wizard menu option."));

  m_browser = new ChecklistGuideBrowser;
  setWidget (m_browser);
}

bool ChecklistGuide::browserIsEmpty () const
{
  return m_browser->toPlainText().isEmpty();
}

void ChecklistGuide::closeEvent(QCloseEvent * /* event */)
{

  emit signalChecklistClosed();
}

void ChecklistGuide::setTemplateHtml (const QString &html,
                                      const QStringList &curveNames)
{

  m_browser->setTemplateHtml (html,
                              curveNames);
}

void ChecklistGuide::update (const CmdMediator &cmdMediator,
                             bool documentIsExported)
{

  ENGAUGE_CHECK_PTR (m_browser);

  m_browser->update (cmdMediator,
                     documentIsExported);
}
