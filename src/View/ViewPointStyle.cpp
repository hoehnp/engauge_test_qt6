/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#include "EnumsToQt.h"
#include "Logger.h"
#include <QPainter>
#include "ViewPointStyle.h"

// Use solid background since transparency approach never worked, even with an alpha channel
const QColor COLOR_FOR_BRUSH_ENABLED (Qt::white);
const QColor COLOR_FOR_BRUSH_DISABLED (Qt::gray);

ViewPointStyle::ViewPointStyle(QWidget *parent) :
  QLabel (parent),
  m_enabled (false)
{
  // Note the size is set externally by the layout engine
}

QPixmap ViewPointStyle::pixmapForCurrentSettings () const
{

  // Polygon that is sized for the main drawing window.
  QPolygonF polygonUnscaled = m_pointStyle.polygon();

  // Resize polygon to fit icon, by builiding a new scaled polygon from the unscaled polygon
  double xMinGot = polygonUnscaled.boundingRect().left();
  double xMaxGot = polygonUnscaled.boundingRect().right();
  double yMinGot = polygonUnscaled.boundingRect().top();
  double yMaxGot = polygonUnscaled.boundingRect().bottom();

  QPolygonF polygonScaled;
  for (int i = 0; i < polygonUnscaled.length(); i++) {
    QPointF pOld = polygonUnscaled.at(i);
    polygonScaled.append (QPointF ((width () - 1) * (pOld.x() - xMinGot) / (xMaxGot - xMinGot),
                                   (height () - 1) * (pOld.y() - yMinGot) / (yMaxGot - yMinGot)));
  }

  // Color
  QColor color = ColorPaletteToQColor(m_pointStyle.paletteColor());
  if (!m_enabled) {
    color = QColor (Qt::black);
  }

  // Image for drawing
  QImage img (width (),
              height (),
              QImage::Format_RGB32);
  QPainter painter (&img);

  painter.fillRect (0,
                    0,
                    width (),
                    height (),
                    QBrush (m_enabled ? COLOR_FOR_BRUSH_ENABLED : COLOR_FOR_BRUSH_DISABLED));

  if (m_enabled) {
    painter.setPen (QPen (color, m_pointStyle.lineWidth()));
    painter.drawPolygon (polygonScaled);
  }

  // Create pixmap from image
  QPixmap pixmap = QPixmap::fromImage (img);

  return pixmap;
}

void ViewPointStyle::setEnabled (bool enabled)
{
  m_enabled = enabled;
  setPixmap (pixmapForCurrentSettings ());
}

void ViewPointStyle::setPointStyle (const PointStyle &pointStyle)
{

  m_pointStyle = pointStyle;
  setPixmap (pixmapForCurrentSettings ());
}

void ViewPointStyle::unsetPointStyle ()
{

  QPixmap pEmpty (width (),
                  height ());
  pEmpty.fill (COLOR_FOR_BRUSH_DISABLED);

  setPixmap (pEmpty);
}
