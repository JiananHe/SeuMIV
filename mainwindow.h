#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QDir>
#include <qstringlist.h>
#include <qmenu.h>
#include <qaction.h>
#include <qfiledialog.h>
#include <qcolordialog.h>
#include <qmessagebox.h>
#include <iostream>
#include <vector>

#include "ui_mainwindow.h"
#include "volumenamedialog.h"
#include "volumeRenderProcess.h"
#include "colorTransferFunction.h"
#include "opacityTransferFunction.h"
using namespace std;

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

	int cur_volume_id;
	vector<bool> volume_flags;
	QStringList volume_names;
	bool multi_render_flag;

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

	void onAddVolumeSlot();
	void onDeleteVolumeSlot();
	void onRenameVolumeSlot();
	void onShowAllVolumesSlot();
	void onClearAllVolumesSlot();
	void onCurVolumeChangedSlot(int);
	void onCurVolumeFlagSlot(bool);
};

#endif // MAINWINDOW_H
