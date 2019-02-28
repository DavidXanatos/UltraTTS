#include "stdafx.h"
#include "OptionsDlg.h"


COptionsDlg::COptionsDlg(QSettings*	pCfg, QWidget *parent, Qt::WindowFlags f)
: QDialog(parent, f)
{
	m_pCfg = pCfg;

	ui.setupUi(this);

	connect(ui.okButton, SIGNAL(clicked()), SLOT(onApply()));

	ui.chkCopy->setChecked(m_pCfg->value("UseHotKey_CopyAndRead", true).toBool());
	ui.kseCopy->setKeySequence(QKeySequence(m_pCfg->value("HotKey_CopyAndRead", "F1").toString()));
	ui.chkNow->setChecked(m_pCfg->value("UseHotKey_CopyAndReadNow", true).toBool());
	ui.kseNow->setKeySequence(QKeySequence(m_pCfg->value("HotKey_CopyAndReadNow", "Alt+F1").toString()));
	ui.chkPause->setChecked(m_pCfg->value("UseHotKey_PauseRead", true).toBool());
	ui.ksePause->setKeySequence(QKeySequence(m_pCfg->value("HotKey_PauseRead", "Ctrl+F1").toString()));
	ui.chkNext->setChecked(m_pCfg->value("UseHotKey_ReadNext", true).toBool());
	ui.kseNext->setKeySequence(QKeySequence(m_pCfg->value("HotKey_ReadNext", "Shift+Ctrl+F1").toString()));
	ui.chkPrev->setChecked(m_pCfg->value("UseHotKey_ReadPreviouse", true).toBool());
	ui.ksePrev->setKeySequence(QKeySequence(m_pCfg->value("HotKey_ReadPreviouse", "Shift+Alt+F1").toString()));
	ui.chkStop->setChecked(m_pCfg->value("UseHotKey_StopReading", true).toBool());
	ui.kseStop->setKeySequence(QKeySequence(m_pCfg->value("HotKey_StopReading", "Ctrl+Alt+F1").toString()));
	ui.chkAdd->setChecked(m_pCfg->value("UseHotKey_StopReading", true).toBool());
	ui.kseAdd->setKeySequence(QKeySequence(m_pCfg->value("HotKey_CopyAndAdd", "Shift+F1").toString()));

	ui.chkBeep->setChecked(m_pCfg->value("PlaySound", true).toBool());
	ui.chkSysTray->setChecked(m_pCfg->value("SysTray", true).toBool());
	ui.editDelay->setText(QString::number(m_pCfg->value("SectionDelay", 750).toInt()));

	ui.editDelay->setValidator(new QIntValidator(0, 10000, this));
}


COptionsDlg::~COptionsDlg()
{
}

void COptionsDlg::onApply()
{
	m_pCfg->setValue("UseHotKey_CopyAndRead", ui.chkCopy->isChecked());
	m_pCfg->setValue("HotKey_CopyAndRead", ui.kseCopy->keySequence().toString());
	m_pCfg->setValue("UseHotKey_CopyAndReadNow", ui.chkNow->isChecked());
	m_pCfg->setValue("HotKey_CopyAndReadNow", ui.kseNow->keySequence().toString());
	m_pCfg->setValue("UseHotKey_PauseRead", ui.chkPause->isChecked());
	m_pCfg->setValue("HotKey_PauseRead", ui.ksePause->keySequence().toString());
	m_pCfg->setValue("UseHotKey_ReadNext", ui.chkNext->isChecked());
	m_pCfg->setValue("HotKey_ReadNext", ui.kseNext->keySequence().toString());
	m_pCfg->setValue("UseHotKey_ReadPreviouse", ui.chkPrev->isChecked());
	m_pCfg->setValue("HotKey_ReadPreviouse", ui.ksePrev->keySequence().toString());
	m_pCfg->setValue("UseHotKey_StopReading", ui.chkStop->isChecked());
	m_pCfg->setValue("HotKey_StopReading", ui.kseStop->keySequence().toString());
	m_pCfg->setValue("UseHotKey_CopyAndAdd", ui.chkAdd->isChecked());
	m_pCfg->setValue("HotKey_CopyAndAdd", ui.kseAdd->keySequence().toString());

	m_pCfg->setValue("PlaySound", ui.chkBeep->isChecked());
	m_pCfg->setValue("SysTray", ui.chkSysTray->isChecked());
	m_pCfg->setValue("SectionDelay", ui.editDelay->text().toInt());
}
