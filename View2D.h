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
#include <vtkImageProperty.h>
#include <vtkImageMapper3D.h>
#include <vtkSliderWidget.h>
#include <vtkSliderRepresentation2D.h>
#include <vtkNIFTIImageReader.h>
#include <vtkImageCast.h>
#include <vtkResliceCursor.h>
#include <vtkResliceCursorWidget.h>
#include <vtkDistanceWidget.h>
#include <vtkDistanceRepresentation.h>
#include <vtkAngleWidget.h>
#include "vtkSliderCallBack.h"
#include "Common.h"

class vtkMyStyle;
class vtkResliceCursorCallback;

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
	void setDICOMData(vtkImageData *data);
	void changeViewPort();
	void DisplayOrignalFile(string file);
	void DisplaySegementFile(string file, int color[]);
	void ChangeActors(int index, bool isVisible, int color[] = NULL);
	void ChangeSlice(int state, int flag);
	void SetWindowLevel(int state, double wl0, double wl1);
	void Zoom(int state, int flag);//flag,1-zoom in,2-zoom out
	void Reset(int state);//1-mpr,2-cpr,3-blend
	void Rotate(int state, int flag);//flag,1-clockwise rotation,2-contra-rotation
	void MeasureDistance();
	void MeasureAngle();

	void sendWLSignal(double wl[]);
	//TODO:
	void DisplaySlices();

	vtkSmartPointer<vtkMyStyle> myStyle = vtkSmartPointer<vtkMyStyle>::New();
protected slots:
	void OnResized();

signals:
	void changeWindowLevel(double w10, double wl1);

public:
	int viewState = 3;//0-左下矢状面,1-右下冠状面,2-左上横断面,3-四窗口视图-三视图+立体三视图
	vtkSmartPointer<vtkDistanceWidget> distanceWidget =	vtkSmartPointer<vtkDistanceWidget>::New();
	vtkSmartPointer<vtkAngleWidget> angleWidget = vtkSmartPointer<vtkAngleWidget>::New();

private:
	bool initM = false;
	bool initC = false;
	int dims[3];
	double range[2];
	int height;
	int width;
	double wl[2];
	vtkSmartPointer<vtkTextActor> textActor[4] = { NULL };
	vtkSmartPointer<vtkRenderer> ren[4];

	vtkSmartPointer<vtkImageData> DICOMData = NULL;
	vtkSmartPointer<vtkImageData> NIFTIData = NULL;
	vector<string> niiSegFiles;

	vtkSmartPointer<vtkRenderWindowInteractor> iren;
	vtkSmartPointer<vtkGenericOpenGLRenderWindow> renWin = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
	// vtkImagePlaneWidget, 该类内部定义了一个vtkImageReslice对象，利用vtkResliceCursor中定义的切分平面来切分图像，在其内部通过纹理映射来绘制到一个平面上，并在用户指定的vtkRenderer进行显示。
	vtkSmartPointer<vtkImagePlaneWidget> planeWidget[3];
	vtkSmartPointer<vtkResliceCursor> resliceCursor;
	vtkSmartPointer< vtkResliceCursorWidget > resliceCursorWidget[3];
	vtkSmartPointer<vtkResliceCursorCallback> rccbk;

	vtkSmartPointer<vtkSliderWidget> sliderWidget[3];
	vtkSmartPointer<vtkSliderCallback> sliderCbk;
};