#include "volumeRenderProcess.h"

VolumeRenderProcess::VolumeRenderProcess(QVTKWidget * qvtk_widget)
{
	this->my_vr_widget = qvtk_widget;

	dicoms_reader = vtkSmartPointer<vtkDICOMImageReader>::New();
	nii_reader = vtkSmartPointer<vtkNIFTIImageReader>::New();
	
	volume_render = vtkSmartPointer<vtkRenderer>::New();

	volumeProperty = vtkSmartPointer<vtkVolumeProperty>::New();
	volumeColor = vtkSmartPointer<vtkColorTransferFunction>::New();
	volumeScalarOpacity = vtkSmartPointer<vtkPiecewiseFunction>::New();
	volumeGradientOpacity = vtkSmartPointer<vtkPiecewiseFunction>::New();
	volume = vtkSmartPointer<vtkVolume>::New();

	origin_data = vtkSmartPointer<vtkImageData>::New();
	multi_data = vtkSmartPointer<vtkImageData>::New();
	multi_property = vtkSmartPointer<vtkVolumeProperty>::New();

	property_id = 0;
}

VolumeRenderProcess::~VolumeRenderProcess()
{
}

void VolumeRenderProcess::dicomsVolumeRenderFlow(QString folder_path)
{
	QByteArray ba = folder_path.toLocal8Bit();
	const char *folderName_str = ba.data();

	//reader
	dicoms_reader = vtkSmartPointer<vtkDICOMImageReader>::New();
	volume_render = vtkSmartPointer<vtkRenderer>::New();

	dicoms_reader->SetDirectoryName(folderName_str);
	dicoms_reader->Update();
	origin_data = dicoms_reader->GetOutput();

	//get images dimension
	int imageDims[3];
	origin_data->GetDimensions(imageDims);
	cout << "dimension[] :" << imageDims[0] << " " << imageDims[1] << " " << imageDims[2] << endl;
	if (imageDims[0] == 0 || imageDims[1] == 0 || imageDims[2] == 0)
		return;

	//Mapper
	vtkSmartPointer<vtkGPUVolumeRayCastMapper> RcGpuMapper = vtkSmartPointer<vtkGPUVolumeRayCastMapper>::New();
	RcGpuMapper->SetInputData(origin_data);

	//vtkVolumeProperty
	volumeProperty->RemoveAllObservers();
	volumeProperty->SetInterpolationTypeToLinear();
	volumeProperty->ShadeOn();
	volumeProperty->SetAmbient(0.1);
	volumeProperty->SetDiffuse(0.9);
	volumeProperty->SetSpecular(0.2);
	volumeProperty->SetSpecularPower(10.0);
	volumeProperty->SetColor(volumeColor);
	volumeProperty->SetGradientOpacity(volumeGradientOpacity);
	volumeProperty->SetScalarOpacity(volumeScalarOpacity);

	//volume
	volume->RemoveAllObservers();
	volume->SetMapper(RcGpuMapper);
	volume->SetProperty(volumeProperty);

	//render
	volume_render->AddViewProp(volume);
	my_vr_widget->GetRenderWindow()->AddRenderer(volume_render);
	my_vr_widget->GetRenderWindow()->Render();
}

void VolumeRenderProcess::niiVolumeRenderFlow(QString file_name)
{
	QByteArray ba = file_name.toLocal8Bit();
	const char *fileName_str = ba.data();

	//reader
	dicoms_reader = vtkSmartPointer<vtkDICOMImageReader>::New();

	nii_reader = vtkSmartPointer<vtkNIFTIImageReader>::New();
	nii_reader->SetFileName(fileName_str);
	nii_reader->Update();

	//get images dimension
	int imageDims[3];
	nii_reader->GetOutput()->GetDimensions(imageDims);
	cout << "dimension[] :" << imageDims[0] << " " << imageDims[1] << " " << imageDims[2] << endl;
	if (imageDims[0] == 0 || imageDims[1] == 0 || imageDims[2] == 0)
		return;

	//get range, and scale if necessary
	double range[2];
	nii_reader->GetOutput()->GetScalarRange(range);
	vtkSmartPointer<vtkImageMathematics> m = vtkSmartPointer<vtkImageMathematics>::New();
	if (range[0] == 0 && range[1] == 1)
	{
		cout << "Binary data!!" << endl;
		m->SetInput1Data(nii_reader->GetOutput());
		m->SetConstantK(255);
		m->SetOperationToMultiplyByK();
		m->Update();
		origin_data = m->GetOutput();
	}
	else
		origin_data = nii_reader->GetOutput();

	//Mapper
	vtkSmartPointer<vtkGPUVolumeRayCastMapper> RcGpuMapper = vtkSmartPointer<vtkGPUVolumeRayCastMapper>::New();
	RcGpuMapper->SetInputData(origin_data);

	//vtkVolumeProperty
	volumeProperty->RemoveAllObservers();
	volumeProperty->SetInterpolationTypeToLinear();
	volumeProperty->ShadeOn();
	volumeProperty->SetAmbient(0.1);
	volumeProperty->SetDiffuse(0.9);
	volumeProperty->SetSpecular(0.2);
	volumeProperty->SetSpecularPower(10.0);
	volumeProperty->SetColor(volumeColor);
	volumeProperty->SetGradientOpacity(volumeGradientOpacity);
	volumeProperty->SetScalarOpacity(volumeScalarOpacity);

	//volume
	volume->RemoveAllObservers();
	volume->SetMapper(RcGpuMapper);
	volume->SetProperty(volumeProperty);

	//render
	volume_render->AddVolume(volume);
	my_vr_widget->GetRenderWindow()->AddRenderer(volume_render);
	my_vr_widget->GetRenderWindow()->Render();
	my_vr_widget->update();
}

void VolumeRenderProcess::addVolume()
{
	if (property_id != 0)
	{
		vtkNew<vtkImageAppendComponents> append;
		append->SetInputData(multi_data);
		append->AddInputData(origin_data);
		append->Update();

		multi_data->DeepCopy(append->GetOutput());
	}
	else
		multi_data->DeepCopy(origin_data);
	
	vtkSmartPointer<vtkColorTransferFunction> ctf = vtkSmartPointer <vtkColorTransferFunction>::New();
	ctf->DeepCopy(volumeColor);
	multi_property->SetColor(property_id, ctf);

	vtkSmartPointer<vtkPiecewiseFunction> pf = vtkSmartPointer<vtkPiecewiseFunction>::New();
	pf->DeepCopy(volumeScalarOpacity);
	multi_property->SetScalarOpacity(property_id, pf);

	vtkSmartPointer<vtkPiecewiseFunction> gf = vtkSmartPointer<vtkPiecewiseFunction>::New();
	gf->DeepCopy(volumeGradientOpacity);
	multi_property->SetGradientOpacity(property_id, gf);

	property_id++;

	int image_dims[3];
	int nc;
	multi_data->GetDimensions(image_dims);
	nc = multi_data->GetNumberOfScalarComponents();
	cout << image_dims[0] << " " << image_dims[1] << " " << image_dims[2] << " " << nc << endl;
}

void VolumeRenderProcess::showAllVolumes()
{
	cout << "Show all voulmes" << endl;

	vtkSmartPointer<vtkGPUVolumeRayCastMapper> multi_mapper = vtkSmartPointer<vtkGPUVolumeRayCastMapper>::New();
	multi_mapper->SetInputData(multi_data);

	//other property
	multi_property->ShadeOn();
	multi_property->SetAmbient(0.1);
	multi_property->SetDiffuse(0.9);
	multi_property->SetSpecular(0.2);
	multi_property->SetSpecularPower(10.0);
	multi_property->SetInterpolationTypeToLinear();

	//visual current volume tf
	volumeColor = multi_property->GetRGBTransferFunction(property_id - 1);
	volumeScalarOpacity = multi_property->GetScalarOpacity(property_id - 1);
	volumeGradientOpacity = multi_property->GetGradientOpacity(property_id - 1);

	//volume
	volume->RemoveAllObservers();
	volume->SetMapper(multi_mapper);
	volume->SetProperty(multi_property);

	//render
	volume_render->AddVolume(volume);
	my_vr_widget->GetRenderWindow()->AddRenderer(volume_render);
	my_vr_widget->GetRenderWindow()->Render();
	my_vr_widget->update();
}

void VolumeRenderProcess::clearVolumesCache()
{

}

void VolumeRenderProcess::setBgColor(QColor color)
{
	vtkSmartPointer<vtkNamedColors> bg_color = vtkSmartPointer<vtkNamedColors>::New();
	bg_color->SetColor("bg_color", color.red() / 255.0, color.green() / 255.0, color.blue() / 255.0);
	volume_render->SetBackground(bg_color->GetColor3d("bg_color").GetData());
	my_vr_widget->GetRenderWindow()->Render();
	my_vr_widget->update();
}

vtkColorTransferFunction * VolumeRenderProcess::getVolumeColorTf()
{
	return this->volumeColor;
}

vtkPiecewiseFunction * VolumeRenderProcess::getVolumeOpacityTf()
{
	return this->volumeScalarOpacity;
}

vtkPiecewiseFunction * VolumeRenderProcess::getVolumeGradientTf()
{
	return this->volumeGradientOpacity;
}

vtkImageData * VolumeRenderProcess::getOriginData()
{
	return origin_data;
}

double VolumeRenderProcess::getMinGrayValue()
{
	double range[2];
	origin_data->GetScalarRange(range);
	
	return range[0];
}

double VolumeRenderProcess::getMaxGrayValue()
{
	double range[2];
	origin_data->GetScalarRange(range);

	return range[1];
}

void VolumeRenderProcess::setVRMapper(const char * str_mapper)
{
	if (str_mapper == "ray_cast")
	{
		vtkSmartPointer<vtkGPUVolumeRayCastMapper> RcGpuMapper = vtkSmartPointer<vtkGPUVolumeRayCastMapper>::New();
		if (nii_reader->GetFileName() == NULL)
			RcGpuMapper->SetInputConnection(dicoms_reader->GetOutputPort());
		if (dicoms_reader->GetDirectoryName() == NULL)
			RcGpuMapper->SetInputConnection(nii_reader->GetOutputPort());

		volume->SetMapper(RcGpuMapper);
		update();
	}
	else if (str_mapper == "smart")
	{
		vtkSmartPointer<vtkSmartVolumeMapper> volumeMapperSmart = vtkSmartPointer<vtkSmartVolumeMapper>::New();
		if (nii_reader->GetFileName() == NULL)
			volumeMapperSmart->SetInputConnection(dicoms_reader->GetOutputPort());
		if (dicoms_reader->GetDirectoryName() == NULL)
			volumeMapperSmart->SetInputConnection(nii_reader->GetOutputPort());

		volume->SetMapper(volumeMapperSmart);
		update();
	}
}

void VolumeRenderProcess::update()
{
	volume_render->ResetCamera();
	my_vr_widget->GetRenderWindow()->AddRenderer(volume_render);
	my_vr_widget->GetRenderWindow()->Render();
	my_vr_widget->update();
}