#pragma once
#include <tuple>
#include <qwidget.h>
#include <qlabel.h>
#include <qscrollbar.h>
#include <qtextedit.h>
#include <QPainter>
#include <QMouseEvent>
#include <qtoolbutton.h>
#include <map>
#include "breakPoints.h"
#include "RangeSlider.h"

template<typename T>
class TransferFunction
{
public:
	TransferFunction();
	~TransferFunction();

	void setMinKey(double key);
	void setMaxKey(double key);
	void setminRange(int);
	void setmaxRange(int);

	T getCurBpValue();
	int getCurBpIdx();
	map<double, T> getTfBpsMap();

	tuple<int, int> getCurBpBorder();
	void changeCurBpValue(T new_value);
	void changeCurBpKey(int coord);
	void changeCurBpKeyByKeyboard(int flag);

	void chooseOrAddBpAt(int coord);
	bool deleteCurTfBp();

	void showTfBpInfoAt(int idx);
	virtual void showTfDiagram() = 0;
	virtual void showCurBpValue() = 0;

protected:
	BreakPoints<T>* tf_bps;
	int min_key;
	int max_key;
	int min_range;
	int max_range;
	int cur_bp_idx;

	QWidget* tf_widgets;
	QWidget* tf_diagram;
	QLabel* min_key_label;
	QLabel* max_key_label;
	QLabel* cur_bpIdx_label;
	QLabel* cur_bpKey_label;
	QLabel* cur_bpValue_label;
	RangeSlider* tf_x_slider;
	QToolButton* tf_leftBp_button;
	QToolButton* tf_rightBp_button;

protected:
	int d; //the diameter of cirlces in breakpoints
	int w;
	int h;
public:
	int getD();
};

template<typename T>
TransferFunction<T>::TransferFunction()
{
	tf_bps = new BreakPoints<T>();
}

template<typename T>
TransferFunction<T>::~TransferFunction()
{
}

template<typename T>
inline void TransferFunction<T>::setMinKey(double key)
{
	min_key = int(key - 0.5);
	min_range = min_key;
	min_key_label->setText(QString::number(min_key));
	tf_x_slider->setMinimum(min_key);
}

template<typename T>
inline void TransferFunction<T>::setMaxKey(double key)
{
	max_key = int(key + 0.5);
	max_range = max_key;
	max_key_label->setText(QString::number(max_key));
	tf_x_slider->setMaximum(max_key);
}

template<typename T>
inline void TransferFunction<T>::setminRange(int m)
{
	min_range = m;
}

template<typename T>
inline void TransferFunction<T>::setmaxRange(int m)
{
	max_range = m;
}

template<typename T>
inline T TransferFunction<T>::getCurBpValue()
{
	return tf_bps->getBpValueAt(cur_bp_idx);
}

template<typename T>
inline int TransferFunction<T>::getCurBpIdx()
{
	return cur_bp_idx;
}

template<typename T>
inline map<double, T> TransferFunction<T>::getTfBpsMap()
{
	return tf_bps->getBreakPointsMap();

}

//return the left and right bp coordinates
template<typename T>
inline tuple<int, int> TransferFunction<T>::getCurBpBorder()
{
	double left_key = tf_bps->getBpKeyAt(cur_bp_idx, -1);
	if (left_key == tf_bps->getBpKeyAt(0, 0))
		left_key = min_key;

	double right_key = tf_bps->getBpKeyAt(cur_bp_idx, 1);
	if (right_key == tf_bps->getBpKeyAt(tf_bps->getMapLength() - 1, 0))
		right_key = max_key;

	int left_border, right_border;
	if (left_key < min_range)
		left_border = d;
	else
		left_border = int((left_key - min_range) / (max_range - min_range) * (w - 2 * d) + d + 0.5);

	if (right_key > max_range)
		right_border = w - d;
	else
		right_border = int((right_key - min_range) / (max_range - min_range) * (w - 2 * d) + d + 0.5);
	
	return make_tuple(left_border, right_border);
}

template<typename T>
inline void TransferFunction<T>::changeCurBpValue(T new_value)
{
	tf_bps->changeBpValueAt(cur_bp_idx, new_value);
	showTfBpInfoAt(cur_bp_idx);
}

//change an exists bp key, used this function when user move a bp
template<typename T>
inline void TransferFunction<T>::changeCurBpKey(int coord)
{
	//不能改变最小和最大断点的key
	if (cur_bp_idx == 0 || cur_bp_idx == tf_bps->getMapLength() - 1)
		return;

	int tf_coord = coord - d;
	int key_move = int((tf_coord / (double)(w - 2 * d)) * (max_range - min_range) + min_range + 0.5);

	int flag = tf_bps->findElementInApprox(key_move, 1);
	if (flag == -1)
	{
		T old_value = tf_bps->getBpValueAt(cur_bp_idx);
		tf_bps->deleteBpAt(cur_bp_idx);
		tf_bps->insertBreakPoint(key_move, old_value);

		showTfBpInfoAt(cur_bp_idx);
	}
}

//change current bp key according to flag: left(-1) or right(1)
template<typename T>
inline void TransferFunction<T>::changeCurBpKeyByKeyboard(int flag)
{
	//不能改变最小和最大断点的key
	if (cur_bp_idx == 0 || cur_bp_idx == tf_bps->getMapLength() - 1)
		return;

	double cur_key = tf_bps->getBpKeyAt(cur_bp_idx, 0);
	int move_gap = int((max_range - min_range) / (double)(w - 2 * d) + 0.5);
	if (flag == -1)
	{
		if (cur_key - move_gap >= min_range)
		{
			T old_value = tf_bps->getBpValueAt(cur_bp_idx);

			double left_key = tf_bps->getBpKeyAt(cur_bp_idx, -1);
			double new_key = cur_key - move_gap > left_key ? cur_key - move_gap : left_key + 1;

			tf_bps->deleteBpAt(cur_bp_idx);
			tf_bps->insertBreakPoint(new_key, old_value);
			showTfBpInfoAt(cur_bp_idx);
		}
	}
	if (flag == 1)
	{
		if (cur_key + move_gap <= max_range)
		{
			T old_value = tf_bps->getBpValueAt(cur_bp_idx);

			double right_key = tf_bps->getBpKeyAt(cur_bp_idx, 1);
			double new_key = cur_key + move_gap < right_key ? cur_key + move_gap : right_key - 1;

			tf_bps->deleteBpAt(cur_bp_idx);
			tf_bps->insertBreakPoint(new_key, old_value);
			showTfBpInfoAt(cur_bp_idx);
		}
	}
}

template<typename T>
inline void TransferFunction<T>::chooseOrAddBpAt(int coord)
{
	int tf_coord = coord - d;
	int key_click = int((tf_coord / (double)(w - 2 * d)) * (max_range - min_range) + min_range + 0.5);
	int key_gap = int((d / (2.0 * (w - 2 * d))) * (max_range - min_range) + 0.5);

	int flag = tf_bps->findElementInApprox(key_click, key_gap);
	if (flag == -1)
	{
		//新增断点
		tf_bps->insertBreakPoint(key_click);
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

template<typename T>
inline bool TransferFunction<T>::deleteCurTfBp()
{
	if (cur_bp_idx == 0 || cur_bp_idx == tf_bps->getMapLength() - 1 || tf_bps->getMapLength() <= 1)
		return false;
	else
	{
		tf_bps->deleteBpAt(cur_bp_idx);
		showTfBpInfoAt(cur_bp_idx);
		return true;
	}
}

template<typename T>
inline void TransferFunction<T>::showTfBpInfoAt(int idx)
{
	if (idx < 0)
		cur_bp_idx = 0;
	else if (idx > tf_bps->getMapLength() - 1)
		cur_bp_idx = tf_bps->getMapLength() - 1;
	else
		cur_bp_idx = idx;

	cur_bpIdx_label->setText(QString::number(cur_bp_idx));
	cur_bpValue_label->repaint();
	cur_bpKey_label->setText(QString::number(tf_bps->getBpKeyAt(cur_bp_idx, 0)));
	tf_diagram->repaint();
}

template<typename T>
inline int TransferFunction<T>::getD()
{
	return d;
}
