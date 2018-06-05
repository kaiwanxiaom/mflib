#include <QCoreApplication>
#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QTime>
#include <QJsonArray>
#include <QProcess>

#include <QtNetwork/QNetworkInterface>

#include "multifactoralglib.h"
#include "actuator.h"


QJsonObject static readfile(QString fileName) {
    QFile paramFile(fileName);

    if(!paramFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "read file fail";
        QJsonObject err;
        return err;
    }

    QString s = paramFile.readAll();
    paramFile.close();
    QJsonObject jo = QJsonDocument::fromJson(s.toUtf8()).object();
//    QJsonObject res = jo.value("RESULT").toObject();
    return jo;
}

void static printReflect(QJsonObject obj) {
    for(QJsonObject::Iterator it = obj.begin(); it!=obj.end(); it++) {
        QString qs = it.key();
        std::string s = qs.toStdString();
        const char* ch = s.c_str();
        qDebug("Q_PROPERTY( double %s READ %s WRITE set%s )", ch, ch, ch);
    }
}

void randTest(QJsonObject sensorJO, MultiFactorAlgLib* mf) {
    QTime startTime = QTime::currentTime();

    int cnt = 500;
    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));
    while(cnt--) {
        double nextTemp = sensorJO["temp"].toDouble() + (qrand()%21-10) / 10.0;
        sensorJO.insert("temp", nextTemp);
//        double nextCO2 = sensorJO["co2Dosage"].toDouble() + (qrand()%101-50);
//        nextCO2 = nextCO2 < 0 ? 0:nextCO2;
//        sensorJO.insert("co2Dosage", nextCO2);
        double nextRad = sensorJO["rad"].toDouble() + (qrand()%11-5);
        sensorJO.insert("rad", nextRad);



        mf->updateClimate(sensorJO);
        mf->updateWeather(sensorJO);
        mf->run();

        Actuator as;
        if(as.nextControlNum == 0) {
            as.control[1] = as.control[0];
            as.control[0] = mf->control;
            as.nextControlNum = 1;
        } else {
            as.control[0] = as.control[1];
            as.control[1] = mf->control;
            as.nextControlNum = 0;
        }

        as.actRoofVent();




        QJsonObject repData = mf->getControl();
        qDebug() << "temp" << nextTemp << "weatherTemp" << mf->weather.temp
                 //<< "heat: " << repData["heating"].toBool()  << endl
                 << "co2dos" << mf->climate.co2Dosage << "CO2:" << repData["co2"].toBool()
                 << "rad" << nextRad
                 //<< "curtain1" << repData["curtain1"].toBool()

                 //<< "curtain2" << repData["curtain2"].toBool()
                 << "curtain3" << repData["curtain3"].toBool()
                 << "mist:" << repData["mist"].toBool()
                 <<"roofVent" << repData["roofVent"].toDouble() << "ventTemp" << mf->control.ventTemp
                 //<< "period" << mf->getPeriodByTime()
                 << "sideVent" << repData["sideVent"].toDouble() //<< "preOpen" << mf->control.sidePreOpen
                    ;

    }

    qDebug() << "Time msecsTo: " << startTime.msecsTo(QTime::currentTime());
}

void static printParam(MultiFactorAlgLib* mf) {
    for(QMap<Param::ParamName, QVariant>::iterator it = mf->param.map.begin(); it != mf->param.map.end(); it++) {
        qDebug() << it.key() << " " << it.value();
    }
    for(int i = 0; i < mf->param.map[Param::ROOFVENT_DAY_PART_COUNT].toInt(); i++) {
        qDebug() << mf->param.ROOFVENT_DAY_PARTS_TEMP_ARRAY[i]
                 << mf->param.ROOFVENT_DAY_PARTS_TIME_ARRAY[i]
                    ;

    }
}

void static testLib() {
    QJsonObject paramJO = readfile("C:\\Users\\test\\Documents\\MyCode\\QT\\testLib\\param.json");
    QJsonObject sensorJO = readfile("C:\\Users\\test\\Documents\\MyCode\\QT\\testLib\\sensor.json");
    MultiFactorAlgLib mf;
    QJsonObject error;

    if(mf.setParam(paramJO) == 1) {
        qDebug() << "存在未识别参数！";
//        QJsonArray arr = error.find("unkownParam").value().toArray();
//        for(int i = 0; i < arr.count(); i++) {
//            qDebug() << arr.at(i).toString();
//        }
    }
    printParam(&mf);
    randTest(sensorJO, &mf);
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);


    QString cpu_id = "";
    QProcess p(0);
    p.start("wmic CPU get ProcessorID");    p.waitForStarted();
    p.waitForFinished();
    cpu_id = QString::fromLocal8Bit(p.readAllStandardOutput());
    qDebug() << cpu_id;
    cpu_id = cpu_id.remove("ProcessorId").trimmed();

    qDebug() << cpu_id;

    return a.exec();
}
