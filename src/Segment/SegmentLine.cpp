/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#include "DataKey.h"
#include "EnumsToQt.h"
#include "GraphicsItemType.h"
#include "InactiveOpacity.h"
#include "Logger.h"
#include <QGraphicsScene>
#include <QPen>
#include "Segment.h"
#include "SegmentLine.h"
#include "ZValues.h"

SegmentLine::SegmentLine(QGraphicsScene  &scene,
                         const DocumentModelSegments &modelSegments,
                         Segment *segment) :
  m_modelSegments (modelSegments),
  m_segment (segment)
{
  setData (DATA_KEY_GRAPHICS_ITEM_TYPE, QVariant (GRAPHICS_ITEM_TYPE_SEGMENT));

  // Make this transparent now, but always visible so hover events work
  scene.addItem (this);
  setPen (QPen (Qt::transparent));
  setZValue (Z_VALUE_CURVE);
  setVisible (true);
  setAcceptHoverEvents (true);
  setHover (false); // Initially the cursor is not hovering over this object. Later a hover event will change this state
  setFlags (QGraphicsItem::ItemIsFocusable);

  connect (this, SIGNAL (signalHover (bool)), segment, SLOT (slotHover (bool)));
}

SegmentLine::~SegmentLine ()
{
}

void SegmentLine::hoverEnterEvent(QGraphicsSceneHoverEvent * /* event */)
{

  emit (signalHover (true));
}

void SegmentLine::hoverLeaveEvent(QGraphicsSceneHoverEvent * /* event */)
{

  emit (signalHover (false));
}

void SegmentLine::mousePressEvent(QGraphicsSceneMouseEvent * /* event */)
{

  m_segment->forwardMousePress();
}

Segment *SegmentLine::segment() const
{
  return m_segment;
}

void SegmentLine::setHover (bool hover)
{
  QColor colorOpaque (ColorPaletteToQColor (m_modelSegments.lineColor()));

  if (hover) {

    setPen (QPen (QBrush (colorOpaque),
                  m_modelSegments.lineWidthActive()));

  } else {

    QColor colorSoft (colorOpaque.red (),
                      colorOpaque.green (),
                      colorOpaque.blue (),
                      inactiveOpacityEnumToAlpha (m_modelSegments.inactiveOpacity()));

    setPen (QPen (QBrush (colorSoft),
                  m_modelSegments.lineWidthInactive()));

  }
}

void SegmentLine::updateModelSegment(const DocumentModelSegments &modelSegments)
{

  m_modelSegments = modelSegments;
}
