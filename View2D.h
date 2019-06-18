#pragma once
#include <QVTKOpenGLWidget.h>
#include <QWidget>
#include <vtkSmartPointer.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkDICOMImageReader.h>
#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkProperty.h>
#include <vtkTextProperty.h>
#include <vtkTextMapper.h>
#include <vtkInteractorStyleImage.h>
#include <vtkImageActor.h>
#include <vtkImageData.h>
#include <vtkImageMapToColors.h>
#include "vtkResliceCursorCallback.h"

class View2D :
	public QVTKOpenGLWidget
{
	Q_OBJECT
public:
	View2D(QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
	~View2D();

public:
	void DisplayMPR();
	void DisplayCPR();
	void DisplayBlend();
	void DisplaySlices();
	void setDICOMData(vtkImageData *data);
	void changeViewPort();

protected slots:
	void OnResized();

public:
	int state = 1;//0-Slices,1-MPR,2-CPR,3-Blend;
	int viewState = 3;//0-左下矢状面,1-右下冠状面,2-左上横断面,3-四窗口视图-三视图+立体三视图

private:
	int height;
	int width;
	vtkSmartPointer<vtkTextActor> textActor[4] = { NULL };
	vtkSmartPointer<vtkRenderer> ren[4];
	vtkSmartPointer<vtkResliceCursorCallback> cbk = vtkSmartPointer<vtkResliceCursorCallback>::New();
	vtkSmartPointer< vtkResliceCursorWidget > resliceCursorWidget[3];
	vtkSmartPointer<vtkGenericOpenGLRenderWindow> renWin = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
	// vtkImagePlaneWidget, 该类内部定义了一个vtkImageReslice对象，利用vtkResliceCursor中定义的切分平面来切分图像，在其内部通过纹理映射来绘制到一个平面上，并在用户指定的vtkRenderer进行显示。
	vtkSmartPointer<vtkImagePlaneWidget> planeWidget[3];

	vtkSmartPointer<vtkImageData> DICOMData;
};