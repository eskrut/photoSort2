#include "photodetaileditem.h"
#include "photosortitem.h"

#include <QApplication>
#include <QStyle>

PhotoDetailedItem::PhotoDetailedItem(const PhotoSortItem *item, QGraphicsItem *parent) :
    QGraphicsItem(parent),
    item_(item),
    cropScaling_(1)
{
    accPix_ = QPixmap(QApplication::style()->standardIcon(QStyle::SP_DialogYesButton).pixmap(30, 30));
    rejPix_ = QPixmap(QApplication::style()->standardIcon(QStyle::SP_DialogNoButton).pixmap(30, 30));
    photo_ = new QGraphicsPixmapItem(item_->data(PhotoSortItem::PixmapRole).value<QPixmap>(), this);
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

void PhotoDetailedItem::crop(float xCenterPortion, float yCenterPortion, float scaling)
{
    cropScaling_ = scaling;
    if(scaling <= 1) {
        photo_->setPixmap(item_->data(PhotoSortItem::PixmapRole).value<QPixmap>());
        cropScaling_ = 1;
        return;
    }
    auto rOrig = photo_->boundingRect();
//    auto r = rOrig;
//    r.setSize(rOrig.size() / scaling);

    auto oW = rOrig.width();
    auto oH = rOrig.height();
    auto w = oW / scaling;
    auto h = oH / scaling;

    if(xCenterPortion*oW - w/2 < 0)
        xCenterPortion = (w/2)/oW;
    if(xCenterPortion*oW + w/2 > oW)
        xCenterPortion = 1.0f - (w/2)/oW;
    if(yCenterPortion*oH - h/2 < 0)
        yCenterPortion = (h/2)/oH;
    if(yCenterPortion*oH + h/2 > oH)
        yCenterPortion = 1.0f - (h/2)/oH;

    auto r = QRectF(QPointF(xCenterPortion*oW - w/2, yCenterPortion*oH - h/2),
                    QSizeF(w, h));

    auto pm = item_->data(PhotoSortItem::PixmapRole).value<QPixmap>();
    pm = pm.copy(r.toRect());
    pm = pm.scaled(rOrig.size().toSize());
    photo_->setPixmap(pm);
    update();
}
