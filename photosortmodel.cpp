#include "photosortmodel.h"

#include <QDir>
#include <QDebug>
#include <QStyle>
#include <QApplication>
#include <QDateTime>

#include <list>
#include <future>
#include <thread>

#include <set>

#include "photosortitem.h"
#include "libexif/exif-data.h"

QString getThumbnailPath(const QString &filePath){
    QFileInfo originFI(filePath);
    return originFI.absolutePath() + "/.thumbnails/" + originFI.fileName();
}

void readSinglePhoto(PhotoSortItem *photo, const QString &path){
    auto imgPath = photo->data(PhotoSortItem::PathRole).toString();
    if( ! imgPath.isEmpty() ) {
        auto thumbnailPath = getThumbnailPath(path + "/" + imgPath);
        bool isLoaded = false;
        QPixmap p;
        if(QFileInfo(thumbnailPath).exists()) {
            try {
                auto img = QImage(thumbnailPath);
                p = QPixmap::fromImage(img);
                photo->setData(p, PhotoSortItem::PixmapRole);
                ExifData *eData = exif_data_new_from_file((path + "/" + imgPath).toLocal8Bit());
                if (eData) {
                    ExifByteOrder byteOrder = exif_data_get_byte_order(eData);
                    ExifEntry *exifEntry = exif_data_get_entry(eData,
                                                    EXIF_TAG_DATE_TIME);
                    auto datetime = QDateTime::fromString(QString::fromLocal8Bit(reinterpret_cast<char *>(exifEntry->data)),
                                                          QString("yyyy:MM:dd hh:mm:ss"));
                    photo->setData(datetime, PhotoSortItem::TimeStamp);
                    exif_data_free(eData);
                }
                isLoaded = true;
            }
            catch(...){}
        }
        if( ! isLoaded ) {
            auto img = QImage(path + "/" + imgPath);
            ExifData *eData = exif_data_new_from_file((path + "/" + imgPath).toLocal8Bit());
            int orientation = 0;
            if (eData) {
                ExifByteOrder byteOrder = exif_data_get_byte_order(eData);
                ExifEntry *exifEntry = exif_data_get_entry(eData,
                                                           EXIF_TAG_ORIENTATION);
                if (exifEntry)
                    orientation = exif_get_short(exifEntry->data, byteOrder);
                exifEntry = exif_data_get_entry(eData,
                                                EXIF_TAG_DATE_TIME);
                auto datetime = QDateTime::fromString(QString::fromLocal8Bit(reinterpret_cast<char *>(exifEntry->data)),
                                                      QString("yyyy:MM:dd hh:mm:ss"));
                photo->setData(datetime, PhotoSortItem::TimeStamp);
                exif_data_free(eData);
            }
            if(orientation == 8) {
                QTransform tr;
                tr.rotate(-90);
                img = img.transformed(tr);
            }
            p = QPixmap::fromImage(img).scaled(QSize(800, 800), Qt::KeepAspectRatio);
            photo->setData(p, PhotoSortItem::PixmapRole);
            auto thisImgPath = QFileInfo(path + "/" + imgPath).absolutePath();
            if ( !QDir(thisImgPath+"/.thumbnails").exists() )
                QDir(thisImgPath).mkdir(".thumbnails");
            p.save(thumbnailPath);
        }
    }
}
int readDown(PhotoSortItem *item, const QString& path){
    if(item->rowCount() == 0)
        readSinglePhoto(item, path);
    else {
        for(int ct = 0; ct < item->rowCount(); ++ct) {
            readDown(reinterpret_cast<PhotoSortItem*>(item->child(ct)), path);
        }
    }
    return 0;
}

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
//    else {
        auto entries = parse(path);
        for(const auto &e : entries) {
            auto photo = new PhotoSortItem;
            photo->setData(e, PhotoSortItem::PathRole);
            photo->setData(path, PhotoSortItem::ProjectPathRole);
            bool found = false;
            for(int ct = 0; ct < invisibleRootItem()->rowCount(); ++ct) {
                auto i = reinterpret_cast<PhotoSortItem*>(invisibleRootItem()->child(ct));
                if(photo->isSame(i)) {
                    found = true;
                    break;
                }
            }
            if(found) {
                delete photo;
                continue;
            }
            photo->setData(p, Qt::DecorationRole);
            invisibleRootItem()->appendRow(photo);
        }
//    }
}

void PhotoSortModel::fill(const QString &path)
{
    unsigned numRows = invisibleRootItem()->rowCount();

    auto read = [this,path](int id, int start, int stop){
        for(int row = start; row < stop; ++row) {
            auto photo = photoItem(row);
            readDown(photo, path);
                QMetaObject::invokeMethod(this,
                                          "partialDone",
                                          Qt::DirectConnection,
                                          Q_ARG(int, id),
                                          Q_ARG(int, row-start));
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

//    read(0, 0, numRows);

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

QModelIndex PhotoSortModel::group(QModelIndex index, int numToGroup)
{
    QModelIndexList indexes;
    for (int ct = 0; ct < numToGroup; ++ct) {
        indexes << this->index(index.row() + ct, 0, index.parent());
    }

    return group(indexes);
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

void PhotoSortModel::sync()
{
    if( ! workDir_.isEmpty() )
        write(workDir_);
}

void PhotoSortModel::exportPhotos(const QString &suf, bool makeAtWorkDir, const QString &filePrefix)
{
    QDir wd(workDir_);
    QString expPath;
    if(makeAtWorkDir) {
        wd.mkdir(suf);
        expPath = workDir_ + "/" + suf;
    }
    else {
        wd.cdUp();
        QString dirName = QDir(workDir_).dirName() + "_" + suf;
        QDir(wd.absolutePath()).mkdir(dirName);
        expPath = wd.absolutePath() + "/" + dirName;
    }
    int count = 0;
    std::multimap<QDateTime, PhotoSortItem *> timeMap;
    for(int ct = 0; ct < rowCount(); ++ct) {
        auto photos = photoItem(ct)->allItems();
        for(const auto &p : photos) {
            if(p->isAccepted()) {
                QFileInfo fi(workDir_ + "/" + p->data(PhotoSortItem::PathRole).toString());
                if(fi.exists()) {
                    timeMap.insert(std::make_pair(fi.created(), p));
                    ++count;
                }
                else {
                    throw std::runtime_error("No such file: " + fi.fileName().toStdString());
                }
            }
        }
    }
    int allAccepted = count;
    count = 1;
    for(const auto &r : timeMap) {
        auto p = r.second;
        QFileInfo fi(workDir_ + "/" + p->data(PhotoSortItem::PathRole).toString());
        QFile(fi.absoluteFilePath()).copy(expPath + "/" + filePrefix + QString::asprintf("%05d", count++) + "." + fi.suffix() );
        progress((count * 100)/allAccepted);
    }
//    for(int ct = 0; ct < rowCount(); ++ct) {
//        auto photos = photoItem(ct)->allItems();
//        for(const auto &p : photos) {
//            if(p->isAccepted()) {
//                QFileInfo fi(workDir_ + "/" + p->data(PhotoSortItem::PathRole).toString());
//                if(fi.exists()) {
//                    QFile(fi.absoluteFilePath()).copy(expPath + "/" + filePrefix + QString::asprintf("%05d", count++) + "." + fi.suffix() );
//                    progress((count * 100)/allAccepted);
//                }
//                else {
//                    throw std::runtime_error("No such file: " + fi.fileName().toStdString());
//                }
//            }
//        }
//    }
    emit(loaded());
}

void PhotoSortModel::groupByTimeStamp(int secondsSpan, int maxnum)
{
    emit(progress(0));
    int curLookingRow = 0;
    auto getDt = [this](int row){ return item(row)->data(PhotoSortItem::TimeStamp).toDateTime();};
    while(curLookingRow < this->rowCount()){
        int numToGroup = 1;
        auto dt = getDt(curLookingRow);
        if(dt.isValid()){
            while(curLookingRow + numToGroup < this->rowCount() && numToGroup < maxnum){
                auto nextDt = getDt(curLookingRow + numToGroup);
                if(!nextDt.isValid())
                    break;
                if(dt.secsTo(nextDt) < secondsSpan)
                    numToGroup++;
                else
                    break;
            }
            if(numToGroup > 1){
                auto grouped = group(index(curLookingRow, 0), numToGroup);
                emit(progress(curLookingRow*100/rowCount()));
                emit(dataChanged(index(curLookingRow, 0), index(curLookingRow + numToGroup, 0)));
            }
        }
        curLookingRow += 1/*numToGroup*/;
    }
    emit(progress(100));
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
    emit(dataChanged(index(0, 0), index(rowCount(), 0), QVector<int>() << Qt::DisplayRole << Qt::DecorationRole));
}

QStringList PhotoSortModel::parse(const QString &path)
{
    QStringList files;

    QDir dir(path);

    auto dirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::SortFlag::Name);

    for(const auto &d : dirs) {
        if(d != QString(".thumbnails")) {
            auto dFiles = parse(path+"/"+d);
            for(const auto &dFile : dFiles)
                files << d + "/" + dFile;
        }
    }

    auto entries = dir.entryList(QStringList() << "*.jpeg" << "*.jpg", QDir::Files | QDir::NoDotAndDotDot, QDir::SortFlag::Name);
    for(const auto &e :  entries){
        files << e;
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
    emit(dataChanged(index(0, 0), index(rowCount(), 0), QVector<int>() << Qt::DisplayRole << Qt::DecorationRole));
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

void PhotoSortModel::flatItem(PhotoSortItem *item)
{
    auto allItems = item->allItems();
    allItems.removeOne(item);
    qSort(allItems.begin(), allItems.end(), [](const PhotoSortItem *left, const PhotoSortItem *right){
        return left->text() < right->text();
    });
}

PhotoSortItem *PhotoSortModel::photoItem(int row)
{
    return reinterpret_cast<PhotoSortItem *>(item(row, 0));
}

void PhotoSortModel::loadFull(PhotoSortItem *item)
{
    auto loadOne = [=](PhotoSortItem *i){
        //TODO this is copy of portion of readSinglePhoto
        auto img = QImage(workDir_ + "/" + i->data(PhotoSortItem::PathRole).toString());
        ExifData *eData = exif_data_new_from_file((workDir_ + "/" + i->data(PhotoSortItem::PathRole).toString()).toLocal8Bit());
        int orientation = 0;
        if (eData) {
            ExifByteOrder byteOrder = exif_data_get_byte_order(eData);
            ExifEntry *exifEntry = exif_data_get_entry(eData,
                                                       EXIF_TAG_ORIENTATION);
            if (exifEntry)
                orientation = exif_get_short(exifEntry->data, byteOrder);
            exifEntry = exif_data_get_entry(eData,
                                            EXIF_TAG_DATE_TIME);
            auto datetime = QDateTime::fromString(QString::fromLocal8Bit(reinterpret_cast<char *>(exifEntry->data)),
                                                  QString("yyyy:MM:dd hh:mm:ss"));
            item->setData(datetime, PhotoSortItem::TimeStamp);
            exif_data_free(eData);
        }
        if(orientation == 8) {
            QTransform tr;
            tr.rotate(-90);
            img = img.transformed(tr);
        }
        i->setFullPixmap(QPixmap::fromImage(img));
        return 0;
    };

    auto list = item->allItems().toStdList();
    auto work = [loadOne](decltype(list.end()) begin, decltype(list.end()) end){
        for(auto it = begin; it != end; ++it)
            loadOne(*it);
    };
    size_t hc = std::thread::hardware_concurrency();
    if(hc == 0) hc = 8;
    auto numThreads = std::min(list.size(), hc);
    auto chunkPerThread = list.size() / numThreads;
    auto threadBegin = list.begin();
    auto threadEnd = threadBegin;
    std::advance(threadEnd, chunkPerThread);
    std::list<std::future<void>> futures;
    for(size_t thId = 0; thId < numThreads - 1; ++thId){
        futures.push_back(std::async(std::launch::async,
                                     work, threadBegin, threadEnd));
        threadBegin = threadEnd;
        std::advance(threadEnd, chunkPerThread);
    }
    work(threadBegin, list.end());
    for(auto &f : futures) f.get();
}

void PhotoSortModel::cleanFull(PhotoSortItem *item)
{
    for(auto i : item->allItems()) {
        i->setFullPixmap();
    }
}
