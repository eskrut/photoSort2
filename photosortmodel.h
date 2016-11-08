#ifndef PHOTOSORTMODEL_H
#define PHOTOSORTMODEL_H

#include <QStandardItemModel>

#include <mutex>

class PhotoSortItem;

class PhotoSortModel : public QStandardItemModel
{
    Q_OBJECT
public:
    explicit PhotoSortModel(QObject *parent = 0);

signals:
    void loaded();
public slots:
    void readDir(QString path);
    QModelIndex group(QModelIndexList indexes);
    QModelIndexList ungroup(QModelIndexList indexes);
private:
    std::mutex mapMutex_;
    std::map<int, int> doneMap_;
private slots:
    void partialDone(int id, int num);
signals:
    void progress(int);
private:
    QStringList parse(const QString &path);
    void read();
    void write();
public:
    PhotoSortItem *photoItem(int row);
};

#endif // PHOTOSORTMODEL_H
