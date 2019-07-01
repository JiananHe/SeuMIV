#pragma once
#include <vtkSmartPointer.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkActor.h>
#include <vtkImageActor.h>
#include <vtkProperty.h>
#include <vtkPoints.h>
#include <vtkLineSource.h>
#include <vtkInteractorStyleImage.h>
#include <vtkParametricSpline.h>
#include <vtkParametricFunctionSource.h>
#include <vtkSplineFilter.h>
#include <vtkSpline.h>
#include <vtkImageData.h>
#include <vtkPointPicker.h>
#include <vtkImageProperty.h>
#include <vtkTextActor.h>
#include <vtkCommand.h>
#include <vtkImageInterpolator.h>
#include <vtkDoubleArray.h>
#include <vtkPointData.h>
#include <vtkAlgorithmOutput.h>
#include <math.h>
#include "View2D.h"

#define PI 3.14159265

enum CurveFitMethod
{
	CardinalSpline,
	Conic,
};

enum InterpolationMethod
{
	Linear,
	Cubic,
	NearestNeighbor,
};

// Define interaction style
class vtkMyStyle : public vtkInteractorStyleImage
{
public:
	static vtkMyStyle *New()
	{
		return new vtkMyStyle;
	}

	virtual void OnLeftButtonDown()
	{
		vtkInteractorStyleImage::OnLeftButtonDown();
		if (CurrentRenderer == ren[3] && imgActor[3] == NULL)
			return;
		changeWindowLevel = true;
	}

	virtual void OnLeftButtonUp()
	{
		vtkInteractorStyleImage::OnLeftButtonUp();
		changeWindowLevel = false;
	}

	virtual void OnRightButtonDown()
	{
		if (!Initialize()) {
			RightButtonEventEnd();
			return;
		}

		InsertPoint();
	}

	virtual void OnRightButtonUp()
	{
		if (data == NULL || clickPos == NULL)
			return;

		InsertPoint();

		lineSource->SetPoints(displayPoints);
		lineSource->Modified();
		lineSource->Update();

		double white[3] = { 1,1,1 };
		showLine(lineSource->GetOutputPort(), polyLineMapper, polyLineActor, white);

		if (displayPoints->GetNumberOfPoints() < 3) {
			RightButtonEventEnd();
			return;
		}

		double distance = dims[cr] * spacing[cr];
		int len = 1;
		double red[3] = { 1,0,0 };
		vtkSmartPointer<vtkPoints> points2;

		if (curveFitMethod == Conic) {
			vtkSmartPointer<vtkLineSource> conicSource = vtkSmartPointer<vtkLineSource>::New();

			//拟合显示的二次曲线
			fitConicCurve(displayPoints, conicSource, 0);
			conicSource->Modified();
			conicSource->Update();
			showLine(conicSource->GetOutputPort(), fitCurveMapper, fitCurveActor, red);

			//拟合模型中的二次曲线,计算长度
			conicSource = vtkSmartPointer<vtkLineSource>::New();
			double b[3], x[2], r[2];
			fitConicCurve(voxelPoints, conicSource, 1, b, x, r);
			conicSource->Modified();
			conicSource->Update();

			len = getLength(conicSource->GetPoints());

			//根据长度,近似均匀采样模型中的二次曲线上的点
			vtkSmartPointer<vtkPoints> pt = vtkSmartPointer<vtkPoints>::New();
			conicSource = vtkSmartPointer<vtkLineSource>::New();
			double cx = x[0], step = 1, symmetricalAxis = -0.5*b[1] / b[2], t = 4 * b[2] * b[2];
			double tx = x[0] - symmetricalAxis;
			while (cx <= x[1]) {
				double cy = b[0] + cx * b[1] + cx * cx*b[2];

				double ttx = r[0] * cx + r[1] * cy, tty = r[0] * cy - r[1] * cx;
				if (cr == 0)
					pt->InsertNextPoint(0, ttx, tty);
				else if (cr == 1)
					pt->InsertNextPoint(ttx, 0, tty);
				else
					pt->InsertNextPoint(ttx, tty, 0);
				double dx = sqrt(1.0 / (t*tx*tx + 1));
				cx += dx;
				tx += dx;
			}

			conicSource->SetPoints(pt);
			conicSource->Modified();
			conicSource->Update();

			//计算构成曲面的点集
			points2 = sweepLine(conicSource->GetOutput(), normal[cr], distance, (int)distance);
		}
		else {
			//计算、显示样条曲线(世界坐标系)
			fitBSplineCurve(displayPoints, functionSource);
			showLine(functionSource->GetOutputPort(), fitCurveMapper, fitCurveActor, red);

			//计算样条曲线(model坐标系-voxel索引)
			len = getLength(voxelPoints);
			vtkSmartPointer<vtkLineSource> ls = vtkSmartPointer<vtkLineSource>::New();
			ls->SetPoints(voxelPoints);
			ls->Update();

			vtkSmartPointer<vtkSplineFilter> splineFilter = vtkSmartPointer<vtkSplineFilter>::New();
			splineFilter->SetInputConnection(ls->GetOutputPort());
			splineFilter->SetSubdivideToSpecified();
			splineFilter->SetNumberOfSubdivisions(len);
			splineFilter->Update();

			//计算构成曲面的点集
			points2 = sweepLine(splineFilter->GetOutput(), normal[cr], distance, (int)distance);
		}

		vtkSmartPointer<vtkImageData> surface = GetImageData((int)distance + 1, len, points2, interpolationMethod);

		if (wl[0] == 0) {
			double range[2];
			data->GetScalarRange(range);
			wl[0] = 0.7 * (range[1] - range[0]);
			wl[1] = 0.7 * range[0] + range[1] * 0.3;
		}

		if (imgActor[3] == NULL)
			imgActor[3] = vtkSmartPointer<vtkImageActor>::New();
		imgActor[3]->SetInputData(surface);
		imgActor[3]->GetProperty()->SetColorWindow(wl[0]);
		imgActor[3]->GetProperty()->SetColorLevel(wl[1]);
		imgActor[3]->Update();

		ren[3]->AddActor(imgActor[3]);
		ren[3]->ResetCamera();

		CurrentRenderer->GetRenderWindow()->Render();

		RightButtonEventEnd();
	}

	//鼠标右击事件结束后，恢复所有renderer交互
	void RightButtonEventEnd() {
		clickPos = 0;
		for (int i = 0; i < 4; i++) {
			ren[i]->InteractiveOn();
		}
	}

	virtual void OnMouseMove()
	{
		vtkInteractorStyleImage::OnMouseMove();

		if (data == NULL)
			return;

		if (clickPos != nullptr) {

			InsertPoint();

			lineSource->SetPoints(displayPoints);
			lineSource->Modified();
			lineSource->Update();

			double white[3] = { 1,1,1 };
			showLine(lineSource->GetOutputPort(), polyLineMapper, polyLineActor, white);
		}

		if (changeWindowLevel) {
			computeWindowLevel();

			for (int i = 0; i < 4; i++) {
				if (imgActor[i] == NULL)
					continue;

				imgActor[i]->GetProperty()->SetColorWindow(wl[0]);
				imgActor[i]->GetProperty()->SetColorLevel(wl[1]);
				imgActor[i]->Update();
			}
		}

		Interactor->GetRenderWindow()->Render();
	}

	bool Initialize() {
		displayPoints->Initialize();
		voxelPoints->Initialize();

		//计算当前交互的Renderer
		clickPos = this->GetInteractor()->GetEventPosition();
		this->FindPokedRenderer(clickPos[0], clickPos[1]);

		cr = 3;
		//获取当前Renderer序号，并禁止其他Renderer交互
		for (int i = 0; i < 4; i++) {
			ren[i]->RemoveActor(polyLineActor);
			ren[i]->RemoveActor(fitCurveActor);
			if (ren[i] == CurrentRenderer) {
				cr = i;
				continue;
			}
			ren[i]->InteractiveOff();
		}

		//若在ren[3]上画线，则直接返回
		if (cr == 3) {
			return false;
		}

		data->GetDimensions(dims);
		data->GetSpacing(spacing);

		if (data != NULL && imgActor[cr] != NULL) {
			return true;
		}
		else {
			cerr << "error" << std::endl;
			return false;
		}
	}

	void InsertPoint() {
		//获取事件发生点在display坐标系下的坐标,左下为(0,0)
		clickPos = this->GetInteractor()->GetEventPosition();

		//display坐标系转换为世界坐标系
		CurrentRenderer->SetDisplayPoint(clickPos[0], clickPos[1], 0);
		CurrentRenderer->DisplayToWorld();

		double *pos = CurrentRenderer->GetWorldPoint();
		//qDebug() << "display points:" << pos[0] << " " << pos[1] << " " << pos[2] << endl;
		displayPoints->InsertNextPoint(pos);
		displayPoints->Modified();

		//display坐标系转换为model坐标系
		pointPicker->Pick(clickPos[0], clickPos[1], 0, CurrentRenderer);
		pos = pointPicker->GetPickPosition();

		//计算像素点的索引
		if (cr == 0) {
			std::swap(pos[0], pos[1]);
			std::swap(pos[0], pos[2]);
		}
		else if (cr == 1) {
			std::swap(pos[0], pos[1]);
			std::swap(pos[1], pos[2]);
		}
		double mp[3] = { pos[0] ,pos[1], pos[2] };
		mp[cr] = 0;
		voxelPoints->InsertNextPoint(mp);
		voxelPoints->Modified();
	}

	void computeWindowLevel() {
		int *size = this->CurrentRenderer->GetSize();

		double window = this->WindowLevelInitial[0];
		double level = this->WindowLevelInitial[1];
		// Compute normalized delta

		double dx = (this->WindowLevelCurrentPosition[0] -
			this->WindowLevelStartPosition[0]) * 4.0 / size[0];
		double dy = (this->WindowLevelStartPosition[1] -
			this->WindowLevelCurrentPosition[1]) * 4.0 / size[1];

		// Scale by current values

		if (fabs(window) > 0.01)
		{
			dx = dx * window;
		}
		else
		{
			dx = dx * (window < 0 ? -0.01 : 0.01);
		}
		if (fabs(level) > 0.01)
		{
			dy = dy * level;
		}
		else
		{
			dy = dy * (level < 0 ? -0.01 : 0.01);
		}

		// Abs so that direction does not flip

		if (window < 0.0)
		{
			dx = -1 * dx;
		}
		if (level < 0.0)
		{
			dy = -1 * dy;
		}

		// Compute new window level

		wl[0] = dx + window;
		wl[1] = level - dy;

		if (wl[0] < 0.01)
		{
			wl[0] = 0.01;
		}

		view2D->sendWLSignal(wl);
	}

	void showLine(vtkSmartPointer<vtkAlgorithmOutput> outputPort, vtkSmartPointer<vtkPolyDataMapper> mapper,
		vtkSmartPointer<vtkActor> actor, double* color) {
		mapper->SetInputConnection(outputPort);
		mapper->Modified();
		mapper->Update();

		actor->SetMapper(mapper);
		actor->GetProperty()->SetColor(color);
		actor->Modified();

		CurrentRenderer->AddActor(actor);
	}

	//最小二乘法拟合二次曲线,type:0表示拟合显示曲线/1表示拟合模型中的曲线
	//b表示拟合二次曲线的参数，x表示旋转后的原始数据的x轴最小值和最大值，r={cos(rad),sin(rad)}表示旋转角度
	void fitConicCurve(vtkSmartPointer<vtkPoints> points, vtkSmartPointer<vtkLineSource> conicSource,
		int type, double b[] = NULL, double x[] = NULL, double r[] = NULL) {
		int rxmin, rxmax, n = points->GetNumberOfPoints();
		double rb0, rb1, rb2, rA, rB, minE = 1e308;

		//预设旋转角度分别拟合，取最小均方误差对应的旋转角度
		for (int ang = 0; ang < 180; ang += 15) {
			double rad = ang * PI / 180;
			double A = cos(rad), B = sin(rad);

			double x1 = 0, x2 = 0, x3 = 0, x4 = 0, y1 = 0, xy = 0, x2y = 0;
			int xmin = 10000001, xmax = -10000000;
			for (int i = 0; i < n; i++) {
				double* p = points->GetPoint(i);
				double cx = 0, cx2 = 0, cy = 0;;
				if (type == 0 || cr == 2)
					cx = p[0], cy = p[1];
				else if (cr == 0)
					cx = p[1], cy = p[2];
				else
					cx = p[0], cy = p[2];

				double tx = cx, ty = cy;
				cx = A * tx - B * ty, cy = B * tx + A * ty;
				cx2 = cx * cx;

				xmin = xmin < cx ? xmin : cx;
				xmax = xmax > cx ? xmax : cx;
				x1 += cx;
				x2 += cx2;
				x3 += cx * cx2;
				x4 += cx2 * cx2;
				y1 += cy;
				xy += cx * cy;
				x2y += cx2 * cy;
			}
			double divisor = x4 * x1*x1 - 2 * x1*x2*x3 + x2 * x2*x2 - n * x4*x2 + n * x3*x3;
			divisor = divisor == 0 ? 1 : divisor;
			double b0 = (x2y*x2 *x2 - xy * x2*x3 - x4 * y1*x2 + y1 * x3 *x3 - x1 * x2y*x3 + x1 * x4*xy) / divisor,
				b1 = (x2 *x2 * xy - n * x4*xy + n * x3*x2y - x1 * x2*x2y + x1 * x4*y1 - x2 * x3*y1) / divisor,
				b2 = (x2y*x1 *x1 - xy * x1*x2 - x3 * y1*x1 + y1 * x2 * x2 - n * x2y*x2 + n * x3*xy) / divisor;

			double tempE = 0;
			for (int i = 0; i < n; i++) {
				double* p = points->GetPoint(i);
				double cx = 0, cx2 = 0, cy = 0;;
				if (type == 0 || cr == 2)
					cx = p[0], cy = p[1];
				else if (cr == 0)
					cx = p[1], cy = p[2];
				else
					cx = p[0], cy = p[2];

				double tx = cx, ty = cy;
				cx = A * tx - B * ty, cy = B * tx + A * ty;

				double dy = b0 + cx * b1 + cx * cx * b2 - cy;
				tempE += dy * dy;
			}

			if (tempE < minE) {
				minE = tempE;
				rb0 = b0, rb1 = b1, rb2 = b2;
				rxmin = xmin, rxmax = xmax;
				rA = A, rB = B;
			}
		}
		double z = points->GetPoint(0)[2], dx = 1.0*(rxmax - rxmin) / n;
		double cx = rxmin;
		if (b != NULL)
			b[0] = rb0, b[1] = rb1, b[2] = rb2;
		if (x != NULL)
			x[0] = rxmin, x[1] = rxmax;
		if (r != NULL)
			r[0] = rA, r[1] = rB;

		vtkSmartPointer<vtkPoints> pt2 = vtkSmartPointer<vtkPoints>::New();

		if (type == 1 && dx > 1)
			dx = 1;

		//计算拟合曲线上的点
		while (cx <= rxmax) {
			double cy = rb0 + cx * rb1 + cx * cx*rb2;
			double tx = rA * cx + rB * cy, ty = rA * cy - rB * cx;
			if (type == 1) {
				if (cr == 0)
					pt2->InsertNextPoint(0, tx, ty);
				else if (cr == 1)
					pt2->InsertNextPoint(tx, 0, ty);
				else
					pt2->InsertNextPoint(tx, ty, 0);
			}
			else {
				pt2->InsertNextPoint(tx, ty, z);
			}
			cx += dx;
		}

		conicSource->SetPoints(pt2);
		conicSource->Update();
	}

	//拟合B样条曲线
	void fitBSplineCurve(vtkSmartPointer<vtkPoints> points, vtkSmartPointer<vtkParametricFunctionSource> fS) {
		vtkSmartPointer<vtkParametricSpline> pS = vtkSmartPointer<vtkParametricSpline>::New();
		pS->SetPoints(points);

		int n = points->GetNumberOfPoints();
		fS->SetUResolution(n);
		fS->SetVResolution(n);
		fS->SetWResolution(n);
		fS->SetParametricFunction(pS);
		fS->Update();
	}

	//direction - 扫描轨迹，即边界线
	//distance - 体素在扫描轨迹方向的最大值
	vtkSmartPointer<vtkPoints> sweepLine(vtkSmartPointer<vtkPolyData> line, double direction[3], double distance,
		unsigned int cols)
	{
		unsigned int rows = line->GetNumberOfPoints();

		// Generate the points
		cols++;
		unsigned int numberOfPoints = rows * cols;
		vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
		points->Allocate(numberOfPoints);

		double x[3];
		unsigned int cnt = 0;
		for (unsigned int row = 0; row < rows; row++)
		{
			double p[3];
			line->GetPoint(row, p);
			for (unsigned int col = 0; col < cols; col++)
			{
				x[0] = p[0] + direction[0] * col;
				x[1] = p[1] + direction[1] * col;
				x[2] = p[2] + direction[2] * col;
				points->InsertPoint(cnt++, x);
			}
		}

		return points;
	}

	double getLength(vtkSmartPointer<vtkPoints> tpoints) {
		int npts = tpoints->GetNumberOfPoints();

		if (npts < 2) { return 0.0; }

		double a[3];
		double b[3];
		double sum = 0.0;
		int i = 0;
		tpoints->GetPoint(i, a);
		int imax = (npts % 2 == 0) ? npts - 2 : npts - 1;

		while (i < imax)
		{
			tpoints->GetPoint(i + 1, b);
			sum += sqrt(vtkMath::Distance2BetweenPoints(a, b));
			i = i + 2;
			tpoints->GetPoint(i, a);
			sum = sum + sqrt(vtkMath::Distance2BetweenPoints(a, b));
		}

		if (npts % 2 == 0)
		{
			tpoints->GetPoint(i + 1, b);
			sum += sqrt(vtkMath::Distance2BetweenPoints(a, b));
		}
		return sum;
	}

	double * GetPointValue(double point[3], InterpolationMethod method = InterpolationMethod::Linear)
	{
		int n = data->GetNumberOfScalarComponents();
		double *outValue = new double[n];

		int ix = point[0], iy = point[1], iz = point[2];
		//若插值点为原数据中存在的一个体素，则直接返回该体素值
		if (ix == point[0] && iy == point[1] && iz == point[2]) {
			unsigned char *value = (unsigned char *)data->GetScalarPointer(ix, iy, iz);
			if (value == NULL) {
				for (int i = 0; i < n; i++) {
					outValue[i] = 0;
				}
			}
			else {
				for (int i = 0; i < n; i++) {
					outValue[i] = *(value + i);
				}
			}
			return outValue;
		}

		switch (method) {
		case InterpolationMethod::NearestNeighbor: {
			interpolator->SetInterpolationModeToNearest();
			break;
		}
		case InterpolationMethod::Linear: {
			interpolator->SetInterpolationModeToLinear();
			break;
		}
		case InterpolationMethod::Cubic: {
			interpolator->SetInterpolationModeToCubic();
			break;
		}
		default:
			cerr << "unknown Interpolation method";
			break;
		}
		interpolator->Update();
		interpolator->Interpolate(point, outValue);
		return outValue;
	}

	vtkSmartPointer<vtkImageData> GetImageData(int x, int y, vtkSmartPointer<vtkPoints> points, InterpolationMethod method = InterpolationMethod::Linear)
	{
		vtkSmartPointer<vtkImageData> surface = vtkSmartPointer<vtkImageData>::New();
		surface->SetDimensions(x, y, 1);

		vtkSmartPointer<vtkDoubleArray> dataarr = vtkSmartPointer<vtkDoubleArray>::New();
		int number = points->GetNumberOfPoints();

		interpolator->Initialize(data);
		for (int i = 0; i < number; i++) {
			double *p = points->GetPoint(i);
			double *v = GetPointValue(p, method);
			dataarr->InsertComponent(i, 0, v[0]);
			delete v;
		}

		surface->GetPointData()->SetScalars(dataarr);
		surface->Modified();
		return surface;
	}

public:
	View2D *view2D;
	vtkSmartPointer<vtkImageData> data = NULL;//原始数据
	vtkSmartPointer<vtkRenderer> ren[4];
	vtkSmartPointer<vtkImageActor> imgActor[4] = { 0,0,0,0 };

	InterpolationMethod interpolationMethod = InterpolationMethod::Linear;
	CurveFitMethod curveFitMethod = CurveFitMethod::CardinalSpline;

private:
	int *clickPos = 0;
	int cr = 3;//当前交互renderer的序号
	int dims[3];//维数
	double spacing[3];//间隔
	double normal[3][3] = { {1,0,0}, {0,1,0}, {0,0,1} };
	bool changeWindowLevel = false;
	double wl[2] = { 0,0 };

	vtkSmartPointer<vtkImageInterpolator> interpolator = vtkSmartPointer<vtkImageInterpolator>::New();
	vtkSmartPointer<vtkPointPicker> pointPicker = vtkSmartPointer<vtkPointPicker>::New();

	vtkSmartPointer<vtkPoints> displayPoints = vtkSmartPointer<vtkPoints>::New();
	vtkSmartPointer<vtkPoints> voxelPoints = vtkSmartPointer<vtkPoints>::New();

	vtkSmartPointer<vtkPolyDataMapper> polyLineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	vtkSmartPointer<vtkPolyDataMapper> fitCurveMapper = vtkSmartPointer<vtkPolyDataMapper>::New();

	vtkSmartPointer<vtkLineSource> lineSource = vtkSmartPointer<vtkLineSource>::New();
	vtkSmartPointer<vtkParametricFunctionSource> functionSource = vtkSmartPointer<vtkParametricFunctionSource>::New();

	vtkSmartPointer<vtkActor> polyLineActor = vtkSmartPointer<vtkActor>::New();
	vtkSmartPointer<vtkActor> fitCurveActor = vtkSmartPointer<vtkActor>::New();
};
