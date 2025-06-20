/******************************************************************************************************
 * (C) 2016 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#include "GeometryModel.h"
#include "GeometryWindow.h"
#include "Logger.h"
#include <QTableView>

const int NO_HIGHLIGHTED_ROW = -1;

GeometryModel::GeometryModel () :
  m_rowToBeHighlighted (NO_HIGHLIGHTED_ROW)
{
}

GeometryModel::~GeometryModel()
{
}

QVariant GeometryModel::data(const QModelIndex &index, int role) const
{
  const int HEADER_OFFSET = 1;
  const int NUM_LEGEND_ROWS_UNSPANNED = 2; // Match with GeometryWindow::NUM_LEGEND_ROWS_UNSPANNED

  if ((role == Qt::BackgroundRole) &&
      !m_pointIdentifier.isEmpty () &&
      (index.row () == m_rowToBeHighlighted)) {

    // This row is to be highlighted with gray
    return QVariant (QColor (230, 230, 230));
  }

  bool ambiguousSegment = ((role == Qt::BackgroundRole) &&
                           (m_ambiguousRows.contains (index.row () - HEADER_OFFSET)));
  bool ambiguousFootnote = ((role == Qt::BackgroundRole) &&
                            (m_ambiguousRows.size () > 0) &&
                            (index.row () >= rowCount () - NUM_LEGEND_ROWS_UNSPANNED));
  if (ambiguousSegment || ambiguousFootnote) {

    // This row is to be highlighted with light red. Note that gray color preempts this behavior
    return QVariant (QColor (255, 0, 0, 50));
  }

  // Standard behavior
  return QStandardItemModel::data (index, role);
}

int GeometryModel::rowToBeHighlighted () const
{

  for (int row = 0; row < rowCount(); row++) {

    // Look at the point identifier in the hidden column
    QModelIndex indexPointIdentifier  = index (row,
                                               GeometryWindow::columnBodyPointIdentifiers ());
    QVariant var = QStandardItemModel::data (indexPointIdentifier, Qt::DisplayRole);
    if (var.isValid()) {
      QString pointIdentifierGot = var.toString();
      if (pointIdentifierGot == m_pointIdentifier) {

        // Found it
        return row;
      }
    }
  }

  // Fail
  return NO_HIGHLIGHTED_ROW;
}

void GeometryModel::setCurrentPointIdentifier (const QString &pointIdentifier)
{
  m_pointIdentifier = pointIdentifier;

  int rowTransitioned;
  if (!m_pointIdentifier.isEmpty ()) {

    // Get new row. It will transition from unhighlighted to highlighted
    m_rowToBeHighlighted = rowToBeHighlighted();
    rowTransitioned = m_rowToBeHighlighted;

  } else {

    // Old row will transition from highlighted to unhighlighted
    rowTransitioned = m_rowToBeHighlighted;
    m_rowToBeHighlighted = NO_HIGHLIGHTED_ROW;

  }

  QModelIndex indexTopLeft = createIndex (rowTransitioned, 0);
  QModelIndex indexBottomRight = createIndex (rowTransitioned, columnCount() - 1);

  QVector<int> roles;
  roles << Qt::BackgroundRole;

  emit dataChanged (indexTopLeft,
                    indexBottomRight,
                    roles);
}

void GeometryModel::setPotentialExportAmbiguity (const QVector<bool> &isPotentialExportAmbiguity)
{
  // Save row numbers with ambiguities
  m_ambiguousRows.clear ();
  for (int i = 0; i < isPotentialExportAmbiguity.size (); i++) {
    if (isPotentialExportAmbiguity.at (i)) {
      m_ambiguousRows [i] = true;
    }
  }
}
