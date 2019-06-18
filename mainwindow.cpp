#include "mainwindow.h"
using namespace std;

#define TOP_BUTTON_SELECTED_STYLE QStringLiteral("color: rgb(255, 255, 255);font: 75 10pt \"微软雅黑\";")
#define TOP_BUTTON_UNSELECTED_STYLE QStringLiteral("color: rgb(175, 175, 175);font: 75 10pt \"微软雅黑\";")
//vtkStandardNewMacro(myVtkInteractorStyleImage);
int color_left_border = 10000, color_right_border = 0, opacity_left_border = 10000, opacity_right_border = 0, gradient_left_border = 10000, gradient_right_border = 0;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
	this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);
	ui->stackedWidget->setCurrentIndex(0);

	//页面切换响应
	connect(ui->button2D, SIGNAL(released()), this, SLOT(onView2DSlot()));
	connect(ui->button3D, SIGNAL(released()), this, SLOT(onView3DSlot()));

	//程序最小化以及退出
	connect(ui->exit, SIGNAL(released()), qApp, SLOT(quit()));
	connect(ui->download, SIGNAL(released()), this, SLOT(showMinimized()));

	//为“打开文件按钮”添加菜单，选择DCM或NII
	QMenu *fileMenu = new QMenu();
	QAction *openDicom = new QAction(fileMenu);
	QAction *openNifit = new QAction(fileMenu);
	openDicom->setText(QStringLiteral("打开DICOM文件夹"));
	openNifit->setText(QStringLiteral("打开NIFIT文件"));
	fileMenu->addAction(openDicom);
	fileMenu->addAction(openNifit);
	ui->pushButton->setMenu(fileMenu);

	connect(openDicom, SIGNAL(triggered()), this, SLOT(onOpenDicomSlot()));
	connect(openNifit, SIGNAL(triggered()), this, SLOT(onOpenNifitSlot()));

	//体绘制相关对象
	vrProcess = new VolumeRenderProcess(ui->volumeRenderWidget);
	colorTf = new ColorTransferFunction(ui->colorTfWidget);
	opacityTf = new OpacityTransferFunctioin(ui->scalarOpacityWidget, "scalar");
	gradientTf = new OpacityTransferFunctioin(ui->gradientOpacityWidget, "gradient");

	//传递函数可视化图响应事件
	ui->colorTfBar->installEventFilter(this);
	ui->colortf_curbp_color_label->installEventFilter(this);
	connect(ui->colortf_left_button, SIGNAL(released()), this, SLOT(changeCurTfBpInfo()));
	connect(ui->colortf_right_button, SIGNAL(released()), this, SLOT(changeCurTfBpInfo()));
	connect(ui->colortf_x_slider, SIGNAL(lowerValueChanged(int)), this, SLOT(onColorTfMinRangeChange(int)));
	connect(ui->colortf_x_slider, SIGNAL(upperValueChanged(int)), this, SLOT(onColorTfMaxRangeChange(int)));

	ui->scalarOpacityTfBar->installEventFilter(this);
	ui->scalartf_curbp_opacity_label->installEventFilter(this);
	connect(ui->scalartf_left_button, SIGNAL(released()), this, SLOT(changeCurTfBpInfo()));
	connect(ui->scalartf_right_button, SIGNAL(released()), this, SLOT(changeCurTfBpInfo()));
	connect(ui->scalartf_x_slider, SIGNAL(lowerValueChanged(int)), this, SLOT(onScalarTfMinRangeChange(int)));
	connect(ui->scalartf_x_slider, SIGNAL(upperValueChanged(int)), this, SLOT(onScalarTfMaxRangeChange(int)));

/*	ui->gradientOpacityTfBar->installEventFilter(this);
	ui->gradienttf_curbp_opacity_label->installEventFilter(this);
	connect(ui->gradienttf_left_button, SIGNAL(released()), this, SLOT(changeCurTfBpInfo()));
	connect(ui->gradienttf_right_button, SIGNAL(released()), this, SLOT(changeCurTfBpInfo()));
	connect(ui->gradienttf_x_slider, SIGNAL(lowerValueChanged(int)), this, SLOT(onGradientTfMinRangeChange(int)));
	connect(ui->gradienttf_x_slider, SIGNAL(upperValueChanged(int)), this, SLOT(onGradientTfMaxRangeChange(int)));*/
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onView2DSlot()
{
	ui->stackedWidget->setCurrentIndex(0);
	ui->button2D->setStyleSheet(TOP_BUTTON_SELECTED_STYLE);
	ui->button3D->setStyleSheet(TOP_BUTTON_UNSELECTED_STYLE);
}


void MainWindow::onView3DSlot()
{
	ui->stackedWidget->setCurrentIndex(1);
	ui->button3D->setStyleSheet(TOP_BUTTON_SELECTED_STYLE);
	ui->button2D->setStyleSheet(TOP_BUTTON_UNSELECTED_STYLE);
}

void MainWindow::onOpenDicomSlot()
{
	//打开文件选择框
	QString folder_path = QFileDialog::getExistingDirectory(this, QStringLiteral("打开DICOM文件夹"),
		"C:\\Users\\13249\\Documents\\VTK_Related\\dataset", QFileDialog::ShowDirsOnly);

	//体绘制
	if (folder_path.isEmpty())
		return;

	vrProcess->dicomsVolumeRenderFlow(folder_path);
	vrProcess->update();

	//获取体数据标量最值
	double max_gv = vrProcess->getMaxGrayValue();
	double min_gv = vrProcess->getMinGrayValue();

	colorTf->setMaxKey(max_gv);
	colorTf->setMinKey(min_gv);
	opacityTf->setMaxKey(max_gv);
	opacityTf->setMinKey(min_gv);

	//设置初始传递函数
	colorTf->setBoneColorTf(vrProcess->getVolumeColorTf());
	opacityTf->setBoneOpacityTf(vrProcess->getVolumeOpacityTf());
}

void MainWindow::onOpenNifitSlot()
{
	//打开文件选择框
	QString fileName = QFileDialog::getOpenFileName(this, QStringLiteral("打开NIFIT文件"),
		"C:\\Users\\13249\\Documents\\VTK_Related\\dataset", "NII file (*.nii)");

	//体绘制
	if (fileName.isEmpty())
		return;

	vrProcess->niiVolumeRenderFlow(fileName);
	vrProcess->update();

	//获取体数据标量最值
	double max_gv = vrProcess->getMaxGrayValue();
	double min_gv = vrProcess->getMinGrayValue();

	colorTf->setMaxKey(max_gv);
	colorTf->setMinKey(min_gv);
	opacityTf->setMaxKey(max_gv);
	opacityTf->setMinKey(min_gv);

	//设置初始传递函数
	colorTf->setBoneColorTf(vrProcess->getVolumeColorTf());
	opacityTf->setBoneOpacityTf(vrProcess->getVolumeOpacityTf());
}


//***********************3D体绘制相关槽函数***********************
//3D界面事件过滤器
bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
	//颜色传递函数可视化图
	if (watched == ui->colorTfBar)
	{
		int d = opacityTf->getD();
		int x_max = ui->scalarOpacityTfBar->geometry().width() - d;
		//绘制颜色传递函数
		if (event->type() == QEvent::Paint)
		{
			colorTf->showTfDiagram();
		}
		//选定或新建一个断点
		if (event->type() == QEvent::MouseButtonPress)
		{
			QPoint mp = ui->colorTfBar->mapFromGlobal(QCursor::pos());
			if (mp.x() > d && mp.x() < x_max)
			{
				colorTf->chooseOrAddBpAt(mp.x());
				auto border = colorTf->getCurBpBorder();
				color_left_border = get<0>(border);
				color_right_border = get<1>(border);
			}
		}
		//通过拖动鼠标改变当前断点
		if (event->type() == QEvent::MouseMove)
		{
			int pos_x = ui->colorTfBar->mapFromGlobal(QCursor::pos()).x();
			if (pos_x >= color_left_border && pos_x <= color_right_border)
			{
				colorTf->changeCurBpKey(pos_x);
				colorTf->updateVolumeColor(vrProcess->getVolumeColorTf());
				vrProcess->update();
			}
		}
		if (event->type() == QEvent::KeyPress)
		{
			QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
			//通过键盘改变当前断点
			if (keyEvent->key() == Qt::Key_Left)
			{
				colorTf->changeCurBpKeyByKeyboard(-1);
				colorTf->updateVolumeColor(vrProcess->getVolumeColorTf());
				vrProcess->update();
			}
			if (keyEvent->key() == Qt::Key_Right)
			{
				colorTf->changeCurBpKeyByKeyboard(1);
				colorTf->updateVolumeColor(vrProcess->getVolumeColorTf());
				vrProcess->update();
			}
			//删除选定断点
			if (keyEvent->key() == Qt::Key_Delete)
			{
				if (colorTf->deleteCurTfBp())
				{
					colorTf->updateVolumeColor(vrProcess->getVolumeColorTf());
					vrProcess->update();
				}
			}
		}
		return true;
	}
	//颜色传递函数颜色
	if (watched == ui->colortf_curbp_color_label)
	{
		if (event->type() == QEvent::Paint)
		{//draw the color of checked color tf bp
			colorTf->showCurBpValue();
		}
		else if (event->type() == QEvent::MouseButtonRelease)
		{//change the color of the checked color tf bp
			QColor cur_color = colorTf->getCurBpValue();
			QColor new_color = QColorDialog::getColor(cur_color, this, "select color");
			if (new_color.isValid() && new_color != cur_color)
			{
				colorTf->changeCurBpValue(new_color);
				colorTf->updateVolumeColor(vrProcess->getVolumeColorTf());
				vrProcess->update();
			}
		}
		return true;
	}

	//标量-不透明度传递函数可视化图
	if (watched == ui->scalarOpacityTfBar)
	{
		int d = opacityTf->getD();
		int x_max = ui->scalarOpacityTfBar->geometry().width() - d;
		int y_max = ui->scalarOpacityTfBar->geometry().height() - d;
		//绘制标量-不透明度传递函数
		if (event->type() == QEvent::Paint)
		{
			opacityTf->showTfDiagram();
		}
		if (event->type() == QEvent::MouseButtonPress)
		{
			//选定或新建一个断点
			QPoint mp = ui->scalarOpacityTfBar->mapFromGlobal(QCursor::pos());
			if (mp.x() > d && mp.x() < x_max && mp.y() > d && mp.y() < y_max)
			{
				opacityTf->chooseOrAddBpAt(mp.x(), mp.y());
				auto border = opacityTf->getCurBpBorder();
				opacity_left_border = get<0>(border);
				opacity_right_border = get<1>(border);
			}
		}
		//通过拖动鼠标改变当前断点
		if (event->type() == QEvent::MouseMove)
		{
			QPoint m_pos = ui->scalarOpacityTfBar->mapFromGlobal(QCursor::pos());
			int pos_x = m_pos.x();
			int pos_y = m_pos.y();
			if (pos_x >= opacity_left_border && pos_x <= opacity_right_border && pos_y >= d && pos_y <= y_max)
			{
				opacityTf->changeCurBpKey(pos_x);
				opacityTf->changeCurBpValue(pos_y);
				opacityTf->updateVolumeOpacity(vrProcess->getVolumeOpacityTf());
				vrProcess->update();
			}
		}
		//通过键盘改变当前断点
		if (event->type() == QEvent::KeyPress)
		{
			QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
			//删除选中断点
			if (keyEvent->key() == Qt::Key_Delete)
			{
				if (opacityTf->deleteCurTfBp())
				{
					opacityTf->updateVolumeOpacity(vrProcess->getVolumeOpacityTf());
					vrProcess->update();
				}
			}
			//改变当前断点
			if (keyEvent->key() == Qt::Key_Left)
			{
				opacityTf->changeCurBpKeyByKeyboard(-1);
				opacityTf->updateVolumeOpacity(vrProcess->getVolumeOpacityTf());
				vrProcess->update();
			}
			if (keyEvent->key() == Qt::Key_Right)
			{
				opacityTf->changeCurBpKeyByKeyboard(1);
				opacityTf->updateVolumeOpacity(vrProcess->getVolumeOpacityTf());
				vrProcess->update();
			}
			if (keyEvent->key() == Qt::Key_Down)
			{
				opacityTf->changeCurBpValueByboard(-1);
				opacityTf->updateVolumeOpacity(vrProcess->getVolumeOpacityTf());
				vrProcess->update();
			}
			if (keyEvent->key() == Qt::Key_Up)
			{
				opacityTf->changeCurBpValueByboard(1);
				opacityTf->updateVolumeOpacity(vrProcess->getVolumeOpacityTf());
				vrProcess->update();
			}
		}
		return true;
	}
	//标量-不透明度传递函数不透明度
	if (watched == ui->scalartf_curbp_opacity_label)
	{
		if (event->type() == QEvent::Paint)
		{
			opacityTf->showCurBpValue();
		}
		return true;
	}

	//梯度-不透明度传递函数可视化图
	if (watched == ui->gradientOpacityTfBar)
	{
		int d = gradientTf->getD();
		int x_max = ui->gradientOpacityTfBar->geometry().width() - d;
		int y_max = ui->gradientOpacityTfBar->geometry().height() - d;
		//绘制标量-不透明度传递函数
		if (event->type() == QEvent::Paint)
		{
			gradientTf->showTfDiagram();
		}
		if (event->type() == QEvent::MouseButtonPress)
		{
			//选定或新建一个断点
			QPoint mp = ui->gradientOpacityTfBar->mapFromGlobal(QCursor::pos());
			if (mp.x() > d && mp.x() < x_max && mp.y() > d && mp.y() < y_max)
			{
				gradientTf->chooseOrAddBpAt(mp.x(), mp.y());
				auto border = gradientTf->getCurBpBorder();
				gradient_left_border = get<0>(border);
				gradient_right_border = get<1>(border);
			}
		}
		//通过拖动鼠标改变当前断点
		if (event->type() == QEvent::MouseMove)
		{
			QPoint m_pos = ui->gradientOpacityTfBar->mapFromGlobal(QCursor::pos());
			int pos_x = m_pos.x();
			int pos_y = m_pos.y();
			if (pos_x >= gradient_left_border && pos_x <= gradient_right_border && pos_y >= d && pos_y <= y_max)
			{
				gradientTf->changeCurBpKey(pos_x);
				gradientTf->changeCurBpValue(pos_y);
				gradientTf->updateVolumeOpacity(vrProcess->getVolumeGradientTf());
				vrProcess->update();
			}
		}
		//通过键盘改变当前断点
		if (event->type() == QEvent::KeyPress)
		{
			QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
			//删除选中断点
			if (keyEvent->key() == Qt::Key_Delete)
			{
				if (gradientTf->deleteCurTfBp())
				{
					gradientTf->updateVolumeOpacity(vrProcess->getVolumeGradientTf());
					vrProcess->update();
				}
			}
			//改变当前断点
			if (keyEvent->key() == Qt::Key_Left)
			{
				gradientTf->changeCurBpKeyByKeyboard(-1);
				gradientTf->updateVolumeOpacity(vrProcess->getVolumeGradientTf());
				vrProcess->update();
			}
			if (keyEvent->key() == Qt::Key_Right)
			{
				gradientTf->changeCurBpKeyByKeyboard(1);
				gradientTf->updateVolumeOpacity(vrProcess->getVolumeGradientTf());
				vrProcess->update();
			}
			if (keyEvent->key() == Qt::Key_Down)
			{
				gradientTf->changeCurBpValueByboard(-1);
				gradientTf->updateVolumeOpacity(vrProcess->getVolumeGradientTf());
				vrProcess->update();
			}
			if (keyEvent->key() == Qt::Key_Up)
			{
				gradientTf->changeCurBpValueByboard(1);
				gradientTf->updateVolumeOpacity(vrProcess->getVolumeGradientTf());
				vrProcess->update();
			}
		}
		return true;
	}
	//梯度-不透明度传递函数不透明度
	if (watched == ui->gradienttf_curbp_opacity_label)
	{
		if (event->type() == QEvent::Paint)
		{
			gradientTf->showCurBpValue();
		}
		return true;
	}
}

//改变传递函数当前断点信息
void MainWindow::changeCurTfBpInfo()
{
	ui->colorTfBar->setFocus();
	QToolButton* btn = dynamic_cast<QToolButton*>(sender());
	
	if (btn->objectName() == "colortf_left_button")
		colorTf->showTfBpInfoAt(colorTf->getCurBpIdx() - 1);
	if (btn->objectName() == "colortf_right_button")
		colorTf->showTfBpInfoAt(colorTf->getCurBpIdx() + 1);
	if (btn->objectName() == "scalartf_left_button")
		opacityTf->showTfBpInfoAt(opacityTf->getCurBpIdx() - 1);
	if (btn->objectName() == "scalartf_right_button")
		opacityTf->showTfBpInfoAt(opacityTf->getCurBpIdx() + 1);
	if (btn->objectName() == "gradienttf_left_button")
		gradientTf->showTfBpInfoAt(gradientTf->getCurBpIdx() - 1);
	if (btn->objectName() == "gradienttf_right_button")
		gradientTf->showTfBpInfoAt(gradientTf->getCurBpIdx() + 1);
}

//缩放颜色传递函数X轴
void MainWindow::onColorTfMinRangeChange(int lower)
{
	ui->colortf_x_min->setText(QString::number(lower));
	colorTf->setminRange(lower);
	ui->colorTfBar->repaint();
}

void MainWindow::onColorTfMaxRangeChange(int upper)
{
	ui->colortf_x_max->setText(QString::number(upper));
	colorTf->setmaxRange(upper);
	ui->colorTfBar->repaint();
}

void MainWindow::onScalarTfMinRangeChange(int lower)
{
	ui->scalartf_x_min->setText(QString::number(lower));
	opacityTf->setminRange(lower);
	ui->scalarOpacityTfBar->repaint();
}

void MainWindow::onScalarTfMaxRangeChange(int upper)
{
	ui->scalartf_x_max->setText(QString::number(upper));
	opacityTf->setmaxRange(upper);
	ui->scalarOpacityTfBar->repaint();
}

void MainWindow::onGradientTfMinRangeChange(int lower)
{
	ui->gradienttf_x_min->setText(QString::number(lower));
	gradientTf->setminRange(lower);
	ui->gradientOpacityTfBar->repaint();
}

void MainWindow::onGradientTfMaxRangeChange(int upper)
{
	ui->gradienttf_x_max->setText(QString::number(upper));
	gradientTf->setmaxRange(upper);
	ui->gradientOpacityTfBar->repaint();
}
