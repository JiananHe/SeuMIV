#include "dicomVisualizer.h"

DicomVisualizer::DicomVisualizer(QWidget * vtk_frame, QString name, QWidget * slider_frame) :
	SeriesVisualizer(vtk_frame, name, slider_frame)
{
	textActor = vtkSmartPointer<vtkTextActor>::New();
	textActor->GetTextProperty()->SetFontSize(15);
	textActor->GetTextProperty()->SetColor(0.0, 1.0, 0.0);
	textActor->SetDisplayPosition(0, 0);
}

DicomVisualizer::~DicomVisualizer()
{
}

void DicomVisualizer::transferData()
{
	vtkSmartPointer<vtkImageCast> ic = vtkSmartPointer< vtkImageCast>::New();
	ic->SetInputData(getOriginData());
	ic->SetOutputScalarTypeToDouble();
	ic->Update();
    
	setTransferedData(ic->GetOutput());
	setVisualData(ic->GetOutput());
}

void DicomVisualizer::getPositionGray(int x, int y, char* gv_str, double* pickCoords)
{
	vtkSmartPointer<vtkImageData> image = viewer->GetInput();
	vtkSmartPointer<vtkPointData> pointData_gv = vtkSmartPointer<vtkPointData>::New();

	vtkSmartPointer<vtkWorldPointPicker> picker = vtkSmartPointer<vtkWorldPointPicker>::New();

	picker->Pick(x, y, 0, viewer->GetRenderer());
	picker->GetPickPosition(pickCoords);

	// Fixes some numerical problems with the picking
	double *bounds = viewer->GetImageActor()->GetDisplayBounds();
	int axis = viewer->GetSliceOrientation();
	pickCoords[axis] = bounds[2 * axis];

	vtkPointData* pd_gv = image->GetPointData();
	if (!pd_gv)
		sprintf(gv_str, "%s", "None");

	pointData_gv->InterpolateAllocate(pd_gv, 1, 1);

	// Use tolerance as a function of size of source data
	double tol2 = image->GetLength();
	tol2 = tol2 ? tol2 * tol2 / 1000.0 : 0.001;

	// Find the cell that contains pos
	int subId_gv;
	double pcoords_gv[3], weights_gv[8];
	vtkCell* cell_gv = image->FindAndGetCell(pickCoords, NULL, -1, tol2, subId_gv, pcoords_gv, weights_gv);
	if (cell_gv)
	{
		// Interpolate the point data
		pointData_gv->InterpolatePoint(pd_gv, 0, cell_gv->PointIds, weights_gv);
		double* tuple = pointData_gv->GetScalars()->GetTuple(0);
		sprintf(gv_str, "%.2f", tuple[0]);
	}
	else
		sprintf(gv_str, "%s", "None");
}

void DicomVisualizer::showPositionInfo(double* pickCoords, char* gv, char* gd)
{
	double spacing[3];
	viewer->GetInput()->GetSpacing(spacing);
	int dims[3];
	viewer->GetInput()->GetDimensions(dims);

	int x = int(pickCoords[0] / spacing[0] + 0.5);
	int y = int(pickCoords[1] / spacing[0] + 0.5);

	char text[100];
	if (x >= 0 && x < dims[0] && y >= 0 && y < dims[1])
		sprintf(text, "Coords=(%d, %d, %d) \nGray=%s \nMagnitude=%s", x, y, viewer->GetSlice(), gv, gd);
	else
		sprintf(text, "Out of range");

	printf("%s", text);
	textActor->SetInput(text);
	viewer->GetRenderer()->AddActor(textActor);
	viewer->Render();
}
