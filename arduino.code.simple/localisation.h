// localisation.h
#ifndef LOCALISATION_H
#define LOCALISATION_H

#include <ArduinoBLE.h>

#define NUM_MEASUREMENTS 15

struct Position {
  float x;
  float y;
  float z;
};

Position calculatePosition(Position beacons[], float distances[]);

float calculateDistanceToTarget(Position pos1, Position pos2);

float calculateDistance(int rssi);

int calculateAverageRSSI(int beaconIndex, bool historyFilled[], int historyIndex[], int rssiHistory[][NUM_MEASUREMENTS]);

bool findDevices(bool isConnected[], BLEDevice connectedDevices[], const char* targetAddresses[]);

void getCoordinates(bool isConnected[], float RSSI_estimate[], float P[], const float Q[], const float R[], float distances[], BLEDevice connectedDevices[]);

bool detectRightDirection(float distanceToTarget, float &previousDistance);

#endif