#include "Gems.h"
#include <QGraphicsView>
#include <QGraphicsScene>
#include "BlocksGrid.h"

Gems::Gems(QWidget *parent)
  : QMainWindow(parent), _ui(new Ui::GemsClass) {
    _ui->setupUi(this);
    setFixedSize(1280, 720);
    _ui->mainToolBar->close();

    QGraphicsView* view = new QGraphicsView;
    view->setFixedSize(1280, 720);

    view->setRenderHint(QPainter::Antialiasing);
    view->setFrameStyle(0);
    setCentralWidget(view);

    int rowsN = 9;
    int columnsN = 9;
    srand(time(nullptr));
    _scene = new QGraphicsScene(-640, -360, 1280, 720);
    BlocksGrid* blocks = new BlocksGrid(rowsN, columnsN, QRectF(-300, -300, 600, 600));

    _scene->addItem(blocks);
    blocks->setPos(-200, 0);
    view->setScene(_scene);

    QPixmap background(":/Gems/background");
    background = background.scaled(view->width(), view->height(),
                                   Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    QGraphicsPixmapItem* backgroundItem = _scene->addPixmap(background);
    backgroundItem->setZValue(-1);
    backgroundItem->setPos(-view->width() / 2, -view->height() / 2);

    ExplosionWave::init(qMax(rowsN, columnsN));
    TypeChange::init();
    TypeChangeInterface::init();
}

Gems::~Gems() {
  ExplosionWave::clear();
  TypeChange::clear();
  TypeChangeInterface::clear();
  delete _scene;
  delete _ui;
}