/*
 * ╔══════════════════════════════════════════════════════════════════╗
 * ║   F L A P P Y   B I R D  —  Touch Edition                       ║
 * ║   Xiao ESP32-S3  |  SSD1306 OLED  |  TTP223  |  MAX98357A       ║
 * ╠══════════════════════════════════════════════════════════════════╣
 * ║  Wiring:                                                         ║
 * ║    OLED  SDA  → GPIO5  (D4)                                      ║
 * ║    OLED  SCL  → GPIO6  (D5)                                      ║
 * ║    TTP223 OUT → GPIO3  (D2)  (any free digital pin works)        ║
 * ║    SPK  BCLK  → GPIO7  (D8)                                      ║
 * ║    SPK  LRC   → GPIO4  (D3)                                      ║
 * ║    SPK  DIN   → GPIO2  (D1)                                      ║
 * ╠══════════════════════════════════════════════════════════════════╣
 * ║  Controls:                                                        ║
 * ║    TOUCH  → flap (hold = rapid flap)                             ║
 * ║    5 sec idle on score/title → auto-return to title              ║
 * ╚══════════════════════════════════════════════════════════════════╝
 *
 *  Libraries (Arduino Library Manager):
 *    Adafruit SSD1306
 *    Adafruit GFX Library
 */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <driver/i2s.h>
#include <math.h>

// ── DISPLAY ────────────────────────────────────────────────────────────
#define SCREEN_W     128
#define SCREEN_H      64
#define OLED_ADDR    0x3C
Adafruit_SSD1306 display(SCREEN_W, SCREEN_H, &Wire, -1);

// ── TOUCH PIN ──────────────────────────────────────────────────────────
#define TOUCH_PIN     3   // TTP223 OUT → GPIO3 (D2)
// TTP223 outputs HIGH when touched (default mode A)

// ── SPEAKER I2S (MAX98357A) ────────────────────────────────────────────
#define SPK_I2S_PORT  I2S_NUM_0
#define SPK_BCLK       7
#define SPK_LRC        4
#define SPK_DATA       2
#define SPK_RATE   16000

// ══════════════════════════════════════════════════════════════════════
//  SOUND ENGINE
// ══════════════════════════════════════════════════════════════════════
void setupSpeaker() {
  i2s_config_t c = {
    .mode                 = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate          = SPK_RATE,
    .bits_per_sample      = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format       = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags     = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count        = 4,
    .dma_buf_len          = 256,
    .use_apll             = false,
    .tx_desc_auto_clear   = true
  };
  i2s_pin_config_t p = {
    .bck_io_num   = SPK_BCLK,
    .ws_io_num    = SPK_LRC,
    .data_out_num = SPK_DATA,
    .data_in_num  = I2S_PIN_NO_CHANGE
  };
  i2s_driver_install(SPK_I2S_PORT, &c, 0, NULL);
  i2s_set_pin(SPK_I2S_PORT, &p);
}

void tone(float freq, int ms, float vol = 0.3f) {
  int total = SPK_RATE * ms / 1000;
  float ph = 0, inc = 2.0f * M_PI * freq / SPK_RATE;
  int16_t buf[128];
  size_t written;
  for (int i = 0; i < total; i += 64) {
    int n = min(64, total - i);
    for (int j = 0; j < n; j++) {
      float env = 1.0f;
      int pos = i + j;
      int att = SPK_RATE * 3 / 1000, rel = SPK_RATE * 8 / 1000;
      if (pos < att)            env = (float)pos / att;
      if (pos > total - rel)    env = (float)(total - pos) / rel;
      int16_t s = (int16_t)(sinf(ph) * 32767 * vol * env);
      buf[j * 2]     = s;
      buf[j * 2 + 1] = s;
      ph += inc;
    }
    i2s_write(SPK_I2S_PORT, buf, n * 4, &written, portMAX_DELAY);
  }
}

void sfxFlap()  { tone(480, 28, 0.18f); }
void sfxScore() { tone(880, 55, 0.28f); delay(10); tone(1320, 70, 0.28f); }
void sfxDead()  { for (float f = 550; f > 180; f -= 25) tone(f, 20, 0.35f); }
void sfxJingle(){ tone(523,80); tone(659,80); tone(784,80); tone(1047,140); }

// ══════════════════════════════════════════════════════════════════════
//  GAME STATE
// ══════════════════════════════════════════════════════════════════════
enum State { ST_TITLE, ST_PLAYING, ST_DEAD, ST_SCORE };
State gameState = ST_TITLE;

// Bird
float birdY, birdVel;
int   birdFrame;

// Pipes
#define MAX_PIPES 3
struct Pipe { int x, gapY; bool scored; } pipes[MAX_PIPES];

// Scores
int score = 0, hiScore = 0;

// Timing
unsigned long frameCount  = 0;
unsigned long lastActivity = 0;   // millis of last touch / state change
#define IDLE_TIMEOUT_MS  5000

// Touch edge detection
bool prevTouch = false;
bool touchEdge = false;   // true for one frame on press

// Death animation
struct Particle {
  float x, y, vx, vy;
  int   life;
} particles[12];
int   deadAnimFrame = 0;
bool  deadAnimDone  = false;

// Score screen animation tick
int scoreTick = 0;

// Stars for title / idle
struct Star { int x, y; int speed; } stars[20];

// ══════════════════════════════════════════════════════════════════════
//  GAME CONSTANTS (MUST BE BEFORE HELPERS)
// ══════════════════════════════════════════════════════════════════════
#define PIPE_GAP_HALF 11
#define PIPE_W        10
#define PIPE_SPD       2
#define BIRD_X        24
#define GRAVITY     0.36f
#define FLAP_V     -3.8f

// ══════════════════════════════════════════════════════════════════════
//  FORWARD DECLARATIONS
// ══════════════════════════════════════════════════════════════════════
void playReturnToTitleAnim();
void bootSplash();

// ══════════════════════════════════════════════════════════════════════
//  HELPERS
// ══════════════════════════════════════════════════════════════════════
void spawnPipe(int i, int sx) {
  pipes[i] = { sx, (int)random(PIPE_GAP_HALF + 6, SCREEN_H - PIPE_GAP_HALF - 6), false };
}

void initStars() {
  for (int i = 0; i < 20; i++)
    stars[i] = { (int)random(0, 128), (int)random(0, 64), (int)random(1, 4) };
}

void initParticles(float bx, float by) {
  for (int i = 0; i < 12; i++) {
    float ang = i * (2.0f * M_PI / 12);
    float spd = random(8, 20) * 0.12f;
    particles[i] = { bx, by, cosf(ang)*spd, sinf(ang)*spd, 18 };
  }
}

void startGame() {
  birdY    = SCREEN_H / 2.0f;
  birdVel  = 0; birdFrame = 0; score = 0;
  for (int i = 0; i < MAX_PIPES; i++)
    spawnPipe(i, SCREEN_W + i * (SCREEN_W / MAX_PIPES + 4));
  gameState    = ST_PLAYING;
  lastActivity = millis();
}

// ── 5×5 Bird sprites ────────────────────────────────────────────────
const uint8_t SPR_UP[5]   = {0x0E, 0x1F, 0x1E, 0x1F, 0x0C};
const uint8_t SPR_DOWN[5] = {0x0C, 0x1F, 0x1E, 0x1F, 0x0E};

void drawBird(int bx, int by, int fr) {
  const uint8_t* s = (fr % 2 == 0) ? SPR_UP : SPR_DOWN;
  for (int r = 0; r < 5; r++)
    for (int c = 0; c < 5; c++)
      if (s[r] & (0x10 >> c))
        display.drawPixel(bx + c, by + r, SSD1306_WHITE);
  display.drawPixel(bx + 3, by + 1, SSD1306_BLACK); // eye
}

void drawPipe(int px, int gapY) {
  int top = gapY - PIPE_GAP_HALF;
  int bot = gapY + PIPE_GAP_HALF;
  // top pipe body + cap
  display.fillRect(px, 0,       PIPE_W,     top,         SSD1306_WHITE);
  display.fillRect(px-1, top-4, PIPE_W+2,   4,           SSD1306_WHITE);
  // bottom pipe body + cap
  display.fillRect(px, bot,     PIPE_W,     SCREEN_H-bot, SSD1306_WHITE);
  display.fillRect(px-1, bot,   PIPE_W+2,   4,           SSD1306_WHITE);
}

bool hitsPipe(Pipe& p) {
  int bx1=BIRD_X, bx2=BIRD_X+4, by1=(int)birdY, by2=(int)birdY+4;
  int top=p.gapY-PIPE_GAP_HALF, bot=p.gapY+PIPE_GAP_HALF;
  if (bx2 < p.x || bx1 > p.x+PIPE_W) return false;
  return (by1 < top || by2 > bot);
}

// ══════════════════════════════════════════════════════════════════════
//  SCROLLING STAR BACKGROUND
// ══════════════════════════════════════════════════════════════════════
void updateStars() {
  for (int i = 0; i < 20; i++) {
    stars[i].x -= stars[i].speed;
    if (stars[i].x < 0) {
      stars[i].x = 127;
      stars[i].y = random(0, 64);
    }
    // twinkle: draw only when frame parity matches speed
    bool show = (stars[i].speed == 1) ? true : ((frameCount / 4) % 2 == 0);
    if (show) display.drawPixel(stars[i].x, stars[i].y, SSD1306_WHITE);
  }
}

// ══════════════════════════════════════════════════════════════════════
//  TITLE SCREEN  (animated: floating bird + starfield + wave text)
// ══════════════════════════════════════════════════════════════════════
void drawTitle() {
  display.clearDisplay();
  updateStars();

  // Floating bird
  int bfly = 20 + (int)(5 * sinf(frameCount * 0.09f));
  drawBird(10, bfly, frameCount / 6);

  // Big title with shadow effect
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(34, 4);
  display.print("FLAPPY");
  display.setTextSize(1);
  display.setCursor(38, 23);
  display.print("B I R D");

  // Animated dotted underline
  for (int x = 36; x < 92; x += 4) {
    int xv = x + (int)(2 * sinf(x * 0.3f + frameCount * 0.15f));
    display.drawPixel(xv, 32, SSD1306_WHITE);
  }

  // Pulsing "TOUCH" label
  if ((frameCount / 18) % 2 == 0) {
    display.setTextSize(1);
    display.setCursor(28, 42);
    display.print("[ TOUCH TO PLAY ]");
  }

  // Idle timer countdown bar (only shown after 2 s — warn the user)
  unsigned long idle = millis() - lastActivity;
  if (idle > 2000) {
    int barW = (int)map(idle, 2000, IDLE_TIMEOUT_MS, 64, 0);
    barW = constrain(barW, 0, 64);
    display.drawRect(32, 56, 64, 4, SSD1306_WHITE);
    display.fillRect(32, 56, barW, 4, SSD1306_WHITE);
    // "zzz" text when almost at timeout
    if (barW < 20) {
      display.setCursor(54, 55);
      display.setTextSize(1);
      display.print("zzz");
    }
  }

  display.display();
}

// ══════════════════════════════════════════════════════════════════════
//  PLAYING
// ══════════════════════════════════════════════════════════════════════
void updatePlaying() {
  // Flap on touch edge
  if (touchEdge) {
    birdVel = FLAP_V;
    birdFrame++;
    sfxFlap();
    lastActivity = millis();
  }

  birdVel += GRAVITY;
  birdVel  = constrain(birdVel, -6.5f, 7.0f);
  birdY   += birdVel;

  // Ceiling
  if (birdY < 0) { birdY = 0; birdVel = 0; }

  // Floor = death
  if (birdY > SCREEN_H - 5) {
    birdY = SCREEN_H - 5;
    initParticles(BIRD_X + 2, birdY + 2);
    deadAnimFrame = 0; deadAnimDone = false;
    sfxDead();
    if (score > hiScore) hiScore = score;
    gameState    = ST_DEAD;
    lastActivity = millis();
    return;
  }

  // Pipe logic
  for (int i = 0; i < MAX_PIPES; i++) {
    pipes[i].x -= PIPE_SPD;
    if (pipes[i].x < -PIPE_W - 2) spawnPipe(i, SCREEN_W + 4);
    if (!pipes[i].scored && pipes[i].x + PIPE_W < BIRD_X) {
      pipes[i].scored = true;
      score++;
      sfxScore();
    }
    if (hitsPipe(pipes[i])) {
      initParticles(BIRD_X + 2, birdY + 2);
      deadAnimFrame = 0; deadAnimDone = false;
      sfxDead();
      if (score > hiScore) hiScore = score;
      gameState    = ST_DEAD;
      lastActivity = millis();
      return;
    }
  }

  // Draw game
  display.clearDisplay();

  // Ground dashes
  for (int x = 0; x < SCREEN_W; x += 8) {
    int dx = ((x - (int)frameCount * PIPE_SPD) % 128 + 128) % 128;
    display.drawFastHLine(dx, SCREEN_H - 1, 4, SSD1306_WHITE);
  }

  for (int i = 0; i < MAX_PIPES; i++) drawPipe(pipes[i].x, pipes[i].gapY);
  drawBird(BIRD_X, (int)birdY, birdFrame);

  // Score HUD
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(2, 2);
  display.print(score);

  // Velocity indicator (right edge, vertical bar)
  int vy = (int)map((long)constrain(-birdVel, -7, 7), -7, 7, 0, 20);
  display.drawRect(123, 2, 4, 20, SSD1306_WHITE);
  display.fillRect(123, 2 + (20 - vy), 4, vy, SSD1306_WHITE);

  display.display();
}

// ══════════════════════════════════════════════════════════════════════
//  DEATH  (particle explosion → freeze → auto → score screen)
// ══════════════════════════════════════════════════════════════════════
void updateDead() {
  display.clearDisplay();

  // Update and draw particles
  bool anyAlive = false;
  for (int i = 0; i < 12; i++) {
    if (particles[i].life <= 0) continue;
    anyAlive = true;
    particles[i].x   += particles[i].vx;
    particles[i].y   += particles[i].vy;
    particles[i].vy  += 0.15f; // mini gravity
    particles[i].life--;
    int px = (int)particles[i].x;
    int py = (int)particles[i].y;
    if (px >= 0 && px < 128 && py >= 0 && py < 64)
      display.drawPixel(px, py, SSD1306_WHITE);
    // Trail pixel
    if (particles[i].life > 2) {
      int tx = px - (int)particles[i].vx;
      int ty = py - (int)particles[i].vy;
      if (tx >= 0 && tx < 128 && ty >= 0 && ty < 64)
        display.drawPixel(tx, ty, SSD1306_WHITE);
    }
  }

  // Background pipes still visible for context
  for (int i = 0; i < MAX_PIPES; i++) drawPipe(pipes[i].x, pipes[i].gapY);

  deadAnimFrame++;

  // Flash "DEAD" text after particles settle
  if (!anyAlive || deadAnimFrame > 30) {
    deadAnimDone = true;
    if ((deadAnimFrame / 8) % 2 == 0) {
      display.setTextSize(2);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(28, 22);
      display.print("DEAD!");
    }
    display.setTextSize(1);
    display.setCursor(22, 44);
    display.print("TOUCH for score");
  }

  display.display();

  // Touch advances to score screen
  if (touchEdge && deadAnimDone) {
    lastActivity = millis();
    scoreTick    = 0;
    gameState    = ST_SCORE;
  }

  // Auto-advance after 5 s
  if (millis() - lastActivity > IDLE_TIMEOUT_MS) {
    scoreTick = 0; gameState = ST_SCORE;
    lastActivity = millis();
  }
}

// ══════════════════════════════════════════════════════════════════════
//  SCORE SCREEN  (animated counter + starfield + prompt)
// ══════════════════════════════════════════════════════════════════════
void updateScore() {
  display.clearDisplay();
  scoreTick++;

  updateStars();

  // Animated score count-up
  int displayed = min(score, scoreTick / 3);

  // Trophy icon (simple)
  display.drawRect(56, 4, 16, 12, SSD1306_WHITE);
  display.fillRect(58, 6, 12, 8, SSD1306_WHITE);
  display.drawFastHLine(52, 16, 24, SSD1306_WHITE);
  display.drawFastVLine(64, 16, 4, SSD1306_WHITE);
  display.drawFastHLine(60, 20, 8, SSD1306_WHITE);

  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  // Center the score
  int sw = String(score).length() * 12;
  display.setCursor((SCREEN_W - sw) / 2, 26);
  display.print(displayed);

  display.setTextSize(1);
  display.setCursor(18, 44);
  display.print("BEST: ");
  display.print(hiScore);

  // New best banner
  if (score > 0 && score >= hiScore && (scoreTick / 10) % 2 == 0) {
    display.setCursor(36, 54);
    display.print("* NEW BEST *");
  }

  // Pulsing TOUCH prompt
  if ((scoreTick / 14) % 2 == 0) {
    display.setCursor(22, 56);
    display.print("[ TOUCH TO RETRY ]");
  }

  // Idle countdown bar
  unsigned long idle = millis() - lastActivity;
  if (idle > 2000) {
    int bw = (int)map(idle, 2000, IDLE_TIMEOUT_MS, 90, 0);
    bw = constrain(bw, 0, 90);
    display.drawRect(19, 35, 90, 3, SSD1306_WHITE);
    display.fillRect(19, 35, bw, 3, SSD1306_WHITE);
  }

  display.display();

  // Touch to retry
  if (touchEdge) {
    lastActivity = millis();
    startGame();
    return;
  }

  // Auto return to title after idle
  if (millis() - lastActivity > IDLE_TIMEOUT_MS) {
    playReturnToTitleAnim();
    gameState    = ST_TITLE;
    lastActivity = millis();
  }
}

// ══════════════════════════════════════════════════════════════════════
//  RETURN-TO-TITLE ANIMATION  (wipe + spiral stars)
// ══════════════════════════════════════════════════════════════════════
void playReturnToTitleAnim() {
  // Phase 1: horizontal scan-line wipe (white fills down)
  for (int y = 0; y <= SCREEN_H; y += 2) {
    display.fillRect(0, 0, SCREEN_W, y, SSD1306_WHITE);
    display.display();
    delay(12);
  }
  delay(80);

  // Phase 2: spiral star burst clearing the screen
  float cx = 64, cy = 32;
  for (int r = 0; r < 90; r++) {
    display.clearDisplay();
    float ang = r * 0.25f;
    float rad = r * 0.7f;
    for (int s = 0; s < 6; s++) {
      float a2 = ang + s * (M_PI / 3.0f);
      int sx = (int)(cx + cosf(a2) * rad);
      int sy = (int)(cy + sinf(a2) * rad);
      if (sx >= 0 && sx < 128 && sy >= 0 && sy < 64)
        display.drawPixel(sx, sy, SSD1306_WHITE);
      // inner dot
      int ix = (int)(cx + cosf(a2) * rad * 0.4f);
      int iy = (int)(cy + sinf(a2) * rad * 0.4f);
      if (ix >= 0 && ix < 128 && iy >= 0 && iy < 64)
        display.drawPixel(ix, iy, SSD1306_WHITE);
    }
    display.display();
    delay(14);
  }

  // Phase 3: fade-in "FLAPPY" sliding from left
  for (int ox = -80; ox <= 0; ox += 4) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(34 + ox, 20);
    display.print("FLAPPY");
    display.display();
    delay(20);
  }
  delay(300);

  initStars(); // reset stars for fresh title
}

// ══════════════════════════════════════════════════════════════════════
//  SETUP
// ══════════════════════════════════════════════════════════════════════
void setup() {
  Serial.begin(115200);

  // OLED
  Wire.begin(5, 6);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED fail"); for (;;);
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.display();

  // Touch pin
  pinMode(TOUCH_PIN, INPUT);

  // Speaker
  setupSpeaker();

  // Boot splash: pixel rain then title
  bootSplash();

  // Jingle
  sfxJingle();

  randomSeed(analogRead(0));
  initStars();
  gameState    = ST_TITLE;
  lastActivity = millis();
}

// ── Boot splash: random pixel fill ──────────────────────────────────
void bootSplash() {
  display.clearDisplay();
  // Pixel rain: random pixels fall into place
  for (int t = 0; t < 300; t++) {
    int rx = random(0, 128);
    int ry = random(0, 64);
    display.drawPixel(rx, ry, SSD1306_WHITE);
    if (t % 30 == 0) display.display();
    delayMicroseconds(800);
  }
  delay(200);
  display.clearDisplay();
  // Print title burst
  display.setTextSize(2);
  display.setCursor(10, 18);
  display.print("FLAPPY!");
  display.display();
  delay(600);
  display.clearDisplay();
  display.display();
}

// ══════════════════════════════════════════════════════════════════════
//  LOOP
// ══════════════════════════════════════════════════════════════════════
void loop() {
  frameCount++;

  // Read TTP223 (HIGH = touched)
  bool cur  = digitalRead(TOUCH_PIN) == HIGH;
  touchEdge = (cur && !prevTouch);
  prevTouch = cur;

  // Global idle timeout for title screen
  if (gameState == ST_TITLE) {
    if (touchEdge) {
      lastActivity = millis();
      startGame();
      return;
    }
    // "zzz" sleep animation if idle > 5 s (just re-init stars, keep drawing)
    if (millis() - lastActivity > IDLE_TIMEOUT_MS) {
      lastActivity = millis(); // reset so it doesn't spam
      // small zzz flash handled in drawTitle countdown bar
    }
    drawTitle();
    delay(28);
    return;
  }

  switch (gameState) {
    case ST_PLAYING: updatePlaying(); delay(20); break;
    case ST_DEAD:    updateDead();    delay(20); break;
    case ST_SCORE:   updateScore();   delay(25); break;
    default: break;
  }
}
