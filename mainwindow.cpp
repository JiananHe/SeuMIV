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
}

void MainWindow::onOpenNifitSlot()
{
	//打开文件选择框
	QString fileName = QFileDialog::getOpenFileName(this, QStringLiteral("打开NIFIT文件"),
		"C:\\Users\\13249\\Documents\\VTK_Related\\dataset", "NII file (*.nii)");

	//体绘制
	vrProcess->niiVolumeRenderFlow(fileName);
	vrProcess->update();
}


//***********************3D体绘制相关槽函数***********************
//3D界面事件过滤器
bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
	//颜色传递函数可视化图
	if (watched == ui->colorTfBar)
	{
		//绘制颜色传递函数
		if (event->type() == QEvent::Paint)
		{
			colorTf->showTfDiagram();
		}
		//选定或新建一个断点
		if (event->type() == QEvent::MouseButtonPress)
		{
			QPoint mp = ui->colorTfBar->mapFromGlobal(QCursor::pos());
			if (mp.x() > colorTf->getD() && mp.x() < ui->colorTfBar->geometry().width() - colorTf->getD())
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
}

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
