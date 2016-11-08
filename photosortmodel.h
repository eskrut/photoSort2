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
private:
    QString workDir_;
signals:
    void loaded();
public slots:
    void readDir(QString path);
private:
    void build(QString path);
    void fill(const QString &path);
public slots:
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
    void read(const QString &path);
    void write(const QString &path);
public:
    PhotoSortItem *photoItem(int row);
};

#endif // PHOTOSORTMODEL_H
