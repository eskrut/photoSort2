#include "photosortmodel.h"

#include <QDir>
#include <QDebug>
#include <QStyle>
#include <QApplication>

#include <list>
#include <future>
#include <thread>

#include "photosortitem.h"

PhotoSortModel::PhotoSortModel(QObject *parent) :
    QStandardItemModel(parent)
{

}

void PhotoSortModel::readDir(QString path)
{
    auto entries = parse(path);

    QPixmap p(QApplication::style()->standardIcon(QStyle::StandardPixmap::SP_MessageBoxQuestion).pixmap(QSize(150, 150)));
    for(const auto &e : entries) {
        auto photo = new PhotoSortItem;
        photo->setData(e, PhotoSortItem::PathRole);
        photo->setData(p, Qt::DecorationRole);
        invisibleRootItem()->appendRow(photo);
    }
    unsigned numRows = invisibleRootItem()->rowCount();
    auto read = [=](int id, int start, int stop){
        for(int row = start; row < stop; ++row) {
            auto photo = photoItem(row);
            auto imgPath = photo->data(PhotoSortItem::PathRole).toString();
            photo->setData(QPixmap(path + "/" + imgPath).scaled(QSize(150, 150), Qt::KeepAspectRatio), Qt::DecorationRole);
            this->itemChanged(photo);
            if(row % 10 == 0)
                this->partialDone(id, row - start);
        }
        return 0;
    };
    doneMap_.clear();
    doneMap_[-1] = numRows;
    std::list<std::future<int>> futures;
    unsigned numThreads = std::thread::hardware_concurrency();
    for(unsigned ct = 0; ct < numThreads; ++ct) {
        doneMap_[ct] = 0;
        unsigned start = (ct*numRows)/numThreads;
        unsigned stop = ((ct+1)*numRows)/numThreads;
        if( (ct + 1) == numThreads && stop > numRows ) stop = numRows;
        futures.push_back(std::async(read, ct, start, stop));
    }
    for(auto &f : futures)
        f.get();

    emit(loaded());
    for(unsigned row = 0; row < numRows; ++row)
        itemChanged(photoItem(row));
}

void PhotoSortModel::group(QModelIndexList indexes)
{
    QList<QPersistentModelIndex> persist;
    for(auto &i : indexes)
        persist << QPersistentModelIndex(i);
    auto grItem = new PhotoSortItem;
    insertRow(persist.front().row(), grItem);
    for(auto p = persist.begin(); p != persist.end(); ++p) {
        grItem->appendRow(takeItem(p->row()));
    }
//    QPixmap p(QApplication::style()->standardIcon(QStyle::StandardPixmap::SP_MessageBoxQuestion).pixmap(QSize(150, 150)));
//    grItem->setData(p, Qt::DecorationRole);
}

void PhotoSortModel::ungroup(QModelIndexList indexes)
{
    QList<QPersistentModelIndex> persist;
    for(auto &i : indexes)
        persist << QPersistentModelIndex(i);
    for(auto p = persist.begin(); p != persist.end(); ++p) {
        int rowToInsert = p->row();
        auto g = photoItem(rowToInsert);
        if(g->type() == PhotoSortItem::SingleImage)
            continue;
        auto numRows = g->rowCount();
        QList<QStandardItem *> items;
        for(int ct = 0; ct < numRows; ++ct) {
            items << g->takeChild(ct);
        }
        invisibleRootItem()->insertRows(rowToInsert, items);
        delete g;
    }
}

void PhotoSortModel::partialDone(int id, int num)
{
    std::lock_guard<std::mutex> lock(mapMutex_);
    doneMap_[id] = num;
    auto it = doneMap_.begin();
    auto all = (it++)->second;
    int cur = 0;
    for(; it != doneMap_.end(); ++it) {
        cur += it->second;
    }
    emit(progress(cur*100/all));
}

QStringList PhotoSortModel::parse(const QString &path)
{
    QStringList files;

    QDir dir(path);

    auto dirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::SortFlag::Name);

    for(const auto &d : dirs) {
        files << parse(path+"/"+d);
    }

    files << dir.entryList(QStringList() << "*.jpeg" << "*.jpg", QDir::Files | QDir::NoDotAndDotDot, QDir::SortFlag::Name);

    return files;
}

PhotoSortItem *PhotoSortModel::photoItem(int row)
{
    return reinterpret_cast<PhotoSortItem *>(item(row, 0));
}
