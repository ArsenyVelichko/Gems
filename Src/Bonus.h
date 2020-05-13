#ifndef BONUS_H
#define BONUS_H

#include <QList>
#include <QAbstractAnimation>
#include "Block.h"
#include <QGraphicsScene>
#include <QStaticText>
#include "SupportStructures.h"

#define THUNDER_DURATION 750
#define LIGHTNING_DURATION 750
#define TYPE_CHANGES_NUMBER 2

class BlocksGrid;

class FrameByFrameAnimation : public QAbstractAnimation {
  Q_OBJECT
  QVector<QImage*> _frames;
  int _frameDuration;
  int _currFrameIndex;

protected:
  void updateCurrentTime(int currentTime) override;

public:
  FrameByFrameAnimation(int duration, const QVector<QImage*>& frames);

  void setFrames(const QVector<QImage*>& frames) { _frames = frames; }
  void setDuration(int duration) { _frameDuration = duration / _frames.size(); }

  QImage* currFrame() const { return _frames[_currFrameIndex]; }

  int duration() const override;

signals:
  void frameChanged();
};

enum class BonusStage {
  CREATED,
  IDLE,
  ACTIVATED,
};

class Bonus : public QObject {
  Q_OBJECT
  BonusStage _currStage = BonusStage::CREATED;
  
protected:
  virtual void activate() = 0;

public:
  BonusStage stage() { return _currStage; }

public slots:
  void nextStage();
};

class LinkedBonus : public Bonus, public QGraphicsItem {
  Block* _targetBlock;
  FrameByFrameAnimation* _animation = nullptr;
  QRectF _boundRect;

protected:
  LinkedBonus(Block* targetBlock);
  ~LinkedBonus() { delete _animation; }

  QRectF boundingRect() const override { return _boundRect; }

  Block* targetBlock() const { return _targetBlock; }
  FrameByFrameAnimation* animation() const { return _animation; }
  void setAnimation(FrameByFrameAnimation* animation);
  void setBoundingRect(const QRectF& boundRect) { _boundRect = boundRect; }

protected:
  void blockTypeChange(BlockType currType, BlockType prevType);
};

class ExplosionWave : public LinkedBonus {
  BlocksGrid* _grid;
  static QVector<QImage*> _lighthingFrames;
  static QVector<QImage*> _thunderStrikeFrames;

  void activate() override;
  void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

public:
  ExplosionWave(Block* targetBlock, BlocksGrid* grid);

  static void init(int maxGridDimension);
  static void clear();
};

class TypeChange;

class TypeChangeInterface : public QGraphicsObject {
  TypeChange* _bonus;
  QRectF _boundRect;
  QPixmap* _texture;

  static QVector<QStaticText*> _rasterizedNums;

public:
  TypeChangeInterface::TypeChangeInterface(TypeChange* parentBonus);

  static void init();
  static void clear();

  TypeChange* parentBonus() { return _bonus; }

  QRectF boundingRect() const override { return _boundRect; }
  void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
};

class TypeChange : public LinkedBonus {
  BlocksGrid* _grid;
  int _changesRemain;
  BlockType _type;
  TypeChangeInterface* _interface = nullptr;

  static InterfaceQueue _interfaceQueue;
  static QVector<QImage*> _fireFrames;

  void activate() override;
  void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
  void mousePressEvent(QGraphicsSceneMouseEvent *event);

public:
  TypeChange(Block* targetBlock, BlocksGrid* grid);
  ~TypeChange() { delete _interface; }

  int changesRemain() const { return _changesRemain; }
  BlockType blockType() const { return _type; }

  static void init();
  static void clear();
};

#endif //BONUS_H