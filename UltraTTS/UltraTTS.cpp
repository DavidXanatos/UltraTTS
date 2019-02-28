#include "stdafx.h"

#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#if defined(QT_PRINTSUPPORT_LIB)
#include <QtPrintSupport/qtprintsupportglobal.h>
#if QT_CONFIG(printer)
#if QT_CONFIG(printdialog)
#include <QPrintDialog>
#endif // QT_CONFIG(printdialog)
#include <QPrinter>
#endif // QT_CONFIG(printer)
#endif // QT_PRINTSUPPORT_LIB
#include <QFont>
#include <QFontDialog>
#include <QClipboard>
#include "UltraTTS.h"
#include "OptionsDlg.h"

#include "Sapi/TTS.h"


#include "Utils/LangDetect.h"
#include "Utils/Functions.h"

#include "../UGlobalHotkey/uglobalhotkeys.h"


#define HK_READ		1
#define HK_READ_NOW 2
#define HK_PAUSE	3
#define HK_NEXT		4
#define HK_PREV		5
#define HK_STOP		6
#define HK_COPY		7

QString CUltraTTS::STab::name()
{
	if (bReader)
		return CUltraTTS::tr("Clipboard Reader");
	QString Name = QFileInfo(fileName).fileName();
	if (Name.isEmpty())
		return CUltraTTS::tr("NewFile.txt");
	return Name;
}

CUltraTTS::CUltraTTS(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
// Disable menu actions for unavailable features
#if !QT_CONFIG(printer)
    ui.actionPrint->setEnabled(false);
#endif

#if !QT_CONFIG(clipboard)
    ui.actionCut->setEnabled(false);
    ui.actionCopy->setEnabled(false);
    ui.actionPaste->setEnabled(false);
#endif

	ui.actionPause->setCheckable(true);
	ui.actionPause->setEnabled(false);
	ui.actionStop->setEnabled(false);

	ui.textEdit->deleteLater();

	m_pTabs = new QTabWidget();

	ui.verticalLayout->addWidget(m_pTabs);

	m_pReadEdit = NULL;
	m_pTab = NULL;

	on_actionNew_triggered();

	m_pTabs->setTabsClosable(true);
	QTabBar* pTabBar = m_pTabs->tabBar();
	/*m_TabOffset = m_pTabs->count();
	for (int i = 0; i < m_TabOffset; i++)
		pTabBar->setTabButton(i, static_cast<QTabBar::ButtonPosition>(pTabBar->style()->styleHint(QStyle::SH_TabBar_CloseButtonPosition, 0, pTabBar)), 0);*/
	connect(pTabBar, SIGNAL(tabCloseRequested(int)), this, SLOT(on_close_tab(int)));
	connect(m_pTabs, SIGNAL(currentChanged(int)), this, SLOT(on_tab(int)));

	m_pProgress = new QLabel();
	m_pProgress->setMinimumWidth(50);
	ui.statusBar->addPermanentWidget(m_pProgress);

	/*ui.voiceList->deleteLater();
	ui.voiceList = new CComboBoxEx();
	ui.horizontalLayout->insertWidget(1, ui.voiceList);*/


	ui.voiceList->setItemDelegate(new QItemDelegateEx());
	//QAbstractItemDelegate* x = ui.voiceList->itemDelegate();

	m_pVoices = new QStandardItemModelEx();
	ui.voiceList->setModel(m_pVoices);

	connect(ui.voiceList, SIGNAL(currentIndexChanged(int)), SLOT(on_voice_selected(int)));

	connect(ui.sliderRate, SIGNAL(valueChanged(int)), SLOT(on_rate(int)));
	connect(ui.sliderPitch, SIGNAL(valueChanged(int)), SLOT(on_pitch(int)));
	connect(ui.sliderVolume, SIGNAL(valueChanged(int)), SLOT(on_volume(int)));

	connect(ui.btnReloadSAPI, SIGNAL(clicked()), SLOT(on_reload_sapi()));

	connect(ui.chkFilter, SIGNAL(stateChanged(int)), SLOT(on_filter(int)));
	connect(m_pVoices, SIGNAL(itemChanged(QStandardItem*)), SLOT(on_filter_change(QStandardItem*)));

	m_pCfg = new QSettings(QApplication::applicationDirPath() + "/UltraTTS.ini", QSettings::IniFormat, this);

	m_SelectedVoices = m_pCfg->value("TTS_Selection").toStringList();

	m_bHold = false;

	m_bAuto = true;
	m_SectionIndex = -1;
	m_TotalLength = 0;
	m_PassedLength = 0;

	m_bGrabAndRead = false;

	m_pTTS = CTTS::NewTTS();

	CUltraTTS* that = this;
	m_pTTS->SetCallBacks([that]() {
		that->OnStop();
	}, [that](int pos, int len, int total) {
		that->OnProgress(pos, len, total);
	});

	ui.chkFilter->setCheckState((Qt::CheckState)(m_pCfg->value("TTS_Filter").toInt()));

	m_pTTS->SetRate(m_pCfg->value("TTS_Rate", 0).toInt());
	m_pTTS->SetVolume(m_pCfg->value("TTS_Volume", 100).toInt());

	ReloadVoices();

	if (m_bAuto)
	{
		QString Lang;
		const LANGID langId = GetUserDefaultUILanguage();
		WCHAR langName[1000] = { 0 };
		GetLocaleInfoW(MAKELCID(langId, SORT_DEFAULT), LOCALE_SENGLANGUAGE, langName, sizeof langName / sizeof langName[0] - 1);
		Lang = QString::fromWCharArray(langName);

		QString Speaker = m_pCfg->value(Lang + "_Speaker", "").toString();
		if (!Speaker.isEmpty())
			SelectSpeaker(Speaker);
	}

	m_pHotkeyManager = new UGlobalHotkeys(this);
	connect(m_pHotkeyManager, SIGNAL(activated(size_t)), SLOT(on_hot_key(size_t)));

	m_CbAction = eNone;
	m_CbTimeOut = 0;

	//connect(QApplication::clipboard(), SIGNAL(dataChanged()), SLOT(on_clipboard())); // this fails when vnc window is open

	SetupHKs();

	QIcon Icon;
	Icon.addFile(":/images/font.png");
	m_pTrayIcon = new QSystemTrayIcon(Icon, this);
	m_pTrayIcon->setToolTip(this->windowTitle());
	connect(m_pTrayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(on_sys_tray(QSystemTrayIcon::ActivationReason)));

	m_pTrayMenu = new QMenu();

	QAction* pRead = new QAction(tr("&Read Clipboard"), this);
	connect(pRead, SIGNAL(triggered()), this, SLOT(on_actionRead_Clipboard_triggered()));
	m_pTrayMenu->addAction(pRead);

	m_pTrayMenu->addSeparator();

	QAction* pExit = new QAction(tr("E&xit"), m_pTrayMenu);
	connect(pExit, SIGNAL(triggered()), this, SLOT(on_actionExit_triggered()));
	m_pTrayMenu->addAction(pExit);
	
	if(m_pCfg->value("SysTray", true).toBool())
		m_pTrayIcon->show();
	else
		m_pTrayIcon->hide();

	m_uTimerID = startTimer(250);
}

CUltraTTS::~CUltraTTS()
{
	if(m_pTrayIcon->isVisible())
		m_pTrayIcon->hide();

	killTimer(m_uTimerID);

	delete m_pTTS;
}

void CUltraTTS::on_sys_tray(QSystemTrayIcon::ActivationReason Reason)
{
	switch (Reason)
	{
	case QSystemTrayIcon::Context:
		m_pTrayMenu->popup(QCursor::pos());
		break;
	case QSystemTrayIcon::DoubleClick:
		this->show();
		this->setWindowState(this->windowState() & ~Qt::WindowMinimized | Qt::WindowActive);
		break;
	}
}

void CUltraTTS::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::WindowStateChange)
	{
		if (m_pCfg->value("SysTray", true).toBool()) {
			if(!m_pTrayIcon->isVisible()) // just in case
				m_pTrayIcon->show();

			if (isMinimized() == true)
				this->hide();
		}
	}

	return QMainWindow::changeEvent(event);
}

void CUltraTTS::SetupHKs()
{
	m_pHotkeyManager->unregisterAllHotkeys();

	SetupHK("CopyAndRead", HK_READ, "F1");
	SetupHK("CopyAndReadNow", HK_READ_NOW, "Alt+F1");
	SetupHK("PauseRead", HK_PAUSE, "Ctrl+F1");
	SetupHK("ReadNext", HK_NEXT, "Shift+Ctrl+F1");
	SetupHK("ReadPreviouse", HK_PREV, "Shift+Alt+F1");
	SetupHK("StopReading", HK_STOP, "Ctrl+Alt+F1");
	SetupHK("CopyAndAdd", HK_COPY, "Shift+F1");
	//SetupHK("", HK_, "Shift+Alt+Ctrl+F1");
}

void CUltraTTS::SetupHK(const QString& name, size_t id, const QString& seq)
{
	if (m_pCfg->value("UseHotKey_" + name, true).toBool())
	{
		QString HK = m_pCfg->value("HotKey_" + name, seq).toString();
		if(!HK.isEmpty())
			m_pHotkeyManager->registerHotkey(HK, id);
	}
}

void CUltraTTS::on_hot_key(size_t id)
{
	switch (id)
	{
	case HK_READ: GrabAndRead(false); break;
	case HK_READ_NOW: GrabAndRead(true); break;
	case HK_PAUSE: on_actionPause_triggered(); break;
	case HK_NEXT: PlayNext(); break;
	case HK_PREV: PlayPrev();  break;
	case HK_STOP: on_actionStop_triggered(); break;
	case HK_COPY: GrabAndInsert();  break; 
	}
}

void CUltraTTS::on_close_tab(int Index)
{
	STab* pTab = m_Tabs.takeAt(Index);
	m_pTabs->removeTab(Index);
	if (m_pReadEdit == pTab->pEdit);
		m_pReadEdit = NULL;
	pTab->pEdit->deleteLater();
	delete pTab;
	if (m_Tabs.isEmpty())
		on_actionNew_triggered();
}

void CUltraTTS::on_tab(int Index)
{
	m_pTab = Index < 0 ? NULL : m_Tabs[Index];
}

void CUltraTTS::on_reload_sapi()
{
	ReloadVoices();
}

void CUltraTTS::ReloadVoices()
{
	QStringList Stores = m_pCfg->value("TTS_Stores").toStringList();
	if (Stores.isEmpty())
	{
		Stores.append(QString::fromWCharArray(CTTSApi::Voices_Cortana));
		Stores.append(QString::fromWCharArray(CTTSApi::Voices_OneCore));
		Stores.append(QString::fromWCharArray(CTTSApi::Voices_Platform));
		Stores.append(QString::fromWCharArray(CTTSApi::Voices_Speech));

		m_pCfg->setValue("TTS_Stores", Stores);
	}
	m_pCfg->sync();


	m_pTTS->ClearVoices();
	foreach(const QString& Store, Stores)
		m_pTTS->EnumVoices(Store.toStdWString());
	
	if(m_pCfg->value("TTS_Sapi4", true).toBool())
		m_pTTS->EnumVoices();

	m_Voices = m_pTTS->GetVoices();

	on_filter(ui.chkFilter->checkState());
}

void CUltraTTS::on_voice_selected(int index)
{
	if (index == -1 || m_bHold)
		return;

	QStandardItem* item = m_pVoices->item(index);
	if (!item)
		return;

	index = item->data(Qt::UserRole).toInt();

	if (index == -1) {
		m_bAuto = true;
		return;
	}
	m_bAuto = false;

	SVoice* voice = m_Voices[index];

	m_pCfg->setValue(QString::fromStdWString(voice->Lang) + "_Speaker", QString::fromStdWString(voice->Name));

	SelectSpeaker(QString::fromStdWString(voice->Name));
}

bool CUltraTTS::SelectSpeaker(const QString& Speaker)
{
	if (!m_pTTS->SelectVoice(Speaker.toStdWString()))
		return false;

	m_bHold = true;

	ui.statusBar->showMessage(Speaker);
	
	ui.sliderRate->setRange(m_pTTS->GetMinRate(), m_pTTS->GetMaxRate());
	ui.sliderRate->setValue(m_pTTS->GetRate());
	ui.minRate->setText(QString::number(m_pTTS->GetMinRate()));
	ui.curRate->setText(QString::number(m_pTTS->GetRate()));
	ui.maxRate->setText(QString::number(m_pTTS->GetMaxRate()));

	ui.sliderPitch->setRange(m_pTTS->GetMinPitch(), m_pTTS->GetMaxPitch());
	ui.sliderPitch->setValue(m_pTTS->GetPitch());
	ui.minPitch->setText(QString::number(m_pTTS->GetMinPitch()));
	ui.curPitch->setText(QString::number(m_pTTS->GetPitch()));
	ui.maxPitch->setText(QString::number(m_pTTS->GetMaxPitch()));
	ui.sliderPitch->setEnabled(m_pTTS->GetMinPitch() != m_pTTS->GetMaxPitch());

	ui.sliderVolume->setRange(m_pTTS->GetMinVolume(), m_pTTS->GetMaxVolume());
	ui.sliderVolume->setValue(m_pTTS->GetVolume());

	m_bHold = false;

	return true;
}

void CUltraTTS::on_rate(int index)
{
	if (m_bHold)
		return;
	m_pCfg->setValue("TTS_Rate", index);
	m_pTTS->SetRate(index);
	ui.curRate->setText(QString::number(m_pTTS->GetRate()));
}

void CUltraTTS::on_pitch(int index)
{
	if (m_bHold)
		return;
	m_pTTS->SetPitch(index);
	ui.curPitch->setText(QString::number(m_pTTS->GetPitch()));
}

void CUltraTTS::on_volume(int index)
{
	if (m_bHold)
		return;
	m_pCfg->setValue("TTS_Volume", index);
	m_pTTS->SetVolume(index);
	//ui.curVolume->setText(QString::number(m_pTTS->GetVolume()));
}

void CUltraTTS::on_filter(int state)
{
	m_bHold = true;

	m_pCfg->setValue("TTS_Filter", state);

	QMap<QString, QList<int> > LangMap;
	for (int i = 0; i < m_Voices.size(); i++) {
		SVoice* pVoice = m_Voices[i];

		if (state == Qt::Checked)
		{
			if (!m_SelectedVoices.contains(QString::fromStdWString(m_Voices[i]->Name)))
				continue;
		}

		LangMap[QString::fromStdWString(pVoice->Lang)].append(i);
	}
	
	m_pVoices->clear();

	QStandardItem* item = new QStandardItem(tr("Auto"));
	item->setData(-1, Qt::UserRole);
	m_pVoices->appendRow(item);

	foreach(const QString& Lang, LangMap.keys())
	{
		m_pVoices->addParentItem(Lang);
		foreach(int i, LangMap[Lang])
		{
			SVoice* pVoice = m_Voices[i];

			QString name = QString::fromStdWString(m_Voices[i]->Name);
			if (state == Qt::PartiallyChecked)
				m_pVoices->addChildItem(name, m_SelectedVoices.contains(name) ? Qt::Checked : Qt::Unchecked, i);
			else
				m_pVoices->addChildItem(name, i);
		}
	}

	/*switch(state)
	{
	case Qt::Unchecked:
		for(int i=0; i < m_Voices.size(); i++){
			QStandardItem* item = new QStandardItem(QString::fromStdWString(m_Voices[i]->Name));
			item->setData(i, Qt::UserRole);
			m_pVoices->setItem(m_pVoices->rowCount(), 0, item);
		}
		break;
	case Qt::PartiallyChecked:
		for (int i = 0; i < m_Voices.size(); i++) {
			QString name = QString::fromStdWString(m_Voices[i]->Name);
			QStandardItem* item = new QStandardItem(name);
			item->setData(i, Qt::UserRole);
			item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
			item->setData(m_SelectedVoices.contains(name) ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole);
			m_pVoices->setItem(m_pVoices->rowCount(), 0, item);
		}
		break;
	case Qt::Checked:
		for (int i = 0; i < m_Voices.size(); i++) {
			QString name = QString::fromStdWString(m_Voices[i]->Name);
			if (m_SelectedVoices.contains(name))
			{
				QStandardItem* item = new QStandardItem(name);
				item->setData(i, Qt::UserRole);
				m_pVoices->setItem(m_pVoices->rowCount(), 0, item);
			}
		}
		break;
	}*/

	m_bHold = false;
}

void CUltraTTS::on_filter_change(QStandardItem* item)
{
	if (m_bHold)
		return;

	int index = item->data(Qt::UserRole).toInt();
	int state = item->data(Qt::CheckStateRole).toInt();

	SVoice* voice = m_Voices[index];

	QString name = QString::fromStdWString(voice->Name);
	if(state == Qt::Unchecked)
		m_SelectedVoices.removeAll(name);
	else if (!m_SelectedVoices.contains(name))
		m_SelectedVoices.append(name);

	m_pCfg->setValue("TTS_Selection", m_SelectedVoices);
}

void CUltraTTS::OnStop()
{
	if (m_SectionIndex != -1) {
		if (SpeakNext())
			return;
	}

	m_pProgress->setText("100%");
	m_pTrayIcon->setToolTip(this->windowTitle());

	if (m_pReadEdit != NULL)
	{
		/*QTextCursor tmpCursor = m_pReadEdit->textCursor();
		tmpCursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
		m_pReadEdit->setTextCursor(tmpCursor);*/
		m_pReadEdit->setReadOnly(false);
		m_pReadEdit = NULL;
	}

	ui.actionReadAll->setEnabled(true);
	ui.actionRead->setEnabled(true);
	ui.actionPause->setEnabled(false);
	ui.actionStop->setEnabled(false);

	if (m_bGrabAndRead)
	{
		m_bGrabAndRead = false;
		GrabAndRead(true);
	}
}

void CUltraTTS::OnProgress(int pos, int len, int total)
{
	QString temp = QString::number(m_TotalLength ? 100 * (m_PassedLength + pos) / m_TotalLength : 0) + "%";
	m_pProgress->setText(temp);
	m_pTrayIcon->setToolTip(this->windowTitle() + " " + temp);

	if (m_pReadEdit == edit())
	{
		m_pReadEdit->setFocus();

		pos = m_PassedLength + pos + m_SelectionOffset;
		QTextCursor tmpCursor = edit()->textCursor();
		//tmpCursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, pos);
		tmpCursor.setPosition(pos);
		if (len == 0) // sapi 4
		{
			QString text = edit()->toPlainText().mid(pos);
			int tmp = text.indexOf(QRegExp("[\\s.,:;-_+*#/?\"'§$%&{}()\\[\\]=]"));
			if(tmp != -1)
				len = tmp;
		}
		tmpCursor.setPosition(pos + len, QTextCursor::KeepAnchor);
		m_pReadEdit->setTextCursor(tmpCursor);
	}
}

void CUltraTTS::on_actionNew_triggered()
{
	STab* pTab = new STab();
	pTab->pEdit = new QTextEdit();
	connect(pTab->pEdit, SIGNAL(textChanged()), SLOT(on_text_changed()));
	
	m_Tabs.append(pTab);
	m_pTabs->addTab(pTab->pEdit, pTab->name());
	if (m_pTab == NULL)
		m_pTab = pTab; // thats teh first tab
	else
		m_pTabs->setCurrentIndex(m_pTabs->count() - 1);
}

void CUltraTTS::on_text_changed()
{
	if (m_bHold)
		return;

	if (m_pTab->bChanged == false) {
		m_pTab->bChanged = true;
		m_pTabs->setTabText(m_pTabs->currentIndex(), m_pTab->name() + " *");
	}
}

void CUltraTTS::on_actionOpen_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open the file");
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, "Warning", "Cannot open file: " + file.errorString());
        return;
    }
	m_pTab->setFileName(fileName);
	m_pTabs->setTabText(m_pTabs->currentIndex(), m_pTab->name());
    //setWindowTitle(fileName);
    QTextStream in(&file);
    QString text = in.readAll();
	m_bHold = true;
	edit()->setText(text);
	m_bHold = false;
    file.close();
	m_pTab->bChanged = false;
}

void CUltraTTS::on_actionSave_triggered()
{
    QString fileName;
    // If we don't have a filename from before, get one.
	if (m_pTab->fileName.isEmpty()) {
		fileName = QFileDialog::getSaveFileName(this, "Save");
		m_pTab->setFileName(fileName);
	}
	else
		fileName = m_pTab->fileName;
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, "Warning", "Cannot save file: " + file.errorString());
        return;
    }
	m_pTabs->setTabText(m_pTabs->currentIndex(), m_pTab->name());
    //setWindowTitle(fileName);
    QTextStream out(&file);
    QString text = edit()->toPlainText();
    out << text;
    file.close();
	m_pTab->bChanged = false;
}

void CUltraTTS::on_actionSave_as_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save as");
    QFile file(fileName);

    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, "Warning", "Cannot save file: " + file.errorString());
        return;
    }
	m_pTab->setFileName(fileName);
	m_pTabs->setTabText(m_pTabs->currentIndex(), m_pTab->name());
    //setWindowTitle(fileName);
    QTextStream out(&file);
    QString text = edit()->toPlainText();
    out << text;
    file.close();
	m_pTab->bChanged = false;
}

void CUltraTTS::on_actionPrint_triggered()
{
#if QT_CONFIG(printer)
    QPrinter printDev;
#if QT_CONFIG(printdialog)
    QPrintDialog dialog(&printDev, this);
    if (dialog.exec() == QDialog::Rejected)
        return;
#endif // QT_CONFIG(printdialog)
	edit()->print(&printDev);
#endif // QT_CONFIG(printer)
}

void CUltraTTS::on_actionExit_triggered()
{
    QCoreApplication::quit();
}

void CUltraTTS::on_actionCopy_triggered()
{
	edit()->copy();
}

void CUltraTTS::on_actionCut_triggered()
{
	edit()->cut();
}

void CUltraTTS::on_actionPaste_triggered()
{
	edit()->paste();
}

void CUltraTTS::on_actionUndo_triggered()
{
	edit()->undo();
}

void CUltraTTS::on_actionRedo_triggered()
{
	edit()->redo();
}

void CUltraTTS::on_actionFont_triggered()
{
    bool fontSelected;
    QFont font = QFontDialog::getFont(&fontSelected, this);
    if (fontSelected)
		edit()->setFont(font);
}

void CUltraTTS::on_actionUnderline_triggered()
{
	edit()->setFontUnderline(ui.actionUnderline->isChecked());
}

void CUltraTTS::on_actionItalic_triggered()
{
	edit()->setFontItalic(ui.actionItalic->isChecked());
}

void CUltraTTS::on_actionBold_triggered()
{
    ui.actionBold->isChecked() ? edit()->setFontWeight(QFont::Bold) : edit()->setFontWeight(QFont::Normal);
}

void CUltraTTS::on_actionAbout_triggered()
{
   QMessageBox::about(this, tr("About UltraTTS"), tr("<b>UltraTTS</b> Copyright (c) 2019"));
}

void CUltraTTS::on_actionReadAll_triggered()
{
	QString text = edit()->toPlainText();
	m_SelectionOffset = 0;
	m_pReadEdit = edit();
	m_pReadEdit->setReadOnly(true);
	Speak(text, false);
}

void CUltraTTS::on_actionRead_triggered()
{
	QTextCursor cursor = edit()->textCursor();
	int start = cursor.selectionStart();
	int end = cursor.selectionEnd();

	QString text = edit()->toPlainText();
	m_SelectionOffset = start;
	if (end > start)
		text = text.mid(start, end - start);
	else
		text = text.mid(start);
	m_pReadEdit = edit();
	m_pReadEdit->setReadOnly(true);
	Speak(text, false);
}

void CUltraTTS::on_actionPause_triggered()
{
	m_pTTS->Pause();
	ui.actionPause->setChecked(m_pTTS->IsPaused());
}

void CUltraTTS::on_actionStop_triggered()
{
	Stop();
}

void CUltraTTS::on_actionRead_Clipboard_triggered()
{
	QString text = QApplication::clipboard()->text();
	Speak(text, false);
}

void CUltraTTS::GrabCB(ECbAction CbAction)
{
	m_CbTimeOut = QDateTime::currentDateTime().toTime_t() + 3;

	m_old_text = QApplication::clipboard()->text();
	QApplication::clipboard()->clear();

	m_CbAction = CbAction;

	//QTimer::singleShot(500, this, SLOT(on_send_ctrl()));
}

void CUltraTTS::on_send_ctrl()
{
	//SendCtrlC();
}

void CUltraTTS::on_clipboard()
{
	if (m_CbAction == eNone)
		return;

	QString text = QApplication::clipboard()->text();
	if (text.isEmpty())
		return;

	m_CbTimeOut = 0;

	if(m_pCfg->value("PlaySound", true).toBool())
		QApplication::beep();

	switch (m_CbAction)
	{
	case eReadNow:
	case eRead:
		ReadCB(text, m_CbAction == eReadNow);
		break;
	case eInsert:
		InsertCB(text);
		break;
	}
	m_CbAction = eNone;

	QApplication::clipboard()->setText(m_old_text);
}

void CUltraTTS::GrabAndInsert()
{
	GrabCB(eInsert);
}

QString CUltraTTS::InsertCB(QString& text, bool bClear)
{	
	m_bHold = true;
	//edit()->setText(text);

	QTextDocument *doc = edit()->document();
	QTextCursor curs(doc);
	if (bClear)
		curs.select(QTextCursor::Document);
	else
	{
		text = "\n\n" + text;
		curs.movePosition(QTextCursor::End);
	}
	//curs.removeSelectedText();
	curs.insertText(text);

	m_bHold = false;
	m_pTab->bChanged = false;

	return text;
}

void CUltraTTS::ReadCB(QString& text, bool bNow)
{
	// Get CB reader tab
	if (!m_pTab->bReader) {
		int FoundIndex = -1;
		for(int i = 0; i < m_Tabs.size(); i++)
		{
			if (m_Tabs[i]->bReader)
			{
				FoundIndex = i;
				break;
			}
		}

		if (FoundIndex != -1)
			m_pTabs->setCurrentIndex(FoundIndex);
		else
		{
			if (!m_pTab->fileName.isEmpty() || !m_pTab->pEdit->toPlainText().isEmpty())
				on_actionNew_triggered();

			m_pTab->bReader = true;
			m_pTabs->setTabText(m_pTabs->currentIndex(), m_pTab->name());
		}
	}
	//

	if (!m_pTTS->IsSpeaking())
		bNow = true;

	InsertCB(text, bNow);

	m_SelectionOffset = 0;
	m_pReadEdit = edit();
	m_pReadEdit->setReadOnly(true);
	Speak(text, !bNow);
}

void CUltraTTS::Speak(const QString& String, bool bAdd)
{
	ui.actionReadAll->setEnabled(false);
	ui.actionRead->setEnabled(false);
	ui.actionPause->setEnabled(true);
	ui.actionStop->setEnabled(true);

	if (!bAdd)
		m_Sections.clear();
	
	STextSection Section;
	Section.Delay = m_pTTS->IsSpeaking() ? m_pCfg->value("SectionDelay", 750).toInt() : 0;

	if (!m_bAuto)
	{
		Section.Text = String;
		Section.Lang = "Unknown";
		m_Sections.append(Section);
	}
	else
	{
		QStringList StringList;
		for (int Pos = 0;;)
		{
			int Sep = String.indexOf(QRegExp("[.!?\\r\\n]{1}"), Pos);
			if (Sep != -1)
			{
				Sep++;
				StringList.append(String.mid(Pos, Sep - Pos));
				Pos = Sep;
			}
			else
			{
				StringList.append(String.mid(Pos));
				break;
			}
		}

		Section.Text = "";
		Section.Lang = "Unknown";
		m_Sections.append(Section);

		int LangHint = 0;

		foreach(QString Text, StringList)
		{
			QPair<QString, int> Lang = DetectLanguage(Text, &LangHint);
			if (Lang.first != "Unknown" && Lang.first != m_Sections.last().Lang)
			{
				if (m_Sections.last().Lang == "Unknown")
					m_Sections.last().Lang = Lang.first;
				else if (Text.length() > 50) {
					Section.Text = "";
					Section.Lang = Lang.first;
					Section.Delay = 0;
					m_Sections.append(Section);
				}
			}
			m_Sections.last().Text += Text;
		}
	}

	if (m_pTTS->IsSpeaking())
	{
		m_TotalLength += String.length();
	}
	else
	{
		m_SectionIndex = -1;

		m_TotalLength = String.length();
		m_PassedLength = 0;

		SpeakNext();
	}
}

bool CUltraTTS::SpeakNext()
{
	if(m_SectionIndex >= 0 && m_SectionIndex < m_Sections.length())
		m_PassedLength += m_Sections[m_SectionIndex].Text.length();

	if (++m_SectionIndex >= m_Sections.size())
		return false;
	
	QTimer::singleShot(m_Sections[m_SectionIndex].Delay, this, SLOT(on_speak_next()));
	
	return true;
}

void CUltraTTS::on_speak_next()
{
	QString Speaker = m_pCfg->value(m_Sections[m_SectionIndex].Lang + "_Speaker", "").toString();
	if (!Speaker.isEmpty())
		SelectSpeaker(Speaker);

	m_pTTS->Speak(m_Sections[m_SectionIndex].Text.toStdWString());
}

void CUltraTTS::PlayNext()
{
	m_pTTS->Stop();
}

void CUltraTTS::PlayPrev()
{
	if (m_SectionIndex > 0)
		m_SectionIndex--;
	m_pTTS->Stop();
}

void CUltraTTS::Stop()
{
	m_SectionIndex = -1;
	m_pTTS->Stop();
}

void CUltraTTS::GrabAndRead(bool bNow)
{
	if (bNow && m_pTTS->IsSpeaking())
	{
		m_bGrabAndRead = true;
		Stop();
		return;
	}

	GrabCB(bNow ? eReadNow : eRead);
}

void CUltraTTS::on_actionConfiguration_triggered()
{
	COptionsDlg dlg(m_pCfg);
	if (dlg.exec() == QDialog::Accepted) {
		SetupHKs();

		if (m_pCfg->value("SysTray", true).toBool()) {
			if (!m_pTrayIcon->isVisible())
				m_pTrayIcon->show();
		}
		else {
			if (m_pTrayIcon->isVisible())
				m_pTrayIcon->hide();
		}
	}
}

void CUltraTTS::Process()
{
	if (m_CbAction != eNone)
	{
		if (m_CbTimeOut >= QDateTime::currentDateTime().toTime_t())
		{
			SendCtrlC();

			on_clipboard();
		}
		else
		{
			m_CbTimeOut = 0;

			QApplication::clipboard()->setText(m_old_text);
		}
	}
}