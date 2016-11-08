#ifndef DETAILSCENE_H
#define DETAILSCENE_H

#include <QGraphicsScene>

class DetailScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit DetailScene(QObject *parent = nullptr);
};

#endif // DETAILSCENE_H
