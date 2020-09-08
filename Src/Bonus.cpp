#include "Bonus.h"
#include "BlocksGrid.h"
#include <QPainter>
#include <QTimer>
#include <QPropertyAnimation>

static QVector<QImage*> loadTexturesFiles(const QVector<QString>& texturePaths, const QSize& scaleTo) {
  QVector<QImage*> blockTextures(texturePaths.size());

  for (int i = 0; i < blockTextures.size(); i++) {
    QImage texture(texturePaths[i]);
    blockTextures[i] = new QImage(texture.scaled(scaleTo,
      Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
  }

  return blockTextures;
}

void Bonus::nextStage() {
  _currStage = BonusStage(int(_currStage) + 1);

  if (_currStage == BonusStage::ACTIVATED) {
    activate();
  }
}

FrameByFrameAnimation::FrameByFrameAnimation(int duration, const QVector<QImage*>& frames) {
  _frames = frames;
  _frameDuration = duration / _frames.size();
  _currFrameIndex = 0;
}

void FrameByFrameAnimation::updateCurrentTime(int currentTime) {
  int prevIndex = _currFrameIndex;
  _currFrameIndex = currentTime / _frameDuration;

  if (prevIndex != _currFrameIndex) {
    emit frameChanged();
  }
}

int FrameByFrameAnimation::duration() const {
  return _frameDuration * _frames.size() - 1;
}

LinkedBonus::LinkedBonus(Block* targetBlock) : _targetBlock(targetBlock) {
  setParentItem(targetBlock);
  QObject::connect(targetBlock, &Block::typeChanged, this, &ExplosionWave::blockTypeChange);

  _boundRect = targetBlock->boundingRect();
}

void LinkedBonus::setAnimation(FrameByFrameAnimation* animation) {
  delete _animation;
  _animation = animation;
}

void LinkedBonus::blockTypeChange(BlockType currType, BlockType prevType) {
  if (currType == BlockType::EMPTY) {

    if (stage() != BonusStage::CREATED) {
      nextStage();

    } else {
      _targetBlock->setType(prevType);
    }
  }
}

QVector<QImage*> ExplosionWave::_lighthingFrames = QVector<QImage*>();
QVector<QImage*> ExplosionWave::_thunderStrikeFrames = QVector<QImage*>();

void ExplosionWave::init(int maxGridDimension) {
  QVector<QString> texturePaths = { ":/Animations/LFrame1", ":/Animations/LFrame2", ":/Animations/LFrame3",
      ":/Animations/LFrame4", ":/Animations/LFrame5", ":/Animations/LFrame6", ":/Animations/LFrame7" };
  _lighthingFrames = loadTexturesFiles(texturePaths, Block::sizes().toSize());

  texturePaths = { ":/Animations/TFrame1", ":/Animations/TFrame2", ":/Animations/TFrame3",
    ":/Animations/TFrame4", ":/Animations/TFrame5", ":/Animations/TFrame6", ":/Animations/TFrame7",
    ":/Animations/TFrame8", ":/Animations/TFrame9", ":/Animations/TFrame10" };
  int minSideLen = qMin(Block::sizes().width(), Block::sizes().height());
  QSize textureSize(Block::sizes().width() * (maxGridDimension - 1), minSideLen);
  _thunderStrikeFrames = loadTexturesFiles(texturePaths, textureSize);
}


void ExplosionWave::clear() {
  for (QImage* texture : _lighthingFrames) {
    delete texture;
  }
  _lighthingFrames.clear();

  for (QImage* texture : _thunderStrikeFrames) {
    delete texture;
  }
  _thunderStrikeFrames.clear();
}

ExplosionWave::ExplosionWave(Block* targetBlock, BlocksGrid* grid) : _grid(grid), LinkedBonus(targetBlock) {
  FrameByFrameAnimation* currAnimation = new FrameByFrameAnimation(LIGHTNING_DURATION, _lighthingFrames);
  currAnimation->setLoopCount(-1);
  QObject::connect(currAnimation, &FrameByFrameAnimation::frameChanged, [this]() {
    update();
  });
  setAnimation(currAnimation);

  currAnimation->start();
}

void ExplosionWave::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
  QImage frame = *animation()->currFrame();

  if (stage() == BonusStage::IDLE) {
    painter->drawImage(-frame.width() / 2, -frame.height() / 2, frame);

  } else if (stage() == BonusStage::ACTIVATED) {
    double halfBlockWidth = Block::sizes().width() / 2;
    double halfBlockHeight = Block::sizes().height() / 2;

    painter->drawImage(halfBlockWidth, -frame.height() / 2, frame);
    painter->drawImage(-halfBlockWidth - frame.width(), -frame.height() / 2, frame.mirrored(true, false));

    QMatrix rotateMatrix;
    rotateMatrix.rotate(90);
    QImage rotatedFrame = frame.transformed(rotateMatrix);

    painter->drawImage(-rotatedFrame.width() / 2, -halfBlockHeight - rotatedFrame.height(), rotatedFrame);
    painter->drawImage(-rotatedFrame.width() / 2, halfBlockHeight, rotatedFrame.mirrored());
  }
  Q_UNUSED(option);
  Q_UNUSED(widget);
}

void ExplosionWave::activate() {
  QPoint targetCoord = _grid->calcIndexes(targetBlock()->pos());
  QList<QPoint> blocksToRemove;
  int targetRow = targetCoord.x();
  int targetColumn = targetCoord.y();

  for (int i = 0; i < _grid->rowsN(); i++) {
    blocksToRemove.append(QPoint(i, targetColumn));
  }

  for (int i = 0; i < _grid->columnsN(); i++) {
    blocksToRemove.append(QPoint(targetRow, i));
  }
  _grid->removeBlocks(blocksToRemove);

  setBoundingRect(_grid->boundingRect().translated(-targetBlock()->pos()));
  setParentItem(_grid);
  setPos(targetBlock()->pos());

  FrameByFrameAnimation* currAnimation = new FrameByFrameAnimation(THUNDER_DURATION, _thunderStrikeFrames);
  QObject::connect(currAnimation, &FrameByFrameAnimation::finished, this, &ExplosionWave::deleteLater);
  QObject::connect(currAnimation, &FrameByFrameAnimation::frameChanged, [this]() {
    update();
  });
  setAnimation(currAnimation);

  QTimer::singleShot(THUNDER_DURATION / 2, [this, blocksToRemove] {
    for (QPoint coord : blocksToRemove) {
      _grid->at(coord)->setType(BlockType::EMPTY);
    }
  });

  currAnimation->start();
  _grid->prolongWaitingTime(THUNDER_DURATION);
}

InterfaceQueue TypeChange::_interfaceQueue = InterfaceQueue(QPointF(-550, -200), QVector2D(0, 60));
QVector<QImage*> TypeChange::_fireFrames = QVector<QImage*>();

void TypeChange::init() {
  QVector<QString> texturePaths = { ":/Animations/FFrame1", ":/Animations/FFrame2", ":/Animations/FFrame3",
    ":/Animations/FFrame4", ":/Animations/FFrame5", ":/Animations/FFrame6", ":/Animations/FFrame7",
    ":/Animations/FFrame8", ":/Animations/FFrame9", ":/Animations/FFrame10" };
  _fireFrames = loadTexturesFiles(texturePaths, Block::sizes().toSize());
}

void TypeChange::clear() {
  for (QImage* texture : _fireFrames) {
    delete texture;
  }
}

TypeChange::TypeChange(Block* targetBlock, BlocksGrid* grid) : _grid(grid), LinkedBonus(targetBlock) {
  _type = targetBlock->blockType();

  FrameByFrameAnimation* currAnimation = new FrameByFrameAnimation(LIGHTNING_DURATION, _fireFrames);
  currAnimation->setLoopCount(-1);
  QObject::connect(currAnimation, &FrameByFrameAnimation::frameChanged, [this]() {
    update();
  });
  setAnimation(currAnimation);

  currAnimation->start();
  _changesRemain = TYPE_CHANGES_NUMBER;
  setAcceptedMouseButtons(Qt::LeftButton);
}

void TypeChange::activate() {
  setAnimation(nullptr);
  setBoundingRect(_grid->boundingRect());
  setParentItem(_grid);

  QObject::disconnect(targetBlock(), &Block::typeChanged, this, &TypeChange::blockTypeChange);

  _interface = new TypeChangeInterface(this);
  scene()->addItem(_interface);
  _interfaceQueue.enqueue(_interface);
}

void TypeChange::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
  if (stage() == BonusStage::IDLE) {
    QImage frame = *animation()->currFrame();
    painter->drawImage(-frame.width() / 2, -frame.height() / 2, frame);
  }
  Q_UNUSED(option);
  Q_UNUSED(widget);
}

void TypeChange::mousePressEvent(QGraphicsSceneMouseEvent* event) {
  if (stage() == BonusStage::ACTIVATED && _interfaceQueue.head() == _interface) {
    _grid->setEnabled(false);
    QGraphicsSceneMouseEvent* mouseEvent = static_cast<QGraphicsSceneMouseEvent*>(event);
    _changesRemain--;
    _interface->update();

    QPointF blockPos = _grid->mapFromScene(mouseEvent->scenePos());
    QPoint blockCoord = _grid->calcIndexes(blockPos);
    _grid->at(blockCoord)->setType(_type);

    QList<QPoint> changedBlocks = { blockCoord };
    _grid->removeBlocks(changedBlocks);
    _grid->updateBlocks();

    if (_changesRemain == 0) {
      _interfaceQueue.dequeue();
      deleteLater();
    }

  } else {
    event->ignore();
  }
}

QVector<QStaticText*> TypeChangeInterface::_rasterizedNums = QVector<QStaticText*>(TYPE_CHANGES_NUMBER + 1);

void TypeChangeInterface::init() {
  for (int i = 0; i <= TYPE_CHANGES_NUMBER; i++) {
    QFont font("Helvetica [Adobe]", 16, QFont::Bold);
    TypeChangeInterface::_rasterizedNums[i] = new QStaticText(QString::number(i));
    _rasterizedNums[i]->prepare(QTransform(), font);
  }
}

void TypeChangeInterface::clear() {
  for (QStaticText* text : _rasterizedNums) {
    delete text;
  }
}

TypeChangeInterface::TypeChangeInterface(TypeChange* parentBonus) : _bonus(parentBonus) {
  _texture = defineBlockTexture(parentBonus->blockType()).data();
  _boundRect = _texture->rect().translated(-_texture->rect().center());
}

void TypeChangeInterface::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
  painter->drawPixmap(-_texture->width() / 2, -_texture->height() / 2, *_texture);

  QFont font("Helvetica [Adobe]", 16, QFont::Bold);
  painter->setFont(font);

  QStaticText text = *_rasterizedNums[_bonus->changesRemain()];
  QSizeF textSize = text.size();
  QPointF leftTop(-textSize.width() / 2, -textSize.height() / 2);
  painter->drawStaticText(leftTop, text);
  Q_UNUSED(widget);
  Q_UNUSED(option);
}