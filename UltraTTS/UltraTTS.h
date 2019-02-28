#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_UltraTTS.h"

class QItemDelegateEx : public QItemDelegate
{
	Q_OBJECT

public:
	QItemDelegateEx(QObject *parent = nullptr) : QItemDelegate(parent)
	{
	}

	QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
	{
		QString type = index.data(Qt::AccessibleDescriptionRole).toString();
		if (type == QLatin1String("separator"))
			return QSize(5, 5);
		return QItemDelegate::sizeHint(option, index);
	}

	void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
	{
		QString type = index.data(Qt::AccessibleDescriptionRole).toString();
		if (type == QLatin1String("separator")) {
			QItemDelegate::paint(painter, option, index);
			int y = (option.rect.top() + option.rect.bottom()) / 2;
			painter->setPen(option.palette.color(QPalette::Active, QPalette::Dark));
			painter->drawLine(option.rect.left(), y, option.rect.right(), y);
		}
		else if (type == QLatin1String("parent")) {
			QStyleOptionViewItem parentOption = option;
			parentOption.state |= QStyle::State_Enabled;
			QItemDelegate::paint(painter, parentOption, index);
		}
		else if (type == QLatin1String("child")) {
			QStyleOptionViewItem childOption = option;
			int indent = 0;
			if((index.flags() & Qt::ItemIsUserCheckable) == 0)
				indent = option.fontMetrics.width(QString(4, QChar(' ')));
			childOption.rect.adjust(indent, 0, 0, 0);
			childOption.textElideMode = Qt::ElideNone;
			QItemDelegate::paint(painter, childOption, index);
		}
		else {
			QItemDelegate::paint(painter, option, index);
		}
	}
};

class QStandardItemModelEx: public QStandardItemModel
{
	Q_OBJECT
public:

	void addParentItem(const QString& text, const QVariant& data = QVariant())
	{
		QStandardItem* item = new QStandardItem(text);
		item->setData(data, Qt::UserRole);
		item->setFlags(item->flags() & ~(Qt::ItemIsEnabled | Qt::ItemIsSelectable));
		item->setData("parent", Qt::AccessibleDescriptionRole);

		QFont font = item->font();
		font.setBold(true);
		item->setFont(font);

		//QStandardItemModel* itemModel = (QStandardItemModel*)model();
		this->appendRow(item);
	}

	void addChildItem(const QString& text, const QVariant& data = QVariant())
	{
		QStandardItem* item = new QStandardItem(text + QString(4, QChar(' ')));
		item->setData(data, Qt::UserRole);
		item->setData("child", Qt::AccessibleDescriptionRole);

		//QStandardItemModel* itemModel = (QStandardItemModel*)model();
		this->appendRow(item);
	}

	void addChildItem(const QString& text, Qt::CheckState state, const QVariant& data = QVariant())
	{
		QStandardItem* item = new QStandardItem(text + QString(4, QChar(' ')));
		item->setData(data, Qt::UserRole);
		item->setData("child", Qt::AccessibleDescriptionRole);

		item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
		item->setData(state, Qt::CheckStateRole);

		//QStandardItemModel* itemModel = (QStandardItemModel*)model();
		this->appendRow(item);
	}
};

class CUltraTTS : public QMainWindow
{
	Q_OBJECT

public:
	CUltraTTS(QWidget *parent = Q_NULLPTR);
	~CUltraTTS();

	void GrabAndRead(bool bNow);
	void GrabAndInsert();

	void OnStop();
	void OnProgress(int pos, int len, int total);

private slots:
    void on_actionNew_triggered();
    void on_actionOpen_triggered();
    void on_actionSave_triggered();
    void on_actionSave_as_triggered();
    void on_actionPrint_triggered();
    void on_actionExit_triggered();
    void on_actionCopy_triggered();
    void on_actionCut_triggered();
    void on_actionPaste_triggered();
    void on_actionUndo_triggered();
    void on_actionRedo_triggered();
    void on_actionFont_triggered();
    void on_actionBold_triggered();
    void on_actionUnderline_triggered();
    void on_actionItalic_triggered();
    void on_actionAbout_triggered();
	void on_actionConfiguration_triggered();

	void on_actionReadAll_triggered();
	void on_actionRead_triggered();
	void on_actionPause_triggered();
	void on_actionStop_triggered();
	void on_actionRead_Clipboard_triggered();

	void on_voice_selected(int index);
	void on_rate(int index);
	void on_pitch(int index);
	void on_volume(int index);

	void on_reload_sapi();
	void on_filter(int state);
	void on_filter_change(QStandardItem *item);

	void on_close_tab(int);
	void on_tab(int);

	void on_text_changed();

	void on_hot_key(size_t id);

	void on_clipboard();

	void on_send_ctrl();

	void on_speak_next();

	void on_sys_tray(QSystemTrayIcon::ActivationReason Reason);

private:
	QTextEdit* edit() { return m_pTab->pEdit; }

	Ui::UltraTTSClass ui;

	QStandardItemModelEx* m_pVoices;

	QTabWidget* m_pTabs;

	QLabel* m_pProgress;

	struct STab
	{
		STab():bReader(false), bChanged(false) {}
		QTextEdit* pEdit;
		QString fileName;
		bool bReader;
		bool bChanged;
		QString name();
		void setFileName(const QString& fileName) {
			this->fileName = fileName;
			bReader = false;
		}
	};

	QList<STab*> m_Tabs;
	STab* m_pTab;

	QTextEdit* m_pReadEdit;

	class UGlobalHotkeys* m_pHotkeyManager;

	QSystemTrayIcon*	m_pTrayIcon;
	QMenu*				m_pTrayMenu;

protected:
	void				Process();

	void				timerEvent(QTimerEvent* pEvent)
	{
		if (pEvent->timerId() == m_uTimerID)
			Process();
	}

	int					m_uTimerID;

	void changeEvent(QEvent *event);

	void SetupHKs();
	void SetupHK(const QString& name, size_t id, const QString& seq = "");
	void ReloadVoices();
	void ReadCB(QString& text, bool bNow);
	void Speak(const QString& Text, bool bAdd);
	bool SpeakNext();
	bool SelectSpeaker(const QString& Speaker);
	void Stop();
	void PlayNext();
	void PlayPrev();
	QString InsertCB(QString& text, bool bClear = false);

	class CTTS* m_pTTS;

	std::vector<struct SVoice*> m_Voices;
	QStringList m_SelectedVoices;

	QSettings*	m_pCfg;

	bool		m_bHold;

	bool		m_bAuto;
	struct STextSection
	{
		QString Text;
		QString Lang;
		int Delay;
	};
	QList<STextSection> m_Sections;
	int			m_SectionIndex;
	int			m_TotalLength;
	int			m_PassedLength;
	int			m_SelectionOffset;

	bool		m_bGrabAndRead;

	enum ECbAction
	{
		eNone = 0,
		eReadNow,
		eRead,
		eInsert
	};

	ECbAction m_CbAction;

	time_t m_CbTimeOut;

	QString m_old_text;

	void GrabCB(ECbAction CbAction);
};
