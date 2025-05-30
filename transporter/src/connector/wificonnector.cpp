#include "wificonnector.h"
#include <WiFi.h>
#include "device/light.h"
WiFiConnector::WiFiConnector(const std::string& ssid, const std::string& password, std::shared_ptr<Light> ledDebug)
    : mSsid(ssid), mPassword(password), mLedDebug(ledDebug) {
        
}

std::string WiFiConnector::getWiFiStatusString(wl_status_t status) {
    switch (status) {
        case WL_NO_SHIELD:
            return "No Shield";
        case WL_IDLE_STATUS:
            return "Idle Status";
        case WL_NO_SSID_AVAIL:
            return "No SSID Available";
        case WL_SCAN_COMPLETED:
            return "Scan Completed";
        case WL_CONNECTED:
            return "Connected";
        case WL_CONNECT_FAILED:
            return "Connect Failed";
        case WL_CONNECTION_LOST:
            return "Connection Lost";
        case WL_DISCONNECTED:
            return "Disconnected";
        default:
            return "Unknown Status";
    }
}

/**
 * @brief Attempts to connect to the configured WiFi network and synchronize time via NTP.
 *
 * This method performs the following steps:
 * - Initializes the debug LED pin.
 * - Starts the WiFi connection process in station mode using the provided SSID and password.
 * - Registers a WiFi event handler.
 * - Waits up to 5 seconds for a successful connection, toggling the debug LED and printing status updates.
 * - If connection fails, turns off the debug LED, disconnects WiFi, and sets the connection status to false.
 * - If connection succeeds:
 *   - Turns on the debug LED to indicate success.
 *   - Prints the assigned IP address.
 *   - Attempts to synchronize the system time with an NTP server.
 *   - Stores the epoch time and current millis for future reference.
 *   - Prints the obtained current time.
 *
 * @note Adjust `gmtOffset_sec` and `daylightOffset_sec` as needed for your timezone and daylight saving time.
 * @note The debug LED is used to indicate connection status: blinking during connection attempts, off on failure, on when connected.
 *
 * @see WiFiConnector::handlerWiFiEvent
 * @see getWiFiStatusString
 */
/**
 * @brief Attempts to connect the device to a WiFi network using the provided SSID and password.
 *
 * This method configures the WiFi module in station mode and initiates a connection to the specified WiFi network.
 * It provides visual feedback using an LED (mLedDebug) and prints status messages to the serial console.
 * The connection attempt is limited to 5 seconds. If the connection fails, the method ensures the LED is turned off,
 * disconnects from WiFi, and sets the connection status flag (mIsConnected) to false.
 * 
 * On successful connection, the method:
 * - Turns the LED on to indicate success.
 * - Prints the assigned IP address.
 * - Attempts to synchronize the system time with an NTP server ("pool.ntp.org").
 * - Stores the epoch time and the current millis() value for future reference.
 * - Prints the current date and time obtained from the NTP server.
 *
 * @note The method uses blocking delays during the connection attempt and time synchronization.
 * @note Adjust `gmtOffset_sec` and `daylightOffset_sec` as needed for your timezone.
 * 
 * @warning If time synchronization fails, the method prints an error and returns without updating time-related members.
 */
void WiFiConnector::connect() {

    Serial.printf("Connecting to WiFi...\nSSID: %s\n", mSsid.c_str());
    WiFi.mode(WIFI_STA);
    WiFi.begin(mSsid.c_str(), mPassword.c_str());
    WiFi.onEvent(WiFiConnector::handlerWiFiEvent);
    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 5000) {
        mLedDebug->on(); // Toggle LED to indicate connection attempt
        delay(500);
        mLedDebug->off();
        delay(500);
        Serial.printf("Attempting to connect to WiFi, Status: %s\n", getWiFiStatusString(WiFi.status()).c_str());
    }

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Failed to connect to WiFi within 5 seconds. Disconnecting...");
        mIsConnected = false;
        mLedDebug->off();
        WiFi.disconnect();
        return ;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi connected.");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        mIsConnected = true;
        mLedDebug->on();
        Serial.println("Getting time from NTP server...");
        // Configure time from NTP
        const char* ntpServer = "pool.ntp.org";
        // Vietnam timezone is UTC+7, so offset is 7*3600 = 25200 seconds
        const long gmtOffset_sec = 25200; // UTC+7 for Vietnam
        const int daylightOffset_sec = 0; // Vietnam does not use daylight saving time
        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

        // Wait for time synchronization
        struct tm timeinfo;
        if (!getLocalTime(&timeinfo)) {
            Serial.println("Failed to obtain time");
            return;
        }
        mEpochStart = mktime(&timeinfo);
        mMillisStart = millis(); 
        // Print the current time
        Serial.println("Current time get from NTP Server:");
        Serial.printf("%02d:%02d:%02d %02d/%02d/%04d\n",
            timeinfo.tm_hour,
            timeinfo.tm_min,
            timeinfo.tm_sec,
            timeinfo.tm_mday,
            timeinfo.tm_mon + 1,
            timeinfo.tm_year + 1900
        );
    }
}
void WiFiConnector::disconnect() {  
    Serial.println("Disconnecting from WiFi...");
    WiFi.disconnect();
    mIsConnected = false;
    mLedDebug->off();
    Serial.println("WiFi disconnected.");
}

void WiFiConnector::handlerWiFiEvent(WiFiEvent_t event) {
    switch (event) {
        case SYSTEM_EVENT_STA_DISCONNECTED:
            Serial.println("[Event] WiFi disconnected. Attempting reconnect...");
            WiFi.reconnect();
            break;

        case SYSTEM_EVENT_STA_CONNECTED:
            Serial.println("[Event] WiFi connected.");
            break;

        case SYSTEM_EVENT_STA_GOT_IP:
            Serial.print("[Event] Got IP: ");
            Serial.println(WiFi.localIP());
            break;

        default:
            break;
    }
}


void WiFiConnector::setSsid(const std::string& newSSID) {
    mSsid = newSSID;
}

void WiFiConnector::setPassword(const std::string& newPassword) {
    mPassword = newPassword;
}

std::string WiFiConnector::getSsid() const {
    return mSsid;
}

std::string WiFiConnector::getPassword() const {
    return mPassword;
}

bool WiFiConnector::isConnected() const {
    return mIsConnected;
}

void WiFiConnector::setEpochStart(time_t epochStart) {
    mEpochStart = epochStart;
}

time_t WiFiConnector::getEpochStart() const {
    return mEpochStart;
}

void WiFiConnector::setMillisStart(uint32_t millisStart) {
    mMillisStart = millisStart;
}

uint32_t WiFiConnector::getMillisStart() const {
    return mMillisStart;
}

String WiFiConnector::getCurrentTime() const
{
    time_t now;
    if (mIsConnected) {
        now = mEpochStart + (millis() - mMillisStart) / 1000;
    } else {
        struct tm defaultTime = {};
        defaultTime.tm_year = 2025 - 1900; // Year 2025
        defaultTime.tm_mon = 0;            // January
        defaultTime.tm_mday = 1;           // 1st
        defaultTime.tm_hour = 0;
        defaultTime.tm_min = 0;
        defaultTime.tm_sec = 0;
        now = mktime(&defaultTime);
    }
    struct tm* nowTm = localtime(&now);
    char buffer[20];
    snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d %02d/%02d/%04d",
             nowTm->tm_hour, nowTm->tm_min, nowTm->tm_sec,
             nowTm->tm_mday, nowTm->tm_mon + 1, nowTm->tm_year + 1900);
    return String(buffer);
}

WiFiConnector::WifiConnectorBuilder& WiFiConnector::WifiConnectorBuilder::setSsid(const std::string& ssid) {
    mSsid = ssid;
    mIsSetSsid = true;
    return *this;
}

WiFiConnector::WifiConnectorBuilder& WiFiConnector::WifiConnectorBuilder::setPassword(const std::string& password) {
    mPassword = password;
    mIsSetPassword = true;
    return *this;
}

WiFiConnector::WifiConnectorBuilder& WiFiConnector::WifiConnectorBuilder::setLedDebug(std::shared_ptr<Light> ledDebug) {
    mLedDebug = ledDebug;
    return *this;
}

WiFiConnector WiFiConnector::WifiConnectorBuilder::build() const {
    if (mPassword.length() < 8) {
        Serial.println("[Error]: Password should be at least 8 characters");
    }
    if (mSsid.empty()) {
        Serial.println("[Error]: SSID cannot be empty");
        return WiFiConnector("", "", mLedDebug);
    }
    if (!mIsSetSsid || !mIsSetPassword) {

        Serial.println("Error: SSID and Password are required");
        return WiFiConnector("", "", mLedDebug);
    }
    return WiFiConnector(mSsid, mPassword, mLedDebug);
}
