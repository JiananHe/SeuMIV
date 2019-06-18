#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QDir>
#include <qmenu.h>
#include <qaction.h>
#include <qfiledialog.h>
#include <qcolordialog.h>
#include <iostream>

#include "ui_mainwindow.h"
#include "volumeRenderProcess.h"
#include "colorTransferFunction.h"
#include "opacityTransferFunction.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
	VolumeRenderProcess * vrProcess;

	ColorTransferFunction * colorTf;
	OpacityTransferFunctioin * opacityTf;
	OpacityTransferFunctioin * gradientTf;

private slots:
	void onView2DSlot();
	void onView3DSlot();

	void onOpenDicomSlot();
	void onOpenNifitSlot();

//3D体绘制相关槽函数
private slots:
	bool eventFilter(QObject *, QEvent *);
	void changeCurTfBpInfo();

	void onColorTfMinRangeChange(int);
	void onColorTfMaxRangeChange(int);
	void onScalarTfMinRangeChange(int);
	void onScalarTfMaxRangeChange(int);
	void onGradientTfMinRangeChange(int);
	void onGradientTfMaxRangeChange(int);
};

#endif // MAINWINDOW_H
