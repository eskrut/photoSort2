#include <QApplication>

#include "photosortmainwindow.h"

int main(int argc, char **argv) {
    QApplication app(argc, argv);

    PhotoSortMainWindow mw;
    mw.showMaximized();

    return app.exec();
}
