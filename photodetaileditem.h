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
    QGraphicsPixmapItem *photo_;
    float cropScaling_;

    QPixmap accPix_;
    QPixmap rejPix_;

public:
    void updateAccept();
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void crop(float xCenterPortion, float yCenterPortion, float scaling);
    float cropScaling() const {return cropScaling_;}
};



#endif // PHOTODETAILEDITEM_H
