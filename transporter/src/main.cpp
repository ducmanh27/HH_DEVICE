#include <Arduino.h>
#include <memory>
#include <vector>
#include <string>
#include <map>
#include <functional>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "connector/wificonnector.h"
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "esp_task_wdt.h"
#include "message/sensordata.h"
#include "message/deviceinfo.h"
#include "device/ledbuiltin.h"
#include "device/command.h"
#include "device/commandmanager.h"
#include "device/lightoncommand.h"
#include "device/lightoffcommand.h"
#define RXD2 16
#define TXD2 17
#define LED_BUILTIN 2

#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
const char* ssid     = "MANHPD9-2.4G";
const char* password = "12345687";
const char* mqtt_server = "192.168.2.30";
const char* topic_node_request = "node/request";
const char* topic_node_reply = "node/reply";
const char* topic_gateway_reply = "gateway/reply";
const char* topic_gateway_request = "gateway/request";
const char* topic_sensor_data = "node/sensor_data";
const char* mqtt_client_id = "ESP32Client";
constexpr uint8_t nodeId = 0;
using MessageHandler = std::function<void(const JsonDocument&)>;

uint32_t value = 0;
WiFiConnector wifiConnector;
WiFiClient wifiClient;
PubSubClient mqttClient;
TaskHandle_t registerTaskHandle;
TaskHandle_t sendSensorDataTaskHandle;
TaskHandle_t receiveSensorDataTaskHandle;
QueueHandle_t sensorDataQueue = nullptr;
std::shared_ptr<Light> ledBuiltin;


// Wifi and MQTT setup functions

void setupWifi(WiFiConnector& connector);
void setupMqttClient(const char* mqtt_server, int port, WiFiClient& wifiClient, PubSubClient& mqttClient, WiFiConnector& wifiConnector);
String getCurrentTime();
void cbMQTTSubscribe(char* topic, byte* payload, unsigned int length);
void reconnectMqtt();

void setupGPIO();
void setupNodeInfo();
// FreeRTOS task declarations
void createAllTasks();
void sendSensorDataTask(void* parameter);
void receiveSensorDataTask(void* parameter);
void registerTask(void* parameter);

// Message handler functions
void handleRegisterReply(const JsonDocument& doc);
void handleControlDevice(const JsonDocument& doc);
void handleKeepAlive(const JsonDocument& doc);
std::map<String, MessageHandler> messageHandlers = {
  {"registerReply", handleRegisterReply},
  {"controlDevice", handleControlDevice},
  {"keepAlive", handleKeepAlive},
  // {"otherType", handleOtherType},
};

CommandManager commandManager;

std::map<String, std::shared_ptr<Command>> commandHandlers = {
  {"lightOn", std::make_shared<LightOnCommand>(std::make_shared<LedBuiltin>(LED_BUILTIN))},
  {"lightOff", std::make_shared<LightOffCommand>(std::make_shared<LedBuiltin>(LED_BUILTIN))},
};

void executeCommandByName(const String& commandName) {
    auto it = commandHandlers.find(commandName);
    if (it != commandHandlers.end()) {
        commandManager.executeCommand(it->second);
    } else {
        Serial.println("Command not found: " + commandName);
    }
}


void setup() {
  ledBuiltin = std::make_shared<LedBuiltin>(LED_BUILTIN);
  Serial.begin(115200);   
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
  setupNodeInfo();
  setupWifi(wifiConnector);
  setupMqttClient(mqtt_server, 1883, wifiClient, mqttClient, wifiConnector);
  setupGPIO();

  sensorDataQueue = xQueueCreate(1, sizeof(SensorData));
  configASSERT(sensorDataQueue != nullptr); 
  createAllTasks();
}
void loop() {
              
}
String getCurrentTime() {
  return wifiConnector.getCurrentTime();
}

void cbMQTTSubscribe(char* topic, byte* payload, unsigned int length)
{
  Serial.println("[" + getCurrentTime() + "] Message arrived [" + String(topic) + "]: ");
  
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, payload, length);

  if (!error) {
    Serial.println(doc.as<String>());
    String msgType = doc["messageType"] | "";
    auto it = messageHandlers.find(msgType);
    if (it != messageHandlers.end()) {
      it->second(doc);
    } else {
      Serial.printf("No handler for messageType of nodeId %d: ", (doc["nodeId"] | 0));
      Serial.println(msgType);
    }
  }
  else {
    Serial.print("Failed to parse JSON: ");
    Serial.println(error.c_str());
  }
}
void reconnectMqtt()
{
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print(getCurrentTime() + " Attempting MQTT connection...\n");
    // Create a random client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (mqttClient.connect(clientId.c_str())) {
      Serial.print("connected MQTT Broker with client ID: ");
      Serial.println(clientId);
      mqttClient.subscribe(topic_gateway_reply);
      mqttClient.subscribe(topic_gateway_request);
    } else {
      Serial.print(getCurrentTime() + " failed, rc=");
      Serial.print(getCurrentTime() + mqttClient.state());
      Serial.println(getCurrentTime() + " try again in 5 seconds");
      esp_task_wdt_reset();
      vTaskDelay(pdMS_TO_TICKS(5000));
    }
    esp_task_wdt_reset(); // Ensure watchdog is reset in every loop iteration
  }
}

void setupGPIO()
{
}

void setupNodeInfo() {
  DeviceInfo::getInstance().setNodeId(nodeId);
  DeviceInfo::getInstance().setWifiName(ssid);
  DeviceInfo::getInstance().setVersion("1.0.0");
  DeviceInfo::getInstance().setMacAddress(WiFi.macAddress());
}

void createAllTasks()
{
  // Create tasks
  BaseType_t status  ;
  status = xTaskCreatePinnedToCore(
    registerTask,                  
    "registerTask",                 
    10000,                          
    NULL,                          
    2,                            
    &registerTaskHandle,           
    0                               
  );
  configASSERT(status == pdPASS);

  status = xTaskCreatePinnedToCore(
    sendSensorDataTask,                
    "sendSensorDataFromNode",      
    20000,                          
    NULL,                         
    2,                             
    &sendSensorDataTaskHandle,           
    0                             
  );
  configASSERT(status == pdPASS);

  status = xTaskCreatePinnedToCore(
    receiveSensorDataTask,
    "ReceiveSensorDataFromDevice",
    10000,
    NULL,
    2,
    &receiveSensorDataTaskHandle,
    1                           
  );
  configASSERT(status == pdPASS);
}

void registerTask(void* parameter) {
  TickType_t lastWakeTime = xTaskGetTickCount();

  while (true) {
    if (!mqttClient.connected()) {
      reconnectMqtt();
    }

    // Always call loop() frequently to process MQTT callbacks
    mqttClient.loop();
  // {
  //   "messageType": "registerReply",
  //   "macAddress": "00:4B:12:39:9E:E0",
  //   "nodeId": 1
  // }
    if (DeviceInfo::getInstance().getNodeId() == 0) {
      JsonDocument doc;
      doc["messageType"] = "register";
      doc["nodeId"] = DeviceInfo::getInstance().getNodeId();
      doc["macAddress"] = DeviceInfo::getInstance().getMacAddress();
      String payload;
      serializeJson(doc, payload);
      mqttClient.publish(topic_node_request, payload.c_str());
    }

    // Instead of a long delay, use a short delay and loop more frequently
    for (int i = 0; i < 30; ++i) { // 30 x 100ms = 3s
      mqttClient.loop();
      vTaskDelay(pdMS_TO_TICKS(100));
    }
  }
}

void sendSensorDataTask(void* parameter) {
  // Suspend itself at start, will be resumed by registerTask
  vTaskSuspend(NULL);

  SensorData data;
  while (true) {
    if (DeviceInfo::getInstance().getNodeId() == 0) {
      vTaskDelay(pdMS_TO_TICKS(1000));
      continue;
    }

    if (!mqttClient.connected()) {
      reconnectMqtt();
    } else {
      mqttClient.loop();
    }

    if (xQueueReceive(sensorDataQueue, &data, pdMS_TO_TICKS(10)) == pdPASS) {
      JsonDocument doc;
      // Info section
      JsonObject info = doc["info"].to<JsonObject>();
      info["nodeId"] = DeviceInfo::getInstance().getNodeId();
      info["macAddress"] = DeviceInfo::getInstance().getMacAddress();
      info["wifiName"] = DeviceInfo::getInstance().getWifiName();
      info["version"] = DeviceInfo::getInstance().getVersion();
      doc["messageType"] = "sensorData";
      // Data section
      JsonObject dataObj = doc["data"].to<JsonObject>();
      dataObj["voltage"] = data.voltage;
      dataObj["current"] = data.current;
      dataObj["power"] = data.power;
      dataObj["energy"] = data.energy;
      dataObj["pf"] = data.pf;
      dataObj["dust"] = data.dust;
      dataObj["frequency"] = data.frequency;
      dataObj["dust"] = data.dust_density;
      dataObj["temperature"] = data.temperature;
      dataObj["humidity"] = data.humidity;

      time_t now;
      if (wifiConnector.isConnected()) {
        now = wifiConnector.getEpochStart() + (millis() - wifiConnector.getMillisStart()) / 1000;
      }
      struct tm* timeinfo = localtime(&now);
      char isoDatetime[25];
      strftime(isoDatetime, sizeof(isoDatetime), "%Y-%m-%dT%H:%M:%S", timeinfo);
      doc["timeStamp"] = isoDatetime;

      String payload;
      serializeJson(doc, payload);

      // Publish to MQTT
      if (!mqttClient.publish(topic_sensor_data, payload.c_str())) {
        Serial.println("Failed to publish sensor data");
      }
      vTaskDelay(pdMS_TO_TICKS(100));
    }
  }
}

void receiveSensorDataTask(void* parameter) {
  const uint16_t bufferSize = 512;
  char buffer[bufferSize];
  int index = 0;

  while (true) {
    while (Serial2.available()) {
      char c = Serial2.read();
      if (c == '\n') {
        buffer[index] = '\0';  // null-terminate
        index = 0;
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, buffer);
        if (!error) {
            SensorData data;
            data.voltage = doc["voltage"] | 0.0f;
            data.current = doc["current"] | 0.0f;
            data.power = doc["power"] | 0.0f;
            data.energy = doc["energy"] | 0.0f;
            data.pf = doc["pf"] | 0.0f;
            data.dust = doc["dust"] | 0.0f;
            data.frequency = doc["frequency"] | 0.0f;
            data.dust_density = doc["dust_density"] | 0.0f;
            data.temperature = doc["temperature"] | 0.0f;
            data.humidity = doc["humidity"] | 0.0f;
            data.time_stamp = doc["time_stamp"] | 0u;

          // Send data to queue (overwrite if full)
          if (sensorDataQueue != nullptr) {
            xQueueOverwrite(sensorDataQueue, &data);
          }
        } else {
          Serial.println("JSON parse failed: " + String(buffer));
        }
      } else {
        if (index < bufferSize - 1)
          buffer[index++] = c;
      }
    }
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

void setupWifi(WiFiConnector& connector) {
  connector = WiFiConnector::WifiConnectorBuilder()
  .setSsid(ssid)
  .setPassword(password)
  .setLedDebug(ledBuiltin)
  .build();
  connector.connect();
}

void setupMqttClient(const char* mqtt_server, int port, WiFiClient& wifiClient, PubSubClient& mqttClient, WiFiConnector& wifiConnector) {
  if (wifiConnector.isConnected()) {
    mqttClient.setClient(wifiClient);
    mqttClient.setServer(mqtt_server, port);
    mqttClient.setCallback(cbMQTTSubscribe);
    mqttClient.setBufferSize(512);
  } else {
    Serial.println("Setup MQTT client because WiFi is not connected");
  }
}

void handleRegisterReply(const JsonDocument& doc) {
  // {
  //   "messageType": "registerReply",
  //   "macAddress":"00:4B:12:39:9E:E0",
  //   "nodeId": 1
  // }
  String mac = doc["macAddress"] | "";
  int newNodeId = doc["nodeId"] | 0;
  if (mac == DeviceInfo::getInstance().getMacAddress()) {
    if (newNodeId != 0 && newNodeId != DeviceInfo::getInstance().getNodeId()) {
      Serial.print("Received new Node ID: ");
      DeviceInfo::getInstance().setNodeId(newNodeId);
      Serial.println("Registered successfully with Node ID: " + String(newNodeId));
      if (sendSensorDataTaskHandle != NULL && eTaskGetState(sendSensorDataTaskHandle) == eSuspended) {
        Serial.println("Resuming sendSensorDataTask");
        vTaskResume(sendSensorDataTaskHandle);
      }
    }
    else if (newNodeId == DeviceInfo::getInstance().getNodeId()) {
      Serial.print("Already registered with Node ID: ");
      Serial.println(DeviceInfo::getInstance().getNodeId());
    } else {
      Serial.println("Received invalid Node ID from gateway: " + String(newNodeId));
      Serial.print("Current Node ID: ");
      Serial.println(DeviceInfo::getInstance().getNodeId());
    }
  }
}


void handleControlDevice(const JsonDocument& doc)
{
  // {
  //   "messageType": "controlDevice",
  //   "command": "lightOn",
  //   "nodeId": 1
  // }

  String commandName = doc["command"] | "";
  int targetNodeId = doc["nodeId"] | 0;
  if (targetNodeId != DeviceInfo::getInstance().getNodeId()) {
    Serial.print("Command for nodeId ");
    Serial.print(targetNodeId);
    Serial.print(" ignored. This nodeId is ");
    Serial.println(DeviceInfo::getInstance().getNodeId());
    return;
  }
  Serial.print("Received command: ");
  Serial.println(commandName);
  executeCommandByName(commandName);
}

void handleKeepAlive(const JsonDocument& doc)
{
  // {
  //   "messageType": "keepAlive",
  //   "nodeId": 1,
  //   "timeStamp": "2023-10-01T12:00:00"
  // }
  int targetNodeId = doc["nodeId"] | 0;
  String mac = doc["macAddress"] | "";
  if (targetNodeId == DeviceInfo::getInstance().getNodeId() && 
      mac == DeviceInfo::getInstance().getMacAddress()) {
    JsonDocument replyDoc;
    replyDoc["messageType"] = "keepAliveReply";
    replyDoc["nodeId"] = DeviceInfo::getInstance().getNodeId();
    replyDoc["macAddress"] = DeviceInfo::getInstance().getMacAddress();
    replyDoc["timeStamp"] = getCurrentTime(); 
    String payload;
    serializeJson(replyDoc, payload);
    mqttClient.publish(topic_node_reply, payload.c_str());
    Serial.print("Keep alive reply sent for nodeId: ");
    Serial.println(DeviceInfo::getInstance().getNodeId());
  }
}