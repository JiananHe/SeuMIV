#include "mainwindow.h"

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
	ui->colorTfWidget->setVisible(false);
	ui->scalarOpacityWidget->setVisible(false);
	ui->gradientOpacityWidget->setVisible(false);

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

	//增量绘制相关响应事件
	cur_volume_id = -1;
	multi_render_flag = false;
	connect(ui->ir_add_button, SIGNAL(released()), this, SLOT(onAddVolumeSlot()));
	connect(ui->ir_delete_button, SIGNAL(released()), this, SLOT(onDeleteVolumeSlot()));
	connect(ui->ir_rename_button, SIGNAL(released()), this, SLOT(onRenameVolumeSlot()));
	connect(ui->ir_show_button, SIGNAL(released()), this, SLOT(onShowAllVolumesSlot()));
	connect(ui->ir_clear_button, SIGNAL(released()), this, SLOT(onClearAllVolumesSlot()));
	connect(ui->ir_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onCurVolumeChangedSlot(int)));
	connect(ui->ir_radioButton, SIGNAL(toggled(bool)), this, SLOT(onCurVolumeFlagSlot(bool)));
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
	ui->colorTfWidget->setVisible(true);
	ui->scalarOpacityWidget->setVisible(true);
	ui->gradientOpacityWidget->setVisible(true);
	ui->ir_add_button->setEnabled(true);

	colorTf->setBoneColorTf(vrProcess->getVolumeColorTf());
	opacityTf->setBoneOpacityTf(vrProcess->getVolumeOpacityTf());

	multi_render_flag = false;
}

void MainWindow::onOpenNifitSlot()
{
	//打开文件选择框
	QString fileName = QFileDialog::getOpenFileName(this, QStringLiteral("打开NIFIT文件"),
		"C:\\Users\\13249\\Documents\\VTK_Related\\dataset", "NII file (*.nii)");

	//体绘制
	if (fileName.isEmpty())
		return;

	multi_render_flag = false;
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
	ui->colorTfWidget->setVisible(true);
	ui->scalarOpacityWidget->setVisible(true);
	ui->gradientOpacityWidget->setVisible(true);
	ui->ir_add_button->setEnabled(true);

	colorTf->setBoneColorTf(vrProcess->getVolumeColorTf());
	opacityTf->setBoneOpacityTf(vrProcess->getVolumeOpacityTf());

	map<double, double> init_gradient_tf;
	init_gradient_tf.insert(pair<double, double>(0, 1.0));
	init_gradient_tf.insert(pair<double, double>(255, 1.0));
	gradientTf->setCustomizedOpacityTf(vrProcess->getVolumeGradientTf(), init_gradient_tf);
	vrProcess->update();
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

//叠加体绘制图
void MainWindow::onAddVolumeSlot()
{
	volumeNameDialog vnDialog;
	vnDialog.exec();
	
	QString name = vnDialog.getInputName();
	if (name.isEmpty())
		return;
	else if (volume_names.contains(name))
		QMessageBox::warning(NULL, "warning", QStringLiteral("Volume名字重复"));
	else if (volume_flags.size() >= 4)
		QMessageBox::warning(NULL, "warning", QStringLiteral("内存不足，无法添加"));
	else
	{
		ui->ir_clear_button->setEnabled(true);
		ui->ir_rename_button->setEnabled(true);
		ui->ir_delete_button->setEnabled(true);
		ui->ir_radioButton->setChecked(true);

		cur_volume_id = volume_flags.size();
		volume_flags.push_back(true);
		if (volume_flags.size() > 1)
			ui->ir_show_button->setEnabled(true);


		volume_names << name;
		ui->ir_comboBox->clear();
		ui->ir_comboBox->addItems(volume_names); // cur_volume_id变为0
		cur_volume_id = volume_flags.size() - 1;
		ui->ir_comboBox->setCurrentText(ui->ir_comboBox->itemText(cur_volume_id));

		vrProcess->addVolume();
	}
}

void MainWindow::onRenameVolumeSlot()
{
	volumeNameDialog vnDialog;
	vnDialog.exec();

	QString name = vnDialog.getInputName();
	if (name.isEmpty())
		return;
	else if (volume_names.contains(name))
		QMessageBox::warning(NULL, "warning", QStringLiteral("Volume名字重复"));
	else
	{
		int cur_temp = cur_volume_id;
		volume_names.removeAt(cur_volume_id);
		volume_names.insert(cur_volume_id, name);
		ui->ir_comboBox->clear();
		ui->ir_comboBox->addItems(volume_names); // cur_volume_id变为0
		cur_volume_id = cur_temp;
		ui->ir_comboBox->setCurrentText(ui->ir_comboBox->itemText(cur_volume_id));
	}
}

void MainWindow::onShowAllVolumesSlot()
{
	vrProcess->showAllVolumes();

	ui->ir_radioButton->setEnabled(true);
	ui->ir_radioButton->setChecked(true);
	ui->ir_comboBox->setEnabled(true);
	ui->ir_add_button->setEnabled(false);
	for (int i = 0; i < volume_flags.size(); ++i)
		volume_flags[i] = true;
	cur_volume_id = volume_flags.size() - 1;
	ui->ir_comboBox->setCurrentText(ui->ir_comboBox->itemText(cur_volume_id));
	multi_render_flag = true;
}

void MainWindow::onClearAllVolumesSlot()
{
	volume_flags.clear();
	volume_names.clear();
	ui->ir_comboBox->clear();
	ui->ir_radioButton->setChecked(false);
	ui->ir_show_button->setEnabled(false);
	ui->ir_clear_button->setEnabled(false);
	ui->ir_comboBox->setEnabled(false);
	ui->ir_radioButton->setEnabled(false);
	ui->ir_rename_button->setEnabled(false);
	ui->ir_delete_button->setEnabled(false);
}

void MainWindow::onDeleteVolumeSlot()
{
	vector<bool>::iterator iter = volume_flags.begin();
	int ft = cur_volume_id;
	while (ft-- > 0)
		++iter;
	
	volume_flags.erase(iter);
	volume_names.removeAt(cur_volume_id);
	cur_volume_id = cur_volume_id == volume_flags.size() ? cur_volume_id - 1 : cur_volume_id;
	int cur_temp = cur_volume_id;

	if (volume_flags.size())//还有Volume
	{
		ui->ir_radioButton->setChecked(volume_flags[cur_volume_id]);
		ui->ir_comboBox->clear();
		ui->ir_comboBox->addItems(volume_names);// cur_volume_id变为0
		cur_volume_id = cur_temp;
		ui->ir_comboBox->setCurrentText(ui->ir_comboBox->itemText(cur_volume_id));
	}
	else//没有Volume了
	{
		ui->ir_comboBox->clear();
		ui->ir_radioButton->setChecked(false);
		ui->ir_show_button->setEnabled(false);
		ui->ir_clear_button->setEnabled(false);
		ui->ir_comboBox->setEnabled(false);
		ui->ir_radioButton->setEnabled(false);
		ui->ir_rename_button->setEnabled(false);
		ui->ir_delete_button->setEnabled(false);
	}
}

void MainWindow::onCurVolumeChangedSlot(int name_id)
{
	if (name_id == -1)
		return;

	cur_volume_id = name_id;
	if (volume_flags.size())
	{
		ui->ir_radioButton->setChecked(volume_flags[cur_volume_id]);
		if (multi_render_flag)
		{
			//改变传递函数可视化图
			vrProcess->changeCurVolume(name_id);
			colorTf->updateVisualColor(vrProcess->getVolumeColorTf());
			opacityTf->updateVisualOpacity(vrProcess->getVolumeOpacityTf());
			gradientTf->updateVisualOpacity(vrProcess->getVolumeGradientTf());
			//若当前Volume不可视，则传递函数可视化图不可视
			if (!volume_flags[cur_volume_id])
			{
				ui->colorTfWidget->setVisible(false);
				ui->scalarOpacityWidget->setVisible(false);
				ui->gradientOpacityWidget->setVisible(false);
			}
			else
			{
				ui->colorTfWidget->setVisible(true);
				ui->scalarOpacityWidget->setVisible(true);
				ui->gradientOpacityWidget->setVisible(true);
			}
		}
	}
	else
		ui->ir_radioButton->setChecked(true);
}

void MainWindow::onCurVolumeFlagSlot(bool check)
{
	if (volume_flags.size() != 0 && volume_flags[cur_volume_id] != check)
	{
		volume_flags[cur_volume_id] = check;
		if (check)
		{
			vrProcess->showCurVolume(cur_volume_id);
			vrProcess->update();

			ui->colorTfWidget->setVisible(true);
			ui->scalarOpacityWidget->setVisible(true);
			ui->gradientOpacityWidget->setVisible(true);
		}
		else
		{
			vrProcess->hideCurVolume(cur_volume_id);
			vrProcess->update();

			ui->colorTfWidget->setVisible(false);
			ui->scalarOpacityWidget->setVisible(false);
			ui->gradientOpacityWidget->setVisible(false);
		}
	}
}
