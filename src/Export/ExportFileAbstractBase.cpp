/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#include "CoordScale.h"
#include "CurveConnectAs.h"
#include "Document.h"
#include "DocumentModelCoords.h"
#include "EngaugeAssert.h"
#include "ExportFileAbstractBase.h"
#include "Logger.h"
#include <qmath.h>
#include <QTextStream>
#include "Transformation.h"

using namespace std;

ExportFileAbstractBase::ExportFileAbstractBase()
{
}

ExportFileAbstractBase::~ExportFileAbstractBase()
{
}

QStringList ExportFileAbstractBase::curvesToInclude (const DocumentModelExportFormat &modelExportOverride,
                                                     const Document &document,
                                                     const QStringList &curvesGraphsNames,
                                                     CurveConnectAs curveConnectAs1,
                                                     CurveConnectAs curveConnectAs2) const
{

  QStringList curvesToInclude;

  // Build a list of curves to include by subtracting the excluded curves from the the complete list.
  // Special case is to use only first included curve if appropriate flag is set
  QStringList::const_iterator itr;
  for (itr = curvesGraphsNames.begin(); itr != curvesGraphsNames.end(); itr++) {

    QString curvesGraphName = *itr;

    if (!modelExportOverride.curveNamesNotExported().contains (curvesGraphName)) {

      const Curve *curve = document.curveForCurveName(curvesGraphName);
      ENGAUGE_CHECK_PTR (curve);

      // Not excluded which means it gets included, but only if it is a function
      if (curve->curveStyle().lineStyle().curveConnectAs() == curveConnectAs1 ||
          curve->curveStyle().lineStyle().curveConnectAs() == curveConnectAs2) {

        curvesToInclude.push_back (curvesGraphName);
      }
    }
  }

  return curvesToInclude;
}

void ExportFileAbstractBase::destroy2DArray (QVector<QVector<QString*> > &array) const
{

  int colCount = array.count();
  int rowCount = array [0].count();
  for (int row = 0; row < rowCount; row++) {
    for (int col = 0; col < colCount; col++) {
      delete array [col] [row];
    }
  }
}

QString ExportFileAbstractBase::gnuplotComment() const
{
  return QString ("# ");
}

void ExportFileAbstractBase::insertLineSeparator (bool isFirst,
                                                  ExportHeader exportHeader,
                                                  QTextStream &str) const
{
  // Insert line(s) between previous curve and this curve
  if (!isFirst) {
    if (exportHeader == EXPORT_HEADER_GNUPLOT) {
      str << "\n\n"; // Gnuplot requires two blank lines between curves
    } else {
      str << "\n"; // Single blank line
    }
  }
}

double ExportFileAbstractBase::linearlyInterpolateYRadiusFromTwoPoints (double xThetaLinearized,
                                                                        const QPointF &posGraphBefore,
                                                                        const QPointF &posGraph) const
{
  // Assumption is that inputs are linearized, so:
  // - output will also be linearized
  // - external code is responsible for linearizing and delinearizing
  double s = (xThetaLinearized - posGraphBefore.x()) / (posGraph.x() - posGraphBefore.x());

  double yRadiusLinearized = (1.0 - s) * posGraphBefore.y() + s * posGraph.y();

  return yRadiusLinearized;
}

QString ExportFileAbstractBase::wrapInDoubleQuotesIfNeeded (const DocumentModelExportFormat &modelExportOverride,
                                                            const QString &valueString) const
{
  QString newValueString = valueString;

  if ((modelExportOverride.delimiter () == EXPORT_DELIMITER_COMMA) &&
      (valueString.indexOf (",") >= 0)) {

    // Eliminate ambiguities according to RFC 4180
    newValueString = QString ("\"%1\"").arg (valueString);
  }

  return newValueString;
}
