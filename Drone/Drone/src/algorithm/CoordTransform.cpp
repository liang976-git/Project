#include "CoordTransform.h"
#include <cmath>

const double a = 6378245.0;
const double ee = 0.00669342162296594323;

bool outOfChina(double lat, double lng) {
    return lng < 72.004 || lng > 137.8347 || lat < 0.8293 || lat > 55.8271;
}

static double transformLat(double x, double y) {
    double ret = -100.0 + 2.0 * x + 3.0 * y + 0.2 * y * y + 0.1 * x * y + 0.2 * sqrt(fabs(x));
    ret += (20.0 * sin(6.0 * x * M_PI) + 20.0 * sin(2.0 * x * M_PI)) * 2.0 / 3.0;
    ret += (20.0 * sin(y * M_PI) + 40.0 * sin(y / 3.0 * M_PI)) * 2.0 / 3.0;
    ret += (160.0 * sin(y / 12.0 * M_PI) + 320.0 * sin(y * M_PI / 30.0)) * 2.0 / 3.0;
    return ret;
}

static double transformLng(double x, double y) {
    double ret = 300.0 + x + 2.0 * y + 0.1 * x * x + 0.1 * x * y + 0.1 * sqrt(fabs(x));
    ret += (20.0 * sin(6.0 * x * M_PI) + 20.0 * sin(2.0 * x * M_PI)) * 2.0 / 3.0;
    ret += (20.0 * sin(x * M_PI) + 40.0 * sin(x / 3.0 * M_PI)) * 2.0 / 3.0;
    ret += (150.0 * sin(x / 12.0 * M_PI) + 300.0 * sin(x / 30.0 * M_PI)) * 2.0 / 3.0;
    return ret;
}

void wgs84ToGcj02(double wgsLat, double wgsLng, double &gcjLat, double &gcjLng) {
    if (outOfChina(wgsLat, wgsLng)) {
        gcjLat = wgsLat;
        gcjLng = wgsLng;
        return;
    }
    double dLat = transformLat(wgsLng - 105.0, wgsLat - 35.0);
    double dLng = transformLng(wgsLng - 105.0, wgsLat - 35.0);
    double radLat = wgsLat / 180.0 * M_PI;
    double magic = sin(radLat);
    magic = 1.0 - ee * magic * magic;
    double sqrtMagic = sqrt(magic);
    dLat = (dLat * 180.0) / ((a * (1.0 - ee)) / (magic * sqrtMagic) * M_PI);
    dLng = (dLng * 180.0) / (a / sqrtMagic * cos(radLat) * M_PI);
    gcjLat = wgsLat + dLat;
    gcjLng = wgsLng + dLng;
}

void gcj02ToBd09(double gcjLat, double gcjLng, double &bdLat, double &bdLng) {
    double x = gcjLng, y = gcjLat;
    double z = sqrt(x * x + y * y) + 0.00002 * sin(y * M_PI);
    double theta = atan2(y, x) + 0.000003 * cos(x * M_PI);
    bdLng = z * cos(theta) + 0.0065;
    bdLat = z * sin(theta) + 0.006;
}

void bd09ToGcj02(double bdLat, double bdLng, double &gcjLat, double &gcjLng) {
    double x = bdLng - 0.0065, y = bdLat - 0.006;
    double z = sqrt(x * x + y * y) - 0.00002 * sin(y * M_PI);
    double theta = atan2(y, x) - 0.000003 * cos(x * M_PI);
    gcjLng = z * cos(theta);
    gcjLat = z * sin(theta);
}

void gcj02ToWgs84(double gcjLat, double gcjLng, double &wgsLat, double &wgsLng) {
    if (outOfChina(gcjLat, gcjLng)) {
        wgsLat = gcjLat;
        wgsLng = gcjLng;
        return;
    }
    double wgsLat0 = gcjLat, wgsLng0 = gcjLng;
    double tmpLat, tmpLng;
    for (int i = 0; i < 5; i++) {
        wgs84ToGcj02(wgsLat0, wgsLng0, tmpLat, tmpLng);
        wgsLat0 -= (tmpLat - gcjLat);
        wgsLng0 -= (tmpLng - gcjLng);
    }
    wgsLat = wgsLat0;
    wgsLng = wgsLng0;
}
