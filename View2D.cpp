#include "View2D.h"

View2D::View2D(QWidget* parent, Qt::WindowFlags f)
	:QVTKOpenGLWidget(parent, f)
{
	renWin->SetMultiSamples(0);

	for (int i = 0; i < 4; i++)
	{
		ren[i] = vtkSmartPointer<vtkRenderer>::New();
		renWin->AddRenderer(ren[i]);
		sliderWidget[i] = vtkSmartPointer<vtkSliderWidget>::New();
	}
	this->SetRenderWindow(renWin);
	iren = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	iren->SetRenderWindow(renWin);

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
	}
	height = width = 0;

	// 绑定renderwindow大小变换信号
	this->connect(this, SIGNAL(resized()), SLOT(OnResized()));
}

View2D::~View2D()
{
}

void View2D::DisplayMPR()
{
	if (DICOMData == NULL || initM)
		return;

	// vtkImagePlaneWidget, 该类内部定义了一个vtkImageReslice对象，利用vtkResliceCursor中定义的切分平面来切分图像，在其内部通过纹理映射来绘制到一个平面上，并在用户指定的vtkRenderer进行显示。
	vtkSmartPointer<vtkImagePlaneWidget> planeWidget[3];

	for (int i = 0; i < 4; i++)
	{
		ren[i]->RemoveAllViewProps();
		if (i < 3 && planeWidget[i] != NULL) {
			planeWidget[i]->RemoveAllObservers();
		}
		sliderWidget[i]->RemoveAllObservers();
	}

	vtkSmartPointer<vtkResliceCursorCallback> rccbk = vtkSmartPointer<vtkResliceCursorCallback>::New();
	DICOMData->GetDimensions(rccbk->dims);

	for (int i = 0; i < 4; i++)
		rccbk->ren[i] = this->ren[i];

	for (int i = 0; i < 3; i++)
	{
		planeWidget[i] = vtkSmartPointer<vtkImagePlaneWidget>::New();
		planeWidget[i]->SetInteractor(iren);
		double color[3] = { 0, 0, 0 };
		color[i] = 1;
		planeWidget[i]->GetPlaneProperty()->SetColor(color);
		planeWidget[i]->SetResliceInterpolateToCubic();
		planeWidget[i]->SetInputData(DICOMData);
		planeWidget[i]->SetPlaneOrientation(i);
		planeWidget[i]->SetSliceIndex(rccbk->dims[i] / 2);
		planeWidget[i]->DisplayTextOn();
		planeWidget[i]->SetDefaultRenderer(ren[3]);
		planeWidget[i]->On();
		planeWidget[i]->InteractionOn();
	}

	vtkSmartPointer<vtkResliceCursor> resliceCursor = vtkSmartPointer<vtkResliceCursor>::New();
	resliceCursor->SetCenter(DICOMData->GetCenter());
	resliceCursor->SetThickMode(0);
	resliceCursor->SetImage(DICOMData);

	vtkSmartPointer< vtkResliceCursorWidget > resliceCursorWidget[3];
	vtkSmartPointer<vtkResliceCursorLineRepresentation> resliceCursorRep[3];

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
		rccbk->IPW[i] = planeWidget[i];
		rccbk->RCW[i] = resliceCursorWidget[i];
		resliceCursorWidget[i]->AddObserver(vtkResliceCursorWidget::ResliceAxesChangedEvent, rccbk);
		resliceCursorRep[i]->SetWindowLevel(0.9 * (range[1] - range[0]), 0.7 * range[0] + range[1] * 0.3);
		planeWidget[i]->SetWindowLevel(0.9 * (range[1] - range[0]), 0.7 * range[0] + range[1] * 0.3);
		resliceCursorRep[i]->SetLookupTable(resliceCursorRep[0]->GetLookupTable());
		planeWidget[i]->GetColorMap()->SetLookupTable(resliceCursorRep[0]->GetLookupTable());
	}
	//ren[1]->GetActiveCamera()->Azimuth(90);

	ren[3]->SetActiveCamera(vtkSmartPointer<vtkCamera>::New());
	ren[3]->GetActiveCamera()->Elevation(110);
	ren[3]->GetActiveCamera()->SetViewUp(0, 0, -1);
	ren[3]->GetActiveCamera()->Azimuth(45);
	ren[3]->GetActiveCamera()->Dolly(1.15);
	ren[3]->ResetCameraClippingRange();
	ren[3]->ResetCamera();

	textActor[3]->SetInput("MPR");
	for (int i = 0; i < 4; i++)
		ren[i]->AddActor(textActor[i]);

	changeViewPort();

	renWin->Render();

	vtkSmartPointer< vtkInteractorStyleImage > style = vtkSmartPointer< vtkInteractorStyleImage >::New();
	iren->SetInteractorStyle(style);
	iren->Initialize();

	initM = true;
}

void View2D::DisplayCPR()
{
	if (DICOMData == NULL || initC)
		return;

	for (int i = 0; i < 4; i++)
	{
		ren[i]->RemoveAllViewProps();
		sliderWidget[i]->RemoveAllObservers();
	}

	DICOMData->GetScalarRange(range);
	DICOMData->GetDimensions(dims);

	vtkSmartPointer<vtkImagePlaneWidget> planeWidget[3];
	vtkSmartPointer<vtkImageReslice> reslice[3];

	for (int i = 0; i < 3; i++)
	{
		planeWidget[i] = vtkSmartPointer<vtkImagePlaneWidget>::New();
		planeWidget[i]->SetInteractor(iren);
		planeWidget[i]->RestrictPlaneToVolumeOn();
		planeWidget[i]->TextureInterpolateOff();
		planeWidget[i]->SetResliceInterpolateToCubic();
		planeWidget[i]->SetInputData(DICOMData);
		planeWidget[i]->SetPlaneOrientation(i);
		planeWidget[i]->SetSliceIndex(dims[i] / 2);

		reslice[i] = planeWidget[i]->GetReslice();
		reslice[i]->SetInterpolationModeToCubic();
	}

	vtkSmartPointer<vtkImageActor> imgActor[3];
	double wl[2] = { 0.7 * (range[1] - range[0]),0.7 * range[0] + range[1] * 0.3 };
	for (int i = 0; i < 3; i++) {
		reslice[i]->Update();
		imgActor[i] = vtkSmartPointer<vtkImageActor>::New();
		imgActor[i]->SetInputData(reslice[i]->GetOutput());
		imgActor[i]->GetProperty()->SetColorWindow(wl[0]);
		imgActor[i]->GetProperty()->SetColorLevel(wl[1]);
		imgActor[i]->Update();
		ren[i]->AddActor(imgActor[i]);
	}

	sliderCbk = vtkSmartPointer<vtkSliderCallback>::New();
	sliderCbk->renWin = renWin;
	sliderCbk->state = 1;
	for (int i = 0; i < 3; i++) {
		//实例化vtkSliderRepresentation2D，并设置属性。该对象用做滑块在场景中的2D表示
		vtkSmartPointer<vtkSliderRepresentation2D> sliderRep = vtkSmartPointer<vtkSliderRepresentation2D>::New();
		//滑动条两端的值，默认0-100
		sliderRep->SetMinimumValue(0);
		sliderRep->SetMaximumValue(dims[i] - 1);
		sliderRep->SetValue(dims[i] / 2);
		//sliderRep->SetTitleText("z value");
		//text position
		sliderRep->GetPoint1Coordinate()->SetCoordinateSystemToNormalizedDisplay();
		sliderRep->GetPoint1Coordinate()->SetValue(0.0, 0.01);
		sliderRep->GetPoint2Coordinate()->SetCoordinateSystemToNormalizedDisplay();
		sliderRep->GetPoint2Coordinate()->SetValue(0.5, 0.01);

		sliderRep->SetSliderLength(0.05);
		sliderRep->SetSliderWidth(0.05);
		sliderRep->SetEndCapLength(0.025);

		sliderWidget[i]->SetInteractor(iren);
		sliderWidget[i]->SetDefaultRenderer(ren[i]);
		sliderWidget[i]->SetRepresentation(sliderRep);
		sliderWidget[i]->SetAnimationModeToAnimate();
		sliderWidget[i]->EnabledOn();//用于使用滑块

		sliderCbk->imgActor[i] = imgActor[i];
		sliderCbk->planeWidget[i] = planeWidget[i];
		sliderWidget[i]->AddObserver(vtkCommand::InteractionEvent, sliderCbk);
	}

	myStyle->data = this->DICOMData;
	for (int i = 0; i < 4; i++) {
		myStyle->ren[i] = this->ren[i];
		if (i < 3)
			myStyle->imgActor[i] = imgActor[i];
	}

	textActor[3]->SetInput("CPR");
	for (int i = 0; i < 4; i++)
		ren[i]->AddActor(textActor[i]);

	ren[3]->SetActiveCamera(vtkSmartPointer<vtkCamera>::New());
	ren[3]->ResetCamera();

	changeViewPort();

	iren->SetInteractorStyle(myStyle);
	iren->Initialize();
	initC = true;
}

void View2D::DisplayBlend()
{
	for (int i = 0; i < 3; i++)
		ren[i]->AddActor(textActor[i]);

	changeViewPort();

	iren->Initialize();
}

void View2D::DisplaySlices()
{
	if (DICOMData == NULL)
		return;
	for (int i = 0; i < 4; i++)
	{
		ren[i]->RemoveAllViewProps();
	}
}

void View2D::setDICOMData(vtkImageData * data)
{
	DICOMData = data;
	initM = false;
	initC = false;
	initS = false;
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

	if (sliderWidget[0] != NULL) {
		for (int i = 0; i < 3; i++) {
			sliderWidget[i]->Modified();
		}
	}

	height = ren[viewState]->GetSize()[1] - 40;
	for (int i = 0; i < 4; i++)
		textActor[i]->SetPosition(15, height);

	renWin->Render();
}

void View2D::DisplayOrignalFile(string file)
{
	niiSegFiles.clear();
	for (int i = 0; i < 3; i++)
	{
		ren[i]->RemoveAllViewProps();
		sliderWidget[i]->RemoveAllObservers();
	}

	vtkSmartPointer<vtkNIFTIImageReader> reader = vtkSmartPointer<vtkNIFTIImageReader>::New();
	reader->SetFileName(file.c_str());
	reader->Update();
	niiSegFiles.push_back(file);

	NIFTIData = reader->GetOutput();

	NIFTIData->GetDimensions(dims);

	double range[2];
	reader->GetOutput()->GetScalarRange(range);

	vtkSmartPointer<vtkImageData> imgData[3];
	for (int i = 0; i < 3; i++)
		Common::getReslice(i, dims[i] / 2, NIFTIData, imgData[i]);

	vector<vtkSmartPointer<vtkImageActor>> imgActor(3);
	double wl[2] = { 0.6 * (range[1] - range[0]),0.63 * range[1] + 0.37 * range[0] };
	for (int i = 0; i < 3; i++) {
		imgActor[i] = vtkSmartPointer<vtkImageActor>::New();

		imgActor[i]->SetInputData(imgData[i]);
		imgActor[i]->GetProperty()->SetColorWindow(wl[0]);
		imgActor[i]->GetProperty()->SetColorLevel(wl[1]);
		imgActor[i]->Update();
		ren[i]->AddActor(imgActor[i]);
	}

	sliderCbk = vtkSmartPointer<vtkSliderCallback>::New();
	sliderCbk->renWin = renWin;
	sliderCbk->state = 2;
	sliderCbk->segData.push_back(NIFTIData);
	sliderCbk->actors.push_back(imgActor);

	for (int i = 0; i < 3; i++) {
		//实例化vtkSliderRepresentation2D，并设置属性。该对象用做滑块在场景中的2D表示
		vtkSmartPointer<vtkSliderRepresentation2D> sliderRep = vtkSmartPointer<vtkSliderRepresentation2D>::New();
		//滑动条两端的值，默认0-100
		sliderRep->SetMinimumValue(0);
		sliderRep->SetMaximumValue(dims[i] - 1);
		sliderRep->SetValue(dims[i] / 2);
		//text position
		sliderRep->GetPoint1Coordinate()->SetCoordinateSystemToNormalizedDisplay();
		sliderRep->GetPoint1Coordinate()->SetValue(0.0, 0.01);
		sliderRep->GetPoint2Coordinate()->SetCoordinateSystemToNormalizedDisplay();
		sliderRep->GetPoint2Coordinate()->SetValue(0.5, 0.01);

		sliderRep->SetSliderLength(0.05);
		sliderRep->SetSliderWidth(0.05);
		sliderRep->SetEndCapLength(0.025);

		sliderWidget[i]->SetInteractor(iren);
		sliderWidget[i]->SetDefaultRenderer(ren[i]);
		sliderWidget[i]->SetRepresentation(sliderRep);
		sliderWidget[i]->SetAnimationModeToAnimate();
		sliderWidget[i]->EnabledOn();//用于使用滑块

		sliderCbk->imgActor[i] = imgActor[i];
		sliderWidget[i]->AddObserver(vtkCommand::InteractionEvent, sliderCbk);
	}

	ren[0]->ResetCamera();
	ren[1]->ResetCamera();
	ren[2]->ResetCamera();

	for (int i = 0; i < 3; i++)
		ren[i]->AddActor(textActor[i]);

	renWin->Render();

	vtkSmartPointer<vtkInteractorStyleImage> style = vtkSmartPointer<vtkInteractorStyleImage>::New();
	iren->SetInteractorStyle(style);
	iren->Initialize();
}

void View2D::DisplaySegementFile(string file, int color[])
{
	if (NIFTIData == NULL)
		return;

	niiSegFiles.push_back(file);
	vtkSmartPointer<vtkNIFTIImageReader> reader = vtkSmartPointer<vtkNIFTIImageReader>::New();
	reader->SetFileName(file.c_str());

	reader->Update();

	double spacing[3];
	reader->GetOutput()->GetSpacing(spacing);

	int niiDims[3];
	reader->GetOutput()->GetDimensions(niiDims);
	for (int i = 0; i < 3; i++)
		if (niiDims[i] != dims[i])
			return;

	vtkSmartPointer<vtkImageData> seg =
		vtkSmartPointer<vtkImageData>::New();
	seg->SetDimensions(niiDims);
	seg->SetSpacing(spacing);
	seg->AllocateScalars(VTK_UNSIGNED_CHAR, 4);

	unsigned char * p = (unsigned char *)seg->GetScalarPointer();
	vtkSmartPointer<vtkImageCast> castFilter =
		vtkSmartPointer<vtkImageCast>::New();
	castFilter->SetInputConnection(reader->GetOutputPort());
	castFilter->SetOutputScalarTypeToUnsignedChar();
	castFilter->Update();

	vtkSmartPointer<vtkImageData> data = castFilter->GetOutput();

	unsigned char * op = (unsigned char *)data->GetScalarPointer();
	for (int i = 0; i < dims[0] * dims[1] * dims[2]; i++) {
		if (*op == 0) {
			p += 3;
			*p++ = 0;
		}
		else {
			*p++ = color[0];
			*p++ = color[1];
			*p++ = color[2];
			*p++ = color[3];
		}
		op++;
	}

	sliderCbk->segData.push_back(seg);

	vtkSmartPointer<vtkImageData> imgData[3];
	for (int i = 0; i < 3; i++)
		Common::getReslice(i, dims[i] / 2, seg, imgData[i]);

	vector<vtkSmartPointer<vtkImageActor>> actor3(3);
	for (int i = 0; i < 3; i++) {
		actor3[i] = vtkSmartPointer<vtkImageActor>::New();
		actor3[i]->SetInputData(imgData[i]);
		actor3[i]->Update();

		ren[i]->AddActor(actor3[i]);
	}
	sliderCbk->actors.push_back(actor3);

	for (int i = 0; i < 3; i++)
		ren[i]->AddActor(textActor[i]);

	renWin->Render();
}

void View2D::ChangeActors(int index, bool isVisible, int color[])
{
	if (color == NULL) {
		for (int i = 0; i < 3; i++) {
			sliderCbk->actors[index][i]->SetVisibility(isVisible);
			sliderCbk->actors[index][i]->Update();
		}
	}
	else {
		vtkSmartPointer<vtkNIFTIImageReader> reader = vtkSmartPointer<vtkNIFTIImageReader>::New();
		reader->SetFileName(niiSegFiles[index].c_str());

		reader->Update();

		vtkSmartPointer<vtkImageData> data = reader->GetOutput();

		int niiDims[3];
		data->GetDimensions(niiDims);

		double spacing[3];
		reader->GetOutput()->GetSpacing(spacing);

		vtkSmartPointer<vtkImageData> seg =
			vtkSmartPointer<vtkImageData>::New();
		seg->SetDimensions(niiDims);
		seg->SetSpacing(spacing);
		seg->AllocateScalars(VTK_UNSIGNED_CHAR, 4);

		unsigned char * op = (unsigned char *)data->GetScalarPointer();
		unsigned char * p = (unsigned char *)seg->GetScalarPointer();

		for (int i = 0; i < dims[0] * dims[1] * dims[2]; i++) {
			if (*op == 0) {
				p += 3;
				*p++ = 0;
			}
			else {
				*p++ = color[0];
				*p++ = color[1];
				*p++ = color[2];
				*p++ = color[3];
			}
			op++;
		}

		sliderCbk->segData[index] = seg;

		vtkSmartPointer<vtkImageData> imgData[3];
		for (int i = 0; i < 3; i++)
			Common::getReslice(i, dims[i] / 2, seg, imgData[i]);

		for (int i = 0; i < 3; i++) {
			sliderCbk->actors[index][i]->SetInputData(imgData[i]);
			sliderCbk->actors[index][i]->Update();
		}
	}

	for (int i = 0; i < 3; i++)
		ren[i]->AddActor(textActor[i]);

	renWin->Render();
}

void View2D::OnResized()
{
	this->resize(this->parentWidget()->width() - 4, this->parentWidget()->height() - 4);
}
