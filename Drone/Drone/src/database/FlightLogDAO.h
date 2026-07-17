#ifndef FLIGHTLOGDAO_H
#define FLIGHTLOGDAO_H

#include <QList>
#include <QSqlQuery>
#include "src/model/FlightLog.h"

class FlightLogDAO{
public:
    static FlightLogDAO &instance();
   bool insertLog(FlightLog &log);
   bool insertPoint(int logId,const FlightLogPoint &point);
   bool removeLog(int logId);
   FlightLog findById(int logId) const;
   QList<FlightLog> findByDroneId(int droneId) const;
   QList<FlightLog> findByTimeRange(const QString &start,const QString &end) const;
   QList<FlightLogPoint> findPointsByLogId(int logId) const;
private:
   FlightLogDAO()=default;
   FlightLog fromQuery(const QSqlQuery &query) const;
   FlightLogPoint fromPointQuery(const QSqlQuery &query) const;


};

#endif //FLIGHTLOGDAO_H
