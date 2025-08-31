#include <Arduino.h>
#include <lvgl.h>

#include "GT911.h"
#include "TFT_eSPI.h"

#include "WifiConfig.h"
#include "lvml.h"

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

TFT_eSPI tft;
GT911 gt911;
LVML lvml;

#define BUF_ROWS 120
static lv_color_t *buf1 = NULL; // Primary buffer (will be allocated in PSRAM)
static lv_color_t *buf2 = NULL; // Secondary buffer (will be allocated in PSRAM)

// Tick callback function for LVGL
static uint32_t my_tick(void) { return millis(); }

// Touch input callback (dummy implementation - can be enhanced later)

void touch_read_cb(lv_indev_t *indev, lv_indev_data_t *data) {
  // use GT911_MODE_INTERRUPT for less queries to the touch controller
  if (gt911.touched(GT911_MODE_INTERRUPT)) {
    // Serial.println("Touch detected");
    // Get touch points
    GTPoint *tp = gt911.getPoints();

    // Use first touch point
    uint16_t x = tp[0].x;
    uint16_t y = tp[0].y;

    data->point.x = x;
    data->point.y = y;
    data->state = LV_INDEV_STATE_PRESSED;

    // Serial.printf("Touch: (%d,%d)\n", x, y);
  } else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  // Set the display window to the area we need to update
  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);

  // Push pixels (not swapped colors) to the TFT display
  tft.pushPixels((uint16_t *)px_map, w * h);
  tft.endWrite();

  // Tell LVGL we're done flushing this area
  lv_display_flush_ready(disp);
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("Starting...");
  Serial.println("Build timestamp: " + String(__DATE__) + " " + String(__TIME__));

  // Connect to WiFi
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected!");
  Serial.printf("IP address: %s\n", WiFi.localIP().toString().c_str());

  Serial.println("Initializing TFT display...");
  tft.begin();
  tft.fillScreen(TFT_DARKGREY);
  tft.setRotation(3); // Landscape orientation
  Serial.println("Initializing GT911...");
  gt911.begin(TOUCH_INT_PIN, TOUCH_RESET_PIN);

  Serial.println("Initializing LVGL...");
  lv_init();
  
  Serial.println("Allocating display buffers in PSRAM...");
  buf1 = (lv_color_t*)ps_malloc(320 * BUF_ROWS * sizeof(lv_color_t));
  buf2 = (lv_color_t*)ps_malloc(320 * BUF_ROWS * sizeof(lv_color_t));
  
  if (!buf1 || !buf2) {
    Serial.println("ERROR: Failed to allocate buffers in PSRAM!");
    return;
  }
  Serial.printf("Buffers allocated: buf1=%p, buf2=%p\n", (void*)buf1, (void*)buf2);
  
  Serial.println("Setting tick callback...");
  lv_tick_set_cb(my_tick);

  Serial.println("Creating display...");
  lv_display_t *disp = lv_display_create(320, 240);
  if (disp == NULL) {
    Serial.println("ERROR: Display creation failed!");
    return;
  }
  Serial.println("Display created - setting flush callback...");
  lv_display_set_flush_cb(disp, my_disp_flush);
  Serial.println("Flush callback set, setting buffers...");
  lv_display_set_buffers(disp, buf1, buf2, 320 * BUF_ROWS * sizeof(lv_color_t),
                         LV_DISPLAY_RENDER_MODE_PARTIAL);

  Serial.println("Creating input device...");
  lv_indev_t *touch_indev = lv_indev_create();
  lv_indev_set_type(touch_indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(touch_indev, touch_read_cb);
  lv_indev_set_display(touch_indev, disp);


  // Load XML from LittleFS and register component
  String xmlContent = "";
 
  Serial.printf("Loading main.xml...\n");
  lvml.begin();
  lvml.loadScreenUrl("http://192.168.1.105:8866/main.xml");
  
}

void loop() {
  lv_timer_handler();
  delay(5);
}