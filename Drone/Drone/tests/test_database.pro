QT += core sql
CONFIG += c++11
INCLUDEPATH += ..
SOURCES += test_database.cpp \
    ../src/database/DatabaseManager.cpp \
    ../src/database/DroneDAO.cpp \
    ../src/database/FlightLogDAO.cpp \
    ../src/database/GeoFenceDAO.cpp
