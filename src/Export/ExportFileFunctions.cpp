/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#include "CallbackGatherXThetasInCurves.h"
#include "CallbackGatherXThetasInGridLines.h"
#include "CurveConnectAs.h"
#include "Document.h"
#include "DocumentModelGeneral.h"
#include "EngaugeAssert.h"
#include "ExportFileFunctions.h"
#include "ExportLayoutFunctions.h"
#include "ExportOrdinalsSmooth.h"
#include "ExportXThetaValuesMergedFunctions.h"
#include "FormatCoordsUnits.h"
#include "GridLineLimiter.h"
#include "LinearToLog.h"
#include "Logger.h"
#include <qmath.h>
#include <QTextStream>
#include <QVector>
#include "Spline.h"
#include "SplinePair.h"
#include "Transformation.h"
#include <vector>

using namespace std;

ExportFileFunctions::ExportFileFunctions()
{
}

void ExportFileFunctions::exportAllPerLineXThetaValuesMerged (const DocumentModelExportFormat &modelExportOverride,
                                                              const Document &document,
                                                              const MainWindowModel &modelMainWindow,
                                                              const QStringList &curvesIncluded,
                                                              const ExportValuesXOrY &xThetaValues,
                                                              const QString &delimiter,
                                                              const Transformation &transformation,
                                                              bool isLogXTheta,
                                                              bool isLogYRadius,
                                                              const CurveLimits curveLimitsMin,
                                                              const CurveLimits curveLimitsMax,
                                                              QTextStream &str,
                                                              unsigned int &numWritesSoFar) const
{

  int curveCount = curvesIncluded.count();
  int xThetaCount = xThetaValues.count();
  QVector<QVector<QString*> > yRadiusValues (curveCount, QVector<QString*> (xThetaCount));
  initializeYRadiusValues (curvesIncluded,
                           xThetaValues,
                           yRadiusValues);
  loadYRadiusValues (modelExportOverride,
                     document,
                     modelMainWindow,
                     curvesIncluded,
                     transformation,
                     isLogXTheta,
                     isLogYRadius,
                     xThetaValues,
                     curveLimitsMin,
                     curveLimitsMax,
                     yRadiusValues);

  outputXThetaYRadiusValues (modelExportOverride,
                             document.modelCoords(),
                             document.modelGeneral(),
                             modelMainWindow,
                             curvesIncluded,
                             xThetaValues,
                             transformation,
                             yRadiusValues,
                             delimiter,
                             str,
                             numWritesSoFar);
  destroy2DArray (yRadiusValues);
}

void ExportFileFunctions::exportOnePerLineXThetaValuesMerged (const DocumentModelExportFormat &modelExportOverride,
                                                              const Document &document,
                                                              const MainWindowModel &modelMainWindow,
                                                              const QStringList &curvesIncluded,
                                                              const ExportValuesXOrY &xThetaValues,
                                                              const QString &delimiter,
                                                              const Transformation &transformation,
                                                              bool isLogXTheta,
                                                              bool isLogYRadius,
                                                              const CurveLimits curveLimitsMin,
                                                              const CurveLimits curveLimitsMax,
                                                              QTextStream &str,
                                                              unsigned int &numWritesSoFar) const
{

  QStringList::const_iterator itr;
  for (itr = curvesIncluded.begin(); itr != curvesIncluded.end(); itr++) {

    // This curve
    const int CURVE_COUNT = 1;
    QString curveIncluded = *itr;
    QStringList curvesIncludedIter (curveIncluded);

    int xThetaCount = xThetaValues.count();
    QVector<QVector<QString*> > yRadiusValues (CURVE_COUNT, QVector<QString*> (xThetaCount));
    initializeYRadiusValues (curvesIncludedIter,
                             xThetaValues,
                             yRadiusValues);
    loadYRadiusValues (modelExportOverride,
                       document,
                       modelMainWindow,
                       curvesIncludedIter,
                       transformation,
                       isLogXTheta,
                       isLogYRadius,
                       xThetaValues,
                       curveLimitsMin,
                       curveLimitsMax,
                       yRadiusValues);
    outputXThetaYRadiusValues (modelExportOverride,
                               document.modelCoords(),
                               document.modelGeneral(),
                               modelMainWindow,
                               curvesIncludedIter,
                               xThetaValues,
                               transformation,
                               yRadiusValues,
                               delimiter,
                               str,
                               numWritesSoFar);
    destroy2DArray (yRadiusValues);
  }
}

void ExportFileFunctions::exportToFile (const DocumentModelExportFormat &modelExportOverride,
                                        const Document &document,
                                        const MainWindowModel &modelMainWindow,
                                        const Transformation &transformation,
                                        QTextStream &str,
                                        unsigned int &numWritesSoFar,
                                        bool &isOverrun) const
{

  isOverrun = false;

  // Log coordinates must be temporarily transformed to linear coordinates
  bool isLogXTheta = (document.modelCoords().coordScaleXTheta() == COORD_SCALE_LOG);
  bool isLogYRadius = (document.modelCoords().coordScaleYRadius() == COORD_SCALE_LOG);

  // Identify curves to be included
  QStringList curvesIncluded = curvesToInclude (modelExportOverride,
                                                document,
                                                document.curvesGraphsNames(),
                                                CONNECT_AS_FUNCTION_SMOOTH,
                                                CONNECT_AS_FUNCTION_STRAIGHT);

  // Delimiter
  const QString delimiter = exportDelimiterToText (modelExportOverride.delimiter(),
                                                   modelExportOverride.header() == EXPORT_HEADER_GNUPLOT);

  // Get x/theta values to be used. Also get the endpoint limits, if any
  CurveLimits curveLimitsMin, curveLimitsMax;
  ValuesVectorXOrY valuesVector;
  if (modelExportOverride.pointsSelectionFunctions() == EXPORT_POINTS_SELECTION_FUNCTIONS_INTERPOLATE_GRID_LINES) {
    CallbackGatherXThetasInGridLines ftor (modelMainWindow,
                                           modelExportOverride,
                                           curvesIncluded,
                                           transformation,
                                           document);
    Functor2wRet<const QString &, const Point &, CallbackSearchReturn> ftorWithCallback = functor_ret (ftor,
                                                                                                       &CallbackGatherXThetasInGridLines::callback);
    document.iterateThroughCurvesPointsGraphs(ftorWithCallback);
    valuesVector = ftor.xThetaValuesRaw();
    curveLimitsMin = ftor.curveLimitsMin();
    curveLimitsMax = ftor.curveLimitsMax();
  } else {
    CallbackGatherXThetasInCurves ftor (modelExportOverride,
                                        curvesIncluded,
                                        transformation);
    Functor2wRet<const QString &, const Point &, CallbackSearchReturn> ftorWithCallback = functor_ret (ftor,
                                                                                                       &CallbackGatherXThetasInCurves::callback);
    document.iterateThroughCurvesPointsGraphs(ftorWithCallback);
    valuesVector = ftor.xThetaValuesRaw();
    curveLimitsMin = ftor.curveLimitsMin();
    curveLimitsMax = ftor.curveLimitsMax();
  }

  ExportXThetaValuesMergedFunctions exportXTheta (modelExportOverride,
                                                  modelMainWindow,
                                                  valuesVector,
                                                  transformation);

  ExportValuesXOrY xThetaValuesMerged = exportXTheta.xThetaValues (isOverrun);

  if (isOverrun) {

    // Put note in output to explain why there are no points
    str << QObject::tr ("Too many points");

  } else {

    // Skip if every curve was a relation
    if (xThetaValuesMerged.count() > 0) {

      // Export in one of two layouts
      if (modelExportOverride.layoutFunctions() == EXPORT_LAYOUT_ALL_PER_LINE) {
        exportAllPerLineXThetaValuesMerged (modelExportOverride,
                                            document,
                                            modelMainWindow,
                                            curvesIncluded,
                                            xThetaValuesMerged,
                                            delimiter,
                                            transformation,
                                            isLogXTheta,
                                            isLogYRadius,
                                            curveLimitsMin,
                                            curveLimitsMax,
                                            str,
                                            numWritesSoFar);
      } else {
        exportOnePerLineXThetaValuesMerged (modelExportOverride,
                                            document,
                                            modelMainWindow,
                                            curvesIncluded,
                                            xThetaValuesMerged,
                                            delimiter,
                                            transformation,
                                            isLogXTheta,
                                            isLogYRadius,
                                            curveLimitsMin,
                                            curveLimitsMax,
                                            str,
                                            numWritesSoFar);
      }
    }
  }
}

void ExportFileFunctions::initializeYRadiusValues (const QStringList &curvesIncluded,
                                                   const ExportValuesXOrY &xThetaValuesMerged,
                                                   QVector<QVector<QString*> > &yRadiusValues) const
{

  // Initialize every entry with empty string
  int curveCount = curvesIncluded.count();
  int xThetaCount = xThetaValuesMerged.count();
  for (int row = 0; row < xThetaCount; row++) {
    for (int col = 0; col < curveCount; col++) {
      yRadiusValues [col] [row] = new QString;
    }
  }
}

double ExportFileFunctions::linearlyInterpolate (const FittingPointsConvenient &positionsLinearized,
                                                 double xThetaLinearized) const
{

  // If point is within the range of the function points then interpolation will be used, otherwise
  // extrapolation will be used
  double yRadiusLinearized = 0;
  QPointF posGraphBefore; // Not set until ip=1
  bool foundIt = false;
  FittingPointsConvenient::const_iterator itr;
  for (itr = positionsLinearized.begin(); itr != positionsLinearized.end(); itr++) {
    const QPointF posGraph = *itr;

    // Cases where we have found it at this point in the code
    // (1) interpolation case where (xBefore < xThetaValue < xAfter)
    // (2) extrapolation case where (xThetaValue < xBefore < xAfter and ip=0) for which we delay finding it until ip=1 so we have
    //     two points for extrapolating. This case is why we have the subtle test for ip>0 in the next line
    if (xThetaLinearized <= posGraph.x() && (itr != positionsLinearized.begin())) {

      foundIt = true;

      // Case 1 comments: xThetaValue is between posGraphBefore and posGraph. Note that if posGraph.x()=posGraphBefore.x() then
      // previous iteration of loop would have been used for interpolation, and then the loop was exited. Range of s is 0<s<1
      // Case 2 comments: Range of s is s<0
      yRadiusLinearized = linearlyInterpolateYRadiusFromTwoPoints (xThetaLinearized,
                                                                   posGraphBefore,
                                                                   posGraph);

      break;
    }

    posGraphBefore = posGraph;
  }

  if (!foundIt) {

    if (positionsLinearized.count() > 1) {

      // Extrapolation will be used since point is out of the range of the function points. Specifically, it is greater than the
      // last x value in the function. Range of s is 1<s
      int N = positionsLinearized.count();
      const QPointF &pointBefore = positionsLinearized.at (N - 2);
      const QPointF &pointLast = positionsLinearized.at (N - 1);
      yRadiusLinearized = linearlyInterpolateYRadiusFromTwoPoints (xThetaLinearized,
                                                                   pointBefore,
                                                                   pointLast);

    } else if (positionsLinearized.count() == 1) {

      // Just use the single point
      yRadiusLinearized = posGraphBefore.y();

    } else {

      ENGAUGE_ASSERT (false);

    }
  }

  return yRadiusLinearized;
}

void ExportFileFunctions::loadYRadiusValues (const DocumentModelExportFormat &modelExportOverride,
                                             const Document &document,
                                             const MainWindowModel &modelMainWindow,
                                             const QStringList &curvesIncluded,
                                             const Transformation &transformation,
                                             bool isLogXTheta,
                                             bool isLogYRadius,
                                             const ExportValuesXOrY &xThetaValues,
                                             const CurveLimits &curveLimitsMin,
                                             const CurveLimits &curveLimitsMax,
                                             QVector<QVector<QString*> > &yRadiusValues) const
{

  // Loop through curves
  int curveCount = curvesIncluded.count();
  for (int col = 0; col < curveCount; col++) {

    const QString curveName = curvesIncluded.at (col);

    const Curve *curve = document.curveForCurveName (curveName);
    Points points = curve->points (); // These points will be linearized below if either coordinate is log

    if (modelExportOverride.pointsSelectionFunctions() == EXPORT_POINTS_SELECTION_FUNCTIONS_RAW) {

      // No interpolation. Just raw points. No transformation to/from linear space is required for log coordinates
      loadYRadiusValuesForCurveRaw (document.modelCoords(),
                                    document.modelGeneral(),
                                    modelMainWindow,
                                    points,
                                    xThetaValues,
                                    transformation,
                                    curveName,
                                    curveLimitsMin,
                                    curveLimitsMax,
                                    yRadiusValues [col]);
    } else {

      // Interpolation
      if (curve->curveStyle().lineStyle().curveConnectAs() == CONNECT_AS_FUNCTION_SMOOTH) {

        // Transformation to/from linear space is required for log coordinates
        loadYRadiusValuesForCurveInterpolatedSmooth (document.modelCoords(),
                                                     document.modelGeneral(),
                                                     modelMainWindow,
                                                     points,
                                                     xThetaValues,
                                                     transformation,
                                                     isLogXTheta,
                                                     isLogYRadius,
                                                     curveName,
                                                     curveLimitsMin,
                                                     curveLimitsMax,
                                                     yRadiusValues [col]);

      } else {

        // Transformation to/from linear space is required for log coordinates
        loadYRadiusValuesForCurveInterpolatedStraight (document.modelCoords(),
                                                       document.modelGeneral(),
                                                       modelMainWindow,
                                                       points,
                                                       xThetaValues,
                                                       transformation,
                                                       isLogXTheta,
                                                       isLogYRadius,
                                                       curveName,
                                                       curveLimitsMin,
                                                       curveLimitsMax,
                                                       yRadiusValues [col]);
      }
    }
  }
}

void ExportFileFunctions::loadYRadiusValuesForCurveInterpolatedSmooth (const DocumentModelCoords &modelCoords,
                                                                       const DocumentModelGeneral &modelGeneral,
                                                                       const MainWindowModel &modelMainWindow,
                                                                       const Points &points,
                                                                       const ExportValuesXOrY &xThetaValues,
                                                                       const Transformation &transformation,
                                                                       bool isLogXTheta,
                                                                       bool isLogYRadius,
                                                                       const QString &curveName,
                                                                       const CurveLimits &curveLimitsMin,
                                                                       const CurveLimits &curveLimitsMax,
                                                                       QVector<QString*> &yRadiusValues) const
{

  // Convert screen coordinates to graph coordinates, in vectors suitable for spline fitting
  vector<double> t;
  vector<SplinePair> xyLinearized;
  ExportOrdinalsSmooth ordinalsSmooth;

  // Create spline pairs with x/y coordinates in linearized coordinates and t as the independent variable
  ordinalsSmooth.loadSplinePairsWithTransformation (points,
                                                    transformation,
                                                    isLogXTheta,
                                                    isLogYRadius,
                                                    t,
                                                    xyLinearized);

  // Linearize/delinearize utility
  LinearToLog linearToLog;

  // Formatting
  FormatCoordsUnits format;
  QString dummyXThetaOut;

  if (points.count() == 0) {

    // Since there are no values, leave the field empty
    for (int row = 0; row < xThetaValues.count(); row++) {
      *(yRadiusValues [row]) = "";
    }

  } else if (points.count() == 1 ||
             points.count() == 2) {

    // Apply the single value everywhere (N=1) or do linear interpolation (N=2)
    for (int row = 0; row < xThetaValues.count(); row++) {

      // Compute unlinearized and linearized x/theta
      double xTheta = xThetaValues.at (row);
      double xThetaLinearized = linearToLog.linearize (xTheta,
                                                       isLogXTheta);

      double yRadiusLinearized;
      if (points.count() == 1) {
        yRadiusLinearized = xyLinearized.at (0).y ();
      } else {
        double x0 = xyLinearized.at (0).x ();
        double x1 = xyLinearized.at (1).x ();
        double y0 = xyLinearized.at (0).y ();
        double y1 = xyLinearized.at (1).y ();
        double numerator = (xThetaLinearized - x0);
        double denominator = (x1 - x0);
        if (qAbs (denominator) < qAbs (numerator) / 1.0e6) {
          // Cannot do linear interpolation using two points at the same x value
          yRadiusLinearized = xyLinearized.at (0).y ();
        } else {
          double s = numerator / denominator;
          yRadiusLinearized = (1.0 - s) * y0 + s * y1;
        }
      }

      // Delinearize, which is a noop for linear coordinates
      double yRadius = linearToLog.delinearize (yRadiusLinearized,
                                                isLogYRadius);

      if (xThetaIsNotOutOfBounds (xTheta,
                                  curveName,
                                  curveLimitsMin,
                                  curveLimitsMax)) {
        format.unformattedToFormatted (xTheta,
                                       yRadius,
                                       modelCoords,
                                       modelGeneral,
                                       modelMainWindow,
                                       dummyXThetaOut,
                                       *(yRadiusValues [row]),
                                       transformation);
      } else {
        *(yRadiusValues [row]) = "";
      }
    }

  } else {

    // Iteration accuracy versus number of iterations 8->256, 10->1024, 12->4096. Single pixel accuracy out of
    // typical image size of 1024x1024 means around 10 iterations gives decent accuracy for numbers much bigger
    // than 1. A value of 12 gave some differences in the least significant figures of numbers like 10^-3 in
    // the regression tests. Toggling between 30 and 32 made no difference in the regression tests.
    const int MAX_ITERATIONS = 32;

    // Spline class requires at least one point
    if (xyLinearized.size() > 0) {

      // Fit a spline
      Spline spline (t,
                     xyLinearized);

      // Get value at desired points
      for (int row = 0; row < xThetaValues.count(); row++) {

        // Compute unlinearized and linearized x/theta
        double xTheta = xThetaValues.at (row);
        double xThetaLinearized = linearToLog.linearize (xTheta,
                                                         isLogXTheta);

        SplinePair splinePairFound = spline.findSplinePairForFunctionX (xThetaLinearized,
                                                                        MAX_ITERATIONS);
        double yRadiusLinearized = splinePairFound.y ();

        // Delinearize, which is a noop for linear coordinates
        double yRadius = linearToLog.delinearize (yRadiusLinearized,
                                                  isLogYRadius);

        // Save y/radius value for this row into yRadiusValues, after appropriate formatting
        if (xThetaIsNotOutOfBounds (xTheta,
                                    curveName,
                                    curveLimitsMin,
                                    curveLimitsMax)) {
          format.unformattedToFormatted (xTheta,
                                         yRadius,
                                         modelCoords,
                                         modelGeneral,
                                         modelMainWindow,
                                         dummyXThetaOut,
                                         *(yRadiusValues [row]),
                                         transformation);
        } else {
          *(yRadiusValues [row]) = "";
        }
      }
    }
  }
}

void ExportFileFunctions::loadYRadiusValuesForCurveInterpolatedStraight (const DocumentModelCoords &modelCoords,
                                                                         const DocumentModelGeneral &modelGeneral,
                                                                         const MainWindowModel &modelMainWindow,
                                                                         const Points &points,
                                                                         const ExportValuesXOrY &xThetaValues,
                                                                         const Transformation &transformation,
                                                                         bool isLogXTheta,
                                                                         bool isLogYRadius,
                                                                         const QString &curveName,
                                                                         const CurveLimits &curveLimitsMin,
                                                                         const CurveLimits &curveLimitsMax,
                                                                         QVector<QString*> &yRadiusValues) const
{

  FormatCoordsUnits format;

  // Linearize/delinearize utility
  LinearToLog linearToLog;

  FittingPointsConvenient positionsLinearized = populateLinearizedFittingPositions (points,
                                                                                    transformation,
                                                                                    isLogXTheta,
                                                                                    isLogYRadius);

  // Get value at desired points
  QString dummyXThetaOut;
  for (int row = 0; row < xThetaValues.count(); row++) {

    // Compute unlinearized and linearized x/theta
    double xTheta = xThetaValues.at (row);
    double xThetaLinearized = linearToLog.linearize (xTheta,
                                                     isLogXTheta);

    // Interpolation on curve with no points will trigger an assert so check the point count
    *(yRadiusValues [row]) = "";
    if (positionsLinearized.count () > 0) {

      double yRadiusLinearized = linearlyInterpolate (positionsLinearized,
                                                      xThetaLinearized);
      double yRadius = linearToLog.delinearize (yRadiusLinearized,
                                                isLogYRadius);

      // Save y/radius value for this row into yRadiusValues, after appropriate formatting
      if (xThetaIsNotOutOfBounds (xTheta,
                                  curveName,
                                  curveLimitsMin,
                                  curveLimitsMax)) {
        format.unformattedToFormatted (xTheta,
                                       yRadius,
                                       modelCoords,
                                       modelGeneral,
                                       modelMainWindow,
                                       dummyXThetaOut,
                                       *(yRadiusValues [row]),
                                       transformation);
      }
    }
  }
}

void ExportFileFunctions::loadYRadiusValuesForCurveRaw (const DocumentModelCoords &modelCoords,
                                                        const DocumentModelGeneral &modelGeneral,
                                                        const MainWindowModel &modelMainWindow,
                                                        const Points &points,
                                                        const ExportValuesXOrY &xThetaValues,
                                                        const Transformation &transformation,
                                                        const QString &curveName,
                                                        const CurveLimits &curveLimitsMin,
                                                        const CurveLimits &curveLimitsMax,
                                                        QVector<QString*> &yRadiusValues) const
{

  FormatCoordsUnits format;

  // Since the curve points may be a subset of xThetaValues (in which case the non-applicable xThetaValues will have
  // blanks for the yRadiusValues), we iterate over the smaller set
  for (int pt = 0; pt < points.count(); pt++) {

    const Point &point = points.at (pt);

    QPointF posGraph;
    transformation.transformScreenToRawGraph (point.posScreen(),
                                              posGraph);

    // Find the closest point in xThetaValues. This is probably an N-squared algorithm, which is less than optimial,
    // but the delay should be insignificant with normal-sized export files
    double closestSeparation = 0.0;
    int rowClosest = 0;
    for (int row = 0; row < xThetaValues.count(); row++) {

      double xTheta = xThetaValues.at (row);

      double separation = qAbs (posGraph.x() - xTheta);

      if ((row == 0) ||
          (separation < closestSeparation)) {

        closestSeparation = separation;
        rowClosest = row;

      }
    }

    // Save y/radius value for this row into yRadiusValues, after appropriate formatting
    if (xThetaIsNotOutOfBounds (posGraph.x(),
                                curveName,
                                curveLimitsMin,
                                curveLimitsMax)) {
      QString dummyXThetaOut;
      format.unformattedToFormatted (posGraph.x(),
                                     posGraph.y(),
                                     modelCoords,
                                     modelGeneral,
                                     modelMainWindow,
                                     dummyXThetaOut,
                                     *(yRadiusValues [rowClosest]),
                                     transformation);
    } else {
      *(yRadiusValues [rowClosest]) = "";
    }
  }
}

void ExportFileFunctions::outputXThetaYRadiusValues (const DocumentModelExportFormat &modelExportOverride,
                                                     const DocumentModelCoords &modelCoords,
                                                     const DocumentModelGeneral &modelGeneral,
                                                     const MainWindowModel &modelMainWindow,
                                                     const QStringList &curvesIncluded,
                                                     const ExportValuesXOrY &xThetaValuesMerged,
                                                     const Transformation &transformation,
                                                     QVector<QVector<QString*> > &yRadiusValues,
                                                     const QString &delimiter,
                                                     QTextStream &str,
                                                     unsigned int &numWritesSoFar) const
{

  // Header
  if (modelExportOverride.header() != EXPORT_HEADER_NONE) {
    insertLineSeparator (numWritesSoFar == 0,
                         modelExportOverride.header (),
                         str);
    if (modelExportOverride.header() == EXPORT_HEADER_GNUPLOT) {
      str << gnuplotComment();
    }
    str << modelExportOverride.xLabel();
    QStringList::const_iterator itrHeader;
    for (itrHeader = curvesIncluded.begin(); itrHeader != curvesIncluded.end(); itrHeader++) {
      QString curveName = *itrHeader;
      str << delimiter << curveName;
    }
    str << "\n";
  }

  // Table body
  FormatCoordsUnits format;
  const double DUMMY_Y_RADIUS = 1.0;

  for (int row = 0; row < xThetaValuesMerged.count(); row++) {

    if (rowHasAtLeastOneYRadiusEntry (yRadiusValues,
                                      row)) {

      double xTheta = xThetaValuesMerged.at (row);

      // Output x/theta value for this row
      QString xThetaString, yRadiusString;
      format.unformattedToFormatted (xTheta,
                                     DUMMY_Y_RADIUS,
                                     modelCoords,
                                     modelGeneral,
                                     modelMainWindow,
                                     xThetaString,
                                     yRadiusString,
                                     transformation);
      str << wrapInDoubleQuotesIfNeeded (modelExportOverride,
                                         xThetaString);

      for (int col = 0; col < yRadiusValues.count(); col++) {

        QString yRadiusString = *(yRadiusValues [col] [row]);
        str << delimiter << wrapInDoubleQuotesIfNeeded (modelExportOverride,
                                                        yRadiusString);
      }

      str << "\n";
    }
  }

  ++numWritesSoFar;
}

FittingPointsConvenient ExportFileFunctions::populateLinearizedFittingPositions (const Points &points,
                                                                                 const Transformation &transformation,
                                                                                 bool isLogXTheta,
                                                                                 bool isLogYRadius) const
{
  LinearToLog linearToLog;

  FittingPointsConvenient positionsLinearized;
  Points::const_iterator itr;
  for (itr = points.begin(); itr != points.end(); itr++) {
    const Point &point = *itr;
    QPointF posScreen = point.posScreen();
    QPointF posGraph;
    transformation.transformScreenToRawGraph (posScreen,
                                              posGraph);

    QPointF positionLinearized (linearToLog.linearize (posGraph.x(),
                                                       isLogXTheta),
                                linearToLog.linearize (posGraph.y(),
                                                       isLogYRadius));
    positionsLinearized.push_back (positionLinearized);
  }

  return positionsLinearized;
}

bool ExportFileFunctions::rowHasAtLeastOneYRadiusEntry (const QVector<QVector<QString*> > &yRadiusValues,
                                                        int row) const
{
  bool hasEntry = false;

  for (int col = 0; col < yRadiusValues.count(); col++) {

    QString entry = *(yRadiusValues [col] [row]);
    if (!entry.isEmpty()) {

      hasEntry = true;
      break;

    }
  }

  return hasEntry;
}

bool ExportFileFunctions::xThetaIsNotOutOfBounds (double xTheta,
                                                  const QString &curveName,
                                                  const CurveLimits &curveLimitsMin,
                                                  const CurveLimits &curveLimitsMax) const
{
  bool ok = true;

  if (curveLimitsMin.contains (curveName)) {
    ok = ok && (curveLimitsMin [curveName] <= xTheta);
  }

  if (curveLimitsMax.contains (curveName)) {
    ok = ok && (xTheta <= curveLimitsMax [curveName]);
  }

  return ok;
}
