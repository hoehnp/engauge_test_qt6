/******************************************************************************************************
 * (C) 2019 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#include "EngaugeAssert.h"
#include "GraphicsScene.h"
#include "GuidelineAbstract.h"
#include "GuidelineProjectorConstantT.h"
#include "GuidelineState.h"
#include "GuidelineStateContext.h"
#include "GuidelineStateHandleT.h"
#include "Logger.h"
#include "Transformation.h"

GuidelineStateHandleT::GuidelineStateHandleT (GuidelineStateContext &context) :
  GuidelineStateHandleAbstract (context)
{
}

GuidelineStateHandleT::~GuidelineStateHandleT ()
{
}

void GuidelineStateHandleT::begin ()
{
  beginCommon ();
}

QPointF GuidelineStateHandleT::convertGraphCoordinateToScreenPoint (double valueGraph) const
{
  const double ARBITRARY_RANGE = 0; // Value that is legal in all cases (with log never applying)
  QPointF posScreen;
  context().transformation().transformRawGraphToScreen (QPointF (valueGraph,
                                                                 ARBITRARY_RANGE),
                                                        posScreen);

  return posScreen;
}

double GuidelineStateHandleT::convertScreenPointToGraphCoordinate(const QPointF &posScreen) const
{
  QPointF posGraph;
  context().transformation().transformScreenToRawGraph (posScreen,
                                                        posGraph);

  return posGraph.x();
}

void GuidelineStateHandleT::end ()
{
}

void GuidelineStateHandleT::handleMouseRelease (const QPointF &posScene)
{
  context().guideline().sacrificeHandleAndVisibleGuidelines(posScene,
                                                            GUIDELINE_STATE_DEPLOYED_CONSTANT_T_SELECT_EDIT);
  context().requestStateTransition (GUIDELINE_STATE_DISCARDED);
}

EllipseParameters GuidelineStateHandleT::pointToEllipse (const QPointF & /* posScreen */) const
{
  // pointToLine applies in this state
  return EllipseParameters();
}

QLineF GuidelineStateHandleT::pointToLine (const QPointF &posScreen) const
{
  GuidelineProjectorConstantT projector;

  return projector.fromPosScreen (context().transformation(),
                                  sceneRect (),
                                  posScreen);
}

QString GuidelineStateHandleT::stateName () const
{
  return guidelineStateAsString (GUIDELINE_STATE_HANDLE_T);
}
