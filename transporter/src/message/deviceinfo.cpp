// deviceinfo.cpp
#include "deviceinfo.h"

DeviceInfo::DeviceInfo() {}

DeviceInfo& DeviceInfo::getInstance() {
    static DeviceInfo instance;
    return instance;
}

const int DeviceInfo::getNodeId() const {
    return nodeId;
}

const String& DeviceInfo::getMacAddress() const {
    return macAddress;
}

const String& DeviceInfo::getWifiName() const {
    return wifiName;
}

const String& DeviceInfo::getVersion() const {
    return version;
}

void DeviceInfo::setNodeId(const int id) {
    nodeId = id;
}

void DeviceInfo::setWifiName(const String& ssid) {
    wifiName = ssid;
}

void DeviceInfo::setVersion(const String& ver) {
    version = ver;
}

void DeviceInfo::setMacAddress(const String& mac) {
        macAddress = mac;
}