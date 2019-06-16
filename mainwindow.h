#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
	void onView2DSlot();
	void onView3DSlot();
	void show2DCPR();
	void show2DMPR();
	void show2DBlend();
	void minimize();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
