#include "mainwindow.h"
#include "ui_mainwindow.h"

#define TOP_BUTTON_SELECTED_STYLE QStringLiteral("QPushButton{ color: rgb(255,255,255); font: 75 10pt \"微软雅黑\"; } QPushButton:pressed{ border: none; }")
#define TOP_BUTTON_UNSELECTED_STYLE QStringLiteral("QPushButton{ color: rgb(175,175,175); font: 75 10pt \"微软雅黑\"; } QPushButton:pressed{ color: rgb(255,255,255); border: none; }")
#define VIEW2D_BUTTON_SELECTED_STYLE QStringLiteral("QPushButton{ font: 9pt \"Times New Roman\"; font-weight: 500; color: rgb(0, 0, 105); border-radius: 10px; background-color: qlineargradient(spread:pad, x1:0.5, y1:3, x2:0.5, y2:0, stop:0 rgba(88, 88, 200, 255), stop:1 rgba(255, 255, 255, 255)); } QPushButton:pressed{ background-color: qlineargradient(spread:pad, x1:0.5, y1:1.5, x2:0.5, y2:0, stop:0 rgb(12, 4, 132), stop:1 rgba(255, 255, 255, 255)); };")
#define VIEW2D_BUTTON_UNSELECTED_STYLE QStringLiteral("QPushButton{ font: 9pt \"Times New Roman\"; font-weight: 500; color: rgb(255, 255, 255); border-radius:10px; background-color: qlineargradient(spread:pad, x1:0.5, y1:0.5, x2:0.5, y2:-0.5, stop:0 rgba(46, 51, 138, 255), stop:1 rgba(255, 255, 255, 255)); } QPushButton:pressed{ background-color: qlineargradient(spread:pad, x1:0.5, y1:1.5, x2:0.5, y2:-0.7, stop:0 rgb(12, 4, 132), stop:1 rgba(255, 255, 255, 255)); };")

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint);
	ui->stackedWidget->setCurrentIndex(0);

	view2D = new View2D(ui->viewer);

	//添加隐藏的widget
	ui->horizontalLayout_5->addWidget(ui->widget_38);
	ui->widget_38->hide();
	ui->horizontalLayout_19->addWidget(ui->widget_42);
	ui->widget_42->hide();
	ui->horizontalLayout_19->addWidget(ui->widget_45);
	ui->widget_45->hide();

	//页面切换响应
	connect(ui->button2D, SIGNAL(released()), this, SLOT(onView2DSlot()));
	connect(ui->button3D, SIGNAL(released()), this, SLOT(onView3DSlot()));

	//程序最小化以及退出
	connect(ui->exit, SIGNAL(released()), qApp, SLOT(quit()));
	connect(ui->minimize, &QPushButton::released, this, &MainWindow::minimize);

	//切换二维显示页面--切换子控件
	connect(ui->pushButton_2, &QPushButton::released, this, &MainWindow::show2DMPR);
	connect(ui->pushButton_3, &QPushButton::released, this, &MainWindow::show2DCPR);
	connect(ui->pushButton_4, &QPushButton::released, this, &MainWindow::show2DBlend);

	connect(ui->openFile, &QPushButton::released, this, &MainWindow::openFile);

	//切换2D界面窗口布局
	QSignalMapper *signalMapper = new QSignalMapper(this);
	connect(ui->button0, SIGNAL(released()), signalMapper, SLOT(map()));
	connect(ui->button1, SIGNAL(released()), signalMapper, SLOT(map()));
	connect(ui->button2, SIGNAL(released()), signalMapper, SLOT(map()));
	connect(ui->button3, SIGNAL(released()), signalMapper, SLOT(map()));

	signalMapper->setMapping(ui->button0, 0);
	signalMapper->setMapping(ui->button1, 1);
	signalMapper->setMapping(ui->button2, 2);
	signalMapper->setMapping(ui->button3, 3);

	connect(signalMapper, SIGNAL(mapped(int)), this, SLOT(change2DView(int)));
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

void MainWindow::show2DMPR() {
	ui->widget_38->hide();
	ui->widget_42->hide();
	ui->widget_45->hide();
	ui->widget_19->show();
	ui->widget->show();
	ui->pushButton_2->setStyleSheet(VIEW2D_BUTTON_SELECTED_STYLE);
	ui->pushButton_3->setStyleSheet(VIEW2D_BUTTON_UNSELECTED_STYLE);
	ui->pushButton_4->setStyleSheet(VIEW2D_BUTTON_UNSELECTED_STYLE);

	view2D->state = 1;
}

void MainWindow::show2DCPR() {
	ui->widget_38->hide();
	ui->widget_19->hide();
	ui->widget_45->hide();
	ui->widget_42->show();
	ui->widget->show();
	ui->pushButton_2->setStyleSheet(VIEW2D_BUTTON_UNSELECTED_STYLE);
	ui->pushButton_3->setStyleSheet(VIEW2D_BUTTON_SELECTED_STYLE);
	ui->pushButton_4->setStyleSheet(VIEW2D_BUTTON_UNSELECTED_STYLE);

	view2D->state = 2;
}

void MainWindow::show2DBlend() {
	ui->widget->hide();
	ui->widget_19->hide();
	ui->widget_42->hide();
	ui->widget_45->show();
	ui->widget_38->show();
	ui->pushButton_2->setStyleSheet(VIEW2D_BUTTON_UNSELECTED_STYLE);
	ui->pushButton_3->setStyleSheet(VIEW2D_BUTTON_UNSELECTED_STYLE);
	ui->pushButton_4->setStyleSheet(VIEW2D_BUTTON_SELECTED_STYLE);

	view2D->state = 3;
}

void MainWindow::minimize()
{
	this->setWindowState(Qt::WindowMinimized);
}

void MainWindow::openFile()
{
	QString path = QFileDialog::getExistingDirectory(this, QStringLiteral("打开文件或文件夹"), "E:/yzc/vtk");
	if (path.isEmpty() || path.isNull())
		return;
	qDebug() << path << endl;
	QFileInfo fileInfo(path);
	if (!fileInfo.isDir() && !fileInfo.isFile()) {
		QMessageBox msgBox(QMessageBox::Information, QStringLiteral("警告"), QStringLiteral("文件或路径不存在"));
		msgBox.setStandardButtons(QMessageBox::Ok);
		msgBox.setButtonText(QMessageBox::Ok, QStringLiteral("确定"));
		msgBox.exec();
		return;
	}
	if (fileInfo.isFile()) {
		if (path.endsWith(".nii") || path.endsWith(".nii.gz")) {
			vtkSmartPointer<vtkNIFTIImageReader> reader = vtkSmartPointer<vtkNIFTIImageReader>::New();
			reader->SetFileName(path.toLocal8Bit());
			reader->Update();

			data = reader->GetOutput();
		}
		else {
			QMessageBox msgBox(QMessageBox::Information, QStringLiteral("警告"), QStringLiteral("不支持的文件格式"));
			msgBox.setStandardButtons(QMessageBox::Ok);
			msgBox.setButtonText(QMessageBox::Ok, QStringLiteral("确定"));
			msgBox.exec();
			return;
		}
	}
	if (fileInfo.isDir()) {
		QDir dir(path);

		QStringList filters;
		filters << QString("*.dcm");
		dir.setFilter(QDir::Files | QDir::NoSymLinks); //设置类型过滤器，只为文件格式
		dir.setNameFilters(filters);  //设置文件名称过滤器，只为filters格式（后缀为.dcm图片格式）

		if (dir.count() <= 0)
		{
			QMessageBox msgBox(QMessageBox::Information, QStringLiteral("警告"), QStringLiteral("不支持的文件格式"));
			msgBox.setStandardButtons(QMessageBox::Ok);
			msgBox.setButtonText(QMessageBox::Ok, QStringLiteral("确定"));
			msgBox.exec();
			return;
		}
		else {
			vtkSmartPointer<vtkDICOMImageReader> reader = vtkSmartPointer<vtkDICOMImageReader>::New();
			reader->SetDirectoryName(path.toLocal8Bit());
			reader->Update();

			data = reader->GetOutput();
			view2D->setDICOMData(data);
		}
	}

	if (view2D->state == 1)
		view2D->DisplayMPR();
}

void MainWindow::change2DView(int state)
{
	view2D->viewState = state;
	view2D->changeViewPort();
}

MainWindow::~MainWindow()
{
	delete ui;
}
