#include <QCoreApplication>
#include <QDebug>
#include <cmath>
#include "src/algorithm/GeoFence.h"
#include "src/algorithm/CoordTransform.h"

static int passed = 0, failed = 0;

#define TEST(name, expr) do { \
    if (!(expr)) { \
        qDebug() << "  FAIL:" << name; \
        failed++; \
    } else { \
        qDebug() << "  PASS:" << name; \
        passed++; \
    } \
} while(0)

static bool near(double a, double b, double eps) {
    return fabs(a - b) < eps;
}

void testHaversine() {
    qDebug() << "=== haversineDistance ===";

    double d = haversineDistance(39.9, 116.4, 39.9, 116.4);
    TEST("same point distance 0", d < 0.1);

    d = haversineDistance(39.9, 116.4, 40.0, 116.4);
    TEST("~11km north", d > 10000 && d < 12000);

    d = haversineDistance(0, 0, 0, 1);
    TEST("~111km equator", d > 110000 && d < 112000);
}

void testPointInPolygon() {
    qDebug() << "=== pointInPolygon ===";

    QList<QPair<double,double>> square;
    square.append({0, 0});
    square.append({0, 10});
    square.append({10, 10});
    square.append({10, 0});

    TEST("center inside", pointInPolygon(5, 5, square));

    TEST("outside left", !pointInPolygon(-1, 5, square));
    TEST("outside below", !pointInPolygon(5, -1, square));
    TEST("outside above", !pointInPolygon(5, 11, square));

    TEST("on vertex (boundary edge)", pointInPolygon(0, 0, square));
    TEST("on edge", pointInPolygon(0, 5, square));
}

void testPointInCircle() {
    qDebug() << "=== pointInCircle ===";

    TEST("center inside", pointInCircle(40.0, 116.0, 40.0, 116.0, 1000));
    TEST("within radius", pointInCircle(40.005, 116.0, 40.0, 116.0, 1000));
    TEST("outside radius", !pointInCircle(40.01, 116.0, 40.0, 116.0, 1000));
    TEST("zero radius", pointInCircle(40.0, 116.0, 40.0, 116.0, 0));
    TEST("zero radius off-center", !pointInCircle(40.001, 116.0, 40.0, 116.0, 0));
}

void testPointInRectangle() {
    qDebug() << "=== pointInRectangle ===";

    QList<QPair<double,double>> rect;
    rect.append({1, 1});
    rect.append({1, 5});
    rect.append({5, 5});
    rect.append({5, 1});

    TEST("center inside", pointInRectangle(3, 3, rect));
    TEST("outside", !pointInRectangle(6, 3, rect));
    TEST("on corner", pointInRectangle(1, 1, rect));
}

void testIsInFence() {
    qDebug() << "=== isInFence ===";

    GeoFenceZone circle;
    circle.enabled = true;
    circle.type = Circle;
    circle.centerLat = 40.0;
    circle.centerLng = 116.0;
    circle.radius = 1000;

    TEST("circle inside", isInFence(40.005, 116.0, circle));
    TEST("circle outside", !isInFence(40.02, 116.0, circle));

    circle.enabled = false;
    TEST("disabled fence", !isInFence(40.005, 116.0, circle));
}

void testIsNearFence() {
    qDebug() << "=== isNearFence ===";

    GeoFenceZone circle;
    circle.enabled = true;
    circle.type = Circle;
    circle.centerLat = 40.0;
    circle.centerLng = 116.0;
    circle.radius = 1000;

    TEST("inside still near", isNearFence(40.005, 116.0, circle, 500));
    TEST("buffer zone near", isNearFence(40.012, 116.0, circle, 500));
    TEST("outside buffer", !isNearFence(40.02, 116.0, circle, 500));
}

void testCoordTransform() {
    qDebug() << "=== CoordTransform ===";

    double wgsLat = 39.9, wgsLng = 116.4;
    double gcjLat, gcjLng;
    wgs84ToGcj02(wgsLat, wgsLng, gcjLat, gcjLng);

    TEST("gcj02 differs from wgs84",
         fabs(gcjLat - wgsLat) > 1e-6 || fabs(gcjLng - wgsLng) > 1e-6);

    double bdLat, bdLng;
    gcj02ToBd09(gcjLat, gcjLng, bdLat, bdLng);

    TEST("bd09 differs from gcj02",
         fabs(bdLat - gcjLat) > 1e-6 || fabs(bdLng - gcjLng) > 1e-6);

    double gcjBackLat, gcjBackLng;
    bd09ToGcj02(bdLat, bdLng, gcjBackLat, gcjBackLng);
    TEST("bd09 -> gcj02 roundtrip",
         near(gcjBackLat, gcjLat, 1e-6) && near(gcjBackLng, gcjLng, 1e-6));

    double wgsBackLat, wgsBackLng;
    gcj02ToWgs84(gcjLat, gcjLng, wgsBackLat, wgsBackLng);
    TEST("wgs84 -> gcj02 -> wgs84 roundtrip",
         near(wgsBackLat, wgsLat, 1e-4) && near(wgsBackLng, wgsLng, 1e-4));

    TEST("outOfChina returns false for Beijing",
         !outOfChina(39.9, 116.4));
    TEST("outOfChina returns true for London",
         outOfChina(51.5, -0.1));
}

void testOutOfChinaPassthrough() {
    qDebug() << "=== outOfChina passthrough ===";

    double lat = 51.5, lng = -0.1;
    double gcjLat, gcjLng;
    wgs84ToGcj02(lat, lng, gcjLat, gcjLng);
    TEST("London unchanged lat", near(gcjLat, lat, 1e-10));
    TEST("London unchanged lng", near(gcjLng, lng, 1e-10));
}

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    testHaversine();
    testPointInPolygon();
    testPointInCircle();
    testPointInRectangle();
    testIsInFence();
    testIsNearFence();
    testCoordTransform();
    testOutOfChinaPassthrough();

    qDebug() << "";
    qDebug() << "=== 结果:" << passed << "passed," << failed << "failed ===";

    return failed > 0 ? 1 : 0;
}
