#include "photodetaileditem.h"
#include "photosortitem.h"

#include <QApplication>
#include <QStyle>

PhotoDetailedItem::PhotoDetailedItem(const PhotoSortItem *item, QGraphicsItem *parent) :
    QGraphicsItem(parent),
    item_(item)
{
    accPix_ = QPixmap(QApplication::style()->standardIcon(QStyle::SP_DialogYesButton).pixmap(30, 30));
    rejPix_ = QPixmap(QApplication::style()->standardIcon(QStyle::SP_DialogNoButton).pixmap(30, 30));
    new QGraphicsPixmapItem(item_->data(PhotoSortItem::PixmapRole).value<QPixmap>(), this);
    acceptance_ = new QGraphicsPixmapItem(this);
    updateAccept();
}

void PhotoDetailedItem::updateAccept()
{
    if(item_->isAccepted())
        acceptance_->setPixmap(accPix_);
    else
        acceptance_->setPixmap(rejPix_);
    update();
}

QRectF PhotoDetailedItem::boundingRect() const
{
    return childrenBoundingRect();
}

void PhotoDetailedItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(painter);
    Q_UNUSED(option);
    Q_UNUSED(widget);
}
