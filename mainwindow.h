#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#pragma once
#include <QMainWindow>
#include <vtkDICOMImageReader.h>
#include <vtkImageData.h>
#include <vtkSmartPointer.h>
#include <QFileDialog.h>
#include <QMessageBox>
#include <QSignalMapper>
#include <QColorDialog>
#include <vector>
#include <string>
#include "View2D.h"
#include "vtkMyStyle.h"
#include <QDebug>

namespace Ui {
	class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = nullptr);
	~MainWindow();

public slots:
	//�л���2D���
	void onView2DSlot();
	//�л���3D���
	void onView3DSlot();
	//(2D)�л��������ؽ�
	void show2DCPR();
	//(2D)�л���ƽ���ؽ�
	void show2DMPR();
	//(2D)�л���NIFTIͼ���ں���ʾ
	void show2DBlend();
	//��С��
	void minimize();
	//��DICOM�ļ�
	void openFile();
	//(2D)�л����ڲ���
	void change2DView(int state);
	//(2D)��NIFTIԭʼͼ��
	void openOriginalFile();
	//(2D)��NIFTI�ָ�ͼ��
	void openSegmentFile();
	//(2D)�ı�NIFTIͼ�����״̬
	void OnChangeNiiFileVisible();
	//(2D)�ı䵱ǰѡ�е�NIFTIͼ��
	void OnClickNiiFilesComboBox(int index);
	//(2D)�ı�CPR��ֵ����
	void OnInterpolationMethodChanged(int index);
	//(2D)�ı�CPR������Ϸ���
	void OnCurveFitMethodChanged(int index);
	//(2D)�ı䵱ǰ��ʾ����Ƭ.1-prev,2-next,3-first,4-last
	void OnChangeSlice(int flag);
	//(2D)���µ�ǰ�Ĵ�����λ
	void OnChangeWindowLevel(double wl0, double wl1);
	//(2D)�ı�ͼ��Ĵ���λ
	void SetWindowLevel();
	//(2D)�Ŵ�
	void OnZoomIn();
	//(2D)��С
	void OnZoomOut();
	//(2D)���õ�ǰҳ��
	void OnReset();
	//(2D)˳ʱ����ת45��
	void OnClockwiseRotate();
	//(2D)��ʱ����ת45��
	void OnContraRotate();
	//(2D)��������
	void OnMeasureDistance();
	//(2D)�����Ƕ�
	void OnMeasureAngle();
	//(2D)�ָ�����-ȡ������״̬
	void OnHand();

private:
	Ui::MainWindow *ui;

	//��ȡ��DICOM�ļ�
	vtkSmartPointer<vtkImageData> data;

	View2D *mpr;
	View2D *cpr;
	View2D *blend;
	//(2D)��ǰ��ʾҳ��
	int current2DState = 1;//1-mpr,2-cpr,3-blend

	//(2D)�Ѵ򿪵�NIFTI�ļ�
	vector<QString> niiFiles;
	//(2D)NIFTI�ļ��Ŀ���״̬
	vector<bool> niiFilesVisbile;
};

#endif // MAINWINDOW_H
