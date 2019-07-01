#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#pragma once
#include <QMainWindow>
#include <vtkDICOMImageReader.h>
#include <vtkImageData.h>
#include <vtkSmartPointer.h>
#include <QFileDialog.h>
#include <QMessageBox>
#include <QSignalMapper>
#include <QColorDialog>
#include <vector>
#include <string>
#include "View2D.h"
#include "vtkMyStyle.h"
#include <QDebug>

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
	void openOriginalFile();
	void openSegmentFile();
	void OnChangeNiiFileVisible();
	void OnClickNiiFilesComboBox(int index);
	void OnInterpolationMethodChanged(int index);
	void OnCurveFitMethodChanged(int index);
	void OnChangeSlice(int flag);
	void OnChangeWindowLevel(double wl0, double wl1);
	void SetWindowLevel();
	void OnZoomIn();
	void OnZoomOut();
	void OnReset();
	void OnClockwiseRotate();
	void OnContraRotate();
	void OnMeasureDistance();
	void OnMeasureAngle();
	void OnHand();

private:
	Ui::MainWindow *ui;

	vtkSmartPointer<vtkImageData> data;

	View2D *mpr;
	View2D *cpr;
	View2D *blend;
	int current2DState = 1;//1-mpr,2-cpr,3-blend

	vector<QString> niiFiles;
	vector<bool> niiFilesVisbile;
};

#endif // MAINWINDOW_H
