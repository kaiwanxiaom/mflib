#ifndef PARAM_H
#define PARAM_H

#include <QObject>
#include <QMap>
#include <QTime>
#include <QVariant>
#define MAX_DAY_PART 6

class Param : public QObject{
    Q_OBJECT
public:

    enum ParamName {
        WEATHER_WIND_MAX,
        WEATHER_FROST_TEMP,

        CO2_MIN,
        CO2_MAX,

        CUTAIN1_RAD_MAX,
        CUTAIN1_RAD_MAX_BAND,

        CUTAIN2_RAD_MAX,
        CUTAIN2_RAD_MAX_BAND,

        CUTAIN3_NIGHT_BEGIN_TIME,
        CUTAIN3_NIGHT_END_TIME,
        CUTAIN3_WINTER_BEGIN_DAYSINYEAR,
        CUTAIN3_WINTER_END_DAYSINYEAR,

        FANPAD_TEMPMAX,
        FANPAD_TEMPMAX_BAND,

        HEATING_TEMP,
        HEATING_TEMP_BAND,

        MIST_TEMPMAX,
        MIST_TEMPMAX_BAND,

        SIDE_OPEN_TEMP_PLUS,
        SIDE_OPEN_WAITTIME_SECS,

        ROOFVENT_DAY_PART_COUNT,
        ROOFVENT_DAY_PARTS_TIME,
        ROOFVENT_DAY_PARTS_TEMP,

        ROOFVENT_DES_HUMIDITY,
        ROOFVENT_HUMIDITY_INF_RANGE,
        ROOFVENT_HUMIDITY_INF_TEMP_LOW,
        ROOFVENT_HUMIDITY_INF_TEMP_HIGH,
        ROOFVENT_RAD_SET,
        ROOFVENT_RAD_INF_LOW,
        ROOFVENT_RAD_INF_HIGH,
        ROOFVENT_RAD_INF_K,
        ROOFVENT_TEMP_CLIMATE_MIN
    };
    Q_ENUM(ParamName)

    QTime ROOFVENT_DAY_PARTS_TIME_ARRAY[MAX_DAY_PART];
    double ROOFVENT_DAY_PARTS_TEMP_ARRAY[MAX_DAY_PART];
    QMap<ParamName, QVariant> map;

};

#endif // PARAM_H
