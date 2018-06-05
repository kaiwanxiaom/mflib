#ifndef ACTUATOR_H
#define ACTUATOR_H

#include <QThread>
#include "algstruct.h"

class Actuator: public QThread {

public:
    void actRoofVent();

private:
    void openTarRoofVent(double currPerc, double nextPerc);
    void closeRoofVent(double perc);

public:
    bool roofVentOpen;
    bool roofVentClose;
    bool roofVentStop;
    double MAX_TIME_MSEC = 1000;

    struct Control control[2];
    int nextControlNum = 0;
};

#endif // ACTUATOR_H

