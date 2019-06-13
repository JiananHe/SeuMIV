#include "mainwindow.h"
#include <QApplication>
#include <Windows.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

	int cx = GetSystemMetrics(SM_CXFULLSCREEN);
	int cy = GetSystemMetrics(SM_CYFULLSCREEN);

    MainWindow w;
	w.setFixedSize(cx, cy);
	w.showMaximized();

    return a.exec();
}
