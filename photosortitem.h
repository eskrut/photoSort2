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
        AcceptRole
    };

private:
    QString path_;
    QStringList tags_;
    bool isAccepted_;

public:
    QVariant data(int role) const;
    void setData(const QVariant &value, int role);
    int type() const;
};

#endif // PHOTOSORTITEM_H
