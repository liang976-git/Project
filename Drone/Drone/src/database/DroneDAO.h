#ifndef DRONEDAO_H
#define DRONEDAO_H

#include <QList>
#include <QSqlQuery>
#include "src/model/DroneInfo.h"

class DroneDAO{
public:
    static DroneDAO &instance();
    bool insert(const DroneInfo &drone);
    bool update(const DroneInfo &drone);
    bool remove(int droneId);
    DroneInfo findById(int droneId) const;
    QList<DroneInfo> findAll() const;
    QList<DroneInfo> findByStatus(DroneStatus status) const;
private:
    DroneDAO()=default;
    DroneInfo fromQuery(const QSqlQuery &query) const;
};


#endif //DRONEDAO_H
