#pragma once
#include "seriesVisualizer.h"

class DicomVisualizer : public SeriesVisualizer
{
public:
	DicomVisualizer(QWidget *, QString, QWidget*);
	~DicomVisualizer();

	void transferData();
	double showPositionGray(int x, int y);
	void showPositionMag(QString);

	vtkSmartPointer<vtkImageData> getOriginGrayData();
	vtkSmartPointer<vtkImageData> getOriginMagnitudeData();
};

