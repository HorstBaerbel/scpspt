#include "TerminalWidget.h"

#include <QtWidgets/QMessageBox>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QMenu>
#include <inttypes.h>


const TerminalWidget::StringValue TerminalWidget::Baudrates[] = {
	{ "110", 110 }, { "300", 300 }, { "600", 600 }, { "1200", 1200 }, { "2400", 2400 }, { "4800", 4800 }, { "9600", 9600 }, 
	{ "19200", 19200 }, { "38400", 38400 }, { "57600", 57600 }, { "115200", 115200 }, 
	{ "230400", 230400 }, { "460800", 460800 }, { "921600", 921600 }, { "1382400", 1382400 }, { nullptr, 0 } };
const TerminalWidget::StringValue TerminalWidget::Databits[] = { 
	{ "5", QSerialPort::Data5 }, { "6", QSerialPort::Data6 }, { "7", QSerialPort::Data7 }, { "8", QSerialPort::Data8 }, { nullptr, 0 } };
const TerminalWidget::StringValue TerminalWidget::Parity[] = { 
	{ "None", QSerialPort::NoParity }, { "Even", QSerialPort::EvenParity }, { "Odd", QSerialPort::OddParity }, 
	{ "Space", QSerialPort::SpaceParity }, { "Mark", QSerialPort::MarkParity }, { nullptr, 0 } };
const TerminalWidget::StringValue TerminalWidget::Stopbits[] = { 
	{ "1", QSerialPort::OneStop }, { "2", QSerialPort::TwoStop }, { nullptr, 0 } };
const TerminalWidget::StringValue TerminalWidget::Flowcontrol[] = {
	{ "None", QSerialPort::NoFlowControl }, { "Hardware (RTS/CTS)", QSerialPort::HardwareControl }, { "Software (XON/XOFF)", QSerialPort::SoftwareControl }, { nullptr, 0 } };

const QString TerminalWidget::HexTextExpString = QStringLiteral("\\b[0-9a-fA-F]{2}\\b");
const QString TerminalWidget::BinTextExpString = QStringLiteral("\\b[0-1]{8}\\b");
const QString TerminalWidget::ConsoleColorExpString = QStringLiteral("\\e\\[(\\d);?(\\d*)m");

void fillComboBox(QComboBox * comboBox, const TerminalWidget::StringValue list[])
{
    comboBox->clear();
    int i = 0;
    while(list[i].string != nullptr) {
        comboBox->addItem(list[i].string, list[i].value);
        i++;
    }
}

TerminalWidget::TerminalWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);
	//install event filter on send text input for history popup
	plainTextEditTx->installEventFilter(this);
	//add version to title
	setWindowTitle(windowTitle() + " v" + m_version);
	//set up regular expressions for hex and binary matching
	m_hexTextExp.setPattern(HexTextExpString);
	//m_hexTextExp.setMinimal(true);
	m_binTextExp.setPattern(BinTextExpString);
	//m_binTextExp.setMinimal(true);
	m_consoleColorExp.setPattern(ConsoleColorExpString);
    //fill baudrate information etc.
    fillComboBox(comboBoxBaudrate, Baudrates);
    fillComboBox(comboBoxDatabits, Databits);
    fillComboBox(comboBoxParity, Parity);
    fillComboBox(comboBoxStopbits, Stopbits);
    fillComboBox(comboBoxFlowcontrol, Flowcontrol);
	//restrict baud rate combobox to integers
	m_baudRateValidator = new QIntValidator(110, 1382400, this);
	comboBoxBaudrate->setValidator(m_baudRateValidator);
	//set baud rate to a default of 9600
	comboBoxBaudrate->setCurrentIndex(6);
	//set stop bits to default of 8
	comboBoxDatabits->setCurrentIndex(3);
	//connect signals for port settings
    connect(comboBoxPort, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(portSelectionChanged(const QString &)));
	connect(pbUpdatePorts, SIGNAL(clicked(bool)), this, SLOT(updatePorts()));
    connect(pushButtonConnect, SIGNAL(clicked(bool)), this, SLOT(connectToPort()));
    connect(pushButtonDisconnect, SIGNAL(clicked(bool)), this, SLOT(disconnectFromPort()));
    connect(pushButtonSend, SIGNAL(clicked(bool)), this, SLOT(sendText()));
	//connect signals for TX text
	connect(plainTextEditTx, SIGNAL(textChanged()), this, SLOT(inputTextChanged()));
	connect(rbTxText, SIGNAL(toggled(bool)), this, SLOT(inputFormatchanged()));
	connect(rbTxHex, SIGNAL(toggled(bool)), this, SLOT(inputFormatchanged()));
	connect(rbTxBin, SIGNAL(toggled(bool)), this, SLOT(inputFormatchanged()));
	//connect signals from clear buttons
	connect(pbClearTx, SIGNAL(clicked(bool)), this, SLOT(clearTxEcho()));
	connect(pbClearRx, SIGNAL(clicked(bool)), this, SLOT(clearRxEcho()));
	//fill serial port list
    updatePorts();
}

TerminalWidget::~TerminalWidget(void)
{
	disconnectFromPort();
}

void TerminalWidget::clearTxEcho()
{
	plainTextEditTxPrevious->clear();
}

void TerminalWidget::clearRxEcho()
{
	plainTextEditRx->clear();
}

void TerminalWidget::updatePorts()
{
    //remember selected port
    QString selected = comboBoxPort->currentText();
    comboBoxPort->clear();
    //get port info
    int newIndex = 0;
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    for (int i = 0; i < ports.size(); i++)
	{
		const QSerialPortInfo pi = ports.at(i);
        comboBoxPort->addItem(pi.portName());
        //check if this is the port we've selected before. remember index
        if (pi.portName() == selected) {
            newIndex = i;
        }
    }
    comboBoxPort->setCurrentIndex(newIndex);
}

void TerminalWidget::portSelectionChanged(const QString & text)
{
}

//--------------------------------------------------------------------------------------

void TerminalWidget::connectToPort()
{
    if (!m_port.isOpen()) {
        m_port.setPortName(comboBoxPort->currentText());
        const bool openedOk = m_port.open(QIODevice::ReadWrite);
        if (openedOk) {
            m_port.setBaudRate(comboBoxBaudrate->itemData(comboBoxBaudrate->currentIndex()).toInt());
            m_port.setDataBits((QSerialPort::DataBits)comboBoxDatabits->itemData(comboBoxDatabits->currentIndex()).toInt());
		    m_port.setParity((QSerialPort::Parity)comboBoxParity->itemData(comboBoxParity->currentIndex()).toInt());
		    m_port.setStopBits((QSerialPort::StopBits)comboBoxStopbits->itemData(comboBoxStopbits->currentIndex()).toInt());
		    m_port.setFlowControl((QSerialPort::FlowControl)comboBoxFlowcontrol->itemData(comboBoxFlowcontrol->currentIndex()).toInt());
			connect(&m_port, SIGNAL(readyRead()), this, SLOT(receiveText()));
        }
        enableConnect(!openedOk);
        enablePortSelection(!openedOk);
        enableRx(openedOk);
		enableTx(openedOk);
    }
}

void TerminalWidget::disconnectFromPort()
{
	if (m_port.isOpen()) {
		disconnect(&m_port, SIGNAL(readyRead()), this, SLOT(receiveText()));
		m_port.close();
		m_port.setPortName("");
        enableConnect(true);
        enablePortSelection(true);
        enableRx(false);
		enableTx(false);
    }
}

//--------------------------------------------------------------------------------------

void TerminalWidget::enablePortSelection(const bool enable)
{
	pbUpdatePorts->setEnabled(enable);
    comboBoxPort->setEnabled(enable);
    comboBoxBaudrate->setEnabled(enable);
    comboBoxDatabits->setEnabled(enable);
    comboBoxParity->setEnabled(enable);
    comboBoxStopbits->setEnabled(enable);
    comboBoxFlowcontrol->setEnabled(enable);
}

void TerminalWidget::enableConnect(const bool enable)
{
    pushButtonConnect->setEnabled(enable);
    pushButtonDisconnect->setEnabled(!enable);
}

void TerminalWidget::enableRx(const bool enable)
{
	rbRxText->setEnabled(enable);
	rbRxHex->setEnabled(enable);
	rbRxBin->setEnabled(enable);
	lbRxDisplay->setEnabled(enable);
	pbClearRx->setEnabled(enable);
}

void TerminalWidget::enableTx(const bool enable)
{
	rbTxText->setEnabled(enable);
	rbTxHex->setEnabled(enable);
	rbTxBin->setEnabled(enable);
	lbTxInterpret->setEnabled(enable);
	pushButtonSend->setEnabled(enable);
	cbSendCR->setEnabled(enable ? rbTxText->isChecked() : enable);
	cbSendLF->setEnabled(enable ? rbTxText->isChecked() : enable);
	cbSendOnReturn->setEnabled(enable);
	pbClearTx->setEnabled(enable);
}

//--------------------------------------------------------------------------------------

bool canConvertText(const QString & text, const QRegularExpression & exp)
{
	const int lengthWithoutWhitspace = text.simplified().replace(" ", "").length();
	if (lengthWithoutWhitspace > 0)
	{
		// check if we have an exact match
		auto matches = exp.match(text);
		if (matches.hasMatch())
		{
			return true;
		}
		// try matching individual parts and sum lengths
		int capturedLength = 0;
		for (const auto & text : matches.capturedTexts())
		{
			capturedLength += text.size();
		}
		//check if the size of the matches is the size of the string minus its whitespaces
		return (capturedLength == lengthWithoutWhitspace);
	}
	return false;
}

QByteArray convertTextToData(const QString & text, const QRegularExpression & exp, const int base = 10)
{
	const int lengthWithoutWhitspace = text.simplified().replace(" ", "").length();
	if (lengthWithoutWhitspace > 0)
	{
		QByteArray result;
		// try matching
		auto matches = exp.match(text);
		if (matches.hasMatch() || matches.hasPartialMatch())
		{
			for (const auto & text : matches.capturedTexts())
			{
				// convert match from string to byte
				bool worked = false;
				unsigned char value = text.toInt(&worked, base);
				if (!worked)
				{
					throw std::runtime_error("Input text contains bad strings!");
				}
				result.append(value);
			}
			return result;
		}
	}
	throw std::runtime_error("Input text not in correct fomat!");
}

QString convertDataToText(const QByteArray & data, const int base = 10)
{
	int fillUpWidth = 0;
	if (base == 2)
	{
		fillUpWidth = 8;
	}
	else if (base == 10)
	{
		fillUpWidth = 3;
	}
	else if (base == 16)
	{
		fillUpWidth = 2;
	}
	const int dataSize = data.size();
	QString converted;
	converted.reserve(dataSize * 3);
	for (int i = 0; i < dataSize; ++i)
	{
		const uint8_t value = (uint8_t)data.at(i);
		converted.append(QString("%1").arg(value, fillUpWidth, base, QChar('0')));
		converted.append(' ');
	}
	return converted;
}

//--------------------------------------------------------------------------------------

void TerminalWidget::inputTextChanged()
{
	const QString text = plainTextEditTx->toPlainText();
	if (text.length() == 0)
	{
		//remove stylesheet
		plainTextEditTx->setStyleSheet("");
	}
	else
	{
		//check wether the input format set via radio button can be used
		if (rbTxText->isChecked())
		{
			plainTextEditTx->setStyleSheet("");
			pushButtonSend->setEnabled(m_port.isOpen());
		}
		else if (rbTxHex->isChecked())
		{
			if (canConvertText(text, m_hexTextExp))
			{
				plainTextEditTx->setStyleSheet("");
				pushButtonSend->setEnabled(m_port.isOpen());
			}
			else
			{
				plainTextEditTx->setStyleSheet("background-color: #ff5555");
				pushButtonSend->setEnabled(false);
			}
		}
		else if (rbTxBin->isChecked())
		{
			if (canConvertText(text, m_binTextExp))
			{
				plainTextEditTx->setStyleSheet("");
				pushButtonSend->setEnabled(m_port.isOpen());
			}
			else
			{
				plainTextEditTx->setStyleSheet("background-color: #ff5555");
				pushButtonSend->setEnabled(false);
			}
		}
	}
}

void TerminalWidget::inputFormatchanged()
{
	inputTextChanged();
	//toggle CR/LF button. they are only meaningful in text mode
	if (rbTxText->isChecked())
	{
		cbSendCR->setEnabled(m_port.isOpen());
		cbSendLF->setEnabled(m_port.isOpen());
	}
	else if (rbTxHex->isChecked())
	{
		cbSendCR->setEnabled(false);
		cbSendLF->setEnabled(false);
	}
	else if (rbTxBin->isChecked())
	{
		cbSendCR->setEnabled(false);
		cbSendLF->setEnabled(false);
	}
}

//--------------------------------------------------------------------------------------

void TerminalWidget::sendText()
{
	if (m_port.isOpen() && m_port.isWritable())
	{
		enableTx(false);
		//convert text to format given via radio buttons
		QString text = plainTextEditTx->toPlainText();
		//store in history
		m_sendHistory.append(text);
		QByteArray data;
		if (rbTxText->isChecked())
		{
			if (cbSendCR->isChecked())
			{
				text.append('\r');
			}
			if (cbSendLF->isChecked())
			{
				text.append('\n');
			}
			data = text.toLocal8Bit();
		}
		else if (rbTxHex->isChecked())
		{
			data = convertTextToData(text, m_hexTextExp, 16);
		}
		else if (rbTxBin->isChecked())
		{
			data = convertTextToData(text, m_binTextExp, 2);
		}
		//check if we have data to write
		if (data.size() > 0)
		{
			//write data to port
			m_port.write(data);
			//copy text to echo edit
			QString echoText = plainTextEditTxPrevious->toPlainText();
			if (echoText.length() > 2048) {
				echoText = echoText.right(2048);
			}
			plainTextEditTxPrevious->setPlainText(echoText + text);
			QScrollBar *sb = plainTextEditTxPrevious->verticalScrollBar();
			sb->setValue(sb->maximum());
		}
		enableTx(true);
        plainTextEditTx->clear();
		plainTextEditTx->setFocus();
    }
}

void TerminalWidget::receiveText()
{
	if (m_port.isOpen() && m_port.isReadable() && m_port.bytesAvailable() > 0) {
		QByteArray data = m_port.readAll();
        if (data.size() > 0) {
			//get current text from text edit
            QString text = plainTextEditRx->toPlainText();
            if (text.length() > 16384) {
                text = text.right(16384);
            }
			//convert to format selected
			if (rbRxText->isChecked())
			{
				// check if color commands are conatained
				QString textData = QString::fromLatin1(data);
				textData.replace(m_consoleColorExp, "");
				plainTextEditRx->setPlainText(text + textData);
			}
			else if (rbRxHex->isChecked())
			{
				plainTextEditRx->setPlainText(text + convertDataToText(data, 16));
			}
			else if (rbRxBin->isChecked())
			{
				plainTextEditRx->setPlainText(text + convertDataToText(data, 2));
			}
            QScrollBar *sb = plainTextEditRx->verticalScrollBar();
            sb->setValue(sb->maximum());
        }
	}
}

//--------------------------------------------------------------------------------------

void TerminalWidget::historyEntrySelected()
{
	QAction * action = qobject_cast<QAction*>(sender());
	if (action)
	{
		//set text in text edit
		plainTextEditTx->setPlainText(action->data().toString());
		plainTextEditTx->moveCursor(QTextCursor::End);
	}
}

bool TerminalWidget::eventFilter(QObject *obj, QEvent *event)
{
	if (obj == plainTextEditTx)
	{
		if (event->type() == QEvent::KeyPress)
		{
			QKeyEvent * keyEvent = static_cast<QKeyEvent *>(event);
			if (keyEvent->key() == Qt::Key_Up)
			{
				//if we move the cursor up and it stays at the same position, we're at the top...
				QTextCursor cursor = plainTextEditTx->textCursor();
				int before = cursor.position();
				cursor.movePosition(QTextCursor::Up);
				int after = cursor.position();
				//check if the cursor is at the top and we have a history
				if (before == after && m_sendHistory.size() > 0)
				{
					//first run event filter
					bool result = QObject::eventFilter(obj, event);
					//build history menu
					QAction * action = nullptr;
					QMenu * menu = new QMenu(this);
					for (QString s : m_sendHistory)
					{
						//add text, eliding the text if necessary
						QString displayText = s;
						if (displayText.length() > 15)
						{
							displayText = displayText.left(15) + "...";
						}
						action = menu->addAction(QIcon(":/emblem-symbolic-link.png"), displayText, this, SLOT(historyEntrySelected()));
						action->setData(s);
					}
					//add current text
					QString displayText = plainTextEditTx->toPlainText();
					if (displayText.length() > 0 && displayText != m_sendHistory.last())
					{
						if (displayText.length() > 15)
						{
							displayText = displayText.left(15) + "...";
						}
						action = menu->addAction(QIcon(":/emblem-symbolic-link.png"), displayText, this, SLOT(historyEntrySelected()));
						action->setData(plainTextEditTx->toPlainText());
					}
					//show menu and select current text by default
					QRect cursorRect = plainTextEditTx->cursorRect(plainTextEditTx->textCursor());
					menu->popup(plainTextEditTx->mapToGlobal(cursorRect.topLeft()), action);
					menu->setActiveAction(action);
					return result;
				}
			}
			else if (keyEvent->key() == Qt::Key_Return)
			{
				if (cbSendOnReturn->isChecked())
				{
					sendText();
					return true;
				}
			}
		}
	}
	//do standard event processing
	return QObject::eventFilter(obj, event);
}