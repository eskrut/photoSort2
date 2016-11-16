#ifndef PHOTOSORTITEM_H
#define PHOTOSORTITEM_H

#include "QStandardItem"
#include "QPixmap"

class PhotoSortItem;
using PHListType = QList<PhotoSortItem*>;

class PhotoSortItem : public QStandardItem
{
public:
    PhotoSortItem();

    enum {
        SingleImage = QStandardItem::UserType,
        Group
         };
    enum {
        PathRole = Qt::UserRole,
        TagsRole,
        AcceptRole,
        PixmapRole,
        AllItems,
        CountAccept
    };

private:
    QString path_;
    QStringList tags_;
    bool isAccepted_;
    QPixmap pixmap_;
    QPixmap fullPixmap_;

public:
    QVariant data(int role) const;
    void setData(const QVariant &value, int role);
    int type() const;
    PHListType allItems() const;

    bool isAccepted() const;
    QPixmap pixmap() const;
    void setFullPixmap(const QPixmap &p = QPixmap()) {fullPixmap_ = p;}

    // QStandardItem interface
public:
    void read(QDataStream &in);
    void write(QDataStream &out) const;
};



#endif // PHOTOSORTITEM_H
