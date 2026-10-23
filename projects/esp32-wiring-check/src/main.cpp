// Wiring check firmware for robot1 (Pocket ESP AI on an ESP32-C3 SuperMini).
// Flash it once, then add one wire at a time with USB unplugged and read the report.
// It keeps a Wi-Fi AP running while the OLED, microphone and amplifier are active,
// so a wire that breaks Wi-Fi shows up at the step where it was added.

#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <driver/i2s.h>
#include <esp_system.h>
#include <esp_wifi.h>
#include <math.h>
#include <soc/gpio_periph.h>
#include <stdarg.h>

// Pin map from pocket-wall-e-c3/config.h in the Pocket ESP AI source.
constexpr int PIN_I2S_WS = 1;
constexpr int PIN_I2S_BCLK = 2;
constexpr int PIN_AMP_DIN = 3;
constexpr int PIN_MIC_SD = 8;
constexpr int PIN_BUTTON = 10;
constexpr int PIN_OLED_SCL = 20;
constexpr int PIN_OLED_SDA = 21;
// The Pocket firmware hardcodes 0x3C and aborts at boot when the OLED does not answer.
constexpr uint8_t OLED_ADDR = 0x3C;

constexpr char AP_SSID[] = "NUNU-WIRECHECK";  // open AP, diagnostic use only
constexpr int SAMPLE_RATE = 16000;
constexpr i2s_port_t I2S_PORT = I2S_NUM_0;
constexpr uint32_t RTC_MAGIC = 0x52304231;

struct PinSpec {
  int gpio;
  const char* role;
  bool highOk;  // an external pull-up is expected (OLED resistors, GPIO8 board LED)
};

// GPIO9 (BOOT button) and GPIO18/19 (USB) are never touched.
// GPIO2 and GPIO8 are strapping pins with an on-board pull-up on the SuperMini.
const PinSpec kPins[] = {
    {0, "unused", false},   {1, "WS/LRC", false},   {2, "BCLK/SCK", true},
    {3, "AMP DIN", false},  {4, "unused", false},   {5, "unused", false},
    {6, "unused", false},   {7, "unused", false},   {8, "MIC SD", true},
    {10, "BUTTON", false},  {20, "OLED SCL", true}, {21, "OLED SDA", true},
};
constexpr int kPinCount = sizeof(kPins) / sizeof(kPins[0]);

// Survives software/panic/watchdog resets, so a reboot loop is visible in the count.
RTC_NOINIT_ATTR uint32_t rtcMagic;
RTC_NOINIT_ATTR uint32_t rtcBoots;

String report;  // full boot report, reprinted with 'r'
const char* resetName = "?";
int pinIssues = 0;
bool oledFound = false;
uint32_t oledErrors = 0;
bool i2sOk = false;
bool i2sRunning = false;
bool lowTxPower = true;  // this SuperMini's AP is invisible at full power; boot low
bool apOnly = false;
int scanCount = 0;
int scanBestRssi = 0;

int32_t micMin[2];
int32_t micMax[2];
uint32_t micSamples = 0;

void logLine(const char* fmt, ...) {
  char buf[160];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);
  Serial.println(buf);
  report += buf;
  report += '\n';
}

const char* resetReasonName(esp_reset_reason_t r) {
  switch (r) {
    case ESP_RST_POWERON: return "POWERON";
    case ESP_RST_EXT: return "EXT";
    case ESP_RST_SW: return "SOFTWARE";
    case ESP_RST_PANIC: return "PANIC/CRASH";
    case ESP_RST_INT_WDT: return "INT_WDT";
    case ESP_RST_TASK_WDT: return "TASK_WDT";
    case ESP_RST_WDT: return "WDT";
    case ESP_RST_DEEPSLEEP: return "DEEPSLEEP";
    case ESP_RST_BROWNOUT: return "BROWNOUT (power dip!)";
    default: return "OTHER (USB/flash reset)";
  }
}

// Reads every pin with the internal pull-up, then with the pull-down:
//   up=0           -> something holds the pin at GND (short, pressed button, unpowered module)
//   down=1         -> something pulls it high (OLED resistors, GPIO8 LED, or a short to 3V3/5V)
//   up=1, down=0   -> nothing drives the pin ("open"), normal for wires to module inputs
// Then drives each pin low in turn to find two signal wires touching each other.
void checkPins() {
  int up[kPinCount];
  int down[kPinCount];
  logLine("--- PIN CHECK ---");
  for (int i = 0; i < kPinCount; ++i) {
    const int g = kPins[i].gpio;
    pinMode(g, INPUT_PULLUP);
    delay(2);
    up[i] = digitalRead(g);
    pinMode(g, INPUT_PULLDOWN);
    delay(2);
    down[i] = digitalRead(g);

    const char* verdict = "open";
    bool issue = false;
    if (up[i] == 0) {
      verdict = "LOW: tied to GND or module unpowered";
      issue = g != PIN_BUTTON;  // a held button is fine
      if (g == PIN_MIC_SD) {
        // A powered INMP441 drives SD low until SCK starts; an unpowered one leaves it HIGH.
        verdict = "LOW: driven by powered mic";
        issue = false;
      }
    } else if (down[i] == 1) {
      verdict = kPins[i].highOk ? "HIGH: pull-up present" : "HIGH: tied to 3V3/5V?";
      issue = !kPins[i].highOk;
    }
    if (issue) pinIssues++;
    logLine("GPIO%-2d %-9s up=%d down=%d  %s%s", g, kPins[i].role, up[i], down[i], verdict,
            issue ? "  <-- CHECK" : "");
  }

  for (int i = 0; i < kPinCount; ++i) pinMode(kPins[i].gpio, INPUT_PULLUP);
  for (int a = 0; a < kPinCount; ++a) {
    if (up[a] == 0) continue;
    pinMode(kPins[a].gpio, OUTPUT);
    digitalWrite(kPins[a].gpio, LOW);
    delayMicroseconds(200);
    for (int b = a + 1; b < kPinCount; ++b) {
      if (up[b] == 1 && digitalRead(kPins[b].gpio) == 0) {
        logLine("BRIDGE: GPIO%d and GPIO%d are connected to each other  <-- CHECK",
                kPins[a].gpio, kPins[b].gpio);
        pinIssues++;
      }
    }
    pinMode(kPins[a].gpio, INPUT_PULLUP);
  }
  for (int i = 0; i < kPinCount; ++i) pinMode(kPins[i].gpio, INPUT);
  logLine("pin check: %s", pinIssues ? "ISSUES FOUND" : "OK");
}

void scanNetworks() {
  const int n = WiFi.scanNetworks(false, true);
  scanCount = n > 0 ? n : 0;
  scanBestRssi = -127;
  for (int i = 0; i < scanCount; ++i) scanBestRssi = max(scanBestRssi, (int)WiFi.RSSI(i));
  WiFi.scanDelete();
}

void startWifi() {
  WiFi.persistent(false);
  WiFi.mode(WIFI_AP_STA);  // STA side is only used for the receive check scan
  const bool ok = WiFi.softAP(AP_SSID, nullptr, 1, 0, 4);
  WiFi.setTxPower(lowTxPower ? WIFI_POWER_8_5dBm : WIFI_POWER_19_5dBm);
  logLine("Wi-Fi AP %s: %s (open, channel 1, tx %.1f dBm)", AP_SSID, ok ? "started" : "FAILED",
          WiFi.getTxPower() / 4.0f);
  scanNetworks();
  logLine("Wi-Fi receive check: %d networks seen, strongest %d dBm", scanCount, scanBestRssi);
}

bool oledCmd(uint8_t c) {
  Wire.beginTransmission(OLED_ADDR);
  Wire.write(0x00);
  Wire.write(c);
  return Wire.endTransmission() == 0;
}

void oledInit() {
  // Minimal SSD1306 128x64 init: charge pump on, horizontal addressing, display on.
  static const uint8_t seq[] = {0xAE, 0xD5, 0x80, 0xA8, 0x3F, 0xD3, 0x00, 0x40, 0x8D, 0x14,
                                0x20, 0x00, 0xA1, 0xC8, 0xDA, 0x12, 0x81, 0xCF, 0xD9, 0xF1,
                                0xDB, 0x40, 0xA4, 0xA6, 0xAF};
  for (uint8_t c : seq) {
    if (!oledCmd(c)) oledErrors++;
  }
}

// Blinking 8x8 checkerboard: if it keeps blinking, SDA/SCL/VCC/GND are all solid.
void oledDraw(bool phase) {
  bool ok = oledCmd(0x21) && oledCmd(0) && oledCmd(127) && oledCmd(0x22) && oledCmd(0) && oledCmd(7);
  for (int i = 0; ok && i < 1024; i += 16) {
    Wire.beginTransmission(OLED_ADDR);
    Wire.write(0x40);
    for (int j = 0; j < 16; ++j) {
      const int col = (i + j) % 128;
      const int page = (i + j) / 128;
      Wire.write(((col / 8 + page + phase) & 1) ? 0xFF : 0x00);
    }
    ok = Wire.endTransmission() == 0;
  }
  if (!ok) oledErrors++;
}

void scanI2c() {
  Wire.begin(PIN_OLED_SDA, PIN_OLED_SCL, 100000);
  Wire.setTimeOut(20);
  String found;
  for (uint8_t a = 1; a < 127; ++a) {
    Wire.beginTransmission(a);
    if (Wire.endTransmission() == 0) {
      char hex[8];
      snprintf(hex, sizeof(hex), " 0x%02X", a);
      found += hex;
      if (a == OLED_ADDR) oledFound = true;
    }
  }
  logLine("I2C scan (SDA=GPIO21 SCL=GPIO20):%s", found.length() ? found.c_str() : " nothing");
  if (!oledFound) {
    logLine("OLED not at 0x3C -> Pocket firmware will reboot-loop before Wi-Fi starts");
  }
  Wire.setClock(400000);
}

bool startI2s() {
  // One I2S controller in full duplex: the mic and the amp share BCLK/WS, like the Pocket firmware.
  i2s_config_t cfg = {};
  cfg.mode = static_cast<i2s_mode_t>(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_RX);
  cfg.sample_rate = SAMPLE_RATE;
  cfg.bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT;
  cfg.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT;
  cfg.communication_format = I2S_COMM_FORMAT_STAND_I2S;
  cfg.dma_buf_count = 4;
  cfg.dma_buf_len = 256;
  cfg.tx_desc_auto_clear = true;  // output silence when nothing is written
  if (i2s_driver_install(I2S_PORT, &cfg, 0, nullptr) != ESP_OK) return false;

  i2s_pin_config_t pins = {};
  pins.mck_io_num = I2S_PIN_NO_CHANGE;
  pins.bck_io_num = PIN_I2S_BCLK;
  pins.ws_io_num = PIN_I2S_WS;
  pins.data_out_num = PIN_AMP_DIN;
  pins.data_in_num = PIN_MIC_SD;
  return i2s_set_pin(I2S_PORT, &pins) == ESP_OK;
}

void resetMicWindow() {
  for (int s = 0; s < 2; ++s) {
    micMin[s] = INT32_MAX;
    micMax[s] = INT32_MIN;
  }
  micSamples = 0;
}

void readMic() {
  static int32_t buf[256];
  size_t bytes = 0;
  if (!i2sRunning || i2s_read(I2S_PORT, buf, sizeof(buf), &bytes, pdMS_TO_TICKS(20)) != ESP_OK) return;
  const int n = bytes / sizeof(int32_t);
  for (int i = 0; i < n; ++i) {
    const int slot = i & 1;
    const int32_t s = buf[i] >> 8;  // INMP441: 24-bit sample left-aligned in a 32-bit slot
    micMin[slot] = min(micMin[slot], s);
    micMax[slot] = max(micMax[slot], s);
  }
  micSamples += n;
}

void describeMic(char* out, size_t len) {
  if (!i2sOk) {
    snprintf(out, len, "I2S FAILED");
    return;
  }
  if (micSamples == 0) {
    snprintf(out, len, "no samples");
    return;
  }
  const int slot = (micMax[0] - micMin[0]) >= (micMax[1] - micMin[1]) ? 0 : 1;
  const int32_t span = micMax[slot] - micMin[slot];
  if (span == 0) {
    snprintf(out, len, "STUCK at %ld (no data)", (long)micMax[slot]);
    return;
  }
  const float dbfs = 20.0f * log10f((span / 2.0f) / 8388608.0f);
  // Per-slot level: with L/R tied to GND the mic drives one slot only; audio on both
  // slots, or on the other slot, means the L/R pin is floating.
  float slotDb[2];
  for (int s = 0; s < 2; ++s) {
    const int32_t sp = micMax[s] - micMin[s];
    slotDb[s] = sp > 0 ? 20.0f * log10f((sp / 2.0f) / 8388608.0f) : -999.0f;
  }
  snprintf(out, len, "%.0f dBFS%s s0=%.0f s1=%.0f", dbfs,
           dbfs > -6 ? " (too loud: SD floating?)" : "", slotDb[0], slotDb[1]);
}

void recordMic() {
  // Streams 4 s of slot 0 as raw 16-bit little-endian PCM (16 kHz mono) between text markers,
  // so the laptop can turn it into a WAV and the user can hear what WakeNet hears.
  const int kSamples = SAMPLE_RATE * 4;
  Serial.printf("PCMSTART %d\n", kSamples * 2);
  static int32_t buf[256];
  static int16_t pcm[128];
  int sent = 0;
  while (sent < kSamples) {
    size_t bytes = 0;
    if (i2s_read(I2S_PORT, buf, sizeof(buf), &bytes, pdMS_TO_TICKS(100)) != ESP_OK) break;
    int n = 0;
    for (int i = 0; i + 1 < (int)(bytes / sizeof(int32_t)) && sent + n < kSamples; i += 2) {
      pcm[n++] = (int16_t)(buf[i] >> 16);
    }
    Serial.write(reinterpret_cast<uint8_t*>(pcm), n * sizeof(int16_t));
    sent += n;
  }
  Serial.print("\nPCMEND\n");
}

void beep() {
  // 1 kHz sine at -20 dBFS for 0.6 s, same sample on both slots.
  static int32_t frame[32];
  for (int i = 0; i < 16; ++i) {
    const int32_t v = (int32_t)(sinf(2.0f * PI * i / 16.0f) * 0.1f * INT32_MAX);
    frame[2 * i] = v;
    frame[2 * i + 1] = v;
  }
  size_t written = 0;
  for (int k = 0; k < 600; ++k) i2s_write(I2S_PORT, frame, sizeof(frame), &written, portMAX_DELAY);
}

void printStatus() {
  char mic[72];
  describeMic(mic, sizeof(mic));
  const char* oled = !oledFound ? "none" : (oledErrors ? "ERRORS" : "0x3C OK");
  Serial.printf("[%5lus] reset=%s boots=%lu | %s clients=%d tx=%.1fdBm i2s=%s | pins=%s"
                " | oled=%s(err=%lu) | mic=%s | btn=%s | heap=%lu\n",
                millis() / 1000, resetName, (unsigned long)rtcBoots, apOnly ? "AP" : "AP+STA",
                WiFi.softAPgetStationNum(), WiFi.getTxPower() / 4.0f, i2sRunning ? "on" : "OFF",
                pinIssues ? "CHECK" : "OK", oled,
                (unsigned long)oledErrors, mic, digitalRead(PIN_BUTTON) ? "up" : "DOWN",
                (unsigned long)ESP.getFreeHeap());
}

void handleCommands() {
  while (Serial.available()) {
    switch (Serial.read()) {
      case 'r':
        Serial.print(report);
        break;
      case 'b':
        Serial.println("beep 1 kHz 0.6 s");
        beep();
        break;
      case 't':
        lowTxPower = !lowTxPower;
        WiFi.setTxPower(lowTxPower ? WIFI_POWER_8_5dBm : WIFI_POWER_19_5dBm);
        Serial.printf("tx power -> %.1f dBm\n", WiFi.getTxPower() / 4.0f);
        break;
      case 'w':
        scanNetworks();
        Serial.printf("Wi-Fi receive check: %d networks seen, strongest %d dBm\n", scanCount,
                      scanBestRssi);
        if (apOnly) WiFi.mode(WIFI_AP);  // the scan re-enables STA
        break;
      case 'p': {
        // Steps down through TX power levels (quarter-dBm) to find the highest one that works.
        // 10 dBm is the lowest value ESP-IDF's CONFIG_ESP_PHY_MAX_WIFI_TX_POWER accepts.
        static const int8_t kLevels[] = {80, 68, 60, 52, 44, 40, 34};
        static int level = 0;
        level = (level + 1) % (int)sizeof(kLevels);
        esp_wifi_set_max_tx_power(kLevels[level]);
        Serial.printf("tx power -> %.1f dBm (requested %.1f)\n", WiFi.getTxPower() / 4.0f,
                      kLevels[level] / 4.0f);
        break;
      }
      case 'i':
        // Stops the 1 MHz I2S clocks to see whether they disturb the radio.
        if (!i2sOk) break;
        i2sRunning = !i2sRunning;
        if (i2sRunning) i2s_start(I2S_PORT); else i2s_stop(I2S_PORT);
        Serial.printf("i2s -> %s\n", i2sRunning ? "on" : "OFF");
        break;
      case 'g':
        // Samples the live I2S clock pads: a clock toggles (~50% high), a hard short stays at 0%.
        for (int g : {PIN_I2S_WS, PIN_I2S_BCLK}) {
          PIN_INPUT_ENABLE(GPIO_PIN_MUX_REG[g]);
          int high = 0;
          for (int k = 0; k < 10000; ++k) high += gpio_get_level(static_cast<gpio_num_t>(g));
          Serial.printf("GPIO%d live level: high %d.%02d%% of samples\n", g, high / 100, high % 100);
        }
        break;
      case 'm':
        recordMic();
        break;
      case 'a':
        apOnly = !apOnly;
        WiFi.mode(apOnly ? WIFI_AP : WIFI_AP_STA);
        Serial.printf("wifi mode -> %s\n", apOnly ? "AP" : "AP+STA");
        break;
      case 'h':
        Serial.println(
            "commands: r=report b=beep t=toggle tx power p=next tx level w=wifi scan i=toggle i2s a=toggle AP-only");
        break;
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(2000);  // let the USB serial monitor reconnect; never block on it

  const esp_reset_reason_t reason = esp_reset_reason();
  if (reason == ESP_RST_POWERON || rtcMagic != RTC_MAGIC) {
    rtcMagic = RTC_MAGIC;
    rtcBoots = 0;
  }
  rtcBoots++;
  resetName = resetReasonName(reason);

  logLine("=== ROBOT1 WIRING CHECK ===");
  logLine("reset reason: %s, boots since power-on: %lu", resetName, (unsigned long)rtcBoots);
  checkPins();  // before any driver claims the pins
  startWifi();
  scanI2c();
  if (oledFound) oledInit();
  i2sOk = startI2s();
  i2sRunning = i2sOk;
  logLine("I2S BCLK=GPIO2 WS=GPIO1 DOUT=GPIO3 DIN=GPIO8: %s", i2sOk ? "running" : "FAILED");
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  resetMicWindow();
  logLine("commands: r=report b=beep t=toggle tx power p=next tx level w=wifi scan i=toggle i2s a=toggle AP-only");
}

void loop() {
  static uint32_t lastStatus = 0;
  static uint32_t lastOled = 0;
  handleCommands();
  readMic();
  const uint32_t now = millis();
  if (oledFound && now - lastOled >= 1000) {
    lastOled = now;
    oledDraw((now / 1000) & 1);
  }
  if (now - lastStatus >= 2000) {
    lastStatus = now;
    printStatus();
    resetMicWindow();
  }
}
