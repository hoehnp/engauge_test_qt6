/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#include "BackgroundStateContext.h"
#include "BackgroundStateCurve.h"
#include "DocumentModelColorFilter.h"
#include "DocumentModelGridRemoval.h"
#include "FilterImage.h"
#include "GraphicsScene.h"
#include "GraphicsView.h"
#include "Logger.h"
#include <QPixmap>
#include "Transformation.h"

BackgroundStateCurve::BackgroundStateCurve(BackgroundStateContext &context,
                                           GraphicsScene &scene) :
  BackgroundStateAbstractBase(context,
                              scene)
{
}

void BackgroundStateCurve::begin()
{

  setImageVisible (true);
}

void BackgroundStateCurve::end()
{

  setImageVisible (false);
}

void BackgroundStateCurve::fitInView (GraphicsView &view)
{

  view.fitInView (imageItem ().boundingRect());
}

void BackgroundStateCurve::processImageFromSavedInputs (bool isGnuplot,
                                                        const Transformation &transformation,
                                                        const DocumentModelGridRemoval &modelGridRemoval,
                                                        const DocumentModelColorFilter &modelColorFilter,
                                                        const QString &curveSelected)
{

  // Use the settings if the selected curve is known
  if (!curveSelected.isEmpty()) {

    // Generate filtered image
    FilterImage filterImage;
    QPixmap pixmapFiltered = filterImage.filter (isGnuplot,
                                                 m_pixmapOriginal.toImage(),
                                                 transformation,
                                                 curveSelected,
                                                 modelColorFilter,
                                                 modelGridRemoval);

    setProcessedPixmap (pixmapFiltered);

  } else {

    // Set the image in case BackgroundStateContext::fitInView is called, so the bounding rect is available
    setProcessedPixmap (m_pixmapOriginal);

  }
}

void BackgroundStateCurve::setCurveSelected (bool isGnuplot,
                                             const Transformation &transformation,
                                             const DocumentModelGridRemoval &modelGridRemoval,
                                             const DocumentModelColorFilter &modelColorFilter,
                                             const QString &curveSelected)
{

  // Even if m_curveSelected equals curveSelected we update the image, since the transformation
  // may have changed
  processImageFromSavedInputs (isGnuplot,
                               transformation,
                               modelGridRemoval,
                               modelColorFilter,
                               curveSelected);
}

void BackgroundStateCurve::setPixmap (bool isGnuplot,
                                      const Transformation &transformation,
                                      const DocumentModelGridRemoval &modelGridRemoval,
                                      const DocumentModelColorFilter &modelColorFilter,
                                      const QPixmap &pixmapOriginal,
                                      const QString &curveSelected)
{

  m_pixmapOriginal = pixmapOriginal;
  processImageFromSavedInputs (isGnuplot,
                               transformation,
                               modelGridRemoval,
                               modelColorFilter,
                               curveSelected);
}

QString BackgroundStateCurve::state () const
{
  return "BackgroundStateCurve";
}

void BackgroundStateCurve::updateColorFilter (bool isGnuplot,
                                              const Transformation &transformation,
                                              const DocumentModelGridRemoval &modelGridRemoval,
                                              const DocumentModelColorFilter &modelColorFilter,
                                              const QString &curveSelected)
{

  processImageFromSavedInputs (isGnuplot,
                               transformation,
                               modelGridRemoval,
                               modelColorFilter,
                               curveSelected);
}
