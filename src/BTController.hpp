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

#ifndef BTCONTROLLER_HPP
#define BTCONTROLLER_HPP

#include "ChatManager.hpp"
#include "DeviceListing.hpp"
#include "LocalDeviceInfo.hpp"
#include "RemoteDeviceInfo.hpp"
#include "obdlib.h"
#include "ObdInfo.h"
#include "ObdThread.h"
#include "DashBoard.h"

#include <QObject>

#include <btgatt.h>
#include <btle.h>
#include <btdevice.h>

#include <QtConnectivity/QBluetoothServiceInfo>
#include <QtConnectivity/QBluetoothSocket>
#include <QtConnectivity/QBluetoothAddress>
#include <QtConnectivity/QBluetoothUuid>

using QTM_PREPEND_NAMESPACE(QBluetoothServiceInfo);
using QTM_PREPEND_NAMESPACE(QBluetoothSocket);
using QTM_PREPEND_NAMESPACE(QBluetoothAddress);
using QTM_PREPEND_NAMESPACE(QBluetoothUuid);

//namespace QtMobility {
    //class QBluetoothSocket;
//}

enum EMessage
{
    // Spider to control messages
    MSG_SHARPIR = 0,
    MSG_MOTOR_TARGETPOWER,
    MSG_MOTOR_SETPOWER,
    MSG_MOTOR_TARGETSPEED,
    MSG_MOTOR_TARGETDIST,
    MSG_MOTOR_CURRENT,
    MSG_ENCODER_SPEED,
    MSG_ENCODER_DISTANCE,
    MSG_ODOMETRY,
    MSG_SERVO,
    MSG_BATTERY,
    MSG_IMU,

    // Control to spider messages
    MSG_CMD_MOTORSPEED,
    MSG_CMD_DRIVEDIST,
    MSG_CMD_TURN,
    MSG_CMD_TURNANGLE,
    MSG_CMD_TRANSLATE,
    MSG_CMD_TRANSLATEDIST,
    MSG_CMD_STOP,
    MSG_CMD_FRONTLEDS,

    // Messages between phone and control
    MSG_CAMFRAME,
    MSG_SETZOOM,

    // Messages between bridge and control
    MSG_PING, // Ping from bridge to control
    MSG_PONG, // Ping response from control to bridge
    MSG_CNTRL_DISCONNECT,

    // Serial markers
    MSG_STARTMARKER,
    MSG_ENDMARKER,

    MSG_NONE,

    // Range to pass through unchanged from spider to controller
    MSG_PASS_START = MSG_SHARPIR,
    MSG_PASS_END = MSG_IMU
};

/**
 * @short The central class that encapsulates the access to the bluetooth functionality.
 *
 * The BTController encapsulates the access to the bluetooth functionality and provides properties
 * to make the functionality accessible inside QML.
 */
//! [0]
class BTController : public QObject
{
    Q_OBJECT

    enum { BTQUEUE_DELAY = 100 };

    Q_PROPERTY(QString activeDevice READ activeDevice WRITE setActiveDevice NOTIFY activeDeviceChanged)
    Q_PROPERTY(QString activeService READ activeService WRITE setActiveService NOTIFY activeServiceChanged)
    Q_PROPERTY(QString activeCharacteristic READ activeCharacteristic WRITE setActiveCharacteristic NOTIFY activeCharacteristicChanged)

    // Properties to access various bluetooth functionality
    Q_PROPERTY(ChatManager* chatManager READ chatManager CONSTANT)
    Q_PROPERTY(DeviceListing* deviceListing READ deviceListing CONSTANT)
    Q_PROPERTY(LocalDeviceInfo* localDeviceInfo READ localDeviceInfo CONSTANT)
    Q_PROPERTY(RemoteDeviceInfo* remoteDeviceInfo READ remoteDeviceInfo CONSTANT)

    Q_PROPERTY(bool bluetoothActive READ bluetoothActive NOTIFY bluetoothActiveChanged)
    Q_PROPERTY(bool discoverableActive READ discoverableActive NOTIFY discoverableActiveChanged)

    Q_PROPERTY(QString logHistory READ logHistory NOTIFY logHistoryChanged)
public:
    /**
     * Creates a new BTController object.
     *
     * @param parent The parent object.
     */
    BTController(QObject* parent=0);

    // Destroys the BTController object
    virtual ~BTController();

    // A helper method used for integration with bluetooth low-level C API
    void emitBTDeviceSignal(int event, const QString &btAddr, const QString &eventData);

    // called by callbacks from the bluetooth library
    void gattServiceConnected(const QString &bdaddr, const QString &service, int instance, int err, uint16_t connInt, uint16_t latency, uint16_t superTimeout );

    void connectBT(void);
    void disconnectBT(void);
    bool isConnected(void) const;
    void send(const QByteArray &data);

    DashBoard *getDashBoard(){return m_dashBoard;};
public Q_SLOTS:
    // This slot is invoked to switch the local bluetooth functionality on/off
    void toggleBluetoothActive();

    // This slot is invoked to change whether the bluetooth device is discoverable or not
    void toggleDiscoverableActive();

    // This slot is invoked to set the address of the remote device the user has selected in the UI
    void setRemoteDevice(/*const QString &address*/QVariantList indexPath);

    void updateLogWindows(const QString &msg);

Q_SIGNALS:
    // The change notification signals of the properties
    void activeDeviceChanged();
    void activeServiceChanged();
    void activeCharacteristicChanged();

    // A helper signal used for integration with bluetooth low-level C API
    void BTDeviceSignal(int event, const QString &btAddr, const QString &eventData);

    // The change notification signals of the properties
    void bluetoothActiveChanged();
    void discoverableActiveChanged();

    void logHistoryChanged();
public slots:
    void disconnectServices();

private slots:
    void doBtDisconnect(void);
    void btConnected(void);
    void btDisconnected(void);
    void btError(QBluetoothSocket::SocketError error);
    void btStateChanged(QBluetoothSocket::SocketState state);
	//not use here
    void btReadyRead(void);
    void btSendFromQueue(void);

//protected slots:
	void obdPidReply(QString pid, QString val, int set, double time);
	void obdDebugMessage(QString msg,obdLib::DebugLevel level);
	void obdConnected(QString version);
	void obdDisconnected();
	void obdSingleShotReply(QByteArray request, QByteArray list);
signals:
    void connected(void);
    void disconnected(void);
    void msgReceived(EMessage, QByteArray);

private Q_SLOTS:
    // A helper slot used for integration with bluetooth low-level C API
    void handleBTDeviceEvent(int event, const QString &btAddr, const QString &eventData);

private:
    // A helper method to log data to stdout
    void logQString(const QString &msg);

    QString activeDevice() const;
    void setActiveDevice(const QString&);
    QString activeDeviceName() const;
    void setActiveDeviceName(const QString&);
    QString activeService() const;
    void setActiveService(const QString&);
    QString activeCharacteristic() const;
    void setActiveCharacteristic(const QString&);

    // The accessor methods for the properties
    bool bluetoothActive() const;
    bool discoverableActive() const;

    ChatManager* chatManager() const;
    DeviceListing* deviceListing() const;
    LocalDeviceInfo* localDeviceInfo() const;
    RemoteDeviceInfo* remoteDeviceInfo() const;

    // The property values
    ChatManager* m_chatManager;
    DeviceListing* m_deviceListing;
    LocalDeviceInfo* m_localDeviceInfo;
    RemoteDeviceInfo* m_remoteDeviceInfo;

    QString m_activeDevice;
    QString m_activeDeviceName;
    QString m_activeService;
    QString m_activeCharacteristic;

    QBluetoothSocket *bluetoothSocket;
    QQueue<QByteArray> btSendQueue;
    QTimer *btSendTimer;
    QByteArray recBuffer;

	ObdThread *m_obdThread;
	DashBoard *m_dashBoard;

    void gattServiceDisconnected(const QString &deviceAddress, const QString &uuid, int instance, int reason);
    void gattNotification(int instance, uint16_t handle, const uint8_t *val, uint16_t len);

    void startObdJobs();

    QString logHistory() const;
    QString m_logHistory;
};
//! [0]

#endif
