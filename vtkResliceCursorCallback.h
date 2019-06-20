#pragma once
#include <vtkSmartPointer.h>
#include <vtkActor.h>
#include <vtkCommand.h>
#include <vtkImageActor.h>
#include <vtkImageReslice.h>
#include <vtkImagePlaneWidget.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkImageData.h>
#include <vtkPlaneSource.h>
#include <vtkPlane.h>
#include <vtkResliceCursorActor.h>
#include <vtkResliceCursorPolyDataAlgorithm.h>
#include <vtkResliceCursor.h>
#include <vtkResliceCursorWidget.h>
#include <vtkResliceCursorLineRepresentation.h>
#include <vtkTextActor.h>
#include <vtkProperty2D.h>
#include <QDebug>
#include "View2D.h"

class vtkResliceCursorCallback : public vtkCommand
{
public:
	static vtkResliceCursorCallback *New()
	{
		return new vtkResliceCursorCallback;
	}

	void Execute(vtkObject *caller, unsigned long ev,
		void *callData) override
	{
		if (ev == vtkResliceCursorWidget::WindowLevelEvent) {
			vtkSmartPointer<vtkResliceCursorLineRepresentation> rep =
				dynamic_cast<vtkResliceCursorLineRepresentation *>
				(dynamic_cast<vtkResliceCursorWidget *>(caller)->GetRepresentation());
			double wl[2];
			rep->GetWindowLevel(wl);
			for (int i = 0; i < 3; i++) {
				rep = dynamic_cast<vtkResliceCursorLineRepresentation *>(RCW[i]->GetRepresentation());
				rep->SetWindowLevel(wl[0],wl[1]);
			}
			view2D->sendWLSignal(wl);
		}

		vtkSmartPointer<vtkImagePlaneWidget>  ipw = dynamic_cast<vtkImagePlaneWidget *>(caller);
		if (ipw)
		{
			double* wl = static_cast<double*>(callData);

			if (ipw == this->IPW[0])
			{
				this->IPW[1]->SetWindowLevel(wl[0], wl[1], 1);
				this->IPW[2]->SetWindowLevel(wl[0], wl[1], 1);
			}
			else if (ipw == this->IPW[1])
			{
				this->IPW[0]->SetWindowLevel(wl[0], wl[1], 1);
				this->IPW[2]->SetWindowLevel(wl[0], wl[1], 1);
			}
			else if (ipw == this->IPW[2])
			{
				this->IPW[0]->SetWindowLevel(wl[0], wl[1], 1);
				this->IPW[1]->SetWindowLevel(wl[0], wl[1], 1);
			}
		}

		vtkSmartPointer<vtkResliceCursorWidget> rcw = dynamic_cast<
			vtkResliceCursorWidget *>(caller);

		if (rcw)
		{
			vtkSmartPointer<vtkResliceCursorLineRepresentation> rep = dynamic_cast<
				vtkResliceCursorLineRepresentation *>(rcw->GetRepresentation());

			// Although the return value is not used, we keep the get calls
			// in case they had side-effects

			vtkSmartPointer<vtkResliceCursor> rc = rep->GetResliceCursorActor()->GetCursorAlgorithm()->GetResliceCursor();
			for (int i = 0; i < 3; i++)
			{
				vtkSmartPointer<vtkPlaneSource> ps = static_cast<vtkPlaneSource *>(
					this->IPW[i]->GetPolyDataAlgorithm());
				ps->SetNormal(rc->GetPlane(i)->GetNormal());
				ps->SetCenter(rc->GetPlane(i)->GetOrigin());

				// If the reslice plane has modified, update it on the 3D widget
				this->IPW[i]->UpdatePlacement();
			}
		}

		this->ren[0]->GetRenderWindow()->Render();
	}

public:
	View2D *view2D;
	vtkSmartPointer<vtkImagePlaneWidget> IPW[3];
	vtkSmartPointer<vtkResliceCursorWidget> RCW[3];
	int dims[3];
	vtkSmartPointer<vtkRenderer> ren[4];
};
