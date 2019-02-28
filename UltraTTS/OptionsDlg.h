#pragma once
#include <qdialog.h>
#include "ui_Options.h"

class COptionsDlg : public QDialog
{
	Q_OBJECT
public:
	COptionsDlg(QSettings*	pCfg, QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
	virtual ~COptionsDlg();

private slots:
	void onApply();

private:
	Ui::OptionsDlg ui;

protected:
	QSettings*	m_pCfg;
};

