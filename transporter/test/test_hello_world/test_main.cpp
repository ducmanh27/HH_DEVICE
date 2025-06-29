#include "unity.h"

void setUp(void) {
}

void tearDown(void) {
}

void test_hello_world(void) {
    TEST_ASSERT_EQUAL(1, 1);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_hello_world);
    return UNITY_END();
}
#include "unity.h"

// Mock functions and variables for testing loop()
bool mock_camera_capture_fail = false;
int mock_fb_len = 10;
int mock_http_status = 200;
String mock_http_response = "OK";
bool loop_called = false;

typedef struct {
  uint8_t *buf;
  size_t len;
} camera_fb_t;

camera_fb_t mock_fb;
uint8_t mock_buf[10];

camera_fb_t* esp_camera_fb_get() {
  if (mock_camera_capture_fail) return NULL;
  mock_fb.buf = mock_buf;
  mock_fb.len = mock_fb_len;
  return &mock_fb;
}

void esp_camera_fb_return(camera_fb_t* fb) {
  // Do nothing for mock
}

struct {
  void beginRequest() {}
  void post(const char*) {}
  void sendHeader(const char*, const String&) {}
  void sendHeader(const char*, int) {}
  void beginBody() {}
  void print(const String&) {}
  void write(uint8_t*, size_t) {}
  void endRequest() {}
  int responseStatusCode() { return mock_http_status; }
  String responseBody() { return mock_http_response; }
} http;

void delay(int) {}

namespace {
  int serial_println_count = 0;
  void Serial_println(const char*) { serial_println_count++; }
  void Serial_print(const char*) {}
  void Serial_println(const String&) { serial_println_count++; }
  void Serial_print(int) {}
  void Serial_println(int) { serial_println_count++; }
}

#define Serial Serial_
struct {
  void println(const char* s) { Serial_println(s); }
  void print(const char* s) { Serial_print(s); }
  void println(const String& s) { Serial_println(s); }
  void print(int i) { Serial_print(i); }
  void println(int i) { Serial_println(i); }
} Serial_;

// Copy-paste the loop() function here, but remove delay and use the above mocks
void loop() {
  loop_called = true;
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }

  String boundary = "----123456789";
  String startRequest = "--" + boundary + "\r\n"
              "Content-Disposition: form-data; name=\"image\"; filename=\"esp32.jpg\"\r\n"
              "Content-Type: image/jpeg\r\n\r\n";

  String endRequest = "\r\n--" + boundary + "--\r\n";

  int contentLength = startRequest.length() + fb->len + endRequest.length();

  http.beginRequest();
  http.post("/upload");
  http.sendHeader("Content-Type", "multipart/form-data; boundary=" + boundary);
  http.sendHeader("Content-Length", contentLength);
  http.beginBody();
  http.print(startRequest);
  http.write(fb->buf, fb->len);
  http.print(endRequest);
  http.endRequest();

  int statusCode = http.responseStatusCode();
  String response = http.responseBody();

  Serial.print("Status: ");
  Serial.println(statusCode);
  Serial.println("Response: " + response);

  esp_camera_fb_return(fb);
  // delay(10000); // skip in test
}

void test_loop_camera_capture_success(void) {
  mock_camera_capture_fail = false;
  mock_fb_len = 10;
  mock_http_status = 201;
  mock_http_response = "Uploaded";
  serial_println_count = 0;
  loop_called = false;

  loop();

  TEST_ASSERT_TRUE(loop_called);
  TEST_ASSERT_EQUAL(3, serial_println_count); // Status, status code, response
}

void test_loop_camera_capture_fail(void) {
  mock_camera_capture_fail = true;
  serial_println_count = 0;
  loop_called = false;

  loop();

  TEST_ASSERT_TRUE(loop_called);
  TEST_ASSERT_EQUAL(1, serial_println_count); // Only "Camera capture failed"
}

void test_loop_http_error(void) {
  mock_camera_capture_fail = false;
  mock_fb_len = 10;
  mock_http_status = 500;
  mock_http_response = "Internal Server Error";
  serial_println_count = 0;
  loop_called = false;

  loop();

  TEST_ASSERT_TRUE(loop_called);
  TEST_ASSERT_EQUAL(3, serial_println_count); // Status, status code, response
}

void test_loop_empty_response(void) {
  mock_camera_capture_fail = false;
  mock_fb_len = 10;
  mock_http_status = 200;
  mock_http_response = "";
  serial_println_count = 0;
  loop_called = false;

  loop();

  TEST_ASSERT_TRUE(loop_called);
  TEST_ASSERT_EQUAL(3, serial_println_count); // Status, status code, response
}

// Add these to main()
/*
  RUN_TEST(test_loop_camera_capture_success);
  RUN_TEST(test_loop_camera_capture_fail);
  RUN_TEST(test_loop_http_error);
  RUN_TEST(test_loop_empty_response);
*/