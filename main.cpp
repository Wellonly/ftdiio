
#include "mainwidget.h"

#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>

QT_CHARTS_USE_NAMESPACE

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWidget w;
    w.resize(720, 480);
    w.show();

    return a.exec();
}
