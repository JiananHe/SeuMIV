#pragma once
#include "seriesVisualizer.h"
#include "RangeSlider.h"
#include <vtkThresholdPoints.h>
#include <vector>
#include <set>
#include <map>

using namespace std;

class BoundVisualizer : public SeriesVisualizer
{
public:
	BoundVisualizer(QWidget*, QString, QWidget*);
	~BoundVisualizer();

	void setOriginData(vtkSmartPointer<vtkImageData>);
	void transferData();

	bool setMagnitudeRange(int , int);
	int getMagnitudeRangeMin();
	int getMagnitudeRangeMax();
	void getPositionMag(double*, char*);
	void updateVisualData();

	float getMaxBoundGradientValue();
	float getMinBoundGradientValue();

	map<double, double> getRoiBoundMagBp();
	void kMeansCalc();

private:
	int mag_min_threshold;
	int mag_max_threshold;
	double mag_range[2];
	vtkSmartPointer <vtkImageMagnitude> imgMagnitude;
	vtkSmartPointer <vtkImageData> nonMaxFloat;
	vtkSmartPointer<vtkImageThreshold> mag_thresh_img;
	vtkSmartPointer<vtkThresholdPoints> mag_thresh_poly;

	RangeSlider * magnitude_thresh_slider;
	QLabel * magnitude_max_label;
	QLabel * magnitude_min_label;

	map<double, double> roi_bound_gd;
};