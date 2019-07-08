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
	multi_property_temp = vtkSmartPointer<vtkVolumeProperty>::New();
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
	volumeColor = vtkSmartPointer<vtkColorTransferFunction>::New();
	volumeScalarOpacity = vtkSmartPointer<vtkPiecewiseFunction>::New();
	volumeGradientOpacity = vtkSmartPointer<vtkPiecewiseFunction>::New();

	dicoms_reader->SetDirectoryName(folderName_str);
	dicoms_reader->Update();
	origin_data = dicoms_reader->GetOutput();

	//get images dimension
	int imageDims[3];
	origin_data->GetDimensions(imageDims);
	cout << "dimension[] :" << imageDims[0] << " " << imageDims[1] << " " << imageDims[2] << endl;
	if (imageDims[0] == 0 || imageDims[1] == 0 || imageDims[2] == 0)
		return;

	//get range
	double range[2];
	origin_data->GetScalarRange(range);
	min_gv = range[0];
	max_gv = range[1];

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
	volumeColor = vtkSmartPointer<vtkColorTransferFunction>::New();
	volumeScalarOpacity = vtkSmartPointer<vtkPiecewiseFunction>::New();
	volumeGradientOpacity = vtkSmartPointer<vtkPiecewiseFunction>::New();
	
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

	//get range
	origin_data->GetScalarRange(range);
	min_gv = range[0];
	max_gv = range[1];

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
		vtkSmartPointer<vtkImageAppendComponents> append = vtkSmartPointer<vtkImageAppendComponents>::New();
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
	cur_volume_id = property_id - 1;

	int image_dims[3];
	int nc;
	multi_data->GetDimensions(image_dims);
	nc = multi_data->GetNumberOfScalarComponents();
	cout << image_dims[0] << " " << image_dims[1] << " " << image_dims[2] << " " << nc << endl;
}

void VolumeRenderProcess::showAllVolumes()
{
	cout << "Show all voulmes" << endl;
	for (int i = 0; i < 20; i++)
		volumes_status[i] = 1;

	vtkSmartPointer<vtkGPUVolumeRayCastMapper> multi_mapper = vtkSmartPointer<vtkGPUVolumeRayCastMapper>::New();
	multi_mapper->SetInputData(multi_data);

	multi_property_temp->DeepCopy(multi_property);//备份最初的所有的volumeProperties

	//other property
	multi_property->ShadeOn();
	multi_property->SetAmbient(0.1);
	multi_property->SetDiffuse(0.9);
	multi_property->SetSpecular(0.2);
	multi_property->SetSpecularPower(10.0);
	multi_property->SetInterpolationTypeToLinear();

	//visual current volume tf, which is the last one
	cur_volume_id = property_id - 1;
	volumeColor = multi_property->GetRGBTransferFunction(cur_volume_id);
	volumeScalarOpacity = multi_property->GetScalarOpacity(cur_volume_id);
	volumeGradientOpacity = multi_property->GetGradientOpacity(cur_volume_id);

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

void VolumeRenderProcess::changeCurVolume(int cur_vid)
{
	//备份当前volume的property
	if (volumes_status[cur_volume_id] == 1)
	{
		multi_property_temp->SetColor(cur_volume_id, multi_property->GetRGBTransferFunction(cur_volume_id));
		multi_property_temp->SetGradientOpacity(cur_volume_id, multi_property->GetGradientOpacity(cur_volume_id));

		vtkSmartPointer<vtkPiecewiseFunction> cur_tf = vtkSmartPointer<vtkPiecewiseFunction>::New();
		cur_tf->DeepCopy(multi_property->GetScalarOpacity(cur_volume_id));
		multi_property_temp->SetScalarOpacity(cur_volume_id, cur_tf);
	}
	
	//visual current volume tf
	cur_volume_id = cur_vid;
	volumeColor = multi_property->GetRGBTransferFunction(cur_vid);
	volumeScalarOpacity = multi_property->GetScalarOpacity(cur_vid);
	volumeGradientOpacity = multi_property->GetGradientOpacity(cur_vid);
}

void VolumeRenderProcess::showCurVolume(int cur_vid)
{
	vtkSmartPointer<vtkPiecewiseFunction> cur_tf = vtkSmartPointer<vtkPiecewiseFunction>::New();
	cur_tf->DeepCopy(multi_property_temp->GetScalarOpacity(cur_vid));
	multi_property->SetScalarOpacity(cur_vid, cur_tf);

	volumeScalarOpacity = multi_property->GetScalarOpacity(cur_vid);
	volumes_status[cur_vid] = 1;
}

void VolumeRenderProcess::hideCurVolume(int cur_vid)
{
	//备份当前volume的property
	multi_property_temp->SetColor(cur_vid, multi_property->GetRGBTransferFunction(cur_vid));
	multi_property_temp->SetGradientOpacity(cur_vid, multi_property->GetGradientOpacity(cur_vid));

	vtkSmartPointer<vtkPiecewiseFunction> cur_tf = vtkSmartPointer<vtkPiecewiseFunction>::New();
	cur_tf->DeepCopy(multi_property->GetScalarOpacity(cur_vid));
	multi_property_temp->SetScalarOpacity(cur_vid, cur_tf);

	volumeScalarOpacity->RemoveAllObservers();
	volumeScalarOpacity->AddPoint(min_gv, .0);
	volumeScalarOpacity->AddPoint(max_gv, .0);
	volumes_status[cur_vid] = 0;
}

void VolumeRenderProcess::deleteCurVolume()
{
	int n = multi_data->GetNumberOfScalarComponents();
	if (n <= 1)
	{
		clearVolumesCache();
		return;
	}

	vtkSmartPointer<vtkImageExtractComponents> extract = vtkSmartPointer<vtkImageExtractComponents>::New();
	vtkSmartPointer<vtkImageAppendComponents> append = vtkSmartPointer<vtkImageAppendComponents>::New();
	vtkSmartPointer<vtkVolumeProperty> new_multi_property = vtkSmartPointer<vtkVolumeProperty>::New();
	vtkSmartPointer<vtkColorTransferFunction> vc = vtkSmartPointer<vtkColorTransferFunction>::New();
	vtkSmartPointer<vtkPiecewiseFunction> so = vtkSmartPointer<vtkPiecewiseFunction>::New();
	vtkSmartPointer<vtkPiecewiseFunction> go = vtkSmartPointer<vtkPiecewiseFunction>::New();
	int ex_vid = 0, pro_id = 0;

	extract->SetInputData(multi_data);
	new_multi_property->DeepCopy(multi_property);
	if (cur_volume_id != 0)
		ex_vid = 0;
	else
		ex_vid = 1;

	extract->SetComponents(ex_vid);
	vc->DeepCopy(multi_property->GetRGBTransferFunction(ex_vid));
	new_multi_property->SetColor(pro_id, vc);
	so->DeepCopy(multi_property->GetScalarOpacity(ex_vid));
	new_multi_property->SetScalarOpacity(pro_id, so);
	go->DeepCopy(multi_property->GetGradientOpacity(ex_vid));
	new_multi_property->SetGradientOpacity(pro_id, go);
	ex_vid++;
	pro_id++;

	append->SetInputConnection(extract->GetOutputPort());
	append->Update();

	while(ex_vid < n)
	{
		if (cur_volume_id != ex_vid)
		{
			extract->SetComponents(ex_vid);
			append->AddInputConnection(extract->GetOutputPort());
			vc->DeepCopy(multi_property->GetRGBTransferFunction(ex_vid));
			new_multi_property->SetColor(pro_id, vc);
			so->DeepCopy(multi_property->GetScalarOpacity(ex_vid));
			new_multi_property->SetScalarOpacity(pro_id, so);
			go->DeepCopy(multi_property->GetGradientOpacity(ex_vid));
			new_multi_property->SetGradientOpacity(pro_id, go);
		}
		ex_vid++;
		pro_id++;
	}
	append->Update();
	multi_data->DeepCopy(append->GetOutput());
	multi_property->DeepCopy(new_multi_property);
	update();
}

void VolumeRenderProcess::clearVolumesCache()
{
	multi_data->RemoveAllObservers();
	multi_property->RemoveAllObservers();
	multi_data = vtkSmartPointer<vtkImageData>::New();
	multi_property = vtkSmartPointer<vtkVolumeProperty>::New();
	volume_render->RemoveAllObservers();
	volume_render->RemoveAllViewProps();

	update();
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
	return min_gv;
}

double VolumeRenderProcess::getMaxGrayValue()
{
	return max_gv;
}

void VolumeRenderProcess::update()
{
	volume_render->ResetCamera();
	my_vr_widget->GetRenderWindow()->AddRenderer(volume_render);
	my_vr_widget->GetRenderWindow()->Render();
	my_vr_widget->update();
}