/**************************************************************************
*   Copyright (C) 2010 by Michael Carpenter (malcom2073)                  *
*   malcom2073@gmail.com                                                  *
*                                                                         *
*   This file is a part of libobd                                         *
*                                                                         *
*   libobd is free software: you can redistribute it and/or modify        *
*   it under the terms of the GNU Lesser General Public License as        *
*   published by the Free Software Foundation, either version 2 of        *
*   the License, or (at your option) any later version.                   *
*                                                                         *
*   libobd is distributed in the hope that it will be useful,             *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with libobd.  If not, see <http://www.gnu.org/licenses/>.       *
***************************************************************************/


#ifndef OBDLIB_H
#define OBDLIB_H

//#define WINHACK
//#define WINVER
#define QBLUETOOTH

#ifndef QBLUETOOTH
#include <stdio.h>
#include <string>
#endif


#ifdef WINHACK

#define M_SleepSec(x) Sleep(1000 * x);
#include <windows.h>
//#ifdef WIN32
//#define STDCALL __sttribute__((__stdcall__))
//#define STDCALL __stdcall
#define STDCALL  __declspec(dllexport)

#elif defined QBLUETOOTH

#define STDCALL
#   define byte unsigned char
#   define DWORD long

#else

#define M_SleepSec(x) usleep(x * 1000000);
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#define STDCALL
#	define byte unsigned char
#	define HANDLE int
#	define DWORD long

#endif	// WIN32

#include <vector>

#ifdef QBLUETOOTH
#include <qbluetoothsocket.h>
#endif

struct variantStruct
{
	int intVar;
	double doubleVarOne;
	double doubleVarTwo;
	byte byteVar;
	std::vector<byte> vectorBytes;
	byte typeVar;
	byte btMode;
	byte btPID;
};
struct obdInfoStruct
{
	DWORD dwItemType;
	std::string strItemDesc;
	byte btMode;
	byte btPID;
	byte btItemSizeBytes;
	DWORD dwOperation;
	double dFactor;
	double dOffset;
	int iRangeLow;
	int iRangeHigh;
	std::string strUnitLabel;
	int iBitLookupTable;
};

class obdLib
#ifdef QBLUETOOTH
        : public QObject
#endif
{
#ifdef QBLUETOOTH
    Q_OBJECT
#endif
public:

	enum ObdError
	{
		NODATA=0,
		NOTCONNECTED,
		OTHER,
		SERIALWRITEERROR,
		SERIALREADERROR,
		TIMEOUT,
		NONE
	};
	enum DebugLevel
	{
		DEBUG_VERY_VERBOSE=0,
		DEBUG_VERBOSE=1,
		DEBUG_INFO=2,
		DEBUG_WARN=3,
		DEBUG_ERROR=4,
		DEBUG_FATAL=5
	};
	#ifdef QBLUETOOTH
	obdLib(QtMobility::QBluetoothSocket *bluetoothSocket, QObject *parent=0);
	#else
	obdLib();
	#endif

	int openPort(const char *portName,int baudrate);
	int openPort(const char *portName);
    #ifndef QBLUETOOTH
	void setPortHandle(HANDLE hdnl);
    #endif
	int initPort();
	int closePort();
	void flush();
	void setDebugCallback(void (*callbackptr)(const char*,void*,obdLib::DebugLevel),void *);
	void setCommsCallback(void (*callbackptr)(const char*,void*),void*);
	std::string monitorModeReadLine();
	//byte* sendRequest(byte *reqString,int length);
//	byte* sendRequest(char *asciiReqString, int length);
	static byte byteArrayToByte(byte b1, byte b2);
	std::string getVersion() { return versionString; }
	bool sendObdRequest(const char *req,int len,std::vector<byte> *reply);
	bool sendObdRequest(const char *req,int length,std::vector<byte> *reply,int sleep, int timeout);
	bool sendObdRequestString(const char *req,int len,std::vector<byte> *reply,int sleeptime);
	bool sendObdRequestString(const char *req,int length,std::vector<byte> *reply,int sleeptime, int timeout);
	bool sendObdRequestString(const char *req,int len,std::vector<byte> *reply);
	bool sendObdRequest(const char *req,int len);
	bool sendObdRequest(const char *req,int len,int timeout);
	ObdError lastError();

//#ifdef QBLUETOOTH
//private slots:
	//void btReadyRead(void);
//#endif

private:
	void debug(DebugLevel lvl, const char* msg,...);
	void commsDebug(const char *msg);
	ObdError m_lastError;
	std::vector<std::vector<int> > *modeByteCount;
	std::string versionString;
#ifndef QBLUETOOTH
	HANDLE portHandle;
#endif
	byte* readBytes(int* bytesRead,int numBytesToRead);
	void writeBytes(byte *sendBuffer,int bufferSize,int *bytesWritten);
	byte *sendBuffer;
	byte *rcvdBuffer;

	#ifdef QBLUETOOTH
	QtMobility::QBluetoothSocket *m_btSocket;
	#endif
};
#endif //OBDLIB_H

