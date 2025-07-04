/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#include "ButtonWhatsThis.h"
#include "CmdMediator.h"
#include "CmdSettingsGridRemoval.h"
#include "DlgSettingsGridRemoval.h"
#include "EngaugeAssert.h"
#include "GridInitializer.h"
#include "GridRemoval.h"
#include "Logger.h"
#include "MainWindow.h"
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleValidator>
#include <QGraphicsScene>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPixmap>
#include <QWhatsThis>
#include "ViewPreview.h"

const double CLOSE_DISTANCE_MAX = 64;
const double CLOSE_DISTANCE_MIN = 0;
const int CLOSE_DECIMALS = 1;
const int COUNT_MIN = 1;
const int COUNT_MAX = 100;
const int COUNT_DECIMALS = 0;
const int MINIMUM_HEIGHT = 520;

DlgSettingsGridRemoval::DlgSettingsGridRemoval(MainWindow &mainWindow) :
  DlgSettingsAbstractBase (tr ("Grid Removal"),
                           "DlgSettingsGridRemoval",
                           mainWindow),
  m_validatorCloseDistance (nullptr),
  m_validatorCountX (nullptr),
  m_validatorStartX (nullptr),
  m_validatorStepX (nullptr),
  m_validatorStopX (nullptr),      
  m_validatorCountY (nullptr),
  m_validatorStartY (nullptr),
  m_validatorStepY (nullptr),
  m_validatorStopY (nullptr),  
  m_scenePreview (nullptr),
  m_viewPreview (nullptr),
  m_modelGridRemovalBefore (nullptr),
  m_modelGridRemovalAfter (nullptr)
{

  QWidget *subPanel = createSubPanel ();
  finishPanel (subPanel);
}

DlgSettingsGridRemoval::~DlgSettingsGridRemoval()
{

  delete m_validatorCloseDistance;
  delete m_validatorCountX;
  delete m_validatorStartX;
  delete m_validatorStepX;
  delete m_validatorStopX;      
  delete m_validatorCountY;
  delete m_validatorStartY;
  delete m_validatorStepY;
  delete m_validatorStopY;
}

void DlgSettingsGridRemoval::createOptionalSaveDefault (QHBoxLayout * /* layout */)
{
}

void DlgSettingsGridRemoval::createPreview (QGridLayout *layout, int &row)
{

  QLabel *labelPreview = new QLabel (tr ("Preview"));
  layout->addWidget (labelPreview, row++, 0, 1, 5);

  m_scenePreview = new QGraphicsScene (this);
  m_viewPreview = new ViewPreview (m_scenePreview,
                                   ViewPreview::VIEW_ASPECT_RATIO_VARIABLE,
                                   this);
  m_viewPreview->setWhatsThis (tr ("Preview window that shows how current settings affect grid removal"));
  m_viewPreview->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_viewPreview->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_viewPreview->setMinimumHeight (MINIMUM_PREVIEW_HEIGHT);
  layout->addWidget (m_viewPreview, row++, 0, 1, 5);
}

void DlgSettingsGridRemoval::createRemoveGridLines (QGridLayout *layout, int &row)
{

  m_chkRemoveGridLines = new QCheckBox (tr ("Remove pixels close to defined grid lines"));
  m_chkRemoveGridLines->setWhatsThis (tr ("Check this box to have pixels close to regularly spaced gridlines removed.\n\n"
                                          "This option is only available when the axis points have all been defined."));
  connect (m_chkRemoveGridLines, SIGNAL (stateChanged (int)), this, SLOT (slotRemoveGridLines (int)));
  layout->addWidget (m_chkRemoveGridLines, row++, 1, 1, 3);

  QLabel *labelCloseDistance = new QLabel (QString ("%1:").arg (tr ("Close distance (pixels)")));
  layout->addWidget (labelCloseDistance, row, 2);

  m_editCloseDistance = new QLineEdit;
  m_editCloseDistance->setWhatsThis (tr ("Set closeness distance in pixels.\n\n"
                                         "Pixels that are closer to the regularly spaced gridlines, than this distance, "
                                         "will be removed.\n\n"
                                         "This value cannot be negative. A zero value disables this feature. Decimal values are allowed"));
  m_validatorCloseDistance = new QDoubleValidator (CLOSE_DISTANCE_MIN, CLOSE_DISTANCE_MAX, CLOSE_DECIMALS);
  m_editCloseDistance->setValidator (m_validatorCloseDistance);
  connect (m_editCloseDistance, SIGNAL (textChanged (const QString &)), this, SLOT (slotCloseDistance (const QString &)));
  layout->addWidget (m_editCloseDistance, row++, 3);

  createRemoveGridLinesX (layout, row);
  createRemoveGridLinesY (layout, row);
}

void DlgSettingsGridRemoval::createRemoveGridLinesX (QGridLayout *layout, int &row)
{

  QString titleX = tr ("X Grid Lines");
  if (false) {
    titleX = QString (QChar (0x98, 0x03)) + QString (" %1").arg (tr ("Grid Lines"));
  }
  QGroupBox *groupX = new QGroupBox (titleX);
  layout->addWidget (groupX, row, 2);

  QGridLayout *layoutGroup = new QGridLayout;
  groupX->setLayout (layoutGroup);

  QLabel *labelDisable = new QLabel (QString ("%1:").arg (tr ("Disable")));
  layoutGroup->addWidget (labelDisable, 0, 0);

  m_cmbDisableX = new QComboBox;
  m_cmbDisableX->setWhatsThis (tr ("Disabled value.\n\n"
                                   "The X grid lines are specified using only three values at a time. For flexibility, four values "
                                   "are offered so you must chose which value is disabled. Once disabled, that value is simply "
                                   "updated as the other values change"));
  m_cmbDisableX->addItem(gridCoordDisableToString (GRID_COORD_DISABLE_COUNT),
                         QVariant (GRID_COORD_DISABLE_COUNT));
  m_cmbDisableX->addItem(gridCoordDisableToString (GRID_COORD_DISABLE_START),
                         QVariant (GRID_COORD_DISABLE_START));
  m_cmbDisableX->addItem(gridCoordDisableToString (GRID_COORD_DISABLE_STEP),
                         QVariant (GRID_COORD_DISABLE_STEP));
  m_cmbDisableX->addItem(gridCoordDisableToString (GRID_COORD_DISABLE_STOP),
                         QVariant (GRID_COORD_DISABLE_STOP));
  connect (m_cmbDisableX, SIGNAL (activated (const QString &)), this, SLOT (slotDisableX (const QString &))); // activated() ignores code changes
  layoutGroup->addWidget (m_cmbDisableX, 0, 1);

  QLabel *labelCount = new QLabel (QString ("%1:").arg (tr ("Count")));
  layoutGroup->addWidget (labelCount, 1, 0);

  m_editCountX = new QLineEdit;
  m_editCountX->setWhatsThis (tr ("Number of X grid lines.\n\n"
                                  "The number of X grid lines must be entered as an integer greater than zero"));
  m_validatorCountX = new QDoubleValidator (COUNT_MIN, COUNT_MAX, COUNT_DECIMALS);
  m_editCountX->setValidator (m_validatorCountX);
  connect (m_editCountX, SIGNAL (textChanged (const QString &)), this, SLOT  (slotCountX (const QString &)));
  layoutGroup->addWidget (m_editCountX, 1, 1);

  QLabel *labelStart = new QLabel (QString ("%1:").arg (tr ("Start")));
  layoutGroup->addWidget (labelStart, 2, 0);

  m_editStartX = new QLineEdit;
  m_editStartX->setWhatsThis (tr ("Value of the first X grid line.\n\n"
                                  "The start value cannot be greater than the stop value"));
  m_validatorStartX = new QDoubleValidator;
  m_editStartX->setValidator (m_validatorStartX);
  connect (m_editStartX, SIGNAL (textChanged (const QString &)), this, SLOT  (slotStartX (const QString &)));
  layoutGroup->addWidget (m_editStartX, 2, 1);

  QLabel *labelStep = new QLabel (QString ("%1:").arg (tr ("Step")));
  layoutGroup->addWidget (labelStep, 3, 0);

  m_editStepX = new QLineEdit;
  m_editStepX->setWhatsThis (tr ("Difference in value between two successive X grid lines.\n\n"
                                 "The step value must be greater than zero (linear) or one (log)"));
  m_validatorStepX = new QDoubleValidator;
  m_editStepX->setValidator (m_validatorStepX);
  connect (m_editStepX, SIGNAL (textChanged (const QString &)), this, SLOT  (slotStepX (const QString &)));
  layoutGroup->addWidget (m_editStepX, 3, 1);

  QLabel *labelStop = new QLabel (QString ("%1:").arg (tr ("Stop")));
  layoutGroup->addWidget (labelStop, 4, 0);

  m_editStopX = new QLineEdit;
  m_editStopX->setWhatsThis (tr ("Value of the last X grid line.\n\n"
                                 "The stop value cannot be less than the start value"));
  m_validatorStopX = new QDoubleValidator;
  m_editStopX->setValidator (m_validatorStopX);
  connect (m_editStopX, SIGNAL (textChanged (const QString &)), this, SLOT  (slotStopX (const QString &)));
  layoutGroup->addWidget (m_editStopX, 4, 1);
}

void DlgSettingsGridRemoval::createRemoveGridLinesY (QGridLayout *layout, int &row)
{

  QString titleY = tr ("Y Grid Lines");
  if (false) {
    titleY = QString (tr ("R Grid Lines"));
  }
  QGroupBox *groupY = new QGroupBox (titleY);
  layout->addWidget (groupY, row++, 3);

  QGridLayout *layoutGroup = new QGridLayout;
  groupY->setLayout (layoutGroup);

  QLabel *labelDisable = new QLabel (QString ("%1:").arg (tr ("Disable")));
  layoutGroup->addWidget (labelDisable, 0, 0);

  m_cmbDisableY = new QComboBox;
  m_cmbDisableY->setWhatsThis (tr ("Disabled value.\n\n"
                                   "The Y grid lines are specified using only three values at a time. For flexibility, four values "
                                   "are offered so you must chose which value is disabled. Once disabled, that value is simply "
                                   "updated as the other values change"));
  m_cmbDisableY->addItem(gridCoordDisableToString (GRID_COORD_DISABLE_COUNT),
                         QVariant (GRID_COORD_DISABLE_COUNT));
  m_cmbDisableY->addItem(gridCoordDisableToString (GRID_COORD_DISABLE_START),
                         QVariant (GRID_COORD_DISABLE_START));
  m_cmbDisableY->addItem(gridCoordDisableToString (GRID_COORD_DISABLE_STEP),
                         QVariant (GRID_COORD_DISABLE_STEP));
  m_cmbDisableY->addItem(gridCoordDisableToString (GRID_COORD_DISABLE_STOP),
                         QVariant (GRID_COORD_DISABLE_STOP));
  connect (m_cmbDisableY, SIGNAL (activated (const QString &)), this, SLOT (slotDisableY (const QString &))); // activated() ignores code changes
  layoutGroup->addWidget (m_cmbDisableY, 0, 1);

  QLabel *labelCount = new QLabel (QString ("%1:").arg (tr ("Count")));
  layoutGroup->addWidget (labelCount, 1, 0);

  m_editCountY = new QLineEdit;
  m_editCountY->setWhatsThis (tr ("Number of Y grid lines.\n\n"
                                  "The number of Y grid lines must be entered as an integer greater than zero"));
  m_validatorCountY = new QDoubleValidator (COUNT_MIN, COUNT_MAX, COUNT_DECIMALS);
  m_editCountY->setValidator (m_validatorCountY);
  connect (m_editCountY, SIGNAL (textChanged (const QString &)), this, SLOT  (slotCountY (const QString &)));
  layoutGroup->addWidget (m_editCountY, 1, 1);

  QLabel *labelStart = new QLabel (QString ("%1:").arg (tr ("Start")));
  layoutGroup->addWidget (labelStart, 2, 0);

  m_editStartY = new QLineEdit;
  m_editStartY->setWhatsThis (tr ("Value of the first Y grid line.\n\n"
                                  "The start value cannot be greater than the stop value"));
  m_validatorStartY = new QDoubleValidator;
  m_editStartY->setValidator (m_validatorStartY);
  connect (m_editStartY, SIGNAL (textChanged (const QString &)), this, SLOT  (slotStartY (const QString &)));
  layoutGroup->addWidget (m_editStartY, 2, 1);

  QLabel *labelStep = new QLabel (QString ("%1:").arg (tr ("Step")));
  layoutGroup->addWidget (labelStep, 3, 0);

  m_editStepY = new QLineEdit;
  m_editStepY->setWhatsThis (tr ("Difference in value between two successive Y grid lines.\n\n"
                                 "The step value must be greater than zero (linear) or one (log)"));
  m_validatorStepY = new QDoubleValidator;
  m_editStepY->setValidator (m_validatorStepY);
  connect (m_editStepY, SIGNAL (textChanged (const QString &)), this, SLOT  (slotStepY (const QString &)));
  layoutGroup->addWidget (m_editStepY, 3, 1);

  QLabel *labelStop = new QLabel (QString ("%1:").arg (tr ("Stop")));
  layoutGroup->addWidget (labelStop, 4, 0);

  m_editStopY = new QLineEdit;
  m_editStopY->setWhatsThis (tr ("Value of the last Y grid line.\n\n"
                                 "The stop value cannot be less than the start value"));
  m_validatorStopY = new QDoubleValidator;
  m_editStopY->setValidator (m_validatorStopY);
  connect (m_editStopY, SIGNAL (textChanged (const QString &)), this, SLOT  (slotStopY (const QString &)));
  layoutGroup->addWidget (m_editStopY, 4, 1);
}

QWidget *DlgSettingsGridRemoval::createSubPanel ()
{

  QWidget *subPanel = new QWidget ();
  QGridLayout *layout = new QGridLayout (subPanel);
  subPanel->setLayout (layout);

  layout->setColumnStretch(0, 1); // Empty first column
  layout->setColumnStretch(1, 0); // Checkbox part of "section" checkboxes. In other rows this has empty space as indentation
  layout->setColumnStretch(2, 0); // X
  layout->setColumnStretch(3, 0); // Y
  layout->setColumnStretch(4, 1); // Empty last column

  int row = 0;

  createWhatsThis (layout,
                   m_btnWhatsThis,
                   row++,
                   4);

  createRemoveGridLines (layout, row);
  createPreview (layout, row);

  return subPanel;
}

void DlgSettingsGridRemoval::handleOk ()
{

  // Set the stable flag
  m_modelGridRemovalAfter->setStable ();

  CmdSettingsGridRemoval *cmd = new CmdSettingsGridRemoval (mainWindow (),
                                                            cmdMediator ().document(),
                                                            *m_modelGridRemovalBefore,
                                                            *m_modelGridRemovalAfter);
  cmdMediator ().push (cmd);

  hide ();
}

void DlgSettingsGridRemoval::load (CmdMediator &cmdMediator)
{

  setCmdMediator (cmdMediator);

  // Flush old data
  delete m_modelGridRemovalBefore;
  delete m_modelGridRemovalAfter;

  // Save new data
  m_modelGridRemovalBefore = new DocumentModelGridRemoval (cmdMediator.document());
  m_modelGridRemovalAfter = new DocumentModelGridRemoval (cmdMediator.document());

  // Sanity checks. Incoming defaults must be acceptable to the local limits
  ENGAUGE_ASSERT (CLOSE_DISTANCE_MIN <= m_modelGridRemovalAfter->closeDistance());
  ENGAUGE_ASSERT (CLOSE_DISTANCE_MAX >= m_modelGridRemovalAfter->closeDistance());

  // Populate controls
  m_chkRemoveGridLines->setChecked (m_modelGridRemovalAfter->removeDefinedGridLines());

  m_editCloseDistance->setText (QString::number (m_modelGridRemovalAfter->closeDistance()));

  int indexDisableX = m_cmbDisableX->findData (QVariant (m_modelGridRemovalAfter->gridCoordDisableX()));
  m_cmbDisableX->setCurrentIndex (indexDisableX);

  m_editCountX->setText(QString::number(m_modelGridRemovalAfter->countX()));
  m_editStartX->setText(QString::number(m_modelGridRemovalAfter->startX()));
  m_editStepX->setText(QString::number(m_modelGridRemovalAfter->stepX()));
  m_editStopX->setText(QString::number(m_modelGridRemovalAfter->stopX()));

  int indexDisableY = m_cmbDisableY->findData (QVariant (m_modelGridRemovalAfter->gridCoordDisableY()));
  m_cmbDisableY->setCurrentIndex (indexDisableY);

  m_editCountY->setText(QString::number(m_modelGridRemovalAfter->countY()));
  m_editStartY->setText(QString::number(m_modelGridRemovalAfter->startY()));
  m_editStepY->setText(QString::number(m_modelGridRemovalAfter->stepY()));
  m_editStopY->setText(QString::number(m_modelGridRemovalAfter->stopY()));

  updateControls ();
  enableOk (false); // Disable Ok button since there not yet any changes
  updatePreview();
}

void DlgSettingsGridRemoval::setSmallDialogs(bool smallDialogs)
{
  if (!smallDialogs) {
    setMinimumHeight (MINIMUM_HEIGHT);
  }
}

void DlgSettingsGridRemoval::slotCloseDistance(const QString &)
{

  m_modelGridRemovalAfter->setCloseDistance(m_editCloseDistance->text().toDouble());
  updateControls ();
  updatePreview();
}

void DlgSettingsGridRemoval::slotCountX(const QString &count)
{

  m_modelGridRemovalAfter->setCountX(count.toInt());
  updateDisplayedVariableX ();
  updateControls ();
  updatePreview();
}

void DlgSettingsGridRemoval::slotCountY(const QString &count)
{

  m_modelGridRemovalAfter->setCountY(count.toInt());
  updateDisplayedVariableY ();
  updateControls ();
  updatePreview();
}

void DlgSettingsGridRemoval::slotDisableX(const QString &)
{

  GridCoordDisable gridCoordDisable = static_cast<GridCoordDisable> (m_cmbDisableX->currentData().toInt());
  m_modelGridRemovalAfter->setGridCoordDisableX(gridCoordDisable);
  updateDisplayedVariableX ();
  updateControls();
  updatePreview();
}

void DlgSettingsGridRemoval::slotDisableY(const QString &)
{

  GridCoordDisable gridCoordDisable = static_cast<GridCoordDisable> (m_cmbDisableY->currentData().toInt());
  m_modelGridRemovalAfter->setGridCoordDisableY(gridCoordDisable);
  updateDisplayedVariableY ();
  updateControls();
  updatePreview();
}

void DlgSettingsGridRemoval::slotRemoveGridLines (int state)
{

  m_modelGridRemovalAfter->setRemoveDefinedGridLines(state == Qt::Checked);
  updateControls();
  updatePreview();
}

void DlgSettingsGridRemoval::slotStartX(const QString &startX)
{

  m_modelGridRemovalAfter->setStartX(startX.toDouble());
  updateDisplayedVariableX ();
  updateControls();
  updatePreview();
}

void DlgSettingsGridRemoval::slotStartY(const QString &startY)
{

  m_modelGridRemovalAfter->setStartY(startY.toDouble());
  updateDisplayedVariableY ();
  updateControls();
  updatePreview();
}

void DlgSettingsGridRemoval::slotStepX(const QString &stepX)
{

  m_modelGridRemovalAfter->setStepX(stepX.toDouble());
  updateDisplayedVariableX ();
  updateControls();
  updatePreview();
}

void DlgSettingsGridRemoval::slotStepY(const QString &stepY)
{

  m_modelGridRemovalAfter->setStepY(stepY.toDouble());
  updateDisplayedVariableY ();
  updateControls();
  updatePreview();
}

void DlgSettingsGridRemoval::slotStopX(const QString &stopX)
{

  m_modelGridRemovalAfter->setStopX(stopX.toDouble());
  updateDisplayedVariableX ();
  updateControls();
  updatePreview();
}

void DlgSettingsGridRemoval::slotStopY(const QString &stopY)
{

  m_modelGridRemovalAfter->setStopY(stopY.toDouble());
  updateDisplayedVariableY ();
  updateControls();
  updatePreview();
}

void DlgSettingsGridRemoval::slotWhatsThis ()
{
  QWhatsThis::enterWhatsThisMode();
}

void DlgSettingsGridRemoval::updateControls ()
{
  m_editCloseDistance->setEnabled (m_chkRemoveGridLines->isChecked ());

  m_cmbDisableX->setEnabled (m_chkRemoveGridLines->isChecked ());

  GridCoordDisable disableX = static_cast<GridCoordDisable> (m_cmbDisableX->currentData().toInt());
  m_editCountX->setEnabled (m_chkRemoveGridLines->isChecked () && (disableX != GRID_COORD_DISABLE_COUNT));
  m_editStartX->setEnabled (m_chkRemoveGridLines->isChecked () && (disableX != GRID_COORD_DISABLE_START));
  m_editStepX->setEnabled (m_chkRemoveGridLines->isChecked () && (disableX != GRID_COORD_DISABLE_STEP));
  m_editStopX->setEnabled (m_chkRemoveGridLines->isChecked () && (disableX != GRID_COORD_DISABLE_STOP));

  m_cmbDisableY->setEnabled (m_chkRemoveGridLines->isChecked ());

  GridCoordDisable disableY = static_cast<GridCoordDisable> (m_cmbDisableY->currentData().toInt());
  m_editCountY->setEnabled (m_chkRemoveGridLines->isChecked () && (disableY != GRID_COORD_DISABLE_COUNT));
  m_editStartY->setEnabled (m_chkRemoveGridLines->isChecked () && (disableY != GRID_COORD_DISABLE_START));
  m_editStepY->setEnabled (m_chkRemoveGridLines->isChecked () && (disableY != GRID_COORD_DISABLE_STEP));
  m_editStopY->setEnabled (m_chkRemoveGridLines->isChecked () && (disableY != GRID_COORD_DISABLE_STOP));

  QString textCloseDistance = m_editCloseDistance->text();
  QString textCountX = m_editCountX->text();
  QString textStartX = m_editStartX->text();
  QString textStepX = m_editStepX->text();
  QString textStopX = m_editStopX->text();
  QString textCountY = m_editCountY->text();
  QString textStartY = m_editStartY->text();
  QString textStepY = m_editStepY->text();
  QString textStopY = m_editStopY->text();

  int pos;
  bool isOk = (m_validatorCloseDistance->validate (textCloseDistance, pos) == QValidator::Acceptable) &&
              (m_validatorCountX->validate (textCountX, pos) == QValidator::Acceptable) &&
              (m_validatorStartX->validate (textStartX, pos) == QValidator::Acceptable) &&
              (m_validatorStepX->validate (textStepX, pos) == QValidator::Acceptable) &&
              (m_validatorStopX->validate (textStopX, pos) == QValidator::Acceptable) &&
              (m_validatorCountY->validate (textCountY, pos) == QValidator::Acceptable) &&
              (m_validatorStartY->validate (textStartY, pos) == QValidator::Acceptable) &&
              (m_validatorStepY->validate (textStepY, pos) == QValidator::Acceptable) &&
              (m_validatorStopY->validate (textStopY, pos) == QValidator::Acceptable);
  enableOk (isOk);
}

void DlgSettingsGridRemoval::updateDisplayedVariableX ()
{
  GridInitializer initializer;

  bool linearAxis = (cmdMediator ().document ().modelCoords ().coordScaleXTheta() == COORD_SCALE_LINEAR);

  switch (m_modelGridRemovalAfter->gridCoordDisableX()) {
    case GRID_COORD_DISABLE_COUNT:
      m_editCountX->setText (QString::number (initializer.computeCount (linearAxis,
                                                                        m_modelGridRemovalAfter->startX (),
                                                                        m_modelGridRemovalAfter->stopX (),
                                                                        m_modelGridRemovalAfter->stepX ())));
      break;

    case GRID_COORD_DISABLE_START:
      m_editStartX->setText (QString::number (initializer.computeStart (linearAxis,
                                                                        m_modelGridRemovalAfter->stopX (),
                                                                        m_modelGridRemovalAfter->stepX (),
                                                                        m_modelGridRemovalAfter->countX ())));
      break;

    case GRID_COORD_DISABLE_STEP:
      m_editStepX->setText (QString::number (initializer.computeStep (linearAxis,
                                                                      m_modelGridRemovalAfter->startX (),
                                                                      m_modelGridRemovalAfter->stopX (),
                                                                      m_modelGridRemovalAfter->countX ())));
      break;

    case GRID_COORD_DISABLE_STOP:
      m_editStopX->setText (QString::number (initializer.computeStop (linearAxis,
                                                                      m_modelGridRemovalAfter->startX (),
                                                                      m_modelGridRemovalAfter->stepX (),
                                                                      m_modelGridRemovalAfter->countX ())));
      break;
  }
}

void DlgSettingsGridRemoval::updateDisplayedVariableY ()
{
  GridInitializer initializer;

  bool linearAxis = (cmdMediator ().document ().modelCoords ().coordScaleYRadius () == COORD_SCALE_LINEAR);

  switch (m_modelGridRemovalAfter->gridCoordDisableY()) {
    case GRID_COORD_DISABLE_COUNT:
      m_editCountY->setText (QString::number (initializer.computeCount (linearAxis,
                                                                        m_modelGridRemovalAfter->startY (),
                                                                        m_modelGridRemovalAfter->stopY (),
                                                                        m_modelGridRemovalAfter->stepY ())));
      break;

    case GRID_COORD_DISABLE_START:
      m_editStartY->setText (QString::number (initializer.computeStart (linearAxis,
                                                                        m_modelGridRemovalAfter->stopY (),
                                                                        m_modelGridRemovalAfter->stepY (),
                                                                        m_modelGridRemovalAfter->countY ())));
      break;

    case GRID_COORD_DISABLE_STEP:
      m_editStepY->setText (QString::number (initializer.computeStep (linearAxis,
                                                                      m_modelGridRemovalAfter->startY (),
                                                                      m_modelGridRemovalAfter->stopY (),
                                                                      m_modelGridRemovalAfter->countY ())));
      break;

    case GRID_COORD_DISABLE_STOP:
      m_editStopY->setText (QString::number (initializer.computeStop (linearAxis,
                                                                      m_modelGridRemovalAfter->startY (),
                                                                      m_modelGridRemovalAfter->stepY (),
                                                                      m_modelGridRemovalAfter->countY ())));
      break;
  }
}

void DlgSettingsGridRemoval::updatePreview ()
{
  GridRemoval gridRemoval (mainWindow().isGnuplot());

  QPixmap pixmap = gridRemoval.remove (mainWindow ().transformation(),
                                       *m_modelGridRemovalAfter,
                                       cmdMediator ().document().pixmap().toImage());

  m_scenePreview->clear();
  addPixmap (*m_scenePreview,
             pixmap);
}
