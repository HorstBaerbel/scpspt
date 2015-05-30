#pragma once

#include <QtWidgets/QWidget>
#include <QtGui/QKeyEvent>
#include <QApplication>
#include <QtCore/QTimer>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

#include "ui_scpspt.h"


class TerminalWidget : public QWidget, private Ui::TerminalWidget
{
	Q_OBJECT

	const char * m_version = "0.7";
    QSerialPortInfo m_portInfo;
    QSerialPort m_port;
    QTimer m_timer;
	QIntValidator * m_baudRateValidator;

	QRegExp m_hexTextExp;
	QRegExp m_binTextExp;

    void enablePortSelection(const bool enable);
    void enableConnect(const bool enable);
    void enableRx(const bool enable);
	void enableTx(const bool enable);

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

private slots:
    void updatePorts();
    void portSelectionChanged(const QString & text);
    void connectToPort();
    void disconnectFromPort();
	void inputTextChanged();
	void inputFormatchanged();

    void sendText();
    void receiveText();
};
