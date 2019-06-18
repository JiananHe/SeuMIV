#include "View2D.h"

View2D::View2D(QWidget* parent, Qt::WindowFlags f)
	:QVTKOpenGLWidget(parent, f)
{
	renWin->SetMultiSamples(0);

	for (int i = 0; i < 4; i++)
	{
		ren[i] = vtkSmartPointer<vtkRenderer>::New();
		renWin->AddRenderer(ren[i]);
		cbk->ren[i] = ren[i];
		if (i < 3)
			planeWidget[i] = vtkSmartPointer<vtkImagePlaneWidget>::New();
	}
	this->SetRenderWindow(renWin);
	ren[3]->GetActiveCamera()->Elevation(110);
	ren[3]->GetActiveCamera()->SetViewUp(0, 0, -1);
	ren[3]->GetActiveCamera()->Azimuth(45);
	ren[3]->GetActiveCamera()->Dolly(1.15);
	height = width = 0;

	// 绑定renderwindow大小变换信号
	this->connect(this, SIGNAL(resized()), SLOT(OnResized()));
}

View2D::~View2D()
{
}

void View2D::DisplayMPR()
{
	for (int i = 0; i < 4; i++)
	{
		ren[i]->RemoveAllViewProps();
		if (i < 3) {
			planeWidget[i]->RemoveAllObservers();
		}
	}

	vtkSmartPointer<vtkRenderWindowInteractor> iren = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	iren->SetRenderWindow(renWin);

	DICOMData->GetDimensions(cbk->dimension);

	for (int i = 0; i < 3; i++)
	{
		planeWidget[i]->SetInteractor(iren);
		planeWidget[i]->RestrictPlaneToVolumeOn();
		double color[3] = { 0, 0, 0 };
		color[i] = 1;
		planeWidget[i]->GetPlaneProperty()->SetColor(color);
		planeWidget[i]->TextureInterpolateOn();
		planeWidget[i]->SetResliceInterpolateToCubic();
		planeWidget[i]->SetInputData(DICOMData);
		planeWidget[i]->SetPlaneOrientation(i);
		planeWidget[i]->SetSliceIndex(cbk->dimension[i] / 2);
		planeWidget[i]->DisplayTextOn();
		planeWidget[i]->SetDefaultRenderer(ren[3]);
		//planeWidget[i]->SetWindowLevel(1358, -27);
		planeWidget[i]->On();
		planeWidget[i]->InteractionOn();
	}

	vtkSmartPointer< vtkResliceCursor > resliceCursor = vtkSmartPointer< vtkResliceCursor >::New();
	resliceCursor->SetCenter(DICOMData->GetCenter());
	resliceCursor->SetThickMode(0);
	resliceCursor->SetImage(DICOMData);
	/*
	通常两个类同时使用，每个vtkResliceCursorWidget对象中需要定义相应的vtkResliceCursor对象。vtkResliceCursorWidget通过定义的“十”字坐标轴，
	提供用户方便的切分和交互方式，支持坐标轴的旋转和平移；当坐标系发生改变时即调用vtkResliceCursor来进行图像切分并进行更新到vtkRenderer对象中。
	*/
	//vtkSmartPointer< vtkResliceCursorWidget > resliceCursorWidget[3];
	vtkSmartPointer< vtkResliceCursorLineRepresentation > resliceCursorRep[3];

	double range[2];
	DICOMData->GetScalarRange(range);
	double viewUp[3][3] = { { 0, 0, 1 },{ 0, 0, 1 },{ 0, 1, 0 } };
	for (int i = 0; i < 3; i++)
	{
		resliceCursorWidget[i] = vtkSmartPointer< vtkResliceCursorWidget >::New();
		resliceCursorWidget[i]->SetInteractor(iren);

		resliceCursorRep[i] = vtkSmartPointer< vtkResliceCursorLineRepresentation >::New();
		resliceCursorWidget[i]->SetRepresentation(resliceCursorRep[i]);
		resliceCursorRep[i]->GetResliceCursorActor()->GetCursorAlgorithm()->SetResliceCursor(resliceCursor);
		resliceCursorRep[i]->GetResliceCursorActor()->GetCursorAlgorithm()->SetReslicePlaneNormal(i);

		resliceCursorWidget[i]->SetDefaultRenderer(ren[i]);
		resliceCursorWidget[i]->SetEnabled(1);

		ren[i]->GetActiveCamera()->SetFocalPoint(0, 0, 0);
		double camPos[3] = { 0, 0, 0 };
		camPos[i] = 1;
		ren[i]->GetActiveCamera()->SetPosition(camPos);
		ren[i]->GetActiveCamera()->ParallelProjectionOn();
		ren[i]->GetActiveCamera()->SetViewUp(viewUp[i][0], viewUp[i][1], viewUp[i][2]);
		ren[i]->ResetCamera();
		cbk->IPW[i] = planeWidget[i];
		cbk->RCW[i] = resliceCursorWidget[i];
		resliceCursorWidget[i]->AddObserver(vtkResliceCursorWidget::ResliceAxesChangedEvent, cbk);
		resliceCursorRep[i]->SetWindowLevel(0.9 * (range[1] - range[0]), 0.7 * range[0] + range[1] * 0.3);
		planeWidget[i]->SetWindowLevel(0.9 * (range[1] - range[0]), 0.7 * range[0] + range[1] * 0.3);
		resliceCursorRep[i]->SetLookupTable(resliceCursorRep[0]->GetLookupTable());
		planeWidget[i]->GetColorMap()->SetLookupTable(resliceCursorRep[0]->GetLookupTable());
	}
	ren[1]->GetActiveCamera()->Azimuth(90);
	iren->AddObserver(vtkCommand::MouseMoveEvent, cbk);

	ren[0]->SetBackground(0, 0, 0);
	ren[1]->SetBackground(0, 0, 0);
	ren[2]->SetBackground(0, 0, 0);
	ren[3]->SetBackground(0, 0, 0);

	const char * text[4] = { "Coronal Plane","Sagittal Plane","Transverse Plane","MPR" };
	for (int i = 0; i < 4; i++)
	{
		textActor[i] = vtkSmartPointer<vtkTextActor>::New();
		textActor[i]->SetInput(text[i]);
		textActor[i]->GetTextProperty()->SetFontSize(25);
		textActor[i]->SetPosition(15, height - 40);
		textActor[i]->GetProperty()->SetColor(0.0, 1.0, 0.0);

		ren[i]->AddActor(textActor[i]);
	}

	changeViewPort();

	renWin->Render();

	ren[3]->ResetCameraClippingRange();
	ren[3]->ResetCamera();

	vtkSmartPointer< vtkInteractorStyleImage > style = vtkSmartPointer< vtkInteractorStyleImage >::New();
	iren->SetInteractorStyle(style);

	iren->Initialize();
}

void View2D::DisplayCPR()
{
}

void View2D::DisplayBlend()
{
}

void View2D::DisplaySlices()
{
}

void View2D::setDICOMData(vtkImageData * data)
{
	DICOMData = data;
}

void View2D::changeViewPort()
{
	if (viewState < 0 || viewState > 3)
		return;
	if (viewState == 3) {
		ren[0]->SetViewport(0, 0, 0.5, 0.5);
		ren[1]->SetViewport(0.5, 0, 1, 0.5);
		ren[2]->SetViewport(0, 0.5, 0.5, 1);
		ren[3]->SetViewport(0.5, 0.5, 1, 1);
	}
	else {
		for (int i = 0; i < 4; i++) {
			ren[i]->SetViewport(0, 0, 0, 0);
		}
		ren[viewState]->SetViewport(0, 0, 1, 1);
	}

	height = ren[viewState]->GetSize()[1] - 40;
	for (int i = 0; i < 4; i++)
		textActor[i]->SetPosition(15, height);

	renWin->Render();
}

void View2D::OnResized()
{
	this->resize(this->parentWidget()->width() - 4, this->parentWidget()->height() - 4);
	height = ren[viewState]->GetSize()[1];
}
