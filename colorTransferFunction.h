#pragma once
#include <vtkColorTransferFunction.h>
#include <vtkSmartPointer.h>

#include "volumeRenderProcess.h"
#include "transferFunction.h"
#include "myQColor.h"

using namespace std;

class ColorTransferFunction : public TransferFunction<MyQColor>
{
public:
	explicit ColorTransferFunction(QWidget * );
	~ColorTransferFunction();

	void setInitialColorTf(vtkColorTransferFunction *);
	void setBoneColorTf(vtkColorTransferFunction *);
	void setBone2ColorTf(vtkColorTransferFunction *);
	void setSkinColorTf(vtkColorTransferFunction *);
	void setMuscleColorTf(vtkColorTransferFunction *);

	void updateVolumeColor(vtkColorTransferFunction *);//����bp�޸�tf
	void updateVisualColor(vtkColorTransferFunction *);//����tf�޸�bp

	void showTfDiagram();
	void showCurBpValue();
};