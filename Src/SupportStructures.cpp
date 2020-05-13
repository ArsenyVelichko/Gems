#include "SupportStructures.h"
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include "Bonus.h"

void InterfaceQueue::enqueue(TypeChangeInterface* newInterface) {
  newInterface->setPos(_startPoint + size() * _bias.toPointF());
  QQueue::enqueue(newInterface);
}

TypeChangeInterface* InterfaceQueue::dequeue() {
  size();
  TypeChangeInterface* firstInterface = QQueue::dequeue();
  QParallelAnimationGroup* moveGroup = new QParallelAnimationGroup;

  int i = 0;
  for (QGraphicsObject* currInterface : *this) {
    QPropertyAnimation* singleMove = new QPropertyAnimation(currInterface, "pos");
    singleMove->setEndValue(_startPoint + i * _bias.toPointF());
    moveGroup->addAnimation(singleMove);
    i++;
  }

  moveGroup->start(QAbstractAnimation::DeleteWhenStopped);
  return firstInterface;
}