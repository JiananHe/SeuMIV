#pragma once

#include <vtkCommand.h>
#include <vtkSliderWidget.h>
#include <vtkSliderRepresentation2D.h>
#include <vtkSmartPointer.h>
#include <vtkImageReslice.h>
#include <vtkImageActor.h>
#include <vtkRenderer.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkImageData.h>
#include <vtkImagePlaneWidget.h>
#include <qDebug>
#include "Common.h"

using namespace std;

class vtkSliderCallback :public vtkCommand
{
public:
	static vtkSliderCallback *New()
	{
		return new vtkSliderCallback;
	}

	virtual void Execute(vtkObject*caller, unsigned long, void*)
	{
		vtkSliderWidget *sliderWidget = reinterpret_cast<vtkSliderWidget*>(caller);

		int v = static_cast<vtkSliderRepresentation*>
			(sliderWidget->GetRepresentation())->GetValue();

		//CPR
		if (state == 1) {
			for (int i = 0; i < 3; i++) {
				if (sliderWidget->GetDefaultRenderer()->HasViewProp(imgActor[i])) {
					if (planeWidget[i] != NULL) {
						planeWidget[i]->SetSliceIndex(v);
						planeWidget[i]->UpdatePlacement();
						planeWidget[i]->Modified();

						vtkSmartPointer<vtkImageReslice> reslice = planeWidget[i]->GetReslice();
						reslice->Update();
						imgActor[i]->SetInputData(reslice->GetOutput());
					}
					imgActor[i]->Modified();
					imgActor[i]->Update();
				}
			}
		}
		else if (state == 2) {//Blend
			for (int i = 0; i < 3; i++) {
				if (sliderWidget->GetDefaultRenderer()->HasViewProp(imgActor[i])) {
					if (!segData.empty()) {
						for (int j = 0; j < segData.size(); j++) {
							vtkSmartPointer<vtkImageData> resliceImage;
							Common::getReslice(i, v, segData[j], resliceImage);
							actors[j][i]->SetInputData(resliceImage);
							actors[j][i]->Modified();
							actors[j][i]->Update();
						}
					}
				}
			}
		}

		renWin->Render();
	}

	vtkSmartPointer<vtkGenericOpenGLRenderWindow> renWin;
	vtkSmartPointer<vtkImagePlaneWidget> planeWidget[3] = { NULL };
	vtkSmartPointer<vtkImageActor> imgActor[3];
	vector<vector<vtkSmartPointer<vtkImageActor>>> actors;
	vector<vtkSmartPointer<vtkImageData>> segData;
	int state = 1;//1-CPR滑动条，2-Blend滑动条
};
