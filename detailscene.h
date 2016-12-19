#ifndef DETAILSCENE_H
#define DETAILSCENE_H

#include <QGraphicsScene>
#include <QGraphicsLinearLayout>

#include "photosortitem.h"

#include <set>

class PhotoDetailedItem;

class DetailScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit DetailScene(QObject *parent = nullptr);
public slots:
    void setupPhotoSortItem(PhotoSortItem *item);
    void focusLeft();
    void focusRight();
    void acceptCurrent();
    void rejectCurrent();
    void acceptOnlyCurrent();
    void rejectAll();
    void toggleCurrent();
    void jumpNextAccepted();
    void jumpPrevAccepted();
private:
    std::set<int> getAcceptedIndexes();
private:
    QList<PhotoDetailedItem *> photos_;
    QList<PhotoDetailedItem *> acceptedPhotos_;
    PHListType allItems_;
    int current_;
    bool firstTimeOverflow_;
    QGraphicsLineItem *line_;
    void setCurrent(int index);
    void updateAccepted();
    void ensure(int index, const double scale);
};

#endif // DETAILSCENE_H
