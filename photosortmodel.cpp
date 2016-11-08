#include "photosortmodel.h"

#include <QDir>
#include <QDebug>
#include <QStyle>
#include <QApplication>

#include <list>
#include <future>
#include <thread>

#include <set>

#include "photosortitem.h"

PhotoSortModel::PhotoSortModel(QObject *parent) :
    QStandardItemModel(parent)
{

}

void PhotoSortModel::readDir(QString path)
{
    workDir_ = path;
    build(path);
    fill(path);
}

void PhotoSortModel::build(QString path)
{

    QPixmap p(QApplication::style()->standardIcon(QStyle::StandardPixmap::SP_MessageBoxQuestion).pixmap(QSize(150, 150)));

    if(QFileInfo(path + "/.cache").exists()) { //read cache
        read(path);
    }
    else {
        auto entries = parse(path);
        for(const auto &e : entries) {
            auto photo = new PhotoSortItem;
            photo->setData(e, PhotoSortItem::PathRole);
            photo->setData(p, Qt::DecorationRole);
            invisibleRootItem()->appendRow(photo);
        }
    }
}

void PhotoSortModel::fill(const QString &path)
{
    unsigned numRows = invisibleRootItem()->rowCount();
    auto getThumbnailPath = [](const QString &filePath) -> QString {
        QFileInfo originFI(filePath);
        return originFI.absolutePath() + "/.thumbnails/" + originFI.fileName();
    };
    auto readSinglePhoto = [=](PhotoSortItem *photo, const QString &path){
        auto imgPath = photo->data(PhotoSortItem::PathRole).toString();
        if( ! imgPath.isEmpty() ) {
            auto thumbnailPath = getThumbnailPath(path + "/" + imgPath);
            bool isLoaded = false;
            QPixmap p;
            if(QFileInfo(thumbnailPath).exists()) {
                try {
                    p = QPixmap::fromImage(QImage(thumbnailPath));
                    photo->setData(p, PhotoSortItem::PixmapRole);
                    isLoaded = true;
                }
                catch(...){}
            }
            if( ! isLoaded ) {
                p = QPixmap::fromImage(QImage(path + "/" + imgPath)).scaled(QSize(600, 600), Qt::KeepAspectRatio);
                photo->setData(p, PhotoSortItem::PixmapRole);
                auto thisImgPath = QFileInfo(path + "/" + imgPath).absolutePath();
                if ( !QDir(thisImgPath+"/.thumbnails").exists() )
                    QDir(thisImgPath).mkdir(".thumbnails");
                p.save(thumbnailPath);
            }
        }
    };
    auto read = [=](int id, int start, int stop){
        for(int row = start; row < stop; ++row) {
            auto photo = photoItem(row);
            if(photo->rowCount() == 0)
                readSinglePhoto(photo, path);
            else {
                for(int ct = 0; ct < photo->rowCount(); ++ct)
                    readSinglePhoto(reinterpret_cast<PhotoSortItem*>(photo->child(ct)), path);
            }
            if(row % 10 == 0)
                QMetaObject::invokeMethod(this, "partialDone", Qt::DirectConnection, Q_ARG(int, id), Q_ARG(int, row-start));
        }
        return 0;
    };
    doneMap_.clear();
    doneMap_[-1] = numRows;
    std::list<std::future<int>> futures;
    unsigned numThreads = std::thread::hardware_concurrency() - 1;
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

QModelIndex PhotoSortModel::group(QModelIndexList indexes)
{
    QList<QPersistentModelIndex> persist;
    for(auto &i : indexes)
        persist << QPersistentModelIndex(i);
    auto grItem = new PhotoSortItem;
    insertRow(persist.front().row(), grItem);
    std::set<int> rowsToDelete;
    for(auto p = persist.begin(); p != persist.end(); ++p) {
        rowsToDelete.insert(p->row());
        grItem->appendRow(takeItem(p->row()));
    }
    for(auto r = rowsToDelete.rbegin(); r != rowsToDelete.rend(); ++r) {
        removeRow(*r);
    }
    dataChanged(index(0, 0), index(rowCount(), 0), QVector<int>() << Qt::DisplayRole << Qt::DecorationRole);
    write(workDir_);
    return grItem->index();
}

QModelIndexList PhotoSortModel::ungroup(QModelIndexList indexes)
{
    QList<QPersistentModelIndex> persist;
    QModelIndexList newIndexes;
    for(auto &i : indexes)
        persist << QPersistentModelIndex(i);
    QList<QStandardItem *> allInsertedItems;
    for(auto p = persist.begin(); p != persist.end(); ++p) {
        int rowToInsert = p->row();
        auto g = photoItem(rowToInsert);
        if(g->type() == PhotoSortItem::SingleImage)
            continue;
        auto numRows = g->rowCount();
        QList<QStandardItem *> items;
        for(int ct = 0; ct < numRows; ++ct) {
            items << g->takeChild(ct);
            allInsertedItems << items.back();
        }
        invisibleRootItem()->insertRows(rowToInsert, items);
        removeRow(g->row());
//        delete g;
    }
    for(auto i : allInsertedItems)
        newIndexes << i->index();
    return newIndexes;
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
    emit(dataChanged(index(0, 0), index(rowCount(), 0), QVector<int>() << Qt::DecorationRole));
}

QStringList PhotoSortModel::parse(const QString &path)
{
    QStringList files;

    QDir dir(path);

    auto dirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::SortFlag::Name);

    for(const auto &d : dirs) {
        if(d != QString(".thumbnails"))
            files << parse(path+"/"+d);
    }

    auto entries = dir.entryList(QStringList() << "*.jpeg" << "*.jpg", QDir::Files | QDir::NoDotAndDotDot, QDir::SortFlag::Name);
    for(const auto &e :  entries){
        files << dir.dirName() + "/" + e;
    }

    return files;
}

void PhotoSortModel::read(const QString &path)
{
    QFile file(path + "/.cache");
    file.open(QIODevice::ReadOnly);
    QDataStream in(&file);
    qint32 count;
    in >> count;
    for(int ct = 0; ct < count; ++ct) {
        auto item = new PhotoSortItem;
        item->read(in);
        invisibleRootItem()->appendRow(item);
    }
}

void PhotoSortModel::write(const QString &path)
{
    QFile file(path + "/.cache");
    file.open(QIODevice::WriteOnly);
    QDataStream out(&file);
    out << qint32(rowCount());
    for(int r = 0; r < rowCount(); ++r)
        item(r, 0)->write(out);
}

PhotoSortItem *PhotoSortModel::photoItem(int row)
{
    return reinterpret_cast<PhotoSortItem *>(item(row, 0));
}
