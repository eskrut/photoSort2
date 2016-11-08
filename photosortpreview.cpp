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
    setUniformItemSizes(true);
    setViewMode(QListView::IconMode);
    setSelectionMode(QListView::ExtendedSelection);
}

void PhotoSortPreview::generateDefaultActionMap()
{
    actionMap_[Actions::Group] = Qt::Key_G;
    actionMap_[Actions::UnGroup] = Qt::Key_T;
    actionMap_[Actions::Accept] = Qt::Key_A;
    actionMap_[Actions::Reject] = Qt::Key_S;
    actionMap_[Actions::SwapAcceptReject] = Qt::Key_E;

    actionMap_[Actions::ToggleSelectionFocusToNext] = Qt::Key_J;
    actionMap_[Actions::ToggleSelectionFocusToPrev] = Qt::Key_H;
    actionMap_[Actions::FocusToNext] = Qt::Key_M;
    actionMap_[Actions::FocusToPrev] = Qt::Key_N;
    actionMap_[Actions::ClearSelection] = Qt::Key_U;
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
            update(phModel_->photoItem(i.row())->index());
        }
        event->accept();
        break;
    case Actions::Reject:
        for(auto &i : items) {
            phModel_->photoItem(i.row())->setData(QVariant::fromValue<>(false), PhotoSortItem::AcceptRole);
            update(phModel_->photoItem(i.row())->index());
        }
        event->accept();
        break;
    case Actions::SwapAcceptReject:
        for(auto &i : items) {
            auto ph = phModel_->photoItem(i.row());
            auto val = ! ph->data(PhotoSortItem::AcceptRole).toBool();
            ph->setData(QVariant::fromValue<>(val), PhotoSortItem::AcceptRole);
            update(ph->index());
        }
        event->accept();
        break;
    case Actions::Group:
    {
        auto index = phModel_->group(items);
        if(index.row() < model()->rowCount())
            setCurrentIndex(model()->index(index.row()+1, 0));
        else
            setCurrentIndex(model()->index(index.row(), 0));
        event->accept();
        break;
    }
    case Actions::UnGroup:
    {
        auto indexes = phModel_->ungroup(items);
        selectionModel()->clearSelection();
        for(auto item : indexes)
            selectionModel()->select(item, QItemSelectionModel::Select);
        event->accept();
        break;
    }
    case Actions::FocusToNext:
    {
        auto ind = selectionModel()->currentIndex();
        if(ind.row() < model()->rowCount() - 1){
            selectionModel()->select(ind, QItemSelectionModel::Deselect);
            selectionModel()->select(model()->index(ind.row()+1, 0), QItemSelectionModel::Select);
            selectionModel()->setCurrentIndex(model()->index(ind.row()+1, 0), QItemSelectionModel::NoUpdate);
        }
        event->accept();
        break;
    }
    case Actions::FocusToPrev:
    {
        auto ind = selectionModel()->currentIndex();
        if(ind.row() > 0) {
            selectionModel()->select(ind, QItemSelectionModel::Deselect);
            selectionModel()->select(model()->index(ind.row()-1, 0), QItemSelectionModel::Select);
            selectionModel()->setCurrentIndex(model()->index(ind.row()-1, 0), QItemSelectionModel::NoUpdate);
        }
        event->accept();
        break;
    }
    case Actions::ToggleSelectionFocusToNext:
    {
        auto ind = selectionModel()->currentIndex();
        if(ind.row() < model()->rowCount() - 1)
            selectionModel()->setCurrentIndex(model()->index(ind.row()+1, 0), QItemSelectionModel::Toggle);
        event->accept();
        break;
    }
    case Actions::ToggleSelectionFocusToPrev:
    {
        auto ind = selectionModel()->currentIndex();
        if(ind.row() > 0)
            selectionModel()->setCurrentIndex(model()->index(ind.row()-1, 0), QItemSelectionModel::Toggle);
        event->accept();
        break;
    }
    case Actions::ClearSelection:
    {
        setCurrentIndex(currentIndex());
        event->accept();
        break;
    }
    case Actions::NoAction:
        event->accept();
        break;
    default:
        break;
    }

    update();
}
