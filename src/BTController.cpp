/* Copyright (c) 2012, 2013  BlackBerry Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "BTController.hpp"

#include <QtCore>

#include "parse.h"

//#include <btapi/btdevice.h>
//#include <btapi/btspp.h>

#include <errno.h>
#include <iostream>



/**
 * A static variable that points to the unique instance of BTController
 * This is needed to have access to the BTController instance from within the btdevice API callback.
 */
//! [0]
static BTController *s_btController = 0;
//! [0]

/**
 * Callback for events triggered by btdevice APIs.
 */
//! [1]
void BTControllerCallback(const int event, const char *bt_addr, const char *event_data)
{
    if (s_btController) {
        s_btController->emitBTDeviceSignal(event, bt_addr, event_data);
    }
}
//! [1]

//! [2]
BTController::BTController(QObject* parent)
    : QObject(parent)
    , m_chatManager(new ChatManager(this))
    , m_deviceListing(new DeviceListing(this))
    , m_localDeviceInfo(new LocalDeviceInfo(this))
    , m_remoteDeviceInfo(new RemoteDeviceInfo(this))
    , bluetoothSocket(0)
{
    s_btController = this;
    m_dashBoard = new DashBoard();
    /*
    bool ok = connect(this, SIGNAL(BTDeviceSignal(int, QString, QString)),
                      this, SLOT(handleBTDeviceEvent(int, QString, QString)));
    Q_ASSERT(ok);
    Q_UNUSED(ok);
    */
    // Initialize the btdevice and SPP library APIs.
    bt_device_init(BTControllerCallback);
    //bt_spp_init();

    // Initialize the list of paired and found devices, but do not run a Bluetooth search.
    m_deviceListing->update();

    // Initialize the local device information
    m_localDeviceInfo->update();
}
//! [2]

//! [3]
BTController::~BTController()
{
	m_obdThread->disconnect();

    // De-initialize the btdevice library.
    bt_device_deinit();

    // De-initialize the library.
    //bt_spp_deinit();

    s_btController = 0;
    m_dashBoard = 0;
    btSendTimer = 0;
}
//! [3]

//! [4]
void BTController::emitBTDeviceSignal(int event, const QString &btAddr, const QString &eventData)
{
    emit BTDeviceSignal(event, btAddr, eventData);
}
//! [4]

//! [5]
void BTController::handleBTDeviceEvent(int event, const QString &btAddr, const QString &eventData)
{
    switch (event) {
        case BT_EVT_RADIO_SHUTDOWN:
        case BT_EVT_RADIO_INIT:
        case BT_EVT_ACCESS_CHANGED:
            emit bluetoothActiveChanged();
            emit discoverableActiveChanged();
            m_localDeviceInfo->update();
            break;
        case BT_EVT_DEVICE_CONNECTED:
        case BT_EVT_DEVICE_DISCONNECTED:
            logQString("Connected event " + QString::number(event) + "/" + btAddr);
            break;
        default:
            logQString("Unknown event " + QString::number(event) + "/" + btAddr);
            break;
    }
}

void BTController::disconnectServices()
{
    for (int i = 0; i < m_remoteDeviceInfo->model()->size(); i++) {
        QVariantList indexPath;
        indexPath.clear();
        indexPath.append(QVariant(i));
        const QVariantMap service = m_remoteDeviceInfo->model()->data(indexPath).toMap();
        const int rc = bt_gatt_disconnect_service(activeDevice().toAscii().constData(), service["uuid"].toString().toAscii().constData());
        gattServiceDisconnected(activeDevice(), service["uuid"].toString(), 0, rc);
    }
}

void BTController::gattServiceConnected(const QString&, const QString &_serviceUuid, int instance, int err, uint16_t, uint16_t, uint16_t)
{
    if (err != EOK) {
        //setErrorMessage(QString("GATT service connection failed %1 (%2)").arg(err).arg(strerror(err)));
        return;
    }

    QString serviceUuid(_serviceUuid);
    if (serviceUuid.startsWith("0x"))
        serviceUuid = serviceUuid.mid(2);

    for (int i = 0; i < m_remoteDeviceInfo->model()->size(); i++) {
        QVariantList indexPath;
        indexPath.clear();
        indexPath.append(QVariant(i));
        QVariantMap service = m_remoteDeviceInfo->model()->data(indexPath).toMap();
        if (serviceUuid == service["uuid"].toString()) {
            service["connected"] = true;
            service["instance"] = instance;
            m_remoteDeviceInfo->model()->updateItem (indexPath, service);
            logQString( "GATT service connected:" + serviceUuid);
            break;
        }
    }
}
//! [16]

//! [17]
void BTController::gattServiceDisconnected(const QString&, const QString &_serviceUuid, int, int err )
{
    if (err != EOK) {
        //setErrorMessage(QString("GATT service disconnection failed %1 (%2)").arg(err).arg(strerror(err)));
        return;
    }

    QString serviceUuid(_serviceUuid);
    if (serviceUuid.startsWith("0x"))
        serviceUuid = serviceUuid.mid(2);

    for (int i = 0; i < m_remoteDeviceInfo->model()->size(); i++) {
        QVariantList indexPath;
                indexPath.append(QVariant(i));
        QVariantMap service = m_remoteDeviceInfo->model()->data(indexPath).toMap();
        if (serviceUuid == service["uuid"].toString()) {
            service["connected"] = false;
            m_remoteDeviceInfo->model()->updateItem (indexPath, service);
            logQString( "GATT service disconnected:" + serviceUuid );
            break;
        }
    }
}
//! [5]

//! [6]
bool BTController::bluetoothActive() const
{
    return bt_ldev_get_power();
}

bool BTController::discoverableActive() const
{
    return (bt_ldev_get_discoverable() == BT_DISCOVERABLE_GIAC);
}
//! [6]

ChatManager* BTController::chatManager() const
{
    return m_chatManager;
}

DeviceListing* BTController::deviceListing() const
{
    return m_deviceListing;
}

LocalDeviceInfo* BTController::localDeviceInfo() const
{
    return m_localDeviceInfo;
}

RemoteDeviceInfo* BTController::remoteDeviceInfo() const
{
    return m_remoteDeviceInfo;
}

//! [7]
void BTController::toggleBluetoothActive()
{
    bt_ldev_set_power(!bt_ldev_get_power());

    emit bluetoothActiveChanged();
}

void BTController::toggleDiscoverableActive()
{
    // If discoverable mode is GIAC, then we are discoverable - change to connectable (but not discoverable), and vice-versa.
    if (bt_ldev_get_discoverable() == BT_DISCOVERABLE_GIAC) {
        bt_ldev_set_discoverable(BT_DISCOVERABLE_CONNECTABLE);
    } else {
        bt_ldev_set_discoverable(BT_DISCOVERABLE_GIAC);
    }

    emit discoverableActiveChanged();
}
//! [7]

//! [8]
void BTController::setRemoteDevice(/*const QString &address*/QVariantList indexPath)
{
    //if (which < 0 || which >= m_deviceListing->model()->size())
        //return;
    logQString(QString::number(m_deviceListing->model()->size()) + ": m_deviceListing size");




    //QVariantList indexPath;
    //indexPath.clear();
    //indexPath.append(QVariant(which));

    /*foreach (QVariant v, indexPath)
    {
      qDebug() << "String Value: " << v.toString() << "\n";
    }*/

    const QVariantMap device = m_deviceListing->model()->data(indexPath).toMap();
    if (device.isEmpty())
        return;
    logQString( device["deviceAddress"].toString() );

    QString address;
    address = device["deviceAddress"].toString();
    logQString(address);

    setActiveDevice(device["deviceAddress"].toString());
    setActiveDeviceName(device["deviceName"].toString());

    //TODO if name is OBDII, do something

    //https://developer.blackberry.com/native/reference/core/com.qnx.doc.bluetooth.lib_ref/topic/bt_rdev_pair.html
    //TODO if bt is not pairing, use bt_rdev_pair()
    //if(device["deviceType"] == tr("Bluetooth Devices Paired")){}

    //how to pair refer this https://github.com/blackberry/Cascades-Community-Samples/blob/master/HolidayTrain/src/bluetooth/BluetoothManager.cpp

    disconnectServices();

    // Update the remote address on the helper objects
    m_chatManager->setRemoteAddress(address);
    m_remoteDeviceInfo->update(address);

    btSendTimer = new QTimer(this);
    btSendTimer->setInterval(BTQUEUE_DELAY*50);
    connect(btSendTimer, SIGNAL(timeout()), SLOT(btSendFromQueue()));

    m_logHistory.clear();
    //btSendTimer->start();
    connectBT();
}


QString BTController::activeDevice() const
{
    return m_activeDevice;
}

void BTController::setActiveDevice(const QString &device)
{
    if (m_activeDevice != device) {
        m_activeDevice = device;
        emit activeDeviceChanged();
    }
}

QString BTController::activeDeviceName() const
{
    return m_activeDeviceName;
}

void BTController::setActiveDeviceName(const QString &deviceName)
{
    if (m_activeDeviceName != deviceName) {
        m_activeDeviceName = deviceName;
    }
}

QString BTController::activeService() const
{
    return m_activeService;
}

void BTController::setActiveService(const QString &service)
{
    if (m_activeService != service) {
        m_activeService = service;
        emit activeServiceChanged();
    }
}

QString BTController::activeCharacteristic() const
{
    return m_activeCharacteristic;
}

void BTController::setActiveCharacteristic(const QString &characteristic)
{
    if (m_activeCharacteristic != characteristic) {
        m_activeCharacteristic = characteristic;
        emit activeCharacteristicChanged();
    }
}
//! [8]

void BTController::doBtDisconnect()
{
    if (bluetoothSocket)
    {
        logQString( "doBtDisconnect");
        bluetoothSocket->disconnectFromService();
        bluetoothSocket->deleteLater();
        bluetoothSocket = 0;
    }
}

void BTController::btConnected()
{
    logQString( "Got Bluetooth connection!");
    btSendTimer->start();
    emit connected();

    startObdJobs();
}

void BTController::btDisconnected()
{
    logQString( "Lost bt connection");
    btSendTimer->stop();
    btSendQueue.clear();
    recBuffer.clear();
    emit disconnected();
}

void BTController::btError(QBluetoothSocket::SocketError error)
{
    logQString( "btError: " + error );
    //emit error();
}

void BTController::btStateChanged (QBluetoothSocket::SocketState state){
    //int socketState = bluetoothSocket->state();
    logQString( "btStateChanged: " + state );
    if(state == QAbstractSocket::UnconnectedState)
    {
        logQString("unconnected");
    }
    else if(state == QAbstractSocket::HostLookupState)
    {
        logQString("host lookup");
    }
    else if(state == QAbstractSocket::ConnectingState )
    {
        logQString("connecting");
    }
    else if(state == QAbstractSocket::ConnectedState)
    {
        logQString("connected");
    }
    else if(state == QAbstractSocket::BoundState)
    {
        logQString("bound");
    }
    else if(state == QAbstractSocket::ClosingState)
    {
        logQString("closing");
    }
    else if(state == QAbstractSocket::ListeningState)
    {
        logQString("listening");
    }
}

//not use here
void BTController::btReadyRead()
{
    // BT message format:
    //  0: Start marker
    //  1: Msg size
    //  2: Msg type
    //  3 .. n-1: Data
    //  n: End marker

    qDebug() << "Got some bt data:" << bluetoothSocket->bytesAvailable() << "bytes";
    /*
    while (bluetoothSocket->bytesAvailable() > 0){
        recBuffer += bluetoothSocket->readAll();
    }

    int msgstartind = recBuffer.indexOf(MSG_STARTMARKER);
    if (msgstartind == -1)
    {
        recBuffer.clear(); // No message start detected, remove dirt
        return; // wait untill we got something useful
    }

    const int extramsgsize = 3; // start- and end marker and msg size
    while (msgstartind != -1)
    {
        if (msgstartind > 0) // dirt in between?
        {
            recBuffer.remove(0, msgstartind);
            msgstartind = 0;
        }

        if (recBuffer.size() < extramsgsize)
            return; // wait for more

        const uint16_t msgsize = recBuffer[1];
        if (recBuffer.size() < (msgsize + extramsgsize))
            return; // wait for more

        // End marker present?
        if (recBuffer.at(msgsize + extramsgsize - 1) == MSG_ENDMARKER)
        {
            emit msgReceived(static_cast<EMessage>((int)recBuffer[2]),
                    recBuffer.mid(3, msgsize-1));
        }

        recBuffer.remove(0, msgsize + extramsgsize);
        msgstartind = recBuffer.indexOf(MSG_STARTMARKER);
    }

    while (bluetoothSocket->canReadLine())
        qDebug() << bluetoothSocket->readLine();

    qDebug() << "Data:" << bluetoothSocket->readAll();
    */
}

void BTController::startObdJobs()
{

	m_obdThread = new ObdThread(bluetoothSocket);

	connect(m_obdThread, SIGNAL(debugMessage(QString, obdLib::DebugLevel)), this, SLOT(obdDebugMessage(QString, obdLib::DebugLevel)));
	connect(m_obdThread, SIGNAL(pidReply(QString,QString,int,double)), this, SLOT(obdPidReply(QString,QString,int,double)));
	connect(m_obdThread, SIGNAL(connected(QString)), this, SLOT(obdConnected(QString)));
	connect(m_obdThread, SIGNAL(disconnected()), this, SLOT(obdDisconnected()));
	//connect(m_obdThread, SIGNAL(elmCommandFailed(QString)), this, SLOT(obdElmCommandFailed(QString)));
	connect(m_obdThread, SIGNAL(singleShotReply()), this, SLOT(obdSingleShotReply()));

	m_obdThread->connect(true);
	m_obdThread->start();
	//m_obdThread->setPort("/dev/pts/2");
	//m_obdThread->setBaud(9600);
	m_obdThread->addRequest(0x01, 0x0c, 1, 0);
	m_obdThread->addRequest(0x01, 0x0d, 1, 0);
	m_obdThread->addRequest(0x01, 0x11, 1, 0);


	/*
    QByteArray reset_adapter;
    QByteArray echo_off;
    QByteArray line_feeds_off;
	QByteArray time_out;
    QByteArray proto_dect ;
    QByteArray suported_pids;
    QByteArray dect_proto ;
    QByteArray turn_on_headers;


    //suported_pids.append(0x01);
    //suported_pids.append(0x0C);
    //suported_pids.append(0x0D);

    suported_pids.append("01 00");

    reset_adapter.append("AT Z\r");
    echo_off.append("AT E0\r");
    line_feeds_off.append("AT L0\r");
	time_out.append("AT ST 00\r");
    proto_dect.append("AT SP 00\r");
    dect_proto.append("AT DP\r");
    turn_on_headers.append("AT H1\r");


    // Reset adapter.
    send(reset_adapter);
    //Echo commands for verification.
    send(echo_off);
    //Line feeds off
    send(line_feeds_off);
	//time out
	send(time_out);
    //Automatically detect protocol
    send(proto_dect);
    //Force protocol detection to work now by sending a simple OBD-II command.
    send(suported_pids);
    //Get detected protocol.
    //send(dect_proto);

    //turn on headers (needed for ECU)
    //send(turn_on_headers);
	*/

}

void BTController::obdDebugMessage(QString msg, obdLib::DebugLevel level){
    updateLogWindows(msg);
}

void BTController::obdPidReply(QString pid, QString val, int set, double time)
{
    logQString( "Called BTController::obdPidReply()");
    logQString( "pid is:" + pid + "val is:" + val + "set is:" + set + "time is:" + time);
	qDebug() << "pid is:" << pid << "val is:" << val << "set is:" << set << "time is:" << time;

	//https://github.com/pires/obd-java-api/tree/master/src/main/java/pt/lighthouselabs/obd/commands

	//TODO
	//Displays the current engine revolutions per minute (RPM).
	if (pid == "010C"){
	    m_dashBoard->setRpm(val.toInt());
		//emit xxx;
	}
	//Current speed
	else if (pid == "010D"){
		//emit xxx;
	    m_dashBoard->setSpeed(val.toInt());
	}
	//Read the throttle position in percentage
	else if (pid == "0111"){
		//emit xxx;
	}
}

void BTController::obdConnected(QString version){
    logQString( "Called BTController::obdConnected " + version );
}

void BTController::obdDisconnected(){
    logQString( "Called BTController::obdDisconnected " );
}

void BTController::obdSingleShotReply(QByteArray request, QByteArray list){
    logQString( "Called BTController::obdSingleShotReply()" );
    logQString( "request " + QString(request) + " list " + QString(list) );

}



void BTController::btSendFromQueue()
{

    if (!isConnected())
        btSendTimer->stop();
    else if (!btSendQueue.isEmpty())
        bluetoothSocket->write(btSendQueue.dequeue());
}

void BTController::connectBT()
{
    logQString( "connectBT");
    //QBluetoothSocket socket;// = new QBluetoothSocket();
    //socket->connectToService(QBluetoothAddress(activeDevice()), QBluetoothUuid(QBluetoothUuid::SerialPort));

    /*
    socket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol, this);
    socket->connectToService(QBluetoothAddress(activeDevice()),
                                    QBluetoothUuid(QBluetoothUuid::SerialPort));
    connect(socket, SIGNAL(error(QBluetoothSocket::SocketError)),this, SLOT(socketError(QBluetoothSocket::SocketError)));
    connect(socket, SIGNAL(connected()), this, SLOT(socketConnected()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
    connect(socket, SIGNAL(readyRead()), this, SLOT(socketRead()));
    connect(socket, SIGNAL(stateChanged(QBluetoothSocket::SocketState)), this, SLOT(socketStateChanged()));
    */

    doBtDisconnect();

    if (!bluetoothSocket)
    {
        logQString( "address:" + activeDevice());
        bluetoothSocket = new QBluetoothSocket(QBluetoothSocket::RfcommSocket);
        //bluetoothSocket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol);
        connect(bluetoothSocket, SIGNAL(connected()), this,  SLOT(btConnected()));
        connect(bluetoothSocket, SIGNAL(disconnected()), this, SLOT(btDisconnected()));
        connect(bluetoothSocket, SIGNAL(error(QBluetoothSocket::SocketError)), this, SLOT(btError(QBluetoothSocket::SocketError)));
        connect(bluetoothSocket, SIGNAL(stateChanged(QBluetoothSocket::SocketState)), this, SLOT(btStateChanged(QBluetoothSocket::SocketState)));

        //connect(bluetoothSocket, SIGNAL(readyRead()), this/*m_obdThread->getObdLib()*/, SLOT(btReadyRead()));//not use
    }
    //SerialPort = 0x1101 is the service of OBD device
    //QBluetoothUuid btUuid = QBluetoothUuid::SerialPort; //00001101-0000-1000-8000-00805f9b34fb
    //QBluetoothUuid btUuid = QBluetoothUuid::Rfcomm; //00000003-0000-1000-8000-00805f9b34fb
    //bluetoothSocket->connectToService(QBluetoothAddress(activeDevice()), btUuid, QIODevice::ReadWrite);

    //Hint: If you are connecting to a Bluetooth serial board then try using the well-known SPP UUID 00001101-0000-1000-8000-00805F9B34FB. However if you are connecting to an Android peer then please generate your own unique UUID.
    //http://developer.android.com/reference/android/bluetooth/BluetoothDevice.html
    bluetoothSocket->connectToService(QBluetoothAddress(activeDevice()), QBluetoothUuid(QString("00001101-0000-1000-8000-00805F9B34FB")), QIODevice::ReadWrite);
    logQString( "connnecting....");
}

void BTController::disconnectBT()
{
    if (bluetoothSocket)
    {
        logQString( "Sending disconnect" );
        //send(CBTMessage(MSG_CNTRL_DISCONNECT));

        // Flush
        while (!btSendQueue.isEmpty())
            bluetoothSocket->write(btSendQueue.dequeue());

        QTimer::singleShot(2000, this, SLOT(doBtDisconnect()));
        /*QTime waittime;
        waittime.start();
        while (waittime.elapsed() < 5000)
            qApp->processEvents();
        bluetoothSocket->disconnectFromService();
        bluetoothSocket->deleteLater();
        bluetoothSocket = 0;*/
    }
}

void BTController::send(const QByteArray &data)
{
    if (!isConnected())
        return;

    btSendQueue.enqueue(data);
}

bool BTController::isConnected() const
{
    return (bluetoothSocket && (bluetoothSocket->state() == QBluetoothSocket::ConnectedState));
}

void BTController::logQString(const QString &msg)
{
    std::cout << "Bluetooth Diagnostics: " << msg.toLocal8Bit().constData() << std::endl;

    updateLogWindows(msg);
}


void BTController::updateLogWindows(const QString &msg)
{
    if (m_logHistory.size() > 0)
        m_logHistory.append("\n");

    m_logHistory.append(msg);

    emit logHistoryChanged();
}

QString BTController::logHistory() const
{
    return m_logHistory;
}
