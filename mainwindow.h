#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <vtkDICOMImageReader.h>
#include <vtkNIFTIImageReader.h>
#include <vtkImageData.h>
#include <vtkSmartPointer.h>
#include <QFileDialog.h>
#include <QMessageBox>
#include <QSignalMapper>
#include <QDebug>
#include "View2D.h"

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
	void openFile();
	void change2DView(int state);

private:
    Ui::MainWindow *ui;

	vtkSmartPointer<vtkImageData> data;

	View2D *view2D;
};

#endif // MAINWINDOW_H
