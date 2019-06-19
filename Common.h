#pragma once
#include <vtkImageData.h>
#include <vtkSmartPointer.h>
#include <vtkTransform.h>
#include <iostream>
#include <vtkImageReslice.h>
using namespace std;

class Common
{
public:
	//axis:0=x÷·°¢1=y÷·°¢2=z÷·
	static void getReslice(int axis, int index, vtkSmartPointer<vtkImageData> volume, vtkSmartPointer<vtkImageData>& resliceImage) {
		if (axis < 0 || axis > 2) {
			std::cerr << "Unsupport axis" << std::endl;
			resliceImage = NULL;
			return;
		}

		int dim[3];
		volume->GetDimensions(dim);
		if (index < 0 || (axis == 0 && index >= dim[0]) || (axis == 1 && index >= dim[1]) || (axis == 2 && index >= dim[2])) {
			std::cerr << "invalid index" << std::endl;
			resliceImage = NULL;
			return;
		}

		int extent[6];
		double spacing[3];
		double origin[3];

		volume->GetExtent(extent);
		volume->GetSpacing(spacing);
		volume->GetOrigin(origin);

		double center[3];
		center[0] = origin[0] + spacing[0] * 0.5 * (extent[0] + extent[1]);
		center[1] = origin[1] + spacing[1] * 0.5 * (extent[2] + extent[3]);
		center[2] = origin[2] + spacing[2] * 0.5 * (extent[4] + extent[5]);
		center[axis] = origin[axis] + spacing[axis] * index;

		double matrix[3][16] = {
			{
				0, 0,-1, 0,
				1, 0, 0, 0,
				0,-1, 0, 0,
				0, 0, 0, 1 
			},
			{
				1, 0, 0, 0,
				0, 0, 1, 0,
				0,-1, 0, 0,
				0, 0, 0, 1
			},
			{
				1, 0, 0, 0,
				0, 1, 0, 0,
				0, 0, 1, 0,
				0, 0, 0, 1
			},
		};

		vtkSmartPointer<vtkMatrix4x4> resliceAxes =
			vtkSmartPointer<vtkMatrix4x4>::New();
		resliceAxes->DeepCopy(matrix[axis]);
		resliceAxes->SetElement(0, 3, center[0]);
		resliceAxes->SetElement(1, 3, center[1]);
		resliceAxes->SetElement(2, 3, center[2]);

		vtkSmartPointer<vtkImageReslice> reslice =
			vtkSmartPointer<vtkImageReslice>::New();
		reslice->SetInputData(volume);
		reslice->SetOutputDimensionality(2);
		reslice->SetResliceAxes(resliceAxes);
		reslice->SetInterpolationModeToLinear();
		reslice->Update();

		resliceImage = reslice->GetOutput();
	}

};