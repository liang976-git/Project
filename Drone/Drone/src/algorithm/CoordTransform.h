#ifndef COORDTRANSFORM_H
#define COORDTRANSFORM_H

bool outOfChina(double lat, double lng);

void wgs84ToGcj02(double wgsLat, double wgsLng, double &gcjLat, double &gcjLng);

void gcj02ToBd09(double gcjLat, double gcjLng, double &bdLat, double &bdLng);

void bd09ToGcj02(double bdLat, double bdLng, double &gcjLat, double &gcjLng);

void gcj02ToWgs84(double gcjLat, double gcjLng, double &wgsLat, double &wgsLng);

#endif
