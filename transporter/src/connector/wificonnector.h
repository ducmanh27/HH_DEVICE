/**
 * @author ManhPD9
 */
#ifndef WIFI_CONNECTOR_H
#define WIFI_CONNECTOR_H
#include <WiFi.h>
#include <cstdint>
/**
 * @class WiFiConnector
 * @brief Manages WiFi connectivity for ESP32 devices, including connection, disconnection, and status management.
 *
 * This class provides an interface to connect to and disconnect from WiFi networks,
 * manage connection credentials, and track connection status and timing information.
 * It also includes a nested builder class for convenient and flexible instantiation.
 *
 * @note This class is designed for use with ESP32 and depends on ESP-IDF or Arduino-ESP32 WiFi libraries.
 *
 * @section Usage
 * Example usage:
 * @code
 * WiFiConnector connector("MySSID", "MyPassword");
 * connector.connect();
 * if (connector.isConnected()) {
 *     // Connected to WiFi
 * }
 * @endcode
 *
 * Alternatively, use the builder:
 * @code
 * WiFiConnector connector = WiFiConnector::WifiConnectorBuilder()
 *     .setSsid("MySSID")
 *     .setPassword("MyPassword")
 *     .setLedDebug(2)
 *     .build();
 * @endcode
 *
 * @section Members
 * - WiFiConnector(): Default constructor.
 * - WiFiConnector(const std::string& ssid, const std::string& password, uint8_t ledDebug = 2): Parameterized constructor.
 * - void connect(): Initiates connection to the configured WiFi network.
 * - void disconnect(): Disconnects from the current WiFi network.
 * - bool isConnected() const: Returns true if connected to WiFi.
 * - static void handlerWiFiEvent(WiFiEvent_t event): Handles WiFi events (static).
 * - std::string getSsid() const / setSsid(const std::string&): Get/set the WiFi SSID.
 * - std::string getPassword() const / setPassword(const std::string&): Get/set the WiFi password.
 * - time_t getEpochStart() const / setEpochStart(time_t): Get/set the epoch time when connection started.
 * - uint32_t getMillisStart() const / setMillisStart(uint32_t): Get/set the millisecond timestamp when connection started.
 *
 * @section Builder
 * The nested WifiConnectorBuilder class allows step-by-step configuration of WiFiConnector instances.
 * - setSsid(const std::string&): Set the SSID.
 * - setPassword(const std::string&): Set the password.
 * - setLedDebug(uint8_t): Set the debug LED pin.
 * - build(): Construct a WiFiConnector instance with the specified parameters.
 */
class Light;
class WiFiConnector {
public:
    WiFiConnector() = default;
    WiFiConnector(const std::string& ssid, 
        const std::string& password,
        std::shared_ptr<Light> ledDebug);

    void connect();
    void disconnect();
    bool isConnected() const;
    static void handlerWiFiEvent(WiFiEvent_t event);
    std::string getSsid() const;
    void setSsid(const std::string& ssid);

    std::string getPassword() const;
    void setPassword(const std::string& password);

    time_t getEpochStart() const;
    void setEpochStart(time_t epochStart);
    uint32_t getMillisStart() const;
    void setMillisStart(uint32_t millisStart);
    String getCurrentTime() const;
    // Nested Builder class declaration only (no implementation)
    class WifiConnectorBuilder {
    public:
        WifiConnectorBuilder& setSsid(const std::string& ssid);
        WifiConnectorBuilder& setPassword(const std::string& password);
        WifiConnectorBuilder& setLedDebug(std::shared_ptr<Light> ledDebug);
        WiFiConnector build() const;

    private:
        std::string mSsid {""};
        std::string mPassword {""};
        std::shared_ptr<Light> mLedDebug {nullptr};
        bool mIsSetSsid {false};
        bool mIsSetPassword {false};
    };

private:
    std::string getWiFiStatusString(wl_status_t status);
    std::string mSsid {""};
    std::string mPassword {""};
    bool mIsConnected {false};
    uint8_t mMaxRetry {5};
    std::shared_ptr<Light> mLedDebug {nullptr};
    time_t mEpochStart {0};
    uint32_t mMillisStart {0};
};

#endif // WIFI_CONNECTOR_H