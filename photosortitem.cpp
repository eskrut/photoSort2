#include "photosortitem.h"

#include <QApplication>
#include <QStyle>
#include <QPainter>

Q_DECLARE_METATYPE(PHListType)

PhotoSortItem::PhotoSortItem() :
    isAccepted_(false),
    shiftCount_(0)
{
    setFullPixmap();
}

QVariant PhotoSortItem::data(int role) const
{
    switch (role) {
    case Qt::DisplayRole:
    {
        auto sole = QString("/%1").arg(data(CountAccept).toInt());
        if(shiftCount_ > 0)
            sole.append(QString(" %1").arg(shiftCount_));
        if(type() == SingleImage){
            auto p = path_;
            if(p.length() > 8)
                p.truncate(8);
            return p + sole;
        }
        else
            return QString::number(allItems().size()) + sole;
        break;
    }
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
    case PixmapRole:
        if(fullPixmap_.isNull())
            return pixmap_;
        else
            return fullPixmap_;
        break;
    case AllItems:
    {
        PHListType all;
        const int count = rowCount();
        if( count ) {
            for(int ct = 0; ct < count; ++ct)
                all << child(ct)->data(AllItems).value<PHListType>();
        }
        else
            all << const_cast<PhotoSortItem*>(this);
        return QVariant::fromValue<PHListType>(all);
        break;
    }
    case CountAccept:
    {
        auto items = allItems();
        int count = 0;
        for(const auto &i : items)
            if(i->isAccepted())
                ++count;
        return count;
        break;
    }
    case Qt::ToolTipRole:
    {
        QString tt;
        auto all = allItems();
        for(const auto &i : all) {
            tt.append(i->text() + "\n");
        }
        return tt;
        break;
    }
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
    case ShiftCountRole:
        shiftCount_ = value.toInt();
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

PHListType PhotoSortItem::allItems() const
{
    try {

        auto d = data(AllItems);
        auto v = d.value<PHListType>();
        return v;
    } catch (...) {
        return PHListType();
    }
}

bool PhotoSortItem::isAccepted() const
{
    return data(AcceptRole).toBool();
}

QPixmap PhotoSortItem::pixmap() const
{
    return data(PixmapRole).value<QPixmap>();
}

void PhotoSortItem::setFullPixmap(const QPixmap &p) {
    size_t truncate = 1920;
    fullPixmap_ = std::max(p.width(), p.height()) > truncate ? p.scaled(truncate, truncate, Qt::KeepAspectRatio) : p;
}

bool PhotoSortItem::isSame(PhotoSortItem *item)
{
    QList<PhotoSortItem*> thisList, list;

    thisList = allItems();
    list = item->allItems();
    for(auto &i : list) {
        for(auto &t : thisList) {
            if(i->path_ == t->path_)
                return true;
        }
    }

    return false;
}

void PhotoSortItem::read(QDataStream &in)
{
    QPixmap p(QApplication::style()->standardIcon(QStyle::StandardPixmap::SP_MessageBoxQuestion).pixmap(QSize(150, 150)));
    qint32 num;
    in >> num;
    in >> path_ >> tags_ >> isAccepted_;
    setData(p, Qt::DecorationRole);
    for(int ct = 0; ct < num; ++ct) {
        auto ph = new PhotoSortItem;
        ph->read(in);
        ph->setData(p, Qt::DecorationRole);
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
