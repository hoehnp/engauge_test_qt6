/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#include "ExportAlignLinear.h"
#include "ExportAlignLog.h"
#include "ExportLayoutFunctions.h"
#include "ExportPointsSelectionFunctions.h"
#include "ExportXThetaValuesMergedFunctions.h"
#include "Logger.h"
#include "Point.h"
#include <qmath.h>
#include "Transformation.h"

using namespace std;

ExportXThetaValuesMergedFunctions::ExportXThetaValuesMergedFunctions(const DocumentModelExportFormat &modelExport,
                                                                     const MainWindowModel &modelMainWindow,
                                                                     const ValuesVectorXOrY &xThetaValuesRaw,
                                                                     const Transformation &transformation) :
  m_modelExport (modelExport),
  m_modelMainWindow (modelMainWindow),
  m_xThetaValuesRaw (xThetaValuesRaw),
  m_transformation (transformation)
{
}

bool ExportXThetaValuesMergedFunctions::breakForPointOverrun (unsigned int curveSize,
                                                              bool &isOverrun) const
{
  // Break if we will overrun the maximum point limit. As per issue #401 a change of scale
  // can result in a huge (new) range with a small (old) delta - resulting in so many values
  // that this loop effectively hangs. Set flag if overrun occurred
  isOverrun = (curveSize > (unsigned int) m_modelMainWindow.maximumExportedPointsPerCurve ());
  return isOverrun;
}

void ExportXThetaValuesMergedFunctions::firstSimplestNumberLinear (double &xThetaFirstSimplestNumber,
                                                                   double &xThetaMin,
                                                                   double &xThetaMax) const
{

  // X/theta range
  xThetaMin = m_xThetaValuesRaw.firstKey();
  xThetaMax = m_xThetaValuesRaw.lastKey();

  // Compute offset that gives the simplest numbers
  ExportAlignLinear alignLinear (xThetaMin,
                                 xThetaMax);

  xThetaFirstSimplestNumber = alignLinear.firstSimplestNumber ();
}

void ExportXThetaValuesMergedFunctions::firstSimplestNumberLog (double &xThetaFirstSimplestNumber,
                                                                double &xThetaMin,
                                                                double &xThetaMax) const
{

  // X/theta range
  xThetaMin = m_xThetaValuesRaw.firstKey();
  xThetaMax = m_xThetaValuesRaw.lastKey();

  // Compute offset that gives the simplest numbers
  ExportAlignLog alignLog (xThetaMin,
                           xThetaMax);

  xThetaFirstSimplestNumber = alignLog.firstSimplestNumber();
}

ExportValuesXOrY ExportXThetaValuesMergedFunctions::periodicLinear(bool &isOverrun) const
{

  if (m_xThetaValuesRaw.count () > 0) {

    double xThetaFirstSimplestNumber, xThetaMin, xThetaMax;
    firstSimplestNumberLinear (xThetaFirstSimplestNumber,
                               xThetaMin,
                               xThetaMax);

    // Assuming user picks an appropriate interval increment, numbering starting at xThetaFirstSimplestNumber
    // will give nice x/theta numbers
    if (m_modelExport.pointsIntervalUnitsFunctions() == EXPORT_POINTS_INTERVAL_UNITS_GRAPH) {
      return periodicLinearGraph(xThetaFirstSimplestNumber,
                                 xThetaMin,
                                 xThetaMax,
                                 isOverrun);
    } else {
      return periodicLinearScreen(xThetaMin,
                                  xThetaMax,
                                  isOverrun);
    }
  } else {

    ExportValuesXOrY emptyList;
    return emptyList;
  }
}

ExportValuesXOrY ExportXThetaValuesMergedFunctions::periodicLinearGraph(double xThetaFirstSimplestNumber,
                                                                        double xThetaMin,
                                                                        double xThetaMax,
                                                                        bool &isOverrun) const
{

  // Convert the gathered values into a periodic sequence
  ValuesVectorXOrY values;
  double xTheta = xThetaFirstSimplestNumber;
  while (xTheta > xThetaMin) {
    xTheta -= m_modelExport.pointsIntervalFunctions(); // Go backwards until reaching or passing minimum
  }
  if (xTheta < xThetaMin) {
    values [xThetaMin] = true; // We passed minimum so insert point right at xThetaMin
  }

  xTheta += m_modelExport.pointsIntervalFunctions();
  while (xTheta <= xThetaMax) {

    values [xTheta] = true;
    xTheta += m_modelExport.pointsIntervalFunctions(); // Insert point at a simple number

    if (breakForPointOverrun (values.count(),
                              isOverrun)) {
      break;
    }
  }

  if (xTheta > xThetaMax) {
    values [xThetaMax] = true; // We passed maximum so insert point right at xThetaMax
  }

  return values.keys();
}

ExportValuesXOrY ExportXThetaValuesMergedFunctions::periodicLinearScreen (double xThetaMin,
                                                                          double xThetaMax,
                                                                          bool &isOverrun) const
{

  // This must be greater than zero. Otherwise, logarithmic y axis will trigger errors in the
  // transform, which cascades into NaN values for the x coordinates below
  const double ARBITRARY_Y = 1.0;

  // Screen coordinates of endpoints
  QPointF posScreenFirst, posScreenLast;
  m_transformation.transformRawGraphToScreen(QPointF (xThetaMin,
                                                      ARBITRARY_Y),
                                             posScreenFirst);
  m_transformation.transformRawGraphToScreen(QPointF (xThetaMax,
                                                      ARBITRARY_Y),
                                             posScreenLast);
  double deltaScreenX = posScreenLast.x() - posScreenFirst.x();

  // Need calculations to find the scaling to be applied to successive points
  double s = 1.0;
  double interval = m_modelExport.pointsIntervalFunctions();
  if ((interval > 0) &&
      (interval < deltaScreenX)) {
    s = interval / deltaScreenX;
  }

  // Example: xThetaMin=0.1 and xThetaMax=100 (points are 0.1, 1, 10, 100) with s=1/3 so scale should be 10
  // which multiples 0.1 to get 1. This uses s=(log(xNext)-log(xMin))/(log(xMax)-log(xMin))
  double xNext = xThetaMin + s * (xThetaMax - xThetaMin);
  double delta = xNext - xThetaMin;

  ValuesVectorXOrY values;

  double xTheta = xThetaMin;
  while (xTheta <= xThetaMax) {

    values [xTheta] = true;
    xTheta += delta;

    if (breakForPointOverrun (values.count(),
                              isOverrun)) {
      break;
    }
  }

  return values.keys();
}

ExportValuesXOrY ExportXThetaValuesMergedFunctions::periodicLog(bool &isOverrun) const
{

  double xThetaFirstSimplestNumber, xThetaMin, xThetaMax;
  firstSimplestNumberLog (xThetaFirstSimplestNumber,
                          xThetaMin,
                          xThetaMax);

  // Assuming user picks an appropriate interval increment, numbering starting at xThetaFirstSimplestNumber
  // will give nice x/theta numbers
  if (m_modelExport.pointsIntervalUnitsFunctions() == EXPORT_POINTS_INTERVAL_UNITS_GRAPH) {
    return periodicLogGraph(xThetaFirstSimplestNumber,
                            xThetaMin,
                            xThetaMax,
                            isOverrun);
  } else {
    return periodicLogScreen(xThetaMin,
                             xThetaMax,
                             isOverrun);
  }
}

ExportValuesXOrY ExportXThetaValuesMergedFunctions::periodicLogGraph (double xThetaFirstSimplestNumber,
                                                                      double xThetaMin,
                                                                      double xThetaMax,
                                                                      bool &isOverrun) const
{

  // Convert the gathered values into a periodic sequence
  ValuesVectorXOrY values;
  double xTheta = xThetaFirstSimplestNumber;
  if (m_modelExport.pointsIntervalFunctions() > 1) { // Safe to iterate
    while (xTheta > xThetaMin) {
      xTheta /= m_modelExport.pointsIntervalFunctions(); // Go backwards until reaching or passing minimum
    }
  }
  if (xTheta < xThetaMin) {
    values [xThetaMin] = true; // We passed minimum so insert point right at xThetaMin
  }

  if (m_modelExport.pointsIntervalFunctions() > 1) { // Safe to iterate
    xTheta *= m_modelExport.pointsIntervalFunctions();
    while (xTheta <= xThetaMax) {

      values [xTheta] = true;
      xTheta *= m_modelExport.pointsIntervalFunctions(); // Insert point at a simple number

      if (breakForPointOverrun (values.count(),
                                isOverrun)) {
        break;
      }
    }
  }

  if (xTheta > xThetaMax) {
    values [xThetaMax] = true; // We passed maximum so insert point right at xThetaMax
  }

  return values.keys();
}

ExportValuesXOrY ExportXThetaValuesMergedFunctions::periodicLogScreen (double xThetaMin,
                                                                       double xThetaMax,
                                                                       bool &isOverrun) const
{

  const double ARBITRARY_Y = 0.0;

  // Screen coordinates of endpoints
  QPointF posScreenFirst, posScreenLast;
  m_transformation.transformRawGraphToScreen(QPointF (xThetaMin,
                                                      ARBITRARY_Y),
                                             posScreenFirst);
  m_transformation.transformRawGraphToScreen(QPointF (xThetaMax,
                                                      ARBITRARY_Y),
                                             posScreenLast);
  double deltaScreenX = posScreenLast.x() - posScreenFirst.x();
  double deltaScreenY = posScreenLast.y() - posScreenFirst.y();
  double deltaScreen = qSqrt (deltaScreenX * deltaScreenX + deltaScreenY * deltaScreenY);

  // Need calculations to find the scaling to be applied to successive points
  double s = 1.0;
  double interval = m_modelExport.pointsIntervalFunctions();
  if ((interval > 0) &&
      (interval < deltaScreen)) {
    s = interval / deltaScreen;
  }

  // Example: xThetaMin=0.1 and xThetaMax=100 (points are 0.1, 1, 10, 100) with s=1/3 so scale should be 10
  // which multiples 0.1 to get 1. This uses s=(log(xNext)-log(xMin))/(log(xMax)-log(xMin))
  double xNext = qExp (qLn (xThetaMin) + s * (qLn (xThetaMax) - qLn (xThetaMin)));
  double scale = xNext / xThetaMin;

  ValuesVectorXOrY values;

  double xTheta = xThetaMin;
  while (xTheta <= xThetaMax) {

    values [xTheta] = true;
    xTheta *= scale;

    if (breakForPointOverrun (values.count(),
                              isOverrun)) {
      break;
    }
  }

  return values.keys();
}

ExportValuesXOrY ExportXThetaValuesMergedFunctions::xThetaValues (bool &isOverrun) const
{

  if (m_modelExport.pointsSelectionFunctions() == EXPORT_POINTS_SELECTION_FUNCTIONS_INTERPOLATE_PERIODIC) {

    ExportValuesXOrY values;

    // Special case that occurs when there are no points
    if (qAbs (m_modelExport.pointsIntervalFunctions()) <= 0) {

      // Return empty array

    } else {

      bool isLinear =  (m_transformation.modelCoords().coordScaleXTheta() == COORD_SCALE_LINEAR);

      if (isLinear) {
        values = periodicLinear (isOverrun);
      } else {
        values = periodicLog (isOverrun);
      }

      if (isOverrun) {

        // Empty out the array since it is, in the case of overrun, incomplete and therefore confusing at best
        values.clear();

      }
    }

    return values;

  } else {

    // Return the gathered values
    return m_xThetaValuesRaw.keys();

  }
}
