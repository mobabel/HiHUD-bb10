/*
 * DashBoard.h
 *
 *  Created on: 2015年6月21日
 *      Author: leelight
 */

#ifndef DASHBOARD_H_
#define DASHBOARD_H_

#include <QObject>

class DashBoard: public QObject {
    Q_OBJECT

    Q_PROPERTY(int rpm READ rpm WRITE setRpm NOTIFY rpmChanged)
    Q_PROPERTY(int speed READ speed WRITE setSpeed NOTIFY speedChanged)


public:
    DashBoard(QObject *parent = 0);
    virtual ~DashBoard();

    int rpm();
    void setRpm(int rpm);
    int speed();
    void setSpeed(int speed);

    Q_SIGNALS:

        void rpmChanged(int rpm);
        void speedChanged(int speed);
private:
    int m_rpm;
    int m_speed;
};

#endif /* DASHBOARD_H_ */
