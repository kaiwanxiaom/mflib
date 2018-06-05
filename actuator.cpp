#include "actuator.h"
#include <QThread>
#include <QtConcurrent/QtConcurrent>
#include <QDebug>

void Actuator::openTarRoofVent(double currPerc, double nextPerc)
{
    closeRoofVent(currPerc);

    double openTime = nextPerc*MAX_TIME_MSEC;

    roofVentOpen = true;
    this->sleep(openTime);
    roofVentStop = true;
}

void Actuator::closeRoofVent(double currPerc)
{
    double closeTime = currPerc*MAX_TIME_MSEC;
    roofVentClose = true;
    this->sleep(closeTime);
    roofVentStop = true;
}

void Actuator::actRoofVent()
{
    double currRoofVent = this->control[(this->nextControlNum+1)%2].roofVent;
    double nextRoofVent = this->control[this->nextControlNum].roofVent;
    if(currRoofVent == nextRoofVent) {
        return;
    }

    if(nextRoofVent == 0) {
        QtConcurrent::run(this, &Actuator::closeRoofVent, currRoofVent);
    } else {
        QtConcurrent::run(this, &Actuator::openTarRoofVent, currRoofVent, nextRoofVent);
    }
}

