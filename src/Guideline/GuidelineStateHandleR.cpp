/******************************************************************************************************
 * (C) 2019 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#include "EngaugeAssert.h"
#include "GraphicsScene.h"
#include "GuidelineAbstract.h"
#include "GuidelineProjectorConstantR.h"
#include "GuidelineState.h"
#include "GuidelineStateContext.h"
#include "GuidelineStateHandleR.h"
#include "Logger.h"
#include "Transformation.h"

GuidelineStateHandleR::GuidelineStateHandleR (GuidelineStateContext &context) :
  GuidelineStateHandleAbstract (context)
{
}

GuidelineStateHandleR::~GuidelineStateHandleR ()
{
}

void GuidelineStateHandleR::begin ()
{
  beginCommon ();
}

QPointF GuidelineStateHandleR::convertGraphCoordinateToScreenPoint (double valueGraph) const
{
  const double ARBITRARY_THETA = 1; // Value that is legal in all cases including log
  QPointF posScreen;
  context().transformation().transformRawGraphToScreen (QPointF (ARBITRARY_THETA,
                                                                 valueGraph),
                                                        posScreen);

  return posScreen;
}

double GuidelineStateHandleR::convertScreenPointToGraphCoordinate(const QPointF &posScreen) const
{
  QPointF posGraph;
  context().transformation().transformScreenToRawGraph (posScreen,
                                                        posGraph);

  return posGraph.y();
}

void GuidelineStateHandleR::end ()
{
}

void GuidelineStateHandleR::handleMouseRelease (const QPointF &posScene)
{
  context().guideline().sacrificeHandleAndVisibleGuidelines(posScene,
                                                            GUIDELINE_STATE_DEPLOYED_CONSTANT_R_SELECT_EDIT);
  context().requestStateTransition (GUIDELINE_STATE_DISCARDED);
}

EllipseParameters GuidelineStateHandleR::pointToEllipse (const QPointF &posScreen) const
{
  GuidelineProjectorConstantR projector;

  return projector.fromPosScreen (context().transformation(),
                                  sceneRect (),
                                  posScreen);
}

QLineF GuidelineStateHandleR::pointToLine (const QPointF & /* posScreen */) const
{
  // pointToEllipse applies in this state
  return QLineF (0, 0, 0, 0);
}

QString GuidelineStateHandleR::stateName () const
{
  return guidelineStateAsString (GUIDELINE_STATE_HANDLE_R);
}
