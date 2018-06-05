#ifndef ALGSTRUCT_H
#define ALGSTRUCT_H

#include <QTime>

struct Climate{
    double co2Dosage;
    double temp;
    double humidity;
};

struct Weather{
    double rad;
    bool storm;
    bool frost;
    double wind;
    double temp;
};

struct Control{
    // CO2补充设备
    bool co2;
    // 外遮阳网
    bool curtain1;
    //内遮阳网
    bool curtain2;
    // 内保温网
    bool curtain3;
    // 喷雾
    bool mist;
    // 湿帘
    bool fanPad;
    // 加热
    bool heating;
    // 侧窗开度
    double sideVent;
    // 侧窗延迟相关
    bool sidePreOpen;
    QTime preTime;
    // 天窗开度
    double roofVent;
    // 天窗通风温度
    double ventTemp;
};




#endif // ALGSTRUCT_H
