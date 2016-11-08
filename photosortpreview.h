#ifndef PHOTOSORTPREVIEW_H
#define PHOTOSORTPREVIEW_H

#include <QListView>

class PhotoSortModel;

class PhotoSortPreview : public QListView
{
    Q_OBJECT
public:
    explicit PhotoSortPreview(QWidget *parent = 0);
    void setModel(QAbstractItemModel *model);
private:
    PhotoSortModel *phModel_;
    enum class Actions {
        NoAction = -1,
        Group,
        UnGroup,
        Accept,
        Reject,
        SwapAcceptReject,
        FocusToNext,
        FocusToPrev,
        ToggleSelectionFocusToNext,
        ToggleSelectionFocusToPrev,
        ClearSelection
    };
    QMap<Actions, int> actionMap_;
    void generateDefaultActionMap();

signals:

public slots:

protected:
    void keyPressEvent(QKeyEvent *event);
};

#endif // PHOTOSORTPREVIEW_H
