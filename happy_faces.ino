/*
 * ESP32 Happy Faces OLED Animation
 * Hardware: ESP32 + SSD1306 128x64 OLED (I2C)
 *
 * Wiring:
 *   OLED VCC  -> 3.3V
 *   OLED GND  -> GND
 *   OLED SDA  -> GPIO 21
 *   OLED SCL  -> GPIO 22
 *
 * Library: U8g2 by olikraus
 *   Install via Arduino IDE: Sketch > Include Library > Manage Libraries > search "U8g2"
 */

#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

// --- Display setup (I2C SSD1306, 128x64) ---
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// -------------------------------------------------------
// Face drawing helpers
// -------------------------------------------------------

// Draw a standard happy face centered at (cx, cy) with given radius
void drawHappyFace(int cx, int cy, int r) {
  // Outer circle
  u8g2.drawCircle(cx, cy, r, U8G2_DRAW_ALL);

  int eyeOffsetX = r / 3;
  int eyeOffsetY = r / 4;
  int eyeRadius  = max(1, r / 8);

  // Eyes (filled circles)
  u8g2.drawDisc(cx - eyeOffsetX, cy - eyeOffsetY, eyeRadius, U8G2_DRAW_ALL);
  u8g2.drawDisc(cx + eyeOffsetX, cy - eyeOffsetY, eyeRadius, U8G2_DRAW_ALL);

  // Smile (arc approximated with ellipse segment)
  // We draw the bottom half of a smaller circle as the smile
  int smileR = r / 2;
  u8g2.drawCircle(cx, cy + r / 10, smileR, U8G2_DRAW_LOWER_RIGHT);
  u8g2.drawCircle(cx, cy + r / 10, smileR, U8G2_DRAW_LOWER_LEFT);
}

// Draw a winking face (left eye closed = line, right eye = dot)
void drawWinkFace(int cx, int cy, int r) {
  u8g2.drawCircle(cx, cy, r, U8G2_DRAW_ALL);

  int eyeOffsetX = r / 3;
  int eyeOffsetY = r / 4;
  int eyeRadius  = max(1, r / 8);

  // Left eye: wink line
  u8g2.drawLine(cx - eyeOffsetX - eyeRadius, cy - eyeOffsetY,
                cx - eyeOffsetX + eyeRadius, cy - eyeOffsetY);

  // Right eye: filled dot
  u8g2.drawDisc(cx + eyeOffsetX, cy - eyeOffsetY, eyeRadius, U8G2_DRAW_ALL);

  // Smile
  int smileR = r / 2;
  u8g2.drawCircle(cx, cy + r / 10, smileR, U8G2_DRAW_LOWER_RIGHT);
  u8g2.drawCircle(cx, cy + r / 10, smileR, U8G2_DRAW_LOWER_LEFT);
}

// Draw a laughing face (eyes squeezed shut = arcs)
void drawLaughFace(int cx, int cy, int r) {
  u8g2.drawCircle(cx, cy, r, U8G2_DRAW_ALL);

  int eyeOffsetX = r / 3;
  int eyeOffsetY = r / 4;
  int eyeRadius  = max(2, r / 6);

  // Eyes squeezed (upper half arcs)
  u8g2.drawCircle(cx - eyeOffsetX, cy - eyeOffsetY, eyeRadius, U8G2_DRAW_UPPER_RIGHT);
  u8g2.drawCircle(cx - eyeOffsetX, cy - eyeOffsetY, eyeRadius, U8G2_DRAW_UPPER_LEFT);
  u8g2.drawCircle(cx + eyeOffsetX, cy - eyeOffsetY, eyeRadius, U8G2_DRAW_UPPER_RIGHT);
  u8g2.drawCircle(cx + eyeOffsetX, cy - eyeOffsetY, eyeRadius, U8G2_DRAW_UPPER_LEFT);

  // Big open mouth (wider smile)
  int smileR = r * 2 / 3;
  u8g2.drawCircle(cx, cy + r / 8, smileR, U8G2_DRAW_LOWER_RIGHT);
  u8g2.drawCircle(cx, cy + r / 8, smileR, U8G2_DRAW_LOWER_LEFT);
}

// -------------------------------------------------------
// Animation scenes
// -------------------------------------------------------

// Scene: One big happy face bouncing across the screen
void sceneBounce() {
  static int x = 20, y = 32;
  static int dx = 2, dy = 1;
  const int r = 18;

  for (int frame = 0; frame < 80; frame++) {
    x += dx;
    y += dy;

    if (x - r <= 0 || x + r >= 127) dx = -dx;
    if (y - r <= 0 || y + r >= 63)  dy = -dy;

    u8g2.clearBuffer();
    drawHappyFace(x, y, r);
    u8g2.sendBuffer();
    delay(30);
  }
}

// Scene: Three faces of different sizes arranged nicely
void sceneThreeFaces() {
  u8g2.clearBuffer();

  // Big face on left
  drawHappyFace(22, 32, 20);

  // Medium face in center
  drawWinkFace(64, 32, 16);

  // Small face on right
  drawLaughFace(106, 32, 14);

  u8g2.sendBuffer();
  delay(2000);
}

// Scene: Face grows from tiny to large (zoom in)
void sceneZoomIn() {
  for (int r = 2; r <= 28; r++) {
    u8g2.clearBuffer();
    drawHappyFace(64, 32, r);
    u8g2.sendBuffer();
    delay(40);
  }
  delay(600);

  // Then shrink back
  for (int r = 28; r >= 2; r--) {
    u8g2.clearBuffer();
    drawHappyFace(64, 32, r);
    u8g2.sendBuffer();
    delay(30);
  }
}

// Scene: Face cycles through happy -> wink -> laugh expressions
void sceneExpressions() {
  const int cx = 64, cy = 32, r = 26;

  u8g2.clearBuffer();
  drawHappyFace(cx, cy, r);
  u8g2.sendBuffer();
  delay(1200);

  u8g2.clearBuffer();
  drawWinkFace(cx, cy, r);
  u8g2.sendBuffer();
  delay(1200);

  u8g2.clearBuffer();
  drawLaughFace(cx, cy, r);
  u8g2.sendBuffer();
  delay(1200);
}

// Scene: Many tiny faces filling the screen
void sceneManyFaces() {
  u8g2.clearBuffer();

  // 3 columns x 2 rows of small faces
  int positions[][2] = {
    {21, 18}, {64, 18}, {107, 18},
    {21, 48}, {64, 48}, {107, 48}
  };

  for (int i = 0; i < 6; i++) {
    int type = i % 3;
    if (type == 0) drawHappyFace(positions[i][0], positions[i][1], 14);
    if (type == 1) drawWinkFace (positions[i][0], positions[i][1], 14);
    if (type == 2) drawLaughFace(positions[i][0], positions[i][1], 14);
  }

  u8g2.sendBuffer();
  delay(2500);
}

// -------------------------------------------------------
// Arduino setup & loop
// -------------------------------------------------------

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22); // SDA=21, SCL=22 (default ESP32 I2C pins)

  if (!u8g2.begin()) {
    Serial.println("OLED init failed! Check wiring.");
    while (true) delay(1000);
  }

  u8g2.setFont(u8g2_font_ncenB08_tr); // small font (unused in drawing but good to have)
  u8g2.setDrawColor(1);

  Serial.println("Happy faces starting!");
}

void loop() {
  sceneThreeFaces();    // 3 faces side by side
  sceneZoomIn();        // big face zooms in and out
  sceneExpressions();   // single face cycles expressions
  sceneBounce();        // face bounces around screen
  sceneManyFaces();     // grid of 6 tiny faces
}
