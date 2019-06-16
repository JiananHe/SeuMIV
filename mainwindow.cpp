#include "mainwindow.h"
#include "ui_mainwindow.h"

#define TOP_BUTTON_SELECTED_STYLE QStringLiteral("QPushButton{ color: rgb(255,255,255); font: 75 10pt \"Î¢ÈíÑÅºÚ\"; } QPushButton:pressed{ border: none; }")
#define TOP_BUTTON_UNSELECTED_STYLE QStringLiteral("QPushButton{ color: rgb(175,175,175); font: 75 10pt \"Î¢ÈíÑÅºÚ\"; } QPushButton:pressed{ color: rgb(255,255,255); border: none; }")
#define VIEW2D_BUTTON_SELECTED_STYLE QStringLiteral("QPushButton{ font: 9pt \"Times New Roman\"; font-weight: 500; color: rgb(0, 0, 105); border-radius: 10px; background-color: qlineargradient(spread:pad, x1:0.5, y1:3, x2:0.5, y2:0, stop:0 rgba(88, 88, 200, 255), stop:1 rgba(255, 255, 255, 255)); } QPushButton:pressed{ background-color: qlineargradient(spread:pad, x1:0.5, y1:1.5, x2:0.5, y2:0, stop:0 rgb(12, 4, 132), stop:1 rgba(255, 255, 255, 255)); };")
#define VIEW2D_BUTTON_UNSELECTED_STYLE QStringLiteral("QPushButton{ font: 9pt \"Times New Roman\"; font-weight: 500; color: rgb(255, 255, 255); border-radius:10px; background-color: qlineargradient(spread:pad, x1:0.5, y1:0.5, x2:0.5, y2:-0.5, stop:0 rgba(46, 51, 138, 255), stop:1 rgba(255, 255, 255, 255)); } QPushButton:pressed{ background-color: qlineargradient(spread:pad, x1:0.5, y1:1.5, x2:0.5, y2:-0.7, stop:0 rgb(12, 4, 132), stop:1 rgba(255, 255, 255, 255)); };")

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	this->setWindowFlags(Qt::FramelessWindowHint|Qt::WindowSystemMenuHint|Qt::WindowMinMaxButtonsHint);
	ui->stackedWidget->setCurrentIndex(0);

	//Ìí¼ÓÒþ²ØµÄwidget
	ui->horizontalLayout_5->addWidget(ui->widget_38);
	ui->widget_38->hide();
	ui->horizontalLayout_19->addWidget(ui->widget_42);
	ui->widget_42->hide();
	ui->horizontalLayout_19->addWidget(ui->widget_45);
	ui->widget_45->hide();

	//Ò³ÃæÇÐ»»ÏìÓ¦
	connect(ui->button2D, SIGNAL(released()), this, SLOT(onView2DSlot()));
	connect(ui->button3D, SIGNAL(released()), this, SLOT(onView3DSlot()));

	//³ÌÐò×îÐ¡»¯ÒÔ¼°ÍË³ö
	connect(ui->exit, SIGNAL(released()), qApp, SLOT(quit()));
	connect(ui->minimize, &QPushButton::clicked, this, &MainWindow::minimize);

	//ÇÐ»»¶þÎ¬ÏÔÊ¾Ò³Ãæ--ÇÐ»»×Ó¿Ø¼þ
	connect(ui->pushButton_2, &QPushButton::clicked, this, &MainWindow::show2DMPR);
	connect(ui->pushButton_3, &QPushButton::clicked, this, &MainWindow::show2DCPR);
	connect(ui->pushButton_4, &QPushButton::clicked, this, &MainWindow::show2DBlend);
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
}

void MainWindow::minimize()
{
	this->setWindowState(Qt::WindowMinimized);
}

MainWindow::~MainWindow()
{
	delete ui;
}
