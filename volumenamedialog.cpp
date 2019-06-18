#include "volumenamedialog.h"
#include "ui_volumenamedialog.h"

volumeNameDialog::volumeNameDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::volumeNameDialog)
{
    ui->setupUi(this);
}

volumeNameDialog::~volumeNameDialog()
{
    delete ui;
}

QString volumeNameDialog::getInputName()
{
	return name;
}

void volumeNameDialog::accept()
{
	name = ui->textEdit->toPlainText();
	QDialog::accept();
}