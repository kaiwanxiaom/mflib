#ifndef TEST_H
#define TEST_H

#endif // TEST_H

class MULTIFACTORALGLIBSHARED_EXPORT MultiFactorAlgLib
{
public:
    MultiFactorAlgLib();
    /**
     * @brief setParam 设置控制算法参数
     */
    virtual int setParam(QJsonObject & param);
    /**
     * @brief updateClimate 传入当前室内传感器数据给控制算法
     */
    virtual bool updateClimate(QJsonObject & climate);
    /**
     * @brief updateWeather 传入当前气象站数据给控制算法
     */
    virtual bool updateWeather(QJsonObject & weather);
    /**
     * @brief run 运行控制算法，计算下一步执行机构状态
     */
    virtual void run();
    /**
     * @brief getControl 获取计算所得的下一步执行机构状态
     */
    virtual QJsonObject getControl();
}
