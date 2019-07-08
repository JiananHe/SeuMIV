#pragma once
#include "seriesVisualizer.h"
#include<vtkTextActor.h>

class DicomVisualizer : public SeriesVisualizer
{
public:
	DicomVisualizer(QWidget *, QString, QWidget*);
	~DicomVisualizer();

	void transferData();
	void getPositionGray(int, int, char*, double*);

	void showPositionInfo(double* pickCoords, char * gv, char * gd);

private:
	vtkSmartPointer<vtkTextActor> textActor;
};
