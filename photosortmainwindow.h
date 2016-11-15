#ifndef PHOTOSORTMAINWINDOW_H
#define PHOTOSORTMAINWINDOW_H

#include <QMainWindow>

#include <QListView>
#include <QGraphicsView>
#include <QProgressBar>
#include <QSettings>

class DetailScene;
class PhotoSortModel;
class PhotoSortPreview;

class PhotoSortMainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit PhotoSortMainWindow(QWidget *parent = nullptr);
    ~PhotoSortMainWindow();

private:
    PhotoSortPreview *preview_;
    PhotoSortModel *model_;
    QGraphicsView *detailView_;
    DetailScene *detailScene_;
    QProgressBar *progBar_;
    QSettings *settings_;
    QThread *modelThread_;

private:
    void createConnections();
    void createShortCuts();
private slots:
    void openDir();
    void onLoadFinished();
};

#endif // PHOTOSORTMAINWINDOW_H
