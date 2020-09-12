#pragma once

#include <QVector>
#include <QGraphicsItem>
#include <QSet>
#include "Block.h"
#include "Bonus.h"
#include <QTimer>
#include <QGraphicsSceneMouseEvent>

class BlocksGrid : public QObject, public QGraphicsRectItem {
  Q_OBJECT

  QVector<QVector<Block*>> _grid;
  QList<QPoint> _removedBlocks;
  QTimer* _waitBeforeDrop;

  int findMatches(int row, int column, QSet<int>& removedBlocks);
  void swapBlocks(const QPoint& coord1, const QPoint& coord2, bool updateRequired);
  bool isNeighbors(const QPoint& coord1, const QPoint& coord2) const;
  void dropBlocks();
  void spawnBonus(const QPoint& spawnPoint);

  void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override {}

public:
  BlocksGrid(int rowsNumber, int columnsNumber, const QRectF& rect);
  ~BlocksGrid();

  Block*& at(const QPoint& coord) { return _grid[coord.x()][coord.y()]; }
  const Block* at(const QPoint& coord) const { return _grid[coord.x()][coord.y()]; }
  QVector<Block*> operator[](size_t row) const { return _grid[row]; }
  
  int rowsN() const { return _grid.size(); }
  int columnsN() const { return _grid[0].size(); }
  QPoint calcIndexes(const QPointF& pos) const;

  bool sceneEventFilter(QGraphicsItem *watched, QEvent *event);

public slots:
  void updateBlocks();
  void removeBlocks(const QList<QPoint>& blocksList);
  void prolongWaitingTime(int time);

signals:
  void blocksRemoved();
};