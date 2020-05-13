#include "BlocksGrid.h"
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>

#define MIN_DURATION 350
#define DURATION_STEP 50

BlocksGrid::BlocksGrid(int rowsNumber, int columnsNumber, const QRectF& rect) {
  _waitBeforeDrop.setSingleShot(true);
  QObject::connect(&_waitBeforeDrop, &QTimer::timeout, this, &BlocksGrid::dropBlocks);

  _boundRect = rect;
  double blockWidth = rect.width() / columnsNumber;
  double blockHeight = rect.height() / rowsNumber;
  Block::setSizes(QSizeF(blockWidth, blockHeight));

  _grid.resize(rowsNumber);
  for (int i = 0; i < rowsNumber; i++) {
    _grid[i].resize(columnsNumber);

    for (int j = 0; j < columnsNumber; j++) {
      _grid[i][j] = new Block();
      _grid[i][j]->setParentItem(this);

      double posX = blockWidth * (j + 0.5 - columnsNumber / 2.0);
      double posY = blockHeight * (i + 0.5 - rowsNumber / 2.0);
      _grid[i][j]->setPos(posX, posY);
    }
  }

  for (int i = 0; i < rowsNumber; i++) {
    for (int j = 0; j < columnsNumber; j++) {

      int filledBlocks;
      do {
        BlockType type = BlockType(rand() % (int)BlockType::EMPTY);
        _grid[i][j]->setType(type);

      } while (findMatches(i, j, QSet<int>()) >= 3);
    }
  }
  
  setFiltersChildEvents(true);
  setFlags(QGraphicsItem::ItemContainsChildrenInShape |
           QGraphicsItem::ItemHasNoContents);
}

int BlocksGrid::findMatches(int row, int column, QSet<int>& removedBlocks) {
  int scalarCoord = row * columnsN() + column;

  if (!removedBlocks.contains(scalarCoord)) {
    int matchesNumber = 1;
    BlockType type = _grid[row][column]->blockType();
    removedBlocks.insert(scalarCoord);

    if (row > 0 && _grid[row - 1][column]->blockType() == type) {
      matchesNumber += findMatches(row - 1, column, removedBlocks);
    }

    if (row < rowsN() - 1 && _grid[row + 1][column]->blockType() == type) {
      matchesNumber += findMatches(row + 1, column, removedBlocks);
    }

    if (column > 0 && _grid[row][column - 1]->blockType() == type) {
      matchesNumber += findMatches(row, column - 1, removedBlocks);
    }

    if (column < columnsN() - 1 && _grid[row][column + 1]->blockType() == type) {
      matchesNumber += findMatches(row, column + 1, removedBlocks);
    }

    return matchesNumber;
  }
  return 0;
}

void BlocksGrid::dropBlocks() {
  emit blocksRemoved();
  _waitBeforeDrop.setInterval(0);

  QVector<int> lowestIndexes(columnsN(), -1);

  for (QPoint coord : _removedBlocks) {
    if (at(coord)->blockType() == BlockType::EMPTY) {
      lowestIndexes[coord.y()] = qMax(lowestIndexes[coord.y()], coord.x());
    }
  }
  _removedBlocks.clear();

  QParallelAnimationGroup* dropGroup = new QParallelAnimationGroup;
  double blockHeight = Block::sizes().height();
  double startY = _boundRect.top() - blockHeight / 2;

  for (int j = 0; j < lowestIndexes.size(); j++) {

    QList<Block*> emptyBlocks;
    int endRow = lowestIndexes[j];

    for (int i = lowestIndexes[j]; i >= 0; i--) {
      _removedBlocks.append(QPoint(i, j));

      if (_grid[i][j]->blockType() == BlockType::EMPTY) {
        emptyBlocks.append(_grid[i][j]);

      } else {
        QPropertyAnimation* blockMoving = new QPropertyAnimation(_grid[i][j], "pos");
        blockMoving->setEndValue(_grid[endRow][j]->pos());
        blockMoving->setEasingCurve(QEasingCurve::InOutSine);

        int duration = MIN_DURATION + (endRow - i - 1) * DURATION_STEP;
        blockMoving->setDuration(duration);

        dropGroup->addAnimation(blockMoving);

        _grid[endRow][j] = _grid[i][j];
        endRow--;
      }
    }

    int duration = MIN_DURATION + endRow * DURATION_STEP;
    for (Block* emptyBlock : emptyBlocks) {
      QPropertyAnimation* blockMoving = new QPropertyAnimation(emptyBlock, "pos");

      blockMoving->setStartValue(QPointF(emptyBlock->pos().x(), startY));
      blockMoving->setEndValue(_grid[endRow][j]->pos());
      blockMoving->setEasingCurve(QEasingCurve::InOutSine);
      blockMoving->setDuration(duration);

      dropGroup->addAnimation(blockMoving);

      _grid[endRow][j] = emptyBlock;
      startY -= blockHeight;
      endRow--;

      BlockType type = BlockType(rand() % (int)BlockType::EMPTY);
      emptyBlock->setType(type);
    }
  }

  QObject::connect(dropGroup, &QAbstractAnimation::finished, this, &BlocksGrid::updateBlocks);
  dropGroup->start(QAbstractAnimation::DeleteWhenStopped);
}

void BlocksGrid::updateBlocks() {
  QList<QPoint> prevRemovedBlocks = _removedBlocks;
  _removedBlocks.clear();

  bool isTriplet = false;
  for (QPoint removedCoord : prevRemovedBlocks) {

    BlockType type = at(removedCoord)->blockType();
    if (type != BlockType::EMPTY) {
      QSet<int> sameBlocks;

      int matchesN;
      if ((matchesN = findMatches(removedCoord.x(), removedCoord.y(), sameBlocks)) >= 3) {

        if (matchesN >= 5) {

          Bonus* newBonus;
          if (rand() / double(RAND_MAX) < 0.5) {
            newBonus = new ExplosionWave(at(removedCoord), this);
          } else {
            newBonus = new TypeChange(at(removedCoord), this);
          }

          QMetaObject::Connection* conn = new QMetaObject::Connection;
          *conn = QObject::connect(this, &BlocksGrid::blocksRemoved, [this, newBonus, conn]() {
            newBonus->nextStage();
            QObject::disconnect(*conn);
            delete conn;
          });
        }

        for (int scalarCoord : sameBlocks) {
          QPoint coord(scalarCoord / columnsN(), scalarCoord % columnsN());
          at(coord)->setType(BlockType::EMPTY);
          _removedBlocks.append(coord);
        }
        isTriplet = true;
      }
    }
  }

  if (!isTriplet) {

    if (prevRemovedBlocks.size() == 2) {
      swapBlocks(prevRemovedBlocks[0], prevRemovedBlocks[1], false);
    }

    setEnabled(true);
    return;
  }

  _waitBeforeDrop.start();
}

void BlocksGrid::swapBlocks(const QPoint& coord1, const QPoint& coord2, bool updateRequired) {
  setEnabled(false);

  Block* block1 = at(coord1);
  Block* block2 = at(coord2);

  qSwap(at(coord1), at(coord2));

  QPropertyAnimation* block1Moving = new QPropertyAnimation(block1, "pos");
  block1Moving->setEndValue(block2->pos());

  QPropertyAnimation* block2Moving = new QPropertyAnimation(block2, "pos");
  block2Moving->setEndValue(block1->pos());

  QParallelAnimationGroup* swapGroup = new QParallelAnimationGroup;
  swapGroup->addAnimation(block1Moving);
  swapGroup->addAnimation(block2Moving);

  if (updateRequired) {
    _removedBlocks.append(coord1);
    _removedBlocks.append(coord2);
    QObject::connect(swapGroup, &QAbstractAnimation::finished, this, &BlocksGrid::updateBlocks);
  }

  swapGroup->start(QAbstractAnimation::DeleteWhenStopped);
}

QPoint BlocksGrid::calcIndexes(const QPointF& pos) const {
  int column = int((pos.x() / _boundRect.width() + 0.5) * columnsN());
  int row = int((pos.y() / _boundRect.height() + 0.5) * rowsN());
  return QPoint(row, column);
}

bool BlocksGrid::isNeighbors(const QPoint& coord1, const QPoint& coord2) const {
  if (abs(coord1.x() - coord2.x()) + abs(coord1.y() - coord2.y()) == 1) {
    return true;
  }
  return false;
}

bool BlocksGrid::sceneEventFilter(QGraphicsItem* watched, QEvent* event) {

  if (event->type() == QEvent::GraphicsSceneMousePress) {
    QList<QGraphicsItem*> selectedBlocks = scene()->selectedItems();

    if (selectedBlocks.size() == 1) {
      QGraphicsSceneMouseEvent* mouseEvent = static_cast<QGraphicsSceneMouseEvent*>(event);

      QPoint firstCoord = calcIndexes(selectedBlocks[0]->pos());
      QPoint secondCoord = calcIndexes(mapFromScene(mouseEvent->scenePos()));

      if (isNeighbors(firstCoord, secondCoord)) {
        selectedBlocks[0]->setSelected(false);
        swapBlocks(firstCoord, secondCoord, true);
        return true;

      } else {
        selectedBlocks[0]->setSelected(false);
      }
    }
  }
  return false;
}

void BlocksGrid::prolongWaitingTime(int time) {
  int remainingTime = _waitBeforeDrop.remainingTime();
  if (remainingTime < time) {
    _waitBeforeDrop.start(time);
  }
}

void BlocksGrid::removeBlocks(const QList<QPoint>& blocksList) {
  _removedBlocks.append(blocksList);
}