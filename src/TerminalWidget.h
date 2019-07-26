#pragma once

#include <QtWidgets/QWidget>
#include <QtGui/QKeyEvent>
#include <QApplication>
#include <QtCore/QTimer>
#include <QtCore/QRegularExpression>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

#include "ui_scpspt.h"


class TerminalWidget : public QWidget, private Ui::TerminalWidget
{
	Q_OBJECT

public:
    struct StringValue {
        const char * string;
        const int value;
    };

    static const StringValue Baudrates[];
    static const StringValue Databits[];
    static const StringValue Parity[];
    static const StringValue Stopbits[];
    static const StringValue Flowcontrol[];

    TerminalWidget(QWidget *parent = nullptr);
    ~TerminalWidget(void);

protected:
	bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void updatePorts();
    void portSelectionChanged(const QString & text);
    void connectToPort();
    void disconnectFromPort();
	void inputTextChanged();
	void inputFormatchanged();
	void clearTxEcho();
	void clearRxEcho();
	void historyEntrySelected();
    void sendText();
    void receiveText();

private:
	const char * m_version = "1.0";
    QSerialPortInfo m_portInfo;
    QSerialPort m_port;
    QTimer m_timer;
	QIntValidator * m_baudRateValidator;
	QStringList m_sendHistory;

	QRegularExpression m_hexTextExp;
	QRegularExpression m_binTextExp;
    QRegularExpression m_consoleColorExp;

    static const QString HexTextExpString;
    static const QString BinTextExpString;
    static const QString ConsoleColorExpString;

    void enablePortSelection(const bool enable);
    void enableConnect(const bool enable);
    void enableRx(const bool enable);
	void enableTx(const bool enable);
};
