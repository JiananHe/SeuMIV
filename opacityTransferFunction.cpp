#include "opacityTransferFunction.h"

OpacityTransferFunctioin::OpacityTransferFunctioin(QWidget * widget, QString name)
{
	tf_widgets = widget;

	tf_diagram = tf_widgets->findChild<QWidget *>(name + "OpacityTfBar");

	tf_leftBp_button = tf_widgets->findChild<QToolButton*>(name + "tf_left_button");
	tf_rightBp_button = tf_widgets->findChild<QToolButton*>(name + "tf_right_button");
	cur_bpIdx_label = tf_widgets->findChild<QLabel*>(name + "tf_curbp_idx_label");
	cur_bpValue_label = tf_widgets->findChild<QLabel*>(name + "tf_curbp_opacity_label");
	cur_bpKey_label = tf_widgets->findChild<QLabel*>(name + "tf_curbp_x_label");

	min_key_label = tf_widgets->findChild<QLabel*>(name + "tf_x_min");
	max_key_label = tf_widgets->findChild<QLabel*>(name + "tf_x_max");
	tf_x_slider = tf_widgets->findChild<RangeSlider*>(name  + "tf_x_slider");
}

OpacityTransferFunctioin::~OpacityTransferFunctioin()
{
}

void OpacityTransferFunctioin::setInitialOpacityTf(vtkPiecewiseFunction * volumeOpacity)
{
	tf_bps->removeAllPoints();
	tf_bps->insertBreakPoint(min_key, 0.0);
	tf_bps->insertBreakPoint(max_key, 1.0);

	updateVolumeOpacity(volumeOpacity);

	//show info of the first opacity bp
	cur_bp_idx = 0;
	showTfBpInfoAt(0);
}

void OpacityTransferFunctioin::setBoneOpacityTf(vtkPiecewiseFunction * volumeOpacity)
{
	//bone
	int max_point = 3071 > max_key ? max_key : 3071;
	int min_point = -3024 < min_key ? min_key : -3024;

	tf_bps->removeAllPoints();
	tf_bps->insertBreakPoint(min_point, 0.0);
	tf_bps->insertBreakPoint(max_point, .71);

	if (-16 >min_key && -16 < max_key)
		tf_bps->insertBreakPoint(-16, 0.0);
	if (641 > min_key && 641 < max_key)
		tf_bps->insertBreakPoint(641, .72);

	updateVolumeOpacity(volumeOpacity);

	//show info of the first opacity bp
	cur_bp_idx = 0;
	showTfBpInfoAt(0);
}

void OpacityTransferFunctioin::setBone2OpacityTf(vtkPiecewiseFunction * volumeOpacity)
{
	//bone2
	int max_point = 3071 > max_key ? max_key : 3071;
	int min_point = -3024 < min_key ? min_key : -3024;

	tf_bps->removeAllPoints();
	tf_bps->insertBreakPoint(min_point, 0.0);
	tf_bps->insertBreakPoint(max_point, 0.80);

	if (143 > min_key && 143 < max_key)
		tf_bps->insertBreakPoint(143, 0.0);
	if (166 > min_key && 166 < max_key)
		tf_bps->insertBreakPoint(166, 0.69);
	if (214 > min_key && 214 < max_key)
		tf_bps->insertBreakPoint(214, 0.70);
	if (420 > min_key && 420 < max_key)
		tf_bps->insertBreakPoint(420, 0.83);

	updateVolumeOpacity(volumeOpacity);

	//show info of the first opacity bp
	cur_bp_idx = 0;
	showTfBpInfoAt(0);
}

void OpacityTransferFunctioin::setSkinOpacityTf(vtkPiecewiseFunction * volumeOpacity)
{
	//skin
	int max_point = 3071 > max_key ? max_key : 3071;
	int min_point = -3024 < min_key ? min_key : -3024;

	tf_bps->removeAllPoints();
	tf_bps->insertBreakPoint(min_point, 0.0);
	tf_bps->insertBreakPoint(max_point, 1.0);

	if (-1000 > min_key && -1000 < max_key)
		tf_bps->insertBreakPoint(-1000, 0.0);
	if (-500 > min_key && -500 < max_key)
		tf_bps->insertBreakPoint(-500, 1.0);

	updateVolumeOpacity(volumeOpacity);

	//show info of the first opacity bp
	cur_bp_idx = 0;
	showTfBpInfoAt(0);
}

void OpacityTransferFunctioin::setMuscleOpacityTf(vtkPiecewiseFunction * volumeOpacity)
{
	//muscle
	int max_point = 3071 > max_key ? max_key : 3071;
	int min_point = -3024 < min_key ? min_key : -3024;

	tf_bps->removeAllPoints();
	tf_bps->insertBreakPoint(min_point, 0.0);
	tf_bps->insertBreakPoint(max_point, 0.80);

	if (-155 > min_key && -155 < max_key)
		tf_bps->insertBreakPoint(-155, 0.0);
	if (217 > min_key && 217 < max_key)
		tf_bps->insertBreakPoint(217, 0.68);
	if (420 > min_key && 420 < max_key)
		tf_bps->insertBreakPoint(420, 0.83);

	updateVolumeOpacity(volumeOpacity);

	//show info of the first opacity bp
	cur_bp_idx = 0;
	showTfBpInfoAt(0);
}

void OpacityTransferFunctioin::setCustomizedOpacityTf(vtkPiecewiseFunction * volumeOpacity, map<double, double> my_tf_bps)
{
	tf_bps->removeAllPoints();
	map<double, double>::iterator iter;
	for (iter = my_tf_bps.begin(); iter != my_tf_bps.end(); ++iter)
	{
		this->tf_bps->insertBreakPoint(int(iter->first + 0.5), iter->second);
	}
	updateVolumeOpacity(volumeOpacity);

	//show info of the first opacity bp
	cur_bp_idx = 0;
	showTfBpInfoAt(0);
}

void OpacityTransferFunctioin::updateVisualOpacity(vtkPiecewiseFunction * volumeOpacity)
{
	tf_bps->removeAllPoints();
	int bp_num = volumeOpacity->GetSize();
	double node[4];
	for (int i = 0; i < bp_num; i++)
	{
		volumeOpacity->GetNodeValue(i, node);
		tf_bps->insertBreakPoint(node[0], node[1]);
	}

	volumeOpacity->GetNodeValue(0, node);
	setMinKey(int(node[0]));
	volumeOpacity->GetNodeValue(bp_num - 1, node);
	setMaxKey(int(node[0]));
	printf("min key: %d, max key: %d", min_key, max_key);

	//show info of the first color bp 
	cur_bp_idx = 0;
	showTfBpInfoAt(0);
}

void OpacityTransferFunctioin::updateVolumeOpacity(vtkPiecewiseFunction * volumeOpacity)
{
	volumeOpacity->RemoveAllPoints();
	map<double, double> cur_tf_bps = tf_bps->getBreakPointsMap();
	map<double, double>::iterator iter;

	for (iter = cur_tf_bps.begin(); iter != cur_tf_bps.end(); ++iter)
	{
		volumeOpacity->AddPoint(iter->first, iter->second, 0.5, 0);
	}
}

void OpacityTransferFunctioin::showTfDiagram()
{
	if (tf_bps->getMapLength() == 0)
		return;

	QPainter painter(tf_diagram);
	painter.setRenderHint(QPainter::Antialiasing, true);

	d = 11;
	w = tf_diagram->geometry().width();
	h = tf_diagram->geometry().height();
	double scale = 1.0 / (max_range - min_range);

	map<double, double> cur_opacitytf_bps = tf_bps->getBreakPointsMap();
	map<double, double>::iterator iter;

	painter.setPen(QPen(Qt::darkGray));
	painter.drawRect(d, d, w - 2 * d, h - 2 * d);

	//绘制断点
	int n = cur_opacitytf_bps.size(), t = 0, start_flag = 0;
	double prior_centre_x = .0, prior_centre_y = .0;
	map<double, double> bpInRange;
	
	for (iter = cur_opacitytf_bps.begin(); iter != cur_opacitytf_bps.end(); ++iter)
	{
		double gray_value = iter->first;
		if (gray_value < min_range || gray_value > max_range)
			continue;
		else
			bpInRange.insert(pair<double, double>(iter->first, iter->second));


		double ratio_x = (gray_value - min_range) * scale;
		double centre_x = (w - 2 * d)*ratio_x + d;

		double opacity = iter->second;
		double centre_y = opacityToY(opacity);

		//断点
		QRadialGradient radialGradiant(centre_x, centre_y, d / 2, centre_x + d / 4, centre_y + d / 4);
		radialGradiant.setColorAt(0, Qt::white);
		radialGradiant.setColorAt(1, Qt::darkGray);
		painter.setBrush(radialGradiant);

		painter.drawEllipse(centre_x - d / 2, centre_y - d / 2, d, d);

		//连线
		if (start_flag)
		{
			painter.setPen(QPen(Qt::black));
			painter.drawLine(centre_x, centre_y, prior_centre_x, prior_centre_y);
		}
		start_flag = 1;
		prior_centre_x = centre_x;
		prior_centre_y = centre_y;
	}
	//插值计算min_range与max_range处的值
	double min_range_value = opacityToY(tf_bps->interpolateValue(min_range));
	double max_range_value = opacityToY(tf_bps->interpolateValue(max_range));
	if (!start_flag)
	{
		painter.setPen(QPen(Qt::black));
		painter.drawLine(d, min_range_value, w - d, max_range_value);
	}
	else
	{
		painter.setPen(QPen(Qt::black));

		double min_range_upper_x = (w - 2 * d) * (bpInRange.begin()->first - min_range) * scale + d;
		double min_range_upper_y = opacityToY(bpInRange.begin()->second);
		painter.drawLine(d, min_range_value, min_range_upper_x, min_range_upper_y);


		double max_range_lower_x = (w - 2 * d) * (bpInRange.rbegin()->first - min_range) * scale + d;
		double max_range_lower_y = opacityToY(bpInRange.rbegin()->second);
		painter.drawLine(w - d, max_range_value, max_range_lower_x, max_range_lower_y);
	}

	//当前断点
	double cur_bp_gv = tf_bps->getBpKeyAt(cur_bp_idx, 0);
	double cur_point = (cur_bp_gv - min_range) * scale;
	double cur_opacity = tf_bps->getBpValueAt(cur_bp_idx);

	int out_cr = 1;
	painter.setPen(QPen(Qt::darkBlue, 2 * out_cr));
	painter.setBrush(Qt::NoBrush);
	painter.drawEllipse((w - 2 * d)*cur_point + d - d / 2 - out_cr, opacityToY(cur_opacity) - d / 2 - out_cr, d + 2 * out_cr, d + 2 * out_cr);
}

void OpacityTransferFunctioin::showCurBpValue()
{
	if (tf_bps->getMapLength() == 0)
		return;

	double cur_opacity = tf_bps->getBpValueAt(cur_bp_idx);

	cur_bpValue_label->setText(QString::number(cur_opacity, 10, 2));

	QPainter painter(cur_bpValue_label);
	painter.setRenderHint(QPainter::Antialiasing, true);
	painter.setBrush(Qt::lightGray); 
	painter.setOpacity(cur_opacity);
	int lw = cur_bpValue_label->geometry().width();
	int lh = cur_bpValue_label->geometry().height();
	QRect rect(0, 0, lw, lh);
	painter.drawRect(rect);

	QFont font;
	font.setPointSize(10);
	font.setBold(true);
	painter.setPen(QPen(Qt::darkBlue));
	painter.setFont(font);
	painter.setOpacity(1);
	painter.drawText(rect, Qt::AlignCenter, QString::number(cur_opacity, 10, 2));
}

void OpacityTransferFunctioin::changeCurBpValue(int y)
{
	double opacity = YToOpacity(y);
	TransferFunction::changeCurBpValue(opacity);
}

void OpacityTransferFunctioin::chooseOrAddBpAt(int x, int y)
{
	int tf_x = x - d;
	double key_click = (tf_x / (double)(w - 2 * d)) * (max_range - min_range) + min_range;
	double value_click = YToOpacity(y);
	double key_gap = (d / (2.0 * (w - 2 * d))) * (max_range - min_range);

	int flag = tf_bps->findElementInApprox(key_click, key_gap);
	if (flag == -1)
	{
		//新增断点
		tf_bps->insertBreakPoint(key_click, value_click);
		cur_bp_idx = tf_bps->findElementInApprox(key_click, 0.0);
		showTfBpInfoAt(cur_bp_idx);
	}
	else
	{
		//选中断点
		cur_bp_idx = flag;
		showTfBpInfoAt(cur_bp_idx);
	}
}

//change currrent opacity bp opacity according to falg: downward(-1), upward(1)
void OpacityTransferFunctioin::changeCurBpValueByboard(int flag)
{
	double cur_key = tf_bps->getBpKeyAt(cur_bp_idx, 0);
	double cur_value = tf_bps->getBpValueAt(cur_bp_idx);
	double move_gap = 0.1;
	if (flag == -1)
	{
		tf_bps->deleteBpAt(cur_bp_idx);
		tf_bps->insertBreakPoint(cur_key, cur_value - move_gap < 0 ? 0 : cur_value - move_gap);
		showTfBpInfoAt(cur_bp_idx);
	}
	if (flag == 1)
	{
		tf_bps->deleteBpAt(cur_bp_idx);
		tf_bps->insertBreakPoint(cur_key, cur_value + move_gap > 1 ? 1 : cur_value + move_gap);
		showTfBpInfoAt(cur_bp_idx);
	}
}
