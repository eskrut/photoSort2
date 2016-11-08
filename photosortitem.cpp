#include "photosortitem.h"

#include <QApplication>
#include <QStyle>
#include <QPainter>

PhotoSortItem::PhotoSortItem() :
    isAccepted_(false)
{

}

QVariant PhotoSortItem::data(int role) const
{
    switch (role) {
    case Qt::DisplayRole:
        if(type() == SingleImage)
            return path_;
        else
            return QString::number(rowCount());
        break;
    case Qt::DecorationRole:
        if(type() == SingleImage) {
            auto pm = QStandardItem::data(Qt::DecorationRole).value<QPixmap>();
            QPainter p(&pm);
            if(isAccepted_)
                p.drawPixmap(QRect(0, 0, 10, 10), QApplication::style()->standardIcon(QStyle::SP_DialogYesButton).pixmap(10, 10));
            else
                p.drawPixmap(QRect(0, 0, 10, 10), QApplication::style()->standardIcon(QStyle::SP_DialogNoButton).pixmap(10, 10));
            return pm;
        }
        else {
            for(int row = 0; row < rowCount(); ++row) {
                if( child(row)->type() == PhotoSortItem::SingleImage ) {
                    auto ph = reinterpret_cast<PhotoSortItem*>(child(row));
                    if(ph->isAccepted_)
                        return ph->data(Qt::DecorationRole);
                }
            }
            if(rowCount() > 0)
                return child(0)->data(Qt::DecorationRole);
            return QStandardItem::data(Qt::DecorationRole).value<QPixmap>();
        }
        break;
    case PathRole:
        return path_;
        break;
    case TagsRole:
        return tags_;
        break;
    case AcceptRole:
        return QVariant::fromValue<>(isAccepted_);
        break;
    default:
        return QStandardItem::data(role);
    }
}

void PhotoSortItem::setData(const QVariant &value, int role)
{
    switch (role) {
    case PathRole:
        path_ = value.toString();
        break;
    case TagsRole:
        tags_ = value.toStringList();
        break;
    case AcceptRole:
        isAccepted_ = value.toBool();
        break;
    case PixmapRole:
        pixmap_ = value.value<QPixmap>();
        setData(pixmap_.scaled(QSize(150, 150), Qt::KeepAspectRatio), Qt::DecorationRole);
        break;
    default:
        QStandardItem::setData(value, role);
        break;
    }
}

int PhotoSortItem::type() const
{
    if(rowCount() == 0)
        return SingleImage;
    else
        return Group;
}

void PhotoSortItem::read(QDataStream &in)
{
    qint32 num;
    in >> num;
    in >> path_ >> tags_ >> isAccepted_;
    for(int ct = 0; ct < num; ++ct) {
        auto ph = new PhotoSortItem;
        ph->read(in);
        appendRow(ph);
    }
}

void PhotoSortItem::write(QDataStream &out) const
{
    out << qint32(rowCount());
    out << path_ << tags_ << isAccepted_;
    for(int ct = 0; ct < rowCount(); ++ct) {
        child(ct, 0)->write(out);
    }
}
