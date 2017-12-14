#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication CA_apps(argc, argv);
    MainWindow w;
    w.show();

    return CA_apps.exec();
}
