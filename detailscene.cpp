#include "detailscene.h"

#include <QGraphicsPixmapItem>
#include <QGraphicsView>

#include "photosortitem.h"
#include "photodetaileditem.h"

DetailScene::DetailScene(QObject *parent) :
    QGraphicsScene(parent)
{
    firstTimeOverflow_ = false;
    line_ = new QGraphicsLineItem;
    QPen p(Qt::green, 4);
    line_->setPen(p);
    addItem(line_);
}

void DetailScene::setupPhotoSortItem(PhotoSortItem *item)
{
    for(auto &p : photos_) {
        removeItem(p);
        delete p;
    }
    photos_.clear();
    for(auto &p : acceptedPhotos_) {
        removeItem(p);
        delete p;
    }
    acceptedPhotos_.clear();

    allItems_ = item->allItems();

    for(const auto& i : allItems_) {
        auto p = new PhotoDetailedItem(i);
        addItem(p);
        photos_ << p;
    }
    for(const auto& i : allItems_) {
        auto p = new PhotoDetailedItem(i);
        addItem(p);
        acceptedPhotos_ << p;
        p->setScale(0.2);
        p->setPos(QPointF(0, -p->boundingRect().height()*0.2));
    }

    setCurrent(0);
    updateAccepted();
    update();
}

void DetailScene::focusLeft()
{
    setCurrent(current_ - 1);
}

void DetailScene::focusRight()
{
    setCurrent(current_ + 1);
}

void DetailScene::acceptCurrent()
{
    allItems_[current_]->setData(QVariant::fromValue(true), PhotoSortItem::AcceptRole);
    photos_[current_]->updateAccept();
    updateAccepted();
}

void DetailScene::rejectCurrent()
{
    allItems_[current_]->setData(QVariant::fromValue(false), PhotoSortItem::AcceptRole);
    photos_[current_]->updateAccept();
    updateAccepted();
}

void DetailScene::acceptOnlyCurrent()
{
//    for(int ct = 0; ct < photos_.size(); ++ct) {
//        allItems_[ct]->setData(QVariant::fromValue(false), PhotoSortItem::AcceptRole);
//        photos_[ct]->updateAccept();
//    }
    rejectAll();
    acceptCurrent();
    updateAccepted();
}

void DetailScene::rejectAll()
{
    for(int ct = 0; ct < photos_.size(); ++ct) {
        allItems_[ct]->setData(QVariant::fromValue(false), PhotoSortItem::AcceptRole);
        photos_[ct]->updateAccept();
    }
    updateAccepted();
}

void DetailScene::toggleCurrent()
{
    allItems_[current_]->setData(
                ! allItems_[current_]->data(PhotoSortItem::AcceptRole).toBool()
                , PhotoSortItem::AcceptRole);
    photos_[current_]->updateAccept();
    updateAccepted();
}

void DetailScene::jumpNextAccepted()
{
    auto accepted = getAcceptedIndexes();
    if(accepted.size()) {
        auto it = accepted.lower_bound(current_+1);
        if(it != accepted.end()) {
            setCurrent(*it);
        }
        else {
            setCurrent(*accepted.begin());
        }
    }
}

void DetailScene::jumpPrevAccepted()
{
    auto accepted = getAcceptedIndexes();
    if(accepted.size()) {
        auto it = accepted.upper_bound(current_);
        if(it != accepted.end()) {
            setCurrent(*it);
        }
        else {
            setCurrent(*accepted.rbegin());
        }
    }
}

void DetailScene::zoom()
{
    photos_[current_]->crop(0.5f, 0.5f, photos_[current_]->cropScaling()*1.2f);
}

void DetailScene::unzoom()
{
    photos_[current_]->crop(0, 0, 0);
}

std::set<int> DetailScene::getAcceptedIndexes()
{
    std::set<int> accepted;
    for(int ct = 0; ct < allItems_.size(); ++ct) {
        if( allItems_[ct]->data(PhotoSortItem::AcceptRole).toBool() )
            accepted.insert(ct);
    }
    return accepted;
}

void DetailScene::ensure(int index, const double scale)
{
    auto br = photos_[index]->boundingRect();
    auto h = br.height();
    auto bl = br.bottomLeft();
    for(auto &ap : acceptedPhotos_)
        br |= ap->boundingRect().translated(ap->pos());
    br |= QRectF(bl, QSizeF(1, h*scale));
//    setSceneRect(0, 0, br.width(), br.height()*(1 + scale));
    views().front()->fitInView(br, Qt::KeepAspectRatio);
}

void DetailScene::setCurrent(int index)
{
    if( (index >= 0 && index < photos_.size()) || firstTimeOverflow_ == true) {        
        if(index < 0) index = photos_.size() - 1;
        if(index > photos_.size()-1) index = 0;
        firstTimeOverflow_ = false;
        current_ = index;
        const double scale = 0.2;
        photos_[index]->setPos(0, 0);
        photos_[index]->setScale(1.0 /** photos_[index]->cropScaling()*/);
        photos_[index]->updateAccept();
        auto pos = photos_[index]->boundingRect().topLeft() + QPointF(0, /*photos_[index]->cropScaling()**/photos_[index]->boundingRect().height());
        for(int ct = 0; ct < photos_.size(); ++ct) {
            if(ct != index) {
                photos_[ct]->setScale(scale);
                photos_[ct]->setPos(pos);
                photos_[ct]->updateAccept();
                pos.setX(pos.x() + photos_[ct]->boundingRect().width()*scale);
            }
            else {
                line_->setLine(QLineF(QPointF(), QPointF(0, photos_[ct]->boundingRect().height()*scale)));
                line_->setPos(pos);
                pos.setX(pos.x() + line_->boundingRect().width());
            }
        }
        ensure(index, scale /** photos_[index]->cropScaling()*/);
    }
    else if (firstTimeOverflow_ == false) {
        firstTimeOverflow_ = true;
    }
}

void DetailScene::updateAccepted()
{
    auto pos = acceptedPhotos_.front()->pos();
    auto origin = pos;
    for(int ct = 0; ct < allItems_.size(); ++ct) {
        if(allItems_[ct]->isAccepted()) {
            acceptedPhotos_[ct]->setPos(pos);
            acceptedPhotos_[ct]->setVisible(true);
            pos.setX(pos.x() + acceptedPhotos_[ct]->boundingRect().width()*0.2);
        }
        else {
            acceptedPhotos_[ct]->setPos(origin);
            acceptedPhotos_[ct]->setVisible(false);
        }
    }
    ensure(current_, 0.2);
}
