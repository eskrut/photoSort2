#ifndef PHOTOSORTITEM_H
#define PHOTOSORTITEM_H

#include "QStandardItem"
#include "QPixmap"

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
        PixmapRole
    };

private:
    QString path_;
    QStringList tags_;
    bool isAccepted_;
    QPixmap pixmap_;

public:
    QVariant data(int role) const;
    void setData(const QVariant &value, int role);
    int type() const;

    // QStandardItem interface
public:
    void read(QDataStream &in);
    void write(QDataStream &out) const;
};



#endif // PHOTOSORTITEM_H
