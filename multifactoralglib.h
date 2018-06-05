#ifndef MULTIFACTORALGLIB_H
#define MULTIFACTORALGLIB_H

#include "multifactoralglib_global.h"
#include "algstruct.h"
#include "param.h"
#include <QJsonObject>
#include <QMap>
#include <QTime>
#include <QDate>


class MULTIFACTORALGLIBSHARED_EXPORT MultiFactorAlgLib
{

public:
    MultiFactorAlgLib();

public:
    /**
     * @brief setParam 从QJsonObject中读取设置的参数，用于多因子算法的计算.
     * 具体过程是遍历Json对象键值对，把键存在于enum ParamName枚举类型的字符串相同后进行设置。
     * 存储在QMap<ParamName, QVariant> map中，特殊的参数是数组格式，单独存储。
     * @param param Json的Key值需要与param.h中Param类中的enum ParamName对应，
     * @return error 0:成功 1：存在不匹配的参数名称
     */
    virtual int setParam(QJsonObject & param);

    /**
     * @brief updateClimate 更新算法所需的室内气候数据：温度、湿度、CO2浓度
     * 从Json对象中查询struct Climate内的成员变量名称对应的Key
     * @param climate
     * @return
     */
    virtual bool updateClimate(QJsonObject & climate);

    /**
     * @brief updateWeather 更新算法所需的室外天气数据：温度、光照、雨量、风速等
     * @param weather 从Json对象中查询struct Weather内的成员变量名称对应的Key，
     * frost是否结霜，会根据室外温度的值进行判断，可以不给出数据。
     * @return
     */
    virtual bool updateWeather(QJsonObject & weather);

    /**
     * @brief run 运行一次多因子协调算法，计算出各个控制机构的状态（没有创建线程）。
     * 所以具体运行时间间隔没有进行规定。
     */
    virtual void run();

    /**
     * @brief getControl 获取当前算法计算得出的控制机构状态，在run()函数运行后进行调用。
     * @return
     */
    virtual QJsonObject getControl();

public:
    void co2();
    void cutain1();
    void cutain2();
    void cutain3();
    void heating();
    void mist();
    void roofVent();
    void sideVent();
    void fanPad();

    int getPeriodByTime();
    double calVentTemp();
    QTime getCurrTime();
    QDate getCurrDate();

    Param param;
    struct Climate climate;
    struct Weather weather;
    QTime currTime;
    QDate currDate;

    struct Control control;


};

#endif // MULTIFACTORALGLIB_H
