#pragma once

#include <QGraphicsItem>

enum class BlockType {
  RED,
  GREEN,
  YELLOW,
  BLUE,
  CYAN,
  MAGENTA,
  EMPTY,
};

class Block : public QGraphicsObject {
Q_OBJECT
  static QSizeF _halfSizes;
  BlockType _type;
  QSharedPointer<QPixmap> _texture;

  void mousePressEvent(QGraphicsSceneMouseEvent *event);

public:
  Block(BlockType type = BlockType::EMPTY);

  BlockType blockType() const { return _type; }
  static QSizeF sizes() { return _halfSizes * 2; }
  const QSharedPointer<QPixmap> texture() { return _texture; }

  QRectF boundingRect() const;
  void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

  void setType(BlockType type);
  void setTexture(QSharedPointer<QPixmap> texture) { _texture = texture; }
  static void setSizes(const QSizeF& sizes) { _halfSizes = sizes / 2; }

signals:
  void typeChanged(BlockType currType, BlockType prevType);
};

QSharedPointer<QPixmap> defineBlockTexture(BlockType type);
