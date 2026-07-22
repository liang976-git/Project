QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

LIBS += -lzmq
# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    src/algorithm/CoordTransform.cpp \
    src/algorithm/DouglasPeucker.cpp \
    src/algorithm/GeoFence.cpp \
    src/algorithm/PathPlanner.cpp \
    src/core/CommandSender.cpp \
    src/core/DataDispatcher.cpp \
    src/core/DroneManager.cpp \
    src/core/TelemetryReceiver.cpp \
    src/database/DatabaseManager.cpp \
    src/database/DroneDAO.cpp \
    src/database/FlightLogDAO.cpp \
    src/database/GeoFenceDAO.cpp \
    src/protocol/MavlinkParser.cpp \
    src/protocol/SimulatedLink.cpp \
    src/ui/AlarmWidget.cpp \
    src/ui/AltitudeGaugeWidget.cpp \
    src/ui/CompassWidget.cpp \
    src/ui/DroneListWidget.cpp \
    src/ui/FlightParameterWidget.cpp \
    src/ui/GeoFenceWidget.cpp \
    src/ui/HistoryReplayWidget.cpp \
    src/ui/MapWidget.cpp \
    src/ui/PathPlanningWidget.cpp \
    src/ui/PermissionWidget.cpp \
    src/ui/StatusBarWidget.cpp \
    src/utils/ExcelExporter.cpp \
    src/utils/KMLParser.cpp \
    src/utils/Logger.cpp

HEADERS += \
    mainwindow.h \
    src/algorithm/CoordTransform.h \
    src/algorithm/DouglasPeucker.h \
    src/algorithm/GeoFence.h \
    src/algorithm/PathPlanner.h \
    src/app/AppConfig.h \
    src/core/CommandSender.h \
    src/core/DataDispatcher.h \
    src/core/DroneManager.h \
    src/core/TelemetryReceiver.h \
    src/database/DatabaseManager.h \
    src/database/DroneDAO.h \
    src/database/FlightLogDAO.h \
    src/database/GeoFenceDAO.h \
    src/model/AlarmInfo.h \
    src/model/DroneInfo.h \
    src/model/FlightLog.h \
    src/model/FlightPath.h \
    src/model/GeoFenceZone.h \
    src/protocol/MavlinkMessage.h \
    src/protocol/MavlinkParser.h \
    src/protocol/SimulatedLink.h \
    src/ui/AlarmWidget.h \
    src/ui/AlarmWidget.h \
    src/ui/AltitudeGaugeWidget.h \
    src/ui/CompassWidget.h \
    src/ui/DroneListWidget.h \
    src/ui/FlightParameterWidget.h \
    src/ui/GeoFenceWidget.h \
    src/ui/HistoryReplayWidget.h \
    src/ui/MapWidget.h \
    src/ui/PathPlanningWidget.h \
    src/ui/PermissionWidget.h \
    src/ui/StatusBarWidget.h \
    src/utils/ExcelExporter.h \
    src/utils/KMLParser.h \
    src/utils/Logger.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
