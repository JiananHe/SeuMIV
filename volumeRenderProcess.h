#pragma once

#include <QVTKWidget.h>
#include <vtkSmartPointer.h>
#include <vtkDICOMImageReader.h>
#include <vtkNIFTIImageReader.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkimagedata.h>
#include <vtkGPUVolumeRayCastMapper.h>
#include <vtkVolumeProperty.h>
#include <vtkVolume.h>
#include <vtkPiecewiseFunction.h>
#include <vtkColorTransferFunction.h>
#include <vtkNamedColors.h>
#include <vtkImageAccumulate.h>
#include <vtkVolumeMapper.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkImageDataGeometryFilter.h>
#include <vtkImageMathematics.h>
#include <vtkImageShiftScale.h>
#include <vtkOutlineFilter.h>
#include <vtkPolyDataMapper.h>
#include <vtkImageAppendComponents.h>
#include <queue>

#include <qcolor.h>

class VolumeRenderProcess : public QVTKWidget
{
public:
	explicit VolumeRenderProcess(QVTKWidget *);
	~VolumeRenderProcess();

	void dicomsVolumeRenderFlow(QString );
	void niiVolumeRenderFlow(QString );

	void addVolume();
	void showAllVolumes();
	void clearVolumesCache();

	void changeCurVolume(int);
	void showCurVolume(int);
	void hideCurVolume(int);
	void deleteCurVolume(int);

	void setBgColor(QColor );

	vtkColorTransferFunction* getVolumeColorTf();
	vtkPiecewiseFunction* getVolumeOpacityTf();
	vtkPiecewiseFunction* getVolumeGradientTf();

	vtkImageData* getOriginData();
	double getMinGrayValue();
	double getMaxGrayValue();

	void setVRMapper(const char *);

	void update();

private:
	QVTKWidget* my_vr_widget;
	vtkSmartPointer<vtkDICOMImageReader> dicoms_reader;
	vtkSmartPointer< vtkNIFTIImageReader> nii_reader;

	vtkSmartPointer<vtkImageData> origin_data;
	double min_gv;
	double max_gv;

	vtkSmartPointer<vtkRenderer> volume_render;
	vtkSmartPointer<vtkVolumeProperty> volumeProperty;
	vtkSmartPointer<vtkColorTransferFunction> volumeColor;
	vtkSmartPointer<vtkPiecewiseFunction> volumeScalarOpacity;
	vtkSmartPointer<vtkPiecewiseFunction> volumeGradientOpacity;
	vtkSmartPointer<vtkVolume> volume;

	vtkSmartPointer<vtkImageData> multi_data;
	vtkSmartPointer<vtkVolumeProperty> multi_property_temp;
	vtkSmartPointer<vtkVolumeProperty> multi_property;

	int property_id;
	int cur_volume_id;
	int volumes_status[20];//±£´ævolumeµÄshow/hide×´Ì¬£¬indexÎªproperty_id
};