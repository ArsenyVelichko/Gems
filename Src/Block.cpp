#include "Block.h"
#include <QPainter>
#include <QSharedPointer>

static QVector<QSharedPointer<QPixmap>> loadTexturesFiles(const QVector<QString>& texturePaths) {
  QVector<QSharedPointer<QPixmap>> blockTextures(texturePaths.size());

  for (int i = 0; i < blockTextures.size(); i++) {
    QPixmap texture(texturePaths[i]);
    texture = texture.scaled(Block::sizes().toSize(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    blockTextures[i] = QSharedPointer<QPixmap>::create(texture);
  }

  return blockTextures;
}

QSharedPointer<QPixmap> defineBlockTexture(BlockType type) {
  static QVector<QString> texturePaths = { ":/Gems/red", ":/Gems/green", ":/Gems/yellow", 
                                         ":/Gems/blue", ":/Gems/cyan", ":/Gems/magenta", ""};
  static QVector<QSharedPointer<QPixmap>> blockTextures = loadTexturesFiles(texturePaths);

  return blockTextures[int(type)];
}

QSizeF Block::_halfSizes = QSizeF();

QRectF Block::boundingRect() const {
  return QRectF(-_halfSizes.width(), -_halfSizes.height(), 2 * _halfSizes.width(), 2 * _halfSizes.height());
}

void Block::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
  painter->drawPixmap(-_halfSizes.width(), -_halfSizes.height(), *_texture);
  Q_UNUSED(option);
  Q_UNUSED(widget);
}

Block::Block(BlockType type) {
  _type = type;
  setTexture(defineBlockTexture(type));
  setFlag(QGraphicsItem::ItemIsSelectable);
  setAcceptedMouseButtons(Qt::LeftButton);
}

void Block::setType(BlockType type) {
  BlockType prevType = _type;
  _type = type;
  

  if (prevType != type) {
    setTexture(defineBlockTexture(type));
    emit typeChanged(_type, prevType);
    update();
  }
}

void Block::mousePressEvent(QGraphicsSceneMouseEvent *event) {
  setSelected(true);
}
