#ifndef PHOTODETAILEDITEM_H
#define PHOTODETAILEDITEM_H

#include <QGraphicsItem>
#include <QGraphicsPixmapItem>
#include <mutex>

class PhotoSortItem;

class PhotoDetailedItem : public QGraphicsItem
{
public:
    explicit PhotoDetailedItem(const PhotoSortItem *item, QGraphicsItem *parent = nullptr);
private:
    const PhotoSortItem *item_;
    QGraphicsPixmapItem *acceptance_;

    QPixmap accPix_;
    QPixmap rejPix_;

public:
    void updateAccept();
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
};



#endif // PHOTODETAILEDITEM_H
