#pragma once

#include <QQueue>
#include <QVector2D>
#include "Block.h"

class TypeChangeInterface;

class InterfaceQueue : private QQueue<TypeChangeInterface*> {
  QPointF _startPoint;
  QVector2D _bias;

public:
  InterfaceQueue(const QPointF& startPoint, const QVector2D& bias) :
    _startPoint(startPoint), _bias(bias) {}

  void enqueue(TypeChangeInterface* newInterface);
  TypeChangeInterface* dequeue();
  bool isEmpty() { return QQueue::isEmpty(); }
  TypeChangeInterface* last() { return QQueue::last(); }
  TypeChangeInterface* head() { return QQueue::head(); }
};