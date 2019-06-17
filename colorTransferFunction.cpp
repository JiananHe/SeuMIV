#include "colorTransferFunction.h"

ColorTransferFunction::ColorTransferFunction(QWidget * color_tf_widget)
{
	tf_widgets = color_tf_widget;

	tf_diagram = tf_widgets->findChild<QWidget *>("colorTfBar");

	tf_leftBp_button = tf_widgets->findChild<QToolButton*>("colortf_left_button");
	tf_rightBp_button = tf_widgets->findChild<QToolButton*>("colortf_right_button");
	cur_bpIdx_label = tf_widgets->findChild<QLabel*>("colortf_curbp_idx_label");
	cur_bpValue_label = tf_widgets->findChild<QLabel*>("colortf_curbp_color_label");
	cur_bpKey_label = tf_widgets->findChild<QLabel*>("colortf_curbp_x_label");

	min_key_label = tf_widgets->findChild<QLabel*>("colortf_x_min");
	max_key_label = tf_widgets->findChild<QLabel*>("colortf_x_max");
	tf_x_slider = tf_widgets->findChild<RangeSlider*>("colortf_x_slider");
}

ColorTransferFunction::~ColorTransferFunction()
{
}

void ColorTransferFunction::setBoneColorTf(vtkColorTransferFunction * volumeColor)
{
	//bone
	int max_point = 3071 > max_key ? max_key : 3071;
	int min_point = -3024 < min_key ? min_key : -3024;

	tf_bps->removeAllPoints();
	tf_bps->insertBreakPoint(min_point, MyQColor(QColor(0, 0, 0)));
	tf_bps->insertBreakPoint(-16, MyQColor(QColor(186, 64, 77)));
	tf_bps->insertBreakPoint(641, MyQColor(QColor(230, 209, 143)));
	tf_bps->insertBreakPoint(max_point, MyQColor(QColor(255, 255, 255)));

	updateVolumeColor(volumeColor);

	//show info of the first color bp 
	cur_bp_idx = 0;
	showTfBpInfoAt(0);
}

void ColorTransferFunction::setBone2ColorTf(vtkColorTransferFunction* volumeColor)
{
	//bone2
	int max_point = 3071 > max_key ? max_key : 3071;
	int min_point = -3024 < min_key ? min_key : -3024;

	tf_bps->removeAllPoints();
	tf_bps->insertBreakPoint(min_point, MyQColor(QColor(0, 0, 0)));
	tf_bps->insertBreakPoint(144, MyQColor(QColor(157, 91, 47)));
	tf_bps->insertBreakPoint(166, MyQColor(QColor(255, 154, 74)));
	tf_bps->insertBreakPoint(214, MyQColor(QColor(255, 255, 255)));
	tf_bps->insertBreakPoint(420, MyQColor(QColor(255, 239, 244)));
	tf_bps->insertBreakPoint(max_point, MyQColor(QColor(211, 168, 255)));

	updateVolumeColor(volumeColor);

	//show info of the first color bp 
	cur_bp_idx = 0;
	showTfBpInfoAt(0);
}

void ColorTransferFunction::setSkinColorTf(vtkColorTransferFunction * volumeColor)
{
	//skin
	int max_point = 3071 > max_key ? max_key : 3071;
	int min_point = -3024 < min_key ? min_key : -3024;

	tf_bps->removeAllPoints();
	tf_bps->insertBreakPoint(min_point, MyQColor(QColor(0, 0, 0)));
	tf_bps->insertBreakPoint(-1000, MyQColor(QColor(158, 92, 46)));
	tf_bps->insertBreakPoint(-500, MyQColor(QColor(224, 153, 74)));
	tf_bps->insertBreakPoint(max_point, MyQColor(QColor(212, 168, 255)));

	updateVolumeColor(volumeColor);

	//show info of the first color bp 
	cur_bp_idx = 0;
	showTfBpInfoAt(0);
}

void ColorTransferFunction::setMuscleColorTf(vtkColorTransferFunction * volumeColor)
{
	//muscle
	int max_point = 3071 > max_key ? max_key : 3071;
	int min_point = -3024 < min_key ? min_key : -3024;

	tf_bps->removeAllPoints();
	tf_bps->insertBreakPoint(min_point, MyQColor(QColor(0, 0, 0)));
	tf_bps->insertBreakPoint(-155, MyQColor(QColor(140, 64, 38)));
	tf_bps->insertBreakPoint(420, MyQColor(QColor(255, 240, 242)));
	tf_bps->insertBreakPoint(217, MyQColor(QColor(224, 153, 74)));
	tf_bps->insertBreakPoint(max_point, MyQColor(QColor(212, 168, 255)));

	updateVolumeColor(volumeColor);

	//show info of the first color bp 
	cur_bp_idx = 0;
	showTfBpInfoAt(0);
}

void ColorTransferFunction::updateVolumeColor(vtkColorTransferFunction * volumeColor)
{
	volumeColor->RemoveAllPoints();
	map<double, MyQColor> cur_colortf_bps = tf_bps->getBreakPointsMap();
	map<double, MyQColor>::iterator iter;

	for (iter = cur_colortf_bps.begin(); iter != cur_colortf_bps.end(); ++iter)
	{
		volumeColor->AddRGBPoint(iter->first, (iter->second).red() / 255.0, (iter->second).green() / 255.0, (iter->second).blue() / 255.0);
	}
}

void ColorTransferFunction::showTfDiagram()
{
	if (tf_bps->getMapLength() == 0)
		return;

	QPainter painter(tf_diagram);
	painter.setRenderHint(QPainter::Antialiasing, true);

	d = 15;
	w = tf_diagram->geometry().width();
	h = tf_diagram->geometry().height();
	double scale = 1.0 / (max_range - min_range);

	//绘制颜色传递函数的颜色条，需要插值计算min_range与max_range处的颜色
	map<double, MyQColor> colortf_bps = tf_bps->getBreakPointsMap();
	map<double, MyQColor>::iterator iter;
	QLinearGradient linearGradient(d, h / 2, w - d, h / 2);
	
	int n = colortf_bps.size(), t = 0;
	double* points = new double[n];

	for (iter = colortf_bps.begin(); iter != colortf_bps.end(); ++iter)
	{
		double gray_value = iter->first;
		QColor color = iter->second;

		double point = (gray_value - min_range) * scale;
		points[t++] = point;
		if (point > 0 && point < 1)
			linearGradient.setColorAt(point, color);
	}
	linearGradient.setColorAt(0, tf_bps->interpolateValue(min_range));
	linearGradient.setColorAt(1, tf_bps->interpolateValue(max_range));
	painter.setBrush(QBrush(linearGradient));
	painter.drawRect(d, 0, w - 2 * d, h);
	
	//绘制颜色传递函数断点
	for (int i = 0; i < t; ++i)
	{
		double centre_x = (w - 2 * d)*points[i] + d;
		double centre_y = h / 2;
		QRadialGradient radialGradiant(centre_x, centre_y, d / 2, centre_x + d / 4, centre_y + d / 4);
		radialGradiant.setColorAt(0, Qt::white);
		radialGradiant.setColorAt(1, Qt::darkGray);
		painter.setBrush(radialGradiant);

		//the first and second params is the left top coordiantes of ellipse
		painter.drawEllipse(centre_x - d / 2, centre_y - d / 2, d, d);
	}

	//绘制当前断点
	double cur_bp_gv = tf_bps->getBpKeyAt(cur_bp_idx, 0);
	if (cur_bp_gv >= min_range && cur_bp_gv <= max_range)
	{
		double cur_point = (cur_bp_gv - min_range) * scale;

		int out_cr = 2;
		painter.setPen(QPen(Qt::darkBlue, 2 * out_cr));
		painter.setBrush(Qt::NoBrush);
		painter.drawEllipse((w - 2 * d)*cur_point + d - d / 2 - out_cr, h / 2 - d / 2 - out_cr, d + 2 * out_cr, d + 2 * out_cr);
	}
}

void ColorTransferFunction::showCurBpValue()
{
	if (tf_bps->getMapLength() == 0)
		return;

	QPainter painter(cur_bpValue_label);
	painter.setRenderHint(QPainter::Antialiasing, true);

	int lw = cur_bpValue_label->geometry().width();
	int lh = cur_bpValue_label->geometry().height();
	int l = 20; 

	QColor idx_color = getCurBpValue();
	painter.setBrush(QBrush(QColor(idx_color)));
	painter.drawRect((lw - l) / 2, (lh - l) / 2, l, l);
}