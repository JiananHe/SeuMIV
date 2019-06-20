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

	//添加隐藏的widget
	ui->horizontalLayout_5->addWidget(ui->widget_38);
	ui->widget_38->hide();
	ui->horizontalLayout_19->addWidget(ui->widget_42);
	ui->widget_42->hide();
	ui->horizontalLayout_19->addWidget(ui->widget_45);
	ui->widget_45->hide();
	ui->verticalLayout->addWidget(ui->cpr);
	ui->cpr->hide();
	ui->verticalLayout->addWidget(ui->blend);
	ui->blend->hide();
	ui->widget_19->hide();
	ui->pushButton_32->hide();
	ui->pushButton_33->hide();

	//设置lineEdit输入限制
	QRegExp regExp("^-?\\d{1,4}(\\.\\d)?");
	ui->lineEdit->setValidator(new QRegExpValidator(regExp, this));
	ui->lineEdit_2->setValidator(new QRegExpValidator(regExp, this));

	mpr = new View2D(ui->mpr);
	cpr = new View2D(ui->cpr);
	blend = new View2D(ui->blend);

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

	//切换切片
	QSignalMapper *signalMapper2 = new QSignalMapper(this);
	connect(ui->button4, SIGNAL(released()), signalMapper2, SLOT(map()));
	connect(ui->button5, SIGNAL(released()), signalMapper2, SLOT(map()));
	connect(ui->button6, SIGNAL(released()), signalMapper2, SLOT(map()));
	connect(ui->button7, SIGNAL(released()), signalMapper2, SLOT(map()));

	signalMapper2->setMapping(ui->button4, 3);
	signalMapper2->setMapping(ui->button5, 1);
	signalMapper2->setMapping(ui->button6, 2);
	signalMapper2->setMapping(ui->button7, 4);

	connect(signalMapper2, SIGNAL(mapped(int)), this, SLOT(OnChangeSlice(int)));

	//读取NIFTI文件
	connect(ui->pushButton_34, &QPushButton::released, this, &MainWindow::openOriginalFile);
	connect(ui->pushButton_67, &QPushButton::released, this, &MainWindow::openSegmentFile);
	connect(ui->comboBox_8, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MainWindow::OnClickNiiFilesComboBox);
	connect(ui->radioButton_2, &QRadioButton::clicked, this, &MainWindow::OnChangeNiiFileVisible);

	connect(ui->comboBox_6, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MainWindow::OnInterpolationMethodChanged);
	connect(ui->comboBox_7, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MainWindow::OnCurveFitMethodChanged);

	//设置、更新窗宽窗位
	connect(cpr, &View2D::changeWindowLevel, this, &MainWindow::OnChangeWindowLevel);
	connect(mpr, &View2D::changeWindowLevel, this, &MainWindow::OnChangeWindowLevel);
	connect(ui->setWL, &QPushButton::released, this, &MainWindow::SetWindowLevel);

	//绑定工具按钮事件
	connect(ui->pushButton_22, &QPushButton::released, this, &MainWindow::OnReset);
	connect(ui->pushButton_24, &QPushButton::released, this, &MainWindow::OnZoomIn);
	connect(ui->pushButton_25, &QPushButton::released, this, &MainWindow::OnZoomOut);
	connect(ui->pushButton_26, &QPushButton::released, this, &MainWindow::OnClockwiseRotate);
	connect(ui->pushButton_27, &QPushButton::released, this, &MainWindow::OnContraRotate);
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
	if (current2DState == 1)
		return;

	ui->widget_38->hide();
	ui->widget_42->hide();
	ui->widget_45->hide();

	ui->widget_19->hide();

	ui->widget->show();
	ui->pushButton_2->setStyleSheet(VIEW2D_BUTTON_SELECTED_STYLE);
	ui->pushButton_3->setStyleSheet(VIEW2D_BUTTON_UNSELECTED_STYLE);
	ui->pushButton_4->setStyleSheet(VIEW2D_BUTTON_UNSELECTED_STYLE);

	ui->mpr->show();
	ui->blend->hide();
	ui->cpr->hide();

	current2DState = 1;
	mpr->DisplayMPR();
}

void MainWindow::show2DCPR() {
	if (current2DState == 2)
		return;

	ui->widget_38->hide();
	ui->widget_19->hide();
	ui->widget_45->hide();
	ui->widget_42->show();
	ui->widget->show();
	ui->pushButton_2->setStyleSheet(VIEW2D_BUTTON_UNSELECTED_STYLE);
	ui->pushButton_3->setStyleSheet(VIEW2D_BUTTON_SELECTED_STYLE);
	ui->pushButton_4->setStyleSheet(VIEW2D_BUTTON_UNSELECTED_STYLE);

	ui->mpr->hide();
	ui->blend->hide();
	ui->cpr->show();

	current2DState = 2;
	cpr->DisplayCPR();
}

void MainWindow::show2DBlend() {
	if (current2DState == 3)
		return;

	ui->widget->hide();
	ui->widget_19->hide();
	ui->widget_42->hide();
	ui->widget_45->show();
	ui->widget_38->show();
	ui->pushButton_2->setStyleSheet(VIEW2D_BUTTON_UNSELECTED_STYLE);
	ui->pushButton_3->setStyleSheet(VIEW2D_BUTTON_UNSELECTED_STYLE);
	ui->pushButton_4->setStyleSheet(VIEW2D_BUTTON_SELECTED_STYLE);

	ui->mpr->hide();
	ui->cpr->hide();
	ui->blend->show();

	current2DState = 3;
	blend->DisplayBlend();
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

			//data = reader->GetOutput();
		}
		else if (path.endsWith(".dcm")) {
			vtkSmartPointer<vtkDICOMImageReader> reader = vtkSmartPointer<vtkDICOMImageReader>::New();
			reader->SetFileName(path.toLocal8Bit());
			reader->Update();

			data = reader->GetOutput();
			string text = reader->GetPatientName();
			text = "\t" + text + "\t" + reader->GetStudyID() + "\t" + "-CT";
			ui->info->setText(QString::fromStdString(text));
			mpr->setDICOMData(data);
			cpr->setDICOMData(data);
			blend->setDICOMData(data);
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
			string text = reader->GetPatientName();
			text = "\t" + text + "\t" + reader->GetStudyID() + "\t" + "-CT";
			ui->info->setText(QString::fromStdString(text));
			mpr->setDICOMData(data);
			cpr->setDICOMData(data);
			blend->setDICOMData(data);
		}
	}

	if (current2DState == 1)
		mpr->DisplayMPR();
	else if (current2DState == 2)
		cpr->DisplayCPR();
	else if (current2DState == 3)
		blend->DisplayBlend();
}

void MainWindow::change2DView(int state)
{
	if (current2DState == 1) {
		mpr->viewState = state;
		mpr->changeViewPort();
	}
	else if (current2DState == 2) {
		cpr->viewState = state;
		cpr->changeViewPort();
	}
	else if (current2DState == 3) {
		blend->viewState = state;
		blend->changeViewPort();
	}
}

void MainWindow::openOriginalFile()
{
	QString file = QFileDialog::getOpenFileName(NULL, QStringLiteral("打开文件"), ".");
	qDebug() << file << endl;
	if (file.isEmpty() || file.isNull())
		return;

	string fileName = file.toLocal8Bit();
	file = file.toLower();
	if ((!file.endsWith(".nii.gz") && !file.endsWith(".nii")))
	{
		QMessageBox msgBox(QMessageBox::Information, QStringLiteral("警告"), QStringLiteral("添加nii文件失败"));
		msgBox.setStandardButtons(QMessageBox::Ok);
		msgBox.setButtonText(QMessageBox::Ok, QStringLiteral("确定"));
		msgBox.exec();
	}

	blend->DisplayOrignalFile(fileName);

	ui->comboBox_8->clear();
	niiFiles.clear();
	niiFilesVisbile.clear();

	QString name = file.mid(file.lastIndexOf('/') + 1);
	name = name.left(name.indexOf('.'));
	ui->radioButton_2->setCheckable(true);
	ui->radioButton_2->setChecked(true);
	niiFiles.push_back(name);
	niiFilesVisbile.push_back(true);

	ui->comboBox_8->addItem(name);
	ui->comboBox_8->setCurrentText(name);
}

void MainWindow::openSegmentFile()
{
	if (niiFiles.empty()) {
		QMessageBox msgBox(QMessageBox::Information, QStringLiteral("警告"), QStringLiteral("未读入原始NIFTI文件"));
		msgBox.setStandardButtons(QMessageBox::Ok);
		msgBox.setButtonText(QMessageBox::Ok, QStringLiteral("确定"));
		msgBox.exec();
		return;
	}

	QString file = QFileDialog::getOpenFileName(NULL, QStringLiteral("打开文件"), ".");
	qDebug() << file << endl;
	if (file.isEmpty() || file.isNull())
		return;

	string fileName = file.toLocal8Bit();
	if ((!file.endsWith(".nii.gz") && !file.endsWith(".nii")))
	{
		QMessageBox msgBox(QMessageBox::Information, QStringLiteral("警告"), QStringLiteral("添加nii文件失败"));
		msgBox.setStandardButtons(QMessageBox::Ok);
		msgBox.setButtonText(QMessageBox::Ok, QStringLiteral("确定"));
		msgBox.exec();
		return;
	}

	QColor qcolor = QColorDialog::getColor(Qt::red, this, QStringLiteral("颜色对话框"), QColorDialog::ShowAlphaChannel);
	if (!qcolor.isValid())
		return;

	int color[4] = { qcolor.red(),qcolor.green(),qcolor.blue(),qcolor.alpha() };

	QString name = file.mid(file.lastIndexOf('/') + 1);
	name = name.left(name.indexOf('.'));

	for (QString qs : niiFiles) {
		if (qs == name) {
			int index = ui->comboBox_8->currentIndex();
			blend->ChangeActors(index, niiFilesVisbile[index], color);
			return;
		}
	}

	niiFiles.push_back(name);
	niiFilesVisbile.push_back(true);
	ui->radioButton_2->setChecked(true);
	ui->radioButton_2->setCheckable(true);

	ui->comboBox_8->addItem(name);
	ui->comboBox_8->setCurrentText(name);

	blend->DisplaySegementFile(fileName, color);
}

void MainWindow::OnChangeNiiFileVisible()
{
	if (ui->radioButton_2->isCheckable()) {
		int index = ui->comboBox_8->currentIndex();
		niiFilesVisbile[index] = !niiFilesVisbile[index];
		ui->radioButton_2->setChecked(niiFilesVisbile[index]);
		blend->ChangeActors(index, niiFilesVisbile[index]);
	}
}

void MainWindow::OnClickNiiFilesComboBox(int index)
{
	if (index >= 0 && index < niiFiles.size()) {
		ui->radioButton_2->setChecked(niiFilesVisbile[index]);
	}
}

void MainWindow::OnInterpolationMethodChanged(int index)
{
	cpr->myStyle->interpolationMethod = InterpolationMethod(index);
}

void MainWindow::OnCurveFitMethodChanged(int index)
{
	cpr->myStyle->curveFitMethod = CurveFitMethod(index);
}

void MainWindow::OnChangeSlice(int flag)
{
	switch (current2DState) {
	case 1:
		mpr->ChangeSlice(current2DState, flag);
		break;
	case 2:
		cpr->ChangeSlice(current2DState, flag);
		break;
	case 3:
		blend->ChangeSlice(current2DState, flag);
		break;
	default:
		break;
	}
}

void MainWindow::OnChangeWindowLevel(double wl0, double wl1)
{
	ui->lineEdit->setText(QString::number(wl0, 'f', 1));
	ui->lineEdit_2->setText(QString::number(wl1, 'f', 1));
}

void MainWindow::SetWindowLevel()
{
	QString swl0 = ui->lineEdit->text(), swl1 = ui->lineEdit_2->text();
	if (swl0.isEmpty() || swl0.isNull() || swl1.isEmpty() || swl1.isNull())
		return;
	double wl0 = swl0.toDouble(), wl1 = swl1.toDouble();
	if (wl0 == 0) {
		QMessageBox msgBox(QMessageBox::Information, QStringLiteral("警告"), QStringLiteral("窗宽不能为0"));
		msgBox.setStandardButtons(QMessageBox::Ok);
		msgBox.setButtonText(QMessageBox::Ok, QStringLiteral("确定"));
		msgBox.exec();
		return;
	}
	if (current2DState == 1) {
		mpr->SetWindowLevel(current2DState, wl0, wl1);
	}
	else if (current2DState == 2) {
		cpr->SetWindowLevel(current2DState, wl0, wl1);
	}
}

void MainWindow::OnZoomIn()
{
	if (current2DState == 1) {
		mpr->Zoom(current2DState,1);
	}
	else if (current2DState == 2) {
		cpr->Zoom(current2DState, 1);
	}
	else if (current2DState == 3) {
		blend->Zoom(current2DState, 1);
	}
}

void MainWindow::OnZoomOut()
{
	if (current2DState == 1) {
		mpr->Zoom(current2DState, 2);
	}
	else if (current2DState == 2) {
		cpr->Zoom(current2DState, 2);
	}
	else if (current2DState == 3) {
		blend->Zoom(current2DState, 2);
	}
}

void MainWindow::OnReset()
{
	if (current2DState == 1) {
		mpr->Reset(current2DState);
	}
	else if (current2DState == 2) {
		cpr->Reset(current2DState);
	}
	else if (current2DState == 3) {
		blend->Reset(current2DState);
		ui->radioButton_2->setCheckable(false);
		ui->comboBox_8->clear();
	}
}

void MainWindow::OnClockwiseRotate()
{
	if (current2DState == 1) {
		mpr->Rotate(current2DState, 1);
	}
	else if (current2DState == 2) {
		cpr->Rotate(current2DState, 1);
	}
	else if (current2DState == 3) {
		blend->Rotate(current2DState, 1);
	}
}

void MainWindow::OnContraRotate()
{
	if (current2DState == 1) {
		mpr->Rotate(current2DState, 2);
	}
	else if (current2DState == 2) {
		cpr->Rotate(current2DState, 2);
	}
	else if (current2DState == 3) {
		blend->Rotate(current2DState, 2);
	}
}

MainWindow::~MainWindow()
{
	delete ui;
}
