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
#include "vtkResliceCursorCallback.h"
#include "vtkMyStyle.h"
#include "vtkSliderCallBack.h"
#include "Common.h"

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
	void DisplayOrignalFile(string file);
	void DisplaySegementFile(string file, int color[]);
	void ChangeActors(int index, bool isVisible, int color[] = NULL);

	vtkSmartPointer<vtkMyStyle> myStyle = vtkSmartPointer<vtkMyStyle>::New();
protected slots:
	void OnResized();

public:
	int viewState = 3;//0-����ʸ״��,1-���¹�״��,2-���Ϻ����,3-�Ĵ�����ͼ-����ͼ+��������ͼ

private:
	bool initM = false;
	bool initC = false;
	bool initS = false;
	int dims[3];
	double range[2];
	int height;
	int width;
	vtkSmartPointer<vtkTextActor> textActor[4] = { NULL };
	vtkSmartPointer<vtkRenderer> ren[4];
	vtkSmartPointer<vtkRenderWindowInteractor> iren;
	vtkSmartPointer<vtkGenericOpenGLRenderWindow> renWin = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();

	vtkSmartPointer<vtkImageData> DICOMData = NULL;
	vtkSmartPointer<vtkImageData> NIFTIData = NULL;
	vector<string> niiSegFiles;


	vtkSmartPointer<vtkSliderWidget> sliderWidget[3];
	vtkSmartPointer<vtkSliderCallback> sliderCbk;
};