#include "photosortmainwindow.h"

#include <QHBoxLayout>
#include <QFileDialog>
#include <QShortcut>
#include <QThread>
#include <QMetaObject>

#include "photosortpreview.h"
#include "photosortmodel.h"
#include "photosortitem.h"
#include "detailscene.h"

PhotoSortMainWindow::PhotoSortMainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    qRegisterMetaType<QVector<int>>();

    QWidget *cw = new QWidget(this);
    auto lo = new QHBoxLayout;
    lo->setMargin(2);
    auto vlo = new QVBoxLayout;
    vlo->addLayout(lo);
    cw->setLayout(vlo);
    setCentralWidget(cw);

    preview_ = new PhotoSortPreview(this);

    lo->addWidget(preview_);

    modelThread_ = new QThread(this);
    model_ = new PhotoSortModel;
    model_->moveToThread(modelThread_);
    modelThread_->start();

    preview_->setModel(model_);

    detailView_ = new QGraphicsView(this);

    lo->addWidget(detailView_);

    detailScene_ = new DetailScene(this);
    detailView_->setScene(detailScene_);

    progBar_ = new QProgressBar(this);
    progBar_->setRange(0, 100);
    vlo->addWidget(progBar_);
    progBar_->hide();

    settings_ = new QSettings("kSystem", "photoSort", this);

    lo->setStretch(0, 1);
    lo->setStretch(1, 2);

    createShortCuts();
    createConnections();

    openDir();
}

PhotoSortMainWindow::~PhotoSortMainWindow()
{
    settings_->sync();
}

void PhotoSortMainWindow::createConnections()
{
    connect(model_, &PhotoSortModel::loaded, this, &PhotoSortMainWindow::onLoadFinished, Qt::ConnectionType::QueuedConnection);
    connect(model_, &PhotoSortModel::progress, progBar_, &QProgressBar::setValue);
    connect(model_, &PhotoSortModel::progress, [=](int prog){Q_UNUSED(prog); preview_->update();});

    connect(preview_->selectionModel(), &QItemSelectionModel::currentChanged,
            [=](const QModelIndex &current, const QModelIndex &previous){
        Q_UNUSED(previous);
        auto item = model_->photoItem(current.row());
        detailScene_->setupPhotoSortItem(item);
        detailView_->update();
    });
    connect(preview_, &PhotoSortPreview::leftInGroup, detailScene_, &DetailScene::focusLeft);
    connect(preview_, &PhotoSortPreview::rightInGroup, detailScene_, &DetailScene::focusRight);
    connect(preview_, &PhotoSortPreview::accept, detailScene_, &DetailScene::acceptCurrent);
    connect(preview_, &PhotoSortPreview::reject, detailScene_, &DetailScene::rejectCurrent);
    connect(preview_, &PhotoSortPreview::toggle, detailScene_, &DetailScene::toggleCurrent);
    connect(preview_, &PhotoSortPreview::acceptOnly, detailScene_, &DetailScene::acceptOnlyCurrent);
    connect(preview_, &PhotoSortPreview::rejectAll, detailScene_, &DetailScene::rejectAll);
    connect(preview_, &PhotoSortPreview::accept, model_, &PhotoSortModel::sync);
    connect(preview_, &PhotoSortPreview::reject, model_, &PhotoSortModel::sync);
    connect(preview_, &PhotoSortPreview::toggle, model_, &PhotoSortModel::sync);
    connect(preview_, &PhotoSortPreview::acceptOnly, model_, &PhotoSortModel::sync);
    connect(preview_, &PhotoSortPreview::rejectAll, model_, &PhotoSortModel::sync);
}

void PhotoSortMainWindow::createShortCuts()
{
    auto openShC = new QShortcut(QKeySequence(QKeySequence::Open), this);
    connect(openShC, &QShortcut::activated, this, &PhotoSortMainWindow::openDir);
}

void PhotoSortMainWindow::openDir()
{
//    preview_->setEnabled(false);
    auto dir = QFileDialog::getExistingDirectory(this,
                                                 tr("Choose directory to open"),
                                                 settings_->value("lastReadDir", QString()).toString());
    if( ! dir.isEmpty() ) {
        settings_->setValue("lastReadDir", dir);
        settings_->sync();
        QMetaObject::invokeMethod(model_, "readDir", Qt::ConnectionType::QueuedConnection, Q_ARG(QString, dir));
        progBar_->setValue(0);
        progBar_->show();
    }
}

void PhotoSortMainWindow::onLoadFinished()
{
//    preview_->setEnabled(true);
    progBar_->hide();
    preview_->update();
    preview_->setCurrentIndex(model_->index(0, 0));
    preview_->setFocus();
}
