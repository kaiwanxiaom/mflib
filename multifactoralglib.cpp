#include "multifactoralglib.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QMap>
#include <QMetaEnum>
#include <QDebug>


MultiFactorAlgLib::MultiFactorAlgLib(){

    // 大风 10-12m/s
    param.map[Param::WEATHER_WIND_MAX] = 10;

    // 结霜温度 2℃
    param.map[Param::WEATHER_FROST_TEMP] = 2;

    // ppm
    param.map[Param::CO2_MIN] = 300;
    param.map[Param::CO2_MAX] = 1000;

    // 遮阳 番茄 幼苗期：20 mw/cm2 生长期：30mw/cm2 产果期：80mw/cm2
    param.map[Param::CUTAIN1_RAD_MAX] = 30;
    param.map[Param::CUTAIN1_RAD_MAX_BAND] = 2;

    param.map[Param::CUTAIN2_RAD_MAX] = 30;
    param.map[Param::CUTAIN2_RAD_MAX_BAND] = 2;

    // 降温温度 band 2-4℃
    param.map[Param::FANPAD_TEMPMAX] = 35;
    param.map[Param::FANPAD_TEMPMAX_BAND] = 3;

    // 加热温度 ℃
    param.map[Param::HEATING_TEMP] = 18;
    param.map[Param::HEATING_TEMP_BAND] = 1;

    // 降温温度 ℃
    param.map[Param::MIST_TEMPMAX] = 35;
    param.map[Param::MIST_TEMPMAX_BAND] = 3;

    //    param.map[SIDE_OPEN_TEMP];
    //    param.map[SIDE_VENT_WAITTIME];

    // 苗期 55% 生长期 70% 产果期 80%
    param.map[Param::ROOFVENT_DES_HUMIDITY] = 70;

    param.map[Param::ROOFVENT_DAY_PART_COUNT] = 3;
//    param.map[ROOFVENT_HUMIDITY_INF_RANGE];
//    param.map[ROOFVENT_HUMIDITY_INF_TEMP_HR];
//    param.map[ROOFVENT_RAD_SET];
//    param.map[ROOFVENT_RAD_TEMP_INF_L];
//    param.map[ROOFVENT_RAD_TEMP_INF_H];
//    param.map[ROOFVENT_RAD_INF_K];
//    param.map[ROOFVENT_TEMP_CLIMATE_MIN];

    memset(&control, 0, sizeof(control));
    memset(this->param.ROOFVENT_DAY_PARTS_TEMP_ARRAY, 0, sizeof(double)*MAX_DAY_PART);
    memset(this->param.ROOFVENT_DAY_PARTS_TIME_ARRAY, 0, sizeof(QTime)*MAX_DAY_PART);
}

QJsonObject MultiFactorAlgLib::getControl() {
    QJsonObject res;
    res.insert("co2", control.co2);
    res.insert("curtain1", control.curtain1);
    res.insert("curtain2", control.curtain2);
    res.insert("curtain3", control.curtain3);
    res.insert("mist", control.mist);
    res.insert("fanPad", control.fanPad);
    res.insert("heating", control.heating);
    res.insert("sideVent", control.sideVent);
    res.insert("roofVent", control.roofVent);
    res.insert("ventTemp", control.ventTemp);
    return res;
}

bool MultiFactorAlgLib::updateWeather(QJsonObject & weather) {
    this->weather.rad = weather["rad"].toDouble();
    this->weather.storm = weather["storm"].toDouble() > 0 ? true : false;
    this->weather.wind = weather["wind"].toDouble();
    this->weather.temp = weather["weatherTemp"].toDouble();
    this->weather.frost = this->weather.temp < param.map[Param::WEATHER_FROST_TEMP].toDouble();
    return true;
}

bool MultiFactorAlgLib::updateClimate(QJsonObject & climate) {
    this->climate.co2Dosage = climate["co2Dosage"].toDouble();
    this->climate.temp = climate["temp"].toDouble();
    this->climate.humidity = climate["humidity"].toDouble();
    return true;
}


int MultiFactorAlgLib::setParam(QJsonObject & param) {
    int res = 0;
    for(QJsonObject::Iterator it = param.begin(); it!=param.end(); it++) {
        QMetaEnum menum = QMetaEnum::fromType<Param::ParamName>();
        bool ok = false;
        char szStr[1024] = {0};
        strncpy(szStr, it.key().toStdString().c_str(), sizeof(szStr) - 1);
        Param::ParamName name = Param::ParamName(menum.keyToValue(szStr, &ok));
        if(!ok) {
            res = 1;
//            QJsonArray arr;
//            if(error.contains("unkownParam")) {
//                QJsonObject::iterator up = error.find("uknownParam");
//                arr = up.value().toArray();
//            }

//            arr.append(it.key());
//            error.insert("unkownParam", arr);
            continue;
        }

        if(!it.value().isArray())
            this->param.map[name] = it.value().toVariant();
        else {
            QJsonArray arrays = it.value().toArray();
            if(name == Param::ROOFVENT_DAY_PARTS_TIME) {
                for(int i = 0; i < arrays.size() && i < MAX_DAY_PART; i++) {
                    QString s = arrays.at(i).toString();
                    this->param.ROOFVENT_DAY_PARTS_TIME_ARRAY[i] = QTime::fromString(s);

                }
            }
            if(name == Param::ROOFVENT_DAY_PARTS_TEMP) {
                for(int i = 0; i < arrays.size() && i < MAX_DAY_PART; i++) {
                    this->param.ROOFVENT_DAY_PARTS_TEMP_ARRAY[i] = arrays.at(i).toDouble();
                }
            }
        }

    }
    return res;
}

void MultiFactorAlgLib::run() {
    // 计算co2注入设备状态
    co2();
    // 计算外遮阳网
    cutain1();
    // 湿帘风机
    fanPad();
    // 喷雾
    mist();
    // 天窗
    roofVent();
    // 侧窗
    sideVent();
    // 内保温网
    cutain3();
    // 加热
    heating();
    // 内遮阳网
    cutain2();
}

void MultiFactorAlgLib::co2() {

    if(climate.co2Dosage < param.map[Param::CO2_MIN].toDouble())
        control.co2 = true;
    else if(climate.co2Dosage > param.map[Param::CO2_MAX].toDouble())
        control.co2 = false;
}

void MultiFactorAlgLib::cutain1() {
    double RAD_MAX = param.map[Param::CUTAIN1_RAD_MAX].toDouble();
    double RAD_MAX_BAND = param.map[Param::CUTAIN1_RAD_MAX_BAND].toDouble();
    double WIND_MAX = param.map[Param::WEATHER_WIND_MAX].toDouble();

    if(weather.storm || weather.wind > WIND_MAX) {
        control.curtain1 = false;
        return;
    }

    if(weather.rad > RAD_MAX + RAD_MAX_BAND)
        control.curtain1 = true;
    else if (weather.rad < RAD_MAX - RAD_MAX_BAND)
        control.curtain1 = false;

}

void MultiFactorAlgLib::cutain2() {
    double RAD_MAX = param.map[Param::CUTAIN2_RAD_MAX].toDouble();
    double RAD_MAX_BAND = param.map[Param::CUTAIN2_RAD_MAX_BAND].toDouble();

    if(control.curtain3)
        control.curtain2 = true;
    else if(weather.rad > RAD_MAX + RAD_MAX_BAND)
        control.curtain2 = true;
    else if(weather.rad < RAD_MAX - RAD_MAX_BAND)
        control.curtain2 = false;
}

void MultiFactorAlgLib::cutain3() {
//    if (weather.storm || weather.wind > param.map[Param::WEATHER_WIND_MAX].toDouble() || control.mist) {
//        control.curtain3 = false;
//        return;
//    }

    QTime NIGHT_BEGIN_TIME = QTime::fromString(param.map[Param::CUTAIN3_NIGHT_BEGIN_TIME].toString());
    QTime NIGHT_END_TIME = QTime::fromString(param.map[Param::CUTAIN3_NIGHT_END_TIME].toString());


    QTime time = this->getCurrTime();
    bool night = false;
    if(time >= NIGHT_BEGIN_TIME || time <= NIGHT_END_TIME) {
        night = true;
    }

    bool winter = false;
    int WINTER_BEGIN_DAYSINYEAR = param.map[Param::CUTAIN3_WINTER_BEGIN_DAYSINYEAR].toInt();
    int WINTER_END_DAYSINYEAR = param.map[Param::CUTAIN3_WINTER_END_DAYSINYEAR].toInt();

    QDate date = this->getCurrDate();
    if(date.dayOfYear() >= WINTER_BEGIN_DAYSINYEAR || date.dayOfYear() <= WINTER_END_DAYSINYEAR) {
        winter = true;
    }

    if(night && winter) {
        control.curtain3 = true;
    } else {
        control.curtain3 = false;
    }

}

void MultiFactorAlgLib::fanPad() {
    double TEMPMAX = param.map[Param::FANPAD_TEMPMAX].toDouble();
    double TEMPMAX_BAND = param.map[Param::FANPAD_TEMPMAX_BAND].toDouble();

    if (climate.temp < TEMPMAX - TEMPMAX_BAND)
        control.fanPad = false;
    else if(climate.temp > TEMPMAX + TEMPMAX_BAND)
        control.fanPad = true;
}

void MultiFactorAlgLib::heating() {

    if(climate.temp < param.map[Param::HEATING_TEMP].toDouble() - param.map[Param::HEATING_TEMP_BAND].toDouble())
        control.heating = true;
    else if(climate.temp > param.map[Param::HEATING_TEMP].toDouble() + param.map[Param::HEATING_TEMP_BAND].toDouble())
        control.heating = false;
}

void MultiFactorAlgLib::mist() {
    double TEMPMAX = param.map[Param::MIST_TEMPMAX].toDouble();
    double TEMPMAX_BAND = param.map[Param::MIST_TEMPMAX_BAND].toDouble();

    if(climate.temp > TEMPMAX + TEMPMAX_BAND)
        control.mist = true;
    else if(climate.temp < TEMPMAX - TEMPMAX_BAND)
        control.mist = false;
}

void MultiFactorAlgLib::sideVent() {
    double SIDE_OPEN_TEMP = param.map[Param::SIDE_OPEN_TEMP_PLUS].toDouble();
    double SIDE_VENT_WAITTIME = param.map[Param::SIDE_OPEN_WAITTIME_SECS].toDouble();

    if(weather.storm || control.co2 || control.fanPad || control.roofVent < 99
            || climate.temp <= control.ventTemp + SIDE_OPEN_TEMP) {
        control.sidePreOpen = false;
        control.sideVent = 0;
        return;
    }

    if(!control.sidePreOpen) {
        control.sidePreOpen = true;
        control.preTime = this->getCurrTime();
    } else {
        int secs = control.preTime.msecsTo(this->getCurrTime());
        if(secs > SIDE_VENT_WAITTIME*2) {
            control.sideVent = 100;
        } else if(secs > SIDE_VENT_WAITTIME) {
            control.sideVent = 50;
        } else {
            control.sideVent = 0;
        }
    }

}

void MultiFactorAlgLib::roofVent() {
    control.ventTemp = calVentTemp();
    // 天窗角度
    double WIND_MAX = param.map[Param::WEATHER_WIND_MAX].toDouble();
    double FROST_TEMP = param.map[Param::WEATHER_FROST_TEMP].toDouble();
    if(weather.storm || weather.frost || weather.wind > WIND_MAX || control.co2 || weather.temp < FROST_TEMP) {
        control.roofVent = 0;
        return;
    }
    double TEMP_CLIMATE_MIN = param.map[Param::ROOFVENT_TEMP_CLIMATE_MIN].toDouble();

    if(weather.temp < TEMP_CLIMATE_MIN)
        control.roofVent = 10;
    else if(weather.temp < TEMP_CLIMATE_MIN)
        control.roofVent = 20;
    else if(weather.temp < control.ventTemp)
        control.roofVent = 50;
    else
        control.roofVent = 100;
}

double MultiFactorAlgLib::calVentTemp() {
    // 初始通风温度
    double VENT_TEMP_PARAM[MAX_DAY_PART]; // [25 27 25 17];
    for(int i = 0; i < param.map[Param::ROOFVENT_DAY_PART_COUNT].toInt() && i < MAX_DAY_PART; i++) {
        VENT_TEMP_PARAM[i] = param.ROOFVENT_DAY_PARTS_TEMP_ARRAY[i];
    }
    int period = this->getPeriodByTime();
    double vt = VENT_TEMP_PARAM[period];

    // 湿度修正通风温度 DH:70 R:15 L:3 H:2
    double DES_HUMIDITY = param.map[Param::ROOFVENT_DES_HUMIDITY].toDouble();
    double HUMIDITY_INF_RANGE = param.map[Param::ROOFVENT_HUMIDITY_INF_RANGE].toDouble();
    double HUMIDITY_INF_TEMP_LOW = param.map[Param::ROOFVENT_HUMIDITY_INF_TEMP_LOW].toDouble();
    double HUMIDITY_INF_TEMP_HIGH = param.map[Param::ROOFVENT_HUMIDITY_INF_TEMP_HIGH].toDouble();

    double deltaH = 0;
    if(climate.humidity <= DES_HUMIDITY-HUMIDITY_INF_RANGE-5)
        deltaH = HUMIDITY_INF_TEMP_LOW;
    else if(climate.humidity <= DES_HUMIDITY - 5)
        deltaH = (DES_HUMIDITY-5-climate.humidity) * HUMIDITY_INF_TEMP_LOW/HUMIDITY_INF_RANGE;
    else if(climate.humidity >= DES_HUMIDITY + 5 && climate.humidity <= 101)
        deltaH = -(climate.humidity - DES_HUMIDITY - 5)*HUMIDITY_INF_TEMP_HIGH/(100-DES_HUMIDITY-5);

    vt += deltaH;

    // 光照修正通风温度
    double RAD_SET = param.map[Param::ROOFVENT_RAD_SET].toDouble();
    double RAD_TEMP_INF_L = param.map[Param::ROOFVENT_RAD_INF_LOW].toDouble();
    double RAD_TEMP_INF_H = param.map[Param::ROOFVENT_RAD_INF_HIGH].toDouble();
    double RAD_INF_K = param.map[Param::ROOFVENT_RAD_INF_K].toDouble();

    double deltaR = RAD_INF_K * (RAD_SET - weather.rad);
    if(deltaR > RAD_TEMP_INF_L)
        deltaR = RAD_TEMP_INF_L;
    else if(deltaR < -RAD_TEMP_INF_H)
        deltaR = -RAD_TEMP_INF_H;

    vt += deltaR;
    return vt <= 0 ? 0 : vt;
}

int MultiFactorAlgLib::getPeriodByTime() {
    QTime curTime = this->getCurrTime();
    int pCnt = param.map[Param::ROOFVENT_DAY_PART_COUNT].toInt();
    for(int i = 0; i < pCnt; i++) {
        if(param.ROOFVENT_DAY_PARTS_TIME_ARRAY[i] > curTime) {
            return i-1 < 0 ? pCnt-1 : i-1;
        }
    }
    return pCnt-1;
}

QDate MultiFactorAlgLib::getCurrDate() {
    return this->currDate;
}

QTime MultiFactorAlgLib::getCurrTime() {
    return this->currTime;
}
