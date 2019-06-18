#ifndef VOLUMENAMEDIALOG_H
#define VOLUMENAMEDIALOG_H

#include <QDialog>

namespace Ui {
class volumeNameDialog;
}

class volumeNameDialog : public QDialog
{
    Q_OBJECT

public:
    explicit volumeNameDialog(QWidget *parent = nullptr);
    ~volumeNameDialog();
	QString getInputName();

private:
    Ui::volumeNameDialog *ui;
	QString name;

	void accept();
	
};

#endif // VOLUMENAMEDIALOG_H
