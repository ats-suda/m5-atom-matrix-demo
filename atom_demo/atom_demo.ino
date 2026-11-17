// M5Stack ATOM Matrix デモ
// ボタン（LED面を押す）でモード切替:
//   0: レインボー渦巻き
//   1: 傾けるとボールが転がる（内蔵 IMU MPU6886）
//   2: 文字スクロール
//   3: 花火（ランダムにはじける光）
// LED: GPIO27（WS2812 x25）、ボタン: GPIO39、IMU: I2C SDA=25 / SCL=21

#include <Adafruit_NeoPixel.h>
#include <Wire.h>

const int LED_PIN = 27;
const int BTN_PIN = 39;
const int NUM_LEDS = 25;
const uint8_t BRIGHTNESS = 20;  // ATOM Matrix は発熱するので低めにする
const uint8_t IMU_ADDR = 0x68;
const int NUM_MODES = 4;

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

int mode = 0;
bool imuOk = false;

// ---------- 共通 ----------
void setXY(int x, int y, uint32_t c) {
  if (x < 0 || x > 4 || y < 0 || y > 4) return;
  strip.setPixelColor(y * 5 + x, c);
}

// ---------- IMU (MPU6886) ----------
void imuWrite(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(IMU_ADDR);
  Wire.write(reg);
  Wire.write(val);
  Wire.endTransmission();
}

bool imuInit() {
  Wire.begin(25, 21);
  Wire.beginTransmission(IMU_ADDR);
  Wire.write(0x75);  // WHO_AM_I
  if (Wire.endTransmission(false) != 0) return false;
  Wire.requestFrom(IMU_ADDR, (uint8_t)1);
  if (!Wire.available()) return false;
  uint8_t id = Wire.read();
  Serial.printf("IMU WHO_AM_I = 0x%02X\n", id);
  imuWrite(0x6B, 0x00);  // PWR_MGMT_1: スリープ解除
  delay(10);
  imuWrite(0x6B, 0x01);  // クロック自動選択
  delay(10);
  imuWrite(0x1C, 0x10);  // ACCEL_CONFIG: ±8g
  delay(10);
  return true;
}

void imuReadAccel(float &ax, float &ay, float &az) {
  Wire.beginTransmission(IMU_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(IMU_ADDR, (uint8_t)6);
  int16_t v[3];
  for (int i = 0; i < 3; i++) {
    uint8_t hi = Wire.read();
    uint8_t lo = Wire.read();
    v[i] = (int16_t)((hi << 8) | lo);
  }
  const float scale = 8.0f / 32768.0f;
  ax = v[0] * scale;
  ay = v[1] * scale;
  az = v[2] * scale;
}

// ---------- モード0: レインボー渦巻き ----------
void modeRainbow(uint32_t t) {
  for (int y = 0; y < 5; y++) {
    for (int x = 0; x < 5; x++) {
      float dx = x - 2.0f, dy = y - 2.0f;
      float ang = atan2f(dy, dx);
      float dist = sqrtf(dx * dx + dy * dy);
      uint16_t hue = (uint16_t)(ang / (2 * PI) * 65536.0f + dist * 6000 + t * 40);
      setXY(x, y, strip.gamma32(strip.ColorHSV(hue)));
    }
  }
}

// ---------- モード1: 傾きボール ----------
float ballX = 2, ballY = 2, velX = 0, velY = 0;

void modeTilt(uint32_t) {
  if (!imuOk) {  // IMU が無ければ赤い×
    for (int i = 0; i < 5; i++) {
      setXY(i, i, strip.Color(255, 0, 0));
      setXY(4 - i, i, strip.Color(255, 0, 0));
    }
    return;
  }
  float ax, ay, az;
  imuReadAccel(ax, ay, az);
  velX = velX * 0.85f - ax * 0.25f;
  velY = velY * 0.85f - ay * 0.25f;
  ballX += velX;
  ballY += velY;
  if (ballX < 0) { ballX = 0; velX = -velX * 0.5f; }
  if (ballX > 4) { ballX = 4; velX = -velX * 0.5f; }
  if (ballY < 0) { ballY = 0; velY = -velY * 0.5f; }
  if (ballY > 4) { ballY = 4; velY = -velY * 0.5f; }

  // 背景はうっすら青、ボールは周囲にぼかして描く
  for (int y = 0; y < 5; y++) {
    for (int x = 0; x < 5; x++) {
      float d = sqrtf((x - ballX) * (x - ballX) + (y - ballY) * (y - ballY));
      float k = max(0.0f, 1.0f - d / 1.2f);
      uint8_t r = (uint8_t)(255 * k);
      uint8_t g = (uint8_t)(160 * k);
      uint8_t b = (uint8_t)(10 + 30 * (1 - k));
      setXY(x, y, strip.Color(r, g, b));
    }
  }
}

// ---------- モード2: 文字スクロール ----------
// 各文字は列ごとのビット（bit0 = 一番上の行）
struct Glyph { char c; uint8_t w; uint8_t col[5]; };
const Glyph FONT[] = {
  {'H', 5, {0x1F, 0x04, 0x04, 0x04, 0x1F}},
  {'E', 4, {0x1F, 0x15, 0x15, 0x11}},
  {'L', 4, {0x1F, 0x10, 0x10, 0x10}},
  {'O', 5, {0x0E, 0x11, 0x11, 0x11, 0x0E}},
  {'A', 5, {0x1E, 0x05, 0x05, 0x05, 0x1E}},
  {'T', 5, {0x01, 0x01, 0x1F, 0x01, 0x01}},
  {'M', 5, {0x1F, 0x02, 0x04, 0x02, 0x1F}},
  {'5', 5, {0x17, 0x15, 0x15, 0x15, 0x09}},
  {'!', 1, {0x17}},
  {' ', 2, {0x00, 0x00}},
};
const char *MESSAGE = "HELLO ATOM M5! ";
uint8_t textCols[200];
int textLen = 0;

void buildText() {
  for (int i = 0; i < 5; i++) textCols[textLen++] = 0;  // 右から流れ込むよう空白を置く
  for (const char *p = MESSAGE; *p; p++) {
    for (const Glyph &g : FONT) {
      if (g.c != *p) continue;
      for (int i = 0; i < g.w; i++) textCols[textLen++] = g.col[i];
      textCols[textLen++] = 0;  // 文字間
      break;
    }
  }
}

void modeText(uint32_t t) {
  int offset = (t / 120) % textLen;
  for (int x = 0; x < 5; x++) {
    uint8_t bits = textCols[(offset + x) % textLen];
    uint16_t hue = (uint16_t)(t * 20 + x * 3000);
    uint32_t c = strip.gamma32(strip.ColorHSV(hue));
    for (int y = 0; y < 5; y++) {
      if (bits & (1 << y)) setXY(x, y, c);
    }
  }
}

// ---------- モード3: 花火 ----------
uint8_t heat[NUM_LEDS];
uint16_t hueOf[NUM_LEDS];

void modeSparkle(uint32_t) {
  for (int i = 0; i < NUM_LEDS; i++) heat[i] = heat[i] * 7 / 8;
  if (random(100) < 25) {
    int cx = random(5), cy = random(5);
    uint16_t h = random(65536);
    for (int dy = -1; dy <= 1; dy++)
      for (int dx = -1; dx <= 1; dx++) {
        int x = cx + dx, y = cy + dy;
        if (x < 0 || x > 4 || y < 0 || y > 4) continue;
        int i = y * 5 + x;
        heat[i] = (dx == 0 && dy == 0) ? 255 : 140;
        hueOf[i] = h;
      }
  }
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(hueOf[i], 200, heat[i])));
  }
}

// ---------- メイン ----------
const char *MODE_NAMES[NUM_MODES] = {"Rainbow", "Tilt ball", "Text", "Sparkle"};

void setup() {
  Serial.begin(115200);
  pinMode(BTN_PIN, INPUT);
  strip.begin();
  strip.setBrightness(BRIGHTNESS);
  strip.show();
  imuOk = imuInit();
  buildText();
  randomSeed(esp_random());
  Serial.printf("ATOM Matrix demo start. IMU: %s\n", imuOk ? "OK" : "not found");
  Serial.printf("Mode %d: %s\n", mode, MODE_NAMES[mode]);
}

void loop() {
  static bool lastBtn = HIGH;
  static uint32_t lastChange = 0;
  static uint32_t lastLog = 0;
  bool btn = digitalRead(BTN_PIN);
  uint32_t now = millis();
  if (btn == LOW && lastBtn == HIGH && now - lastChange > 200) {
    mode = (mode + 1) % NUM_MODES;
    lastChange = now;
    Serial.printf("Mode %d: %s\n", mode, MODE_NAMES[mode]);
  }
  lastBtn = btn;

  strip.clear();
  switch (mode) {
    case 0: modeRainbow(now); break;
    case 1: modeTilt(now); break;
    case 2: modeText(now); break;
    case 3: modeSparkle(now); break;
  }
  strip.show();

  if (mode == 1 && imuOk && now - lastLog > 1000) {
    float ax, ay, az;
    imuReadAccel(ax, ay, az);
    Serial.printf("accel x=%.2f y=%.2f z=%.2f\n", ax, ay, az);
    lastLog = now;
  }
  delay(30);
}
