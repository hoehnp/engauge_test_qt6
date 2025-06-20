/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#include "BackgroundStateContext.h"
#include "BackgroundStateCurve.h"
#include "BackgroundStateNone.h"
#include "BackgroundStateOriginal.h"
#include "BackgroundStateUnloaded.h"
#include "DocumentModelColorFilter.h"
#include "DocumentModelGridRemoval.h"
#include "EngaugeAssert.h"
#include "GraphicsView.h"
#include "Logger.h"
#include "MainWindow.h"
#include <QGraphicsPixmapItem>
#include "Transformation.h"

BackgroundStateContext::BackgroundStateContext(MainWindow &mainWindow) :
  m_mainWindow (mainWindow)
{

  // These states follow the same order as the BackgroundState enumeration
  m_states.insert (BACKGROUND_STATE_CURVE   , new BackgroundStateCurve    (*this, mainWindow.scene()));
  m_states.insert (BACKGROUND_STATE_NONE    , new BackgroundStateNone     (*this, mainWindow.scene()));
  m_states.insert (BACKGROUND_STATE_ORIGINAL, new BackgroundStateOriginal (*this, mainWindow.scene()));
  m_states.insert (BACKGROUND_STATE_UNLOADED, new BackgroundStateUnloaded (*this, mainWindow.scene()));
  ENGAUGE_ASSERT (m_states.size () == NUM_BACKGROUND_STATES);

  m_currentState = NUM_BACKGROUND_STATES; // Value that forces a transition right away
  requestStateTransition (BACKGROUND_STATE_UNLOADED);
  completeRequestedStateTransitionIfExists();
}

BackgroundStateContext::~BackgroundStateContext()
{
  qDeleteAll (m_states);
}

void BackgroundStateContext::close()
{

  // It is safe to transition to the new state immediately since no BackgroundState classes are on the stack
  requestStateTransition (BACKGROUND_STATE_UNLOADED);
  completeRequestedStateTransitionIfExists ();
}

void BackgroundStateContext::completeRequestedStateTransitionIfExists()
{

  if (m_currentState != m_requestedState) {

    // A transition is waiting so perform it

    if (m_currentState != NUM_BACKGROUND_STATES) {

      // This is not the first state so close the previous state
      m_states [m_currentState]->end ();
    }

    // Start the new state
    m_currentState = m_requestedState;
    m_states [m_requestedState]->begin ();
  }
}

void BackgroundStateContext::fitInView (GraphicsView &view)
{

  // After initialization, we should be in unloaded state or some other equally valid state
  ENGAUGE_ASSERT (m_currentState != NUM_BACKGROUND_STATES);

  const QGraphicsPixmapItem *imageItem = &m_states [BACKGROUND_STATE_CURVE]->imageItem ();

  double width = imageItem->boundingRect().width();
  double height = imageItem->boundingRect().height();

  // Get the image from a state that is guaranteed to have an image
  view.fitInView (imageItem);

}

QImage BackgroundStateContext::imageForCurveState () const
{
  return m_states [BACKGROUND_STATE_CURVE]->image();
}

void BackgroundStateContext::requestStateTransition (BackgroundState backgroundState)
{

  m_requestedState = backgroundState;
}

void BackgroundStateContext::setBackgroundImage (BackgroundImage backgroundImage)
{

  BackgroundState backgroundState= BACKGROUND_STATE_NONE;
  
  switch (backgroundImage) {
    case BACKGROUND_IMAGE_FILTERED:
      backgroundState = BACKGROUND_STATE_CURVE;
      break;

    case BACKGROUND_IMAGE_NONE:
      backgroundState = BACKGROUND_STATE_NONE;
      break;

     case BACKGROUND_IMAGE_ORIGINAL:
      backgroundState = BACKGROUND_STATE_ORIGINAL;
      break;
  }

  // It is safe to transition to the new state immediately since no BackgroundState classes are on the stack
  requestStateTransition (backgroundState);
  completeRequestedStateTransitionIfExists ();
}

void BackgroundStateContext::setCurveSelected (bool isGnuplot,
                                               const Transformation &transformation,
                                               const DocumentModelGridRemoval &modelGridRemoval,
                                               const DocumentModelColorFilter &modelColorFilter,
                                               const QString &curveSelected)
{

  for (int backgroundState = 0; backgroundState < NUM_BACKGROUND_STATES; backgroundState++) {

    m_states [backgroundState]->setCurveSelected (isGnuplot,
                                                  transformation,
                                                  modelGridRemoval,
                                                  modelColorFilter,
                                                  curveSelected);
  }
}

void BackgroundStateContext::setPixmap (bool isGnuplot,
                                        const Transformation &transformation,
                                        const DocumentModelGridRemoval &modelGridRemoval,
                                        const DocumentModelColorFilter &modelColorFilter,
                                        const QPixmap &pixmapOriginal,
                                        const QString &curveSelected)
{

  for (int backgroundState = 0; backgroundState < NUM_BACKGROUND_STATES; backgroundState++) {

    m_states [backgroundState]->setPixmap (isGnuplot,
                                           transformation,
                                           modelGridRemoval,
                                           modelColorFilter,
                                           pixmapOriginal,
                                           curveSelected);
  }
}

void BackgroundStateContext::updateColorFilter (bool isGnuplot,
                                                const Transformation &transformation,
                                                const DocumentModelGridRemoval &modelGridRemoval,
                                                const DocumentModelColorFilter &modelColorFilter,
                                                const QString &curveSelected)
{

  for (int backgroundState = 0; backgroundState < NUM_BACKGROUND_STATES; backgroundState++) {

    m_states [backgroundState]->updateColorFilter (isGnuplot,
                                                   transformation,
                                                   modelGridRemoval,
                                                   modelColorFilter,
                                                   curveSelected);
  }
}
