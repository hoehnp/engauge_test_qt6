/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#include "CurveStyle.h"
#include "DataKey.h"
#include "EnumsToQt.h"
#include "GeometryWindow.h"
#include "GraphicsItemType.h"
#include "GraphicsPoint.h"
#include "GraphicsPointEllipse.h"
#include "GraphicsPointPolygon.h"
#include "Logger.h"
#include "PointStyle.h"
#include <QGraphicsEllipseItem>
#include <QGraphicsPolygonItem>
#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QObject>
#include <QPen>
#include <QTextStream>
#include "QtToString.h"
#include "ZValues.h"

const double DEFAULT_HIGHLIGHT_OPACITY = 0.35; // 0=transparent to 1=opaque. Values above 0.5 are very hard to notice
const double MAX_OPACITY = 1.0;
const double ZERO_WIDTH = 0.0;

GraphicsPoint::GraphicsPoint(QGraphicsScene &scene,
                             const QString &identifier,
                             const QPointF &posScreen,
                             const QColor &color,
                             unsigned int radius,
                             double lineWidth,
                             GeometryWindow *geometryWindow) :
  GraphicsPointAbstractBase (),
  m_scene (scene),
  m_graphicsItemEllipse (nullptr),
  m_shadowZeroWidthEllipse (nullptr),
  m_graphicsItemPolygon (nullptr),
  m_shadowZeroWidthPolygon (nullptr),
  m_identifier (identifier),
  m_posScreen (posScreen),
  m_color (color),
  m_lineWidth (lineWidth),
  m_wanted (true),
  m_highlightOpacity (DEFAULT_HIGHLIGHT_OPACITY),
  m_geometryWindow (geometryWindow)
{
  createPointEllipse (radius);
}

GraphicsPoint::GraphicsPoint(QGraphicsScene &scene,
                             const QString &identifier,
                             const QPointF &posScreen,
                             const QColor &color,
                             const QPolygonF &polygon,
                             double lineWidth,
                             GeometryWindow *geometryWindow) :
  GraphicsPointAbstractBase (),
  m_scene (scene),
  m_graphicsItemEllipse (nullptr),
  m_shadowZeroWidthEllipse (nullptr),
  m_graphicsItemPolygon (nullptr),
  m_shadowZeroWidthPolygon (nullptr),
  m_identifier (identifier),
  m_posScreen (posScreen),
  m_color (color),
  m_lineWidth (lineWidth),
  m_wanted (true),
  m_highlightOpacity (DEFAULT_HIGHLIGHT_OPACITY),
  m_geometryWindow (geometryWindow)
{
  createPointPolygon (polygon);
}

GraphicsPoint::~GraphicsPoint()
{
  if (m_graphicsItemEllipse == nullptr) {

    QGraphicsScene *scene = m_graphicsItemPolygon->scene();

    // Since m_shadowZeroWidthPolygon is a child of m_graphicsItemPolygon, removing the parent removes both
    scene->removeItem (m_graphicsItemPolygon);
    delete m_graphicsItemPolygon;
    m_graphicsItemPolygon = nullptr;
    m_shadowZeroWidthPolygon = nullptr;


  } else {

    QGraphicsScene *scene = m_graphicsItemEllipse->scene();

    // Since m_shadowZeroWidthEllipse is a child of m_graphicsItemEllipse, removing the parent removes both
    scene->removeItem (m_graphicsItemEllipse);
    delete m_graphicsItemEllipse;
    m_graphicsItemEllipse = nullptr;
    m_shadowZeroWidthEllipse = nullptr;

  }
}

QRectF GraphicsPoint::boundingRect () const
{
  if (m_graphicsItemEllipse == nullptr) {
    return m_graphicsItemPolygon->boundingRect ();
  } else {
    return m_graphicsItemEllipse->boundingRect ();
  }
}

void GraphicsPoint::createPointEllipse (unsigned int radius)
{
  const int radiusSigned = signed (radius); // Radius must be signed before multiplying by -1 below, for Visual Studio
  m_graphicsItemEllipse = new GraphicsPointEllipse (*this,
                                                    QRect (- radiusSigned,
                                                           - radiusSigned,
                                                           2 * radiusSigned + 1,
                                                           2 * radiusSigned + 1));
  m_scene.addItem (m_graphicsItemEllipse);

  m_graphicsItemEllipse->setZValue (Z_VALUE_POINT);
  m_graphicsItemEllipse->setData (DATA_KEY_IDENTIFIER, m_identifier);
  m_graphicsItemEllipse->setData (DATA_KEY_GRAPHICS_ITEM_TYPE, GRAPHICS_ITEM_TYPE_POINT);
  m_graphicsItemEllipse->setPos (m_posScreen.x (),
                                 m_posScreen.y ());
  m_graphicsItemEllipse->setPen (QPen (QBrush (m_color), m_lineWidth));
  m_graphicsItemEllipse->setEnabled (true);
  m_graphicsItemEllipse->setFlags (QGraphicsItem::ItemIsSelectable |
                                   QGraphicsItem::ItemIsMovable |
                                   QGraphicsItem::ItemSendsGeometryChanges);
  m_graphicsItemEllipse->setData (DATA_KEY_GRAPHICS_ITEM_TYPE, GRAPHICS_ITEM_TYPE_POINT);
  if (m_geometryWindow != nullptr) {
    QObject::connect (m_graphicsItemEllipse, SIGNAL (signalPointHoverEnter (QString)), m_geometryWindow, SLOT (slotPointHoverEnter (QString)));
    QObject::connect (m_graphicsItemEllipse, SIGNAL (signalPointHoverLeave (QString)), m_geometryWindow, SLOT (slotPointHoverLeave (QString)));
  }

  // Shadow item is not selectable so it needs no stored data. Do NOT
  // call QGraphicsScene::addItem since the QGraphicsItem::setParentItem call adds the item
  m_shadowZeroWidthEllipse = new GraphicsPointEllipse (*this,
                                                       QRect (- radiusSigned,
                                                              - radiusSigned,
                                                              2 * radiusSigned + 1,
                                                              2 * radiusSigned + 1));
  m_shadowZeroWidthEllipse->setParentItem(m_graphicsItemPolygon); // Dragging parent also drags child

  m_shadowZeroWidthEllipse->setPen (QPen (QBrush (m_color), ZERO_WIDTH));
  m_shadowZeroWidthEllipse->setEnabled (true);

  m_graphicsItemEllipse->setShadow (m_shadowZeroWidthEllipse);
}

void GraphicsPoint::createPointPolygon (const QPolygonF &polygon)
{
  m_graphicsItemPolygon = new GraphicsPointPolygon (*this,
                                                    polygon);
  m_scene.addItem (m_graphicsItemPolygon);

  m_graphicsItemPolygon->setZValue (Z_VALUE_POINT);
  m_graphicsItemPolygon->setData (DATA_KEY_IDENTIFIER, m_identifier);
  m_graphicsItemPolygon->setData (DATA_KEY_GRAPHICS_ITEM_TYPE, GRAPHICS_ITEM_TYPE_POINT);
  m_graphicsItemPolygon->setPos (m_posScreen.x (),
                                 m_posScreen.y ());
  m_graphicsItemPolygon->setPen (QPen (QBrush (m_color), m_lineWidth));
  m_graphicsItemPolygon->setEnabled (true);
  m_graphicsItemPolygon->setFlags (QGraphicsItem::ItemIsSelectable |
                                   QGraphicsItem::ItemIsMovable |
                                   QGraphicsItem::ItemSendsGeometryChanges);
  m_graphicsItemPolygon->setData (DATA_KEY_GRAPHICS_ITEM_TYPE, GRAPHICS_ITEM_TYPE_POINT);
  if (m_geometryWindow != nullptr) {
    QObject::connect (m_graphicsItemPolygon, SIGNAL (signalPointHoverEnter (QString)), m_geometryWindow, SLOT (slotPointHoverEnter (QString)));
    QObject::connect (m_graphicsItemPolygon, SIGNAL (signalPointHoverLeave (QString)), m_geometryWindow, SLOT (slotPointHoverLeave (QString)));
  }

  // Shadow item is not selectable so it needs no stored data. Do NOT
  // call QGraphicsScene::addItem since the QGraphicsItem::setParentItem call adds the item
  m_shadowZeroWidthPolygon = new GraphicsPointPolygon (*this,
                                                       polygon);
  m_shadowZeroWidthPolygon->setParentItem(m_graphicsItemPolygon); // Dragging parent also drags child

  m_shadowZeroWidthPolygon->setPen (QPen (QBrush (m_color), ZERO_WIDTH));
  m_shadowZeroWidthPolygon->setEnabled (true);

  m_graphicsItemPolygon->setShadow (m_shadowZeroWidthPolygon);
}

QVariant GraphicsPoint::data (int key) const
{
  if (m_graphicsItemEllipse == nullptr) {
    return m_graphicsItemPolygon->data (key);
  } else {
    return m_graphicsItemEllipse->data (key);
  }
}

double GraphicsPoint::highlightOpacity () const
{
  return m_highlightOpacity;
}

QPointF GraphicsPoint::pos () const
{
  if (m_graphicsItemEllipse == nullptr) {
    return m_graphicsItemPolygon->pos ();
  } else {
    return m_graphicsItemEllipse->pos ();
  }
}

void GraphicsPoint::printStream (QString indentation,
                                 QTextStream &str,
                                 double ordinalKey) const
{
  str << indentation << "GraphicsPoint\n";

  indentation += INDENTATION_DELTA;

  QString identifier;
  QString pointType;
  QPointF pos;
  if (m_graphicsItemEllipse == nullptr) {
    identifier = m_graphicsItemPolygon->data (DATA_KEY_IDENTIFIER).toString ();
    pointType = "polygon";
    pos = m_graphicsItemPolygon->pos();
  } else {
    identifier = m_graphicsItemEllipse->data (DATA_KEY_IDENTIFIER).toString ();
    pointType = "ellipse";
    pos = m_graphicsItemEllipse->pos();
  }

  DataKey type = static_cast<DataKey> (data (DATA_KEY_GRAPHICS_ITEM_TYPE).toInt());

  str << indentation << identifier
      << " ordinalKey=" << ordinalKey
      << " dataIdentifier=" << data (DATA_KEY_IDENTIFIER).toString().toLatin1().data()
      << " dataType=" << dataKeyToString (type).toLatin1().data()
      << " " << pointType << "Pos=" << QPointFToString (pos) << "\n";
}

void GraphicsPoint::reset ()
{
  m_wanted = false;
}

void GraphicsPoint::setData (int key, const QVariant &data)
{
  if (m_graphicsItemEllipse == nullptr) {
    m_graphicsItemPolygon->setData (key, data);
  } else {
    m_graphicsItemEllipse->setData (key, data);
  }
}

void GraphicsPoint::setHighlightOpacity (double highlightOpacity)
{
  m_highlightOpacity = highlightOpacity;
}

void GraphicsPoint::setPassive ()
{
  if (m_graphicsItemEllipse == nullptr) {
    m_graphicsItemPolygon->setFlag (QGraphicsItem::ItemIsFocusable, false);
    m_graphicsItemPolygon->setFlag (QGraphicsItem::ItemIsMovable, false);
    m_graphicsItemPolygon->setFlag (QGraphicsItem::ItemIsSelectable, false);
  } else {
    m_graphicsItemEllipse->setFlag (QGraphicsItem::ItemIsFocusable, false);
    m_graphicsItemEllipse->setFlag (QGraphicsItem::ItemIsMovable, false);
    m_graphicsItemEllipse->setFlag (QGraphicsItem::ItemIsSelectable, false);
  }
}

void GraphicsPoint::setPointStyle(const PointStyle &pointStyle)
{
  // Setting pen and radius of parent graphics items below also affects the child shadows
  // (m_shadowItemPolygon and m_shadowItemEllipse)
  if (m_graphicsItemEllipse == nullptr) {
    if (pointStyle.shape() == POINT_SHAPE_CIRCLE) {

      // Transition from non-circle to circle. Deleting parent also deletes child shadow
      delete m_graphicsItemPolygon;
      m_graphicsItemPolygon = nullptr;
      m_shadowZeroWidthPolygon = nullptr;

      createPointEllipse (unsigned (pointStyle.radius()));

    } else {

      // Update polygon
      m_graphicsItemPolygon->setPen (QPen (ColorPaletteToQColor(pointStyle.paletteColor()),
                                           pointStyle.lineWidth()));
      m_shadowZeroWidthPolygon->setPen (QPen (ColorPaletteToQColor(pointStyle.paletteColor()),
                                              pointStyle.lineWidth()));
      m_graphicsItemPolygon->setPolygon (pointStyle.polygon());
      m_shadowZeroWidthPolygon->setPolygon (pointStyle.polygon());

    }
  } else {
    if (pointStyle.shape() != POINT_SHAPE_CIRCLE) {

      // Transition from circle to non-circlee. Deleting parent also deletes child shadow
      delete m_graphicsItemEllipse;
      m_graphicsItemEllipse = nullptr;
      m_shadowZeroWidthEllipse = nullptr;

      createPointPolygon (pointStyle.polygon());

    } else {

      // Update circle
      m_graphicsItemEllipse->setPen (QPen (ColorPaletteToQColor(pointStyle.paletteColor()),
                                           pointStyle.lineWidth()));
      m_shadowZeroWidthEllipse->setPen (QPen (ColorPaletteToQColor(pointStyle.paletteColor()),
                                           pointStyle.lineWidth()));
      m_graphicsItemEllipse->setRadius (pointStyle.radius());
      m_shadowZeroWidthEllipse->setRadius (pointStyle.radius());
    }
  }
}

void GraphicsPoint::setPos (const QPointF pos)
{
  if (m_graphicsItemEllipse == nullptr) {
    m_graphicsItemPolygon->setPos (pos);
  } else {
    m_graphicsItemEllipse->setPos (pos);
  }
}

void GraphicsPoint::setWanted ()
{
  m_wanted = true;
}

void GraphicsPoint::updateCurveStyle (const CurveStyle &curveStyle)
{
  setPointStyle (curveStyle.pointStyle()); // This point
}

bool GraphicsPoint::wanted () const
{
  return m_wanted;
}
