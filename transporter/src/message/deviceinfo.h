#pragma once
#include <ArduinoJson.h>
#include <string>

class DeviceInfo {
private:
    int nodeId {0};
    String macAddress;
    String wifiName;
    String version;

    DeviceInfo();

public:
    DeviceInfo(const DeviceInfo&) = delete;
    DeviceInfo& operator=(const DeviceInfo&) = delete;

    static DeviceInfo& getInstance();
    const int getNodeId() const;
    const String& getMacAddress() const;
    const String& getWifiName() const;
    const String& getVersion() const;
    void setNodeId(const int id);
    void setWifiName(const String& ssid);
    void setVersion(const String& ver);
    void setMacAddress(const String& mac);
};





