/******************************************************************************************************
 * (C) 2020 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#include "CentipedeDebugPolar.h"
#include "CentipedeEndpointsPolar.h"
#include "CentipedeSegmentConstantREllipse.h"
#include "DocumentModelCoords.h"
#include "EnumsToQt.h"
#include "GraphicsArcItem.h"
#include "GraphicsArcItemRelay.h"
#include "GraphicsScene.h"
#include "mmsubs.h"
#include <qdebug.h>
#include <qmath.h>
#include <QPen>
#include "QtToString.h"

CentipedeSegmentConstantREllipse::CentipedeSegmentConstantREllipse(GraphicsScene &scene,
                                                                   const DocumentModelCoords &modelCoords,
                                                                   const DocumentModelGuideline &modelGuideline,
                                                                   const Transformation &transformation,
                                                                   const QPointF &posClickScreen) :
  CentipedeSegmentAbstract (modelGuideline,
                            transformation,
                            posClickScreen)
{
  // Radius through click point
  QPointF posClickGraph;
  transformation.transformScreenToRawGraph (posClickScreen,
                                            posClickGraph);
  double rGraph = posClickGraph.y();

  // Compute basis vectors that are used here and when by any callback(s)
  QPointF posOriginGraph (0, 0);
  if (modelCoords.coordScaleYRadius() == COORD_SCALE_LOG) {
    posOriginGraph = QPointF (0, modelCoords.originRadius());
  }

  QPointF posScreen0, posScreen90;
  transformation.transformRawGraphToScreen (posOriginGraph,
                                            m_posOriginScreen);
  transformation.transformRawGraphToScreen (QPointF (0, rGraph),
                                            posScreen0);
  transformation.transformRawGraphToScreen (QPointF (90, rGraph),
                                            posScreen90);

  // Fit the ellipse
  CentipedeEndpointsPolar endpointsPolar (modelCoords,
                                          modelGuideline,
                                          transformation,
                                          posClickScreen,
                                          m_posOriginScreen);

  QRectF rectBounding;
  CentipedeDebugPolar debugPolar;
  double angleRotation;
  endpointsPolar.ellipseScreenConstantRForTHighLowAngles (transformation,
                                                          posClickScreen,
                                                          angleRotation,
                                                          rectBounding,
                                                          debugPolar);;

  debugPolar.display (scene,
                      modelCoords,
                      transformation);

  // Compute position and angle values
  m_posRadialLow = endpointsPolar.posScreenConstantRForLowT (modelGuideline.creationCircleRadius());
  m_posRadialHigh = endpointsPolar.posScreenConstantRForHighT (modelGuideline.creationCircleRadius());
  endpointsPolar.posScreenConstantRHighLow (modelGuideline.creationCircleRadius (),
                                            m_posTangentialLow,
                                            m_posTangentialHigh);

  // Create graphics item and its relay. As explained in GuidelineEllipse::updateGeometry, the correct sequence
  // of graphical operations is very tricky, and less successful if setTransformOriginPoint is used (e.g. works for
  // non-shear cases but not shear cases)
  m_graphicsItem = new GraphicsArcItem (rectBounding);
  m_graphicsItem->setSpanAngle (0); // Prevent flicker by display before span angle is changed from all-inclusive default
  m_graphicsItem->setRotation (qRadiansToDegrees (angleRotation));
  m_graphicsItem->setPos (m_posOriginScreen);
  m_graphicsItemRelay = new GraphicsArcItemRelay (this,
                                                  m_graphicsItem);
  QColor color (ColorPaletteToQColor (modelGuideline.lineColor()));
  m_graphicsItem->setPen (QPen (color,
                                modelGuideline.lineWidthActive ()));
  updateRadius (modelGuideline.creationCircleRadius());

  scene.addItem (m_graphicsItem);
}

CentipedeSegmentConstantREllipse::~CentipedeSegmentConstantREllipse ()
{
  delete m_graphicsItem;
  delete m_graphicsItemRelay;
}

double CentipedeSegmentConstantREllipse::distanceToClosestEndpoint (const QPointF &posScreen) const
{
  double distanceLow = magnitude (posScreen - m_posRadialLow);
  double distanceHigh = magnitude (posScreen - m_posRadialHigh);

  return qMin (distanceLow, distanceHigh);
}

void CentipedeSegmentConstantREllipse::updateRadius (double radius)
{
  // Scale up/down the angles, with them converging to center angle as radius goes to zero
  double scaling = radius / modelGuideline().creationCircleRadius ();

  emit signalUpdateAngles (m_posTangentialLow - m_posOriginScreen,
                           posClickScreen () - m_posOriginScreen,
                           m_posTangentialHigh - m_posOriginScreen,
                           m_graphicsItem->rect().width () / m_graphicsItem->rect().height(),
                           scaling);
}
