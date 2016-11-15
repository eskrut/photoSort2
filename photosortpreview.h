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
        AcceptOnly,
        RejectAll,
        FocusToNext,
        FocusToPrev,
        ToggleSelectionFocusToNext,
        ToggleSelectionFocusToPrev,
        ClearSelection,
        DetailedLeft,
        DetailedRight
    };
    QMap<Actions, int> actionMap_;
    void generateDefaultActionMap();

signals:
    void accept();
    void reject();
    void toggle();
    void acceptOnly();
    void rejectAll();
    void leftInGroup();
    void rightInGroup();
public slots:

protected:
    void keyPressEvent(QKeyEvent *event);
};

#endif // PHOTOSORTPREVIEW_H
