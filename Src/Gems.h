#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_Gems.h"

class Gems : public QMainWindow {
    Q_OBJECT

public:
    Gems(QWidget* parent = Q_NULLPTR);
    ~Gems();

private:
    Ui::GemsClass* _ui;
    QGraphicsScene* _scene;
    
};
