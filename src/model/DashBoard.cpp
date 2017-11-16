/*
 * DashBoard.cpp
 *
 *  Created on: 2015年6月21日
 *      Author: leelight
 */

#include <model/DashBoard.h>

DashBoard::DashBoard(QObject *parent) :
QObject(parent)
{
    m_rpm = 0;
    m_speed = 0;
}

DashBoard::~DashBoard()
{
    // TODO Auto-generated destructor stub
}

int DashBoard::rpm() {
    return m_rpm;
}
void DashBoard::setRpm(int rpm) {
    if (m_rpm != rpm) {
        m_rpm = rpm;
        emit rpmChanged(rpm);
    }
}

int DashBoard::speed() {
    return m_speed;
}
void DashBoard::setSpeed(int speed) {
    if (m_speed != speed) {
        m_speed = speed;
        emit speedChanged(speed);
    }
}
