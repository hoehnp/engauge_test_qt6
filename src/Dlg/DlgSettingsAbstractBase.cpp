/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#include "ButtonWhatsThis.h"
#include "CmdMediator.h"
#include "DlgSettingsAbstractBase.h"
#include "EngaugeAssert.h"
#include "Logger.h"
#include "MainWindow.h"
#include <QColor>
#include <QComboBox>
#include <QGraphicsScene>
#include <QGridLayout>
#include <QPushButton>
#include <QScrollArea>
#include <QSettings>
#include <QSpacerItem>
#include <QVBoxLayout>
#include "Settings.h"

int DlgSettingsAbstractBase::MINIMUM_DIALOG_WIDTH = 380; // May be overridden by subclass
int DlgSettingsAbstractBase::MINIMUM_PREVIEW_HEIGHT = 100;

DlgSettingsAbstractBase::DlgSettingsAbstractBase(const QString &title,
                                                 const QString &dialogName,
                                                 MainWindow &mainWindow) :
  QDialog (&mainWindow),
  m_mainWindow (mainWindow),
  m_cmdMediator (nullptr),
  m_dialogName (dialogName),
  m_disableOkAtStartup (true)
{

  setWindowTitle (title);
  setModal (true);

  // Linux Mint seems to not show help button
  //setWindowFlag (Qt::Window, true);
  //setWindowFlag (Qt::WindowContextHelpButtonHint, true);
  //setWindowFlag (Qt::WindowCloseButtonHint, true);
  //setWindowFlag (Qt::WindowMinimizeButtonHint, false);
  //setWindowFlag (Qt::WindowMaximizeButtonHint, false);
  setWindowFlags (Qt::Window | Qt::WindowContextHelpButtonHint | Qt::WindowCloseButtonHint);
}

DlgSettingsAbstractBase::~DlgSettingsAbstractBase()
{
}

void DlgSettingsAbstractBase::addPixmap (QGraphicsScene &scene,
                                         const QPixmap &pixmap)
{
  scene.addPixmap (pixmap);
  scene.setSceneRect (0,
                      0,
                      pixmap.width(),
                      pixmap.height());
}

CmdMediator &DlgSettingsAbstractBase::cmdMediator ()
{
  ENGAUGE_CHECK_PTR (m_cmdMediator);

  return *m_cmdMediator;
}

void DlgSettingsAbstractBase::createWhatsThis (QGridLayout *layout,
                                               ButtonWhatsThis *button,
                                               int row,
                                               int column)
{
  button = new ButtonWhatsThis ();
  connect (button, SIGNAL (clicked ()), this, SLOT (slotWhatsThis()));
  layout->addWidget (button, row, column, 1, 1, Qt::AlignRight | Qt::AlignTop);

  // Prevent huge space after row with this button, especially with DlgSettingsMainWindow
  layout->setRowStretch (row, 0);
}

void DlgSettingsAbstractBase::enableOk (bool enable)
{
  m_btnOk->setEnabled (enable);
}

void DlgSettingsAbstractBase::finishPanel (QWidget *subPanel,
                                           int minimumWidth,
                                           int minimumHeightOrZero)
{
  const int STRETCH_OFF = 0, STRETCH_ON = 1;

  m_scroll = new QScrollArea (this);
  m_scroll->setStyleSheet ("QScrollArea { border: 0; margin: 0; padding: 0;}"); // Need QScrollArea or interior frames are affected    
  m_scroll->setHorizontalScrollBarPolicy (Qt::ScrollBarAlwaysOff);
  m_scroll->setVerticalScrollBarPolicy (Qt::ScrollBarAsNeeded);
  m_scroll->setSizePolicy (QSizePolicy::Minimum,
                           QSizePolicy::Minimum);
  m_scroll->setMinimumWidth (minimumWidth);

  QWidget *viewport = new QWidget (this);
  m_scroll->setWidget (viewport);
  m_scroll->setWidgetResizable (true);

  QHBoxLayout *scrollLayout = new QHBoxLayout (this);
  scrollLayout->addWidget (m_scroll);
  setLayout (scrollLayout);
  
  QVBoxLayout *panelLayout = new QVBoxLayout (viewport);
  viewport->setLayout (panelLayout);

  panelLayout->addWidget (subPanel);
  panelLayout->setStretch (panelLayout->count () - 1, STRETCH_ON);

  QWidget *panelButtons = new QWidget (this);
  QHBoxLayout *buttonLayout = new QHBoxLayout (panelButtons);

  createOptionalSaveDefault(buttonLayout);

  QHBoxLayout *layoutRightSide = new QHBoxLayout;

  QWidget *widgetRightSide = new QWidget;
  widgetRightSide->setLayout (layoutRightSide);
  buttonLayout->addWidget (widgetRightSide);

  QSpacerItem *spacerExpanding = new QSpacerItem (40, 5, QSizePolicy::Expanding, QSizePolicy::Expanding);
  layoutRightSide->addItem (spacerExpanding);

  m_btnOk = new QPushButton (tr ("Ok"));
  m_btnOk->setEnabled (false); // Nothing to save initially
  layoutRightSide->addWidget (m_btnOk, 0, Qt::AlignRight);
  connect (m_btnOk, SIGNAL (released ()), this, SLOT (slotOk ()));

  QSpacerItem *spacerFixed = new QSpacerItem (40, 5, QSizePolicy::Fixed, QSizePolicy::Fixed);
  layoutRightSide->addItem (spacerFixed);

  m_btnCancel = new QPushButton (tr ("Cancel"));
  layoutRightSide->addWidget (m_btnCancel, 0, Qt::AlignRight);
  connect (m_btnCancel, SIGNAL (released ()), this, SLOT (slotCancel ()));

  panelLayout->addWidget (panelButtons, STRETCH_ON);
  panelLayout->setStretch (panelLayout->count () - 1, STRETCH_OFF);

  setSizePolicy (QSizePolicy::Minimum,
                 QSizePolicy::Minimum);

  if (minimumHeightOrZero > 0) {
     m_scroll->setMinimumHeight (minimumHeightOrZero);
  }
}

MainWindow &DlgSettingsAbstractBase::mainWindow ()
{
  return m_mainWindow;
}

const MainWindow &DlgSettingsAbstractBase::mainWindow () const
{
  return m_mainWindow;
}

void DlgSettingsAbstractBase::populateColorComboWithoutTransparent (QComboBox &combo)
{
  combo.addItem (colorPaletteToString (COLOR_PALETTE_BLUE),
                 QVariant (COLOR_PALETTE_BLUE));
  combo.addItem (colorPaletteToString (COLOR_PALETTE_BLACK),
                 QVariant (COLOR_PALETTE_BLACK));
  combo.addItem (colorPaletteToString (COLOR_PALETTE_CYAN),
                 QVariant (COLOR_PALETTE_CYAN));
  combo.addItem (colorPaletteToString (COLOR_PALETTE_GOLD),
                 QVariant (COLOR_PALETTE_GOLD));
  combo.addItem (colorPaletteToString (COLOR_PALETTE_GREEN),
                 QVariant (COLOR_PALETTE_GREEN));
  combo.addItem (colorPaletteToString (COLOR_PALETTE_MAGENTA),
                 QVariant (COLOR_PALETTE_MAGENTA));
  combo.addItem (colorPaletteToString (COLOR_PALETTE_RED),
                 QVariant (COLOR_PALETTE_RED));
  combo.addItem (colorPaletteToString (COLOR_PALETTE_YELLOW),
                 QVariant (COLOR_PALETTE_YELLOW));
}

void DlgSettingsAbstractBase::populateColorComboWithTransparent (QComboBox &combo)
{
  populateColorComboWithoutTransparent (combo);
  combo.addItem ("Transparent", QVariant (COLOR_PALETTE_TRANSPARENT));
}

void DlgSettingsAbstractBase::saveGeometryToSettings()
{
  // Store the settings for use by showEvent
  QSettings settings (SETTINGS_ENGAUGE, SETTINGS_DIGITIZER);
  settings.setValue (m_dialogName, saveGeometry ());
}

void DlgSettingsAbstractBase::setCmdMediator (CmdMediator &cmdMediator)
{
  m_cmdMediator = &cmdMediator;
}

void DlgSettingsAbstractBase::setDisableOkAtStartup(bool disableOkAtStartup)
{
  m_disableOkAtStartup = disableOkAtStartup;
}

void DlgSettingsAbstractBase::hideEvent (QHideEvent * /* event */)
{
  saveGeometryToSettings();
}

void DlgSettingsAbstractBase::showEvent (QShowEvent * /* event */)
{
  if (m_disableOkAtStartup) {
    m_btnOk->setEnabled (false);
  }

  QSettings settings (SETTINGS_ENGAUGE, SETTINGS_DIGITIZER);
  if (settings.contains (m_dialogName)) {

    // Restore the settings that were stored by the last call to saveGeometryToSettings
    restoreGeometry (settings.value (m_dialogName).toByteArray ());
  }
}

void DlgSettingsAbstractBase::slotCancel ()
{

  hide();
}

void DlgSettingsAbstractBase::slotOk ()
{

  // Forward to leaf class
  handleOk ();
}
