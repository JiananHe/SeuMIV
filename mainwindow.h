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
	//切换到2D浏览
	void onView2DSlot();
	//切换到3D浏览
	void onView3DSlot();
	//(2D)切换到曲面重建
	void show2DCPR();
	//(2D)切换到平面重建
	void show2DMPR();
	//(2D)切换到NIFTI图像融合显示
	void show2DBlend();
	//最小化
	void minimize();
	//打开DICOM文件
	void openFile();
	//(2D)切换窗口布局
	void change2DView(int state);
	//(2D)打开NIFTI原始图像
	void openOriginalFile();
	//(2D)打开NIFTI分割图像
	void openSegmentFile();
	//(2D)改变NIFTI图像可视状态
	void OnChangeNiiFileVisible();
	//(2D)改变当前选中的NIFTI图像
	void OnClickNiiFilesComboBox(int index);
	//(2D)改变CPR插值方法
	void OnInterpolationMethodChanged(int index);
	//(2D)改变CPR曲线拟合方法
	void OnCurveFitMethodChanged(int index);
	//(2D)改变当前显示的切片.1-prev,2-next,3-first,4-last
	void OnChangeSlice(int flag);
	//(2D)更新当前的窗宽、窗位
	void OnChangeWindowLevel(double wl0, double wl1);
	//(2D)改变图像的窗宽窗位
	void SetWindowLevel();
	//(2D)放大
	void OnZoomIn();
	//(2D)缩小
	void OnZoomOut();
	//(2D)重置当前页面
	void OnReset();
	//(2D)顺时针旋转45°
	void OnClockwiseRotate();
	//(2D)逆时针旋转45°
	void OnContraRotate();
	//(2D)测量距离
	void OnMeasureDistance();
	//(2D)测量角度
	void OnMeasureAngle();
	//(2D)恢复手型-取消测量状态
	void OnHand();

private:
	Ui::MainWindow *ui;

	//读取的DICOM文件
	vtkSmartPointer<vtkImageData> data;

	View2D *mpr;
	View2D *cpr;
	View2D *blend;
	//(2D)当前显示页面
	int current2DState = 1;//1-mpr,2-cpr,3-blend

	//(2D)已打开的NIFTI文件
	vector<QString> niiFiles;
	//(2D)NIFTI文件的可视状态
	vector<bool> niiFilesVisbile;
};

#endif // MAINWINDOW_H
