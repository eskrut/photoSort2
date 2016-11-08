#include "photosortpreview.h"

#include "photosortmodel.h"
#include "photosortitem.h"

#include <QKeyEvent>

PhotoSortPreview::PhotoSortPreview(QWidget *parent) : QListView(parent)
{
    generateDefaultActionMap();
}

void PhotoSortPreview::setModel(QAbstractItemModel *model)
{
    phModel_ = dynamic_cast<PhotoSortModel*>(model);
    if(!phModel_) throw std::runtime_error("Not a PhotoSortModel passed to PhotoSortPreview.");
    QListView::setModel(model);
    setSelectionModel(new QItemSelectionModel(phModel_));
}

void PhotoSortPreview::generateDefaultActionMap()
{
    actionMap_[Actions::Group] = Qt::Key_G;
    actionMap_[Actions::UnGroup] = Qt::Key_T;
    actionMap_[Actions::Accept] = Qt::Key_A;
    actionMap_[Actions::Reject] = Qt::Key_S;
    actionMap_[Actions::SwapAcceptReject] = Qt::Key_E;
}

void PhotoSortPreview::keyPressEvent(QKeyEvent *event)
{
    auto items = selectionModel()->selection().indexes();

    auto key = event->key();
    auto act = actionMap_.key(key, Actions::NoAction);

    switch (act) {
    case Actions::Accept:
        for(auto &i : items) {
            phModel_->photoItem(i.row())->setData(QVariant::fromValue<>(true), PhotoSortItem::AcceptRole);
        }
        event->accept();
        break;
    case Actions::Reject:
        for(auto &i : items) {
            phModel_->photoItem(i.row())->setData(QVariant::fromValue<>(false), PhotoSortItem::AcceptRole);
        }
        event->accept();
        break;
    case Actions::SwapAcceptReject:
        for(auto &i : items) {
            auto ph = phModel_->photoItem(i.row());
            auto val = ! ph->data(PhotoSortItem::AcceptRole).toBool();
            ph->setData(QVariant::fromValue<>(val), PhotoSortItem::AcceptRole);
        }
        event->accept();
        break;
    case Actions::Group:
        phModel_->group(items);
        event->accept();
        break;
    case Actions::UnGroup:
        phModel_->ungroup(items);
        selectionModel()->clearSelection();
        event->accept();
        break;
    default:
        break;
    }

    update();
}
