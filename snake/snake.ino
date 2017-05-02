#include <TrueRandom.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

#define TFT_CS  10  // Chip select line for TFT display
#define TFT_DC   9  // Data/command line for TFT
#define TFT_RST  8  // Reset line for TFT (or connect to +5V)

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

#define BUTTON_NONE 0
#define BUTTON_DOWN 1
#define BUTTON_RIGHT 2
#define BUTTON_SELECT 3
#define BUTTON_UP 4
#define BUTTON_LEFT 5
uint16_t pixelColor;
uint8_t buttonPressed, currentDirection;
boolean collision, appleOn, displayEnd, specialOn, allowSpecial, showTitle = true;
int head, timer, snakeSize, score, pixelLocationXAxis, pixelLocationYAxis, x[300], y[300],
    appleX, appleY, yMultiplier, selection = 100, difficulty, specialX, specialY, specialTime;

void setup(void) {
  tft.initR(INITR_BLACKTAB);
  tft.fillScreen(0x0000);
  if (showTitle)
    showTitlScreen();
  tft.fillScreen(0x0000);
  collision = false;
  appleOn = false;
  specialOn = false;
  allowSpecial = false;
  displayEnd = false;
  pixelLocationXAxis = tft.width() / 2 - 1;
  pixelLocationYAxis = tft.height() / 2 - 1;
  pixelColor = ST7735_WHITE;
  buttonPressed = BUTTON_RIGHT;
  currentDirection = BUTTON_RIGHT;
  specialTime = 0;
  score = 0;
  head = 0;
  snakeSize = 5;
  timer = 0;
  updateScore(score);
  drawBoard();
}

uint8_t readButton(void) {
  int a = analogRead(3);

   if (a < 70) return BUTTON_LEFT;
     if (a < 220) return BUTTON_UP;
       if (a < 420) return BUTTON_DOWN;
         if (a < 600) return BUTTON_RIGHT;
           if (a < 850) return BUTTON_SELECT;


  else return BUTTON_NONE;
}

void loop() {
  uint8_t b = readButton();
  if (b != BUTTON_NONE && b != BUTTON_SELECT)
    buttonPressed = b;
  if (!collision){
    appleLogic();
    checkIfAppleGot();
    specialApple();
    checkIfSpecialGot();
    specialTimer();
    updateSnakePosition(buttonPressed);
    if (timer >= snakeSize - 1)
      removeLastFromTail();
    else
      timer++;
  } else {
    if (displayEnd == true)
      displayEndingScreen();
      showTitle = true;
      uint8_t buttonPressed = readButton();
      if (buttonPressed == BUTTON_SELECT)
        setup();
  }
  delay(difficulty);
  
}

void showTitlScreen(){
  tft.fillScreen(ST7735_BLACK);
  tft.setTextSize(4);
  tft.setTextColor(ST7735_GREEN);
  tft.setCursor(5, 30);
  tft.print("SNAKE");
  
  tft.setTextSize(1);
  tft.setTextColor(ST7735_GREEN);
  tft.setCursor(25, 150);
  tft.print("Arduino Snake");
  titleSelection();
}
  
void titleSelection(){
  uint8_t titleButton = readButton();

  if (titleButton == BUTTON_UP)
    selection--;
  if (titleButton == BUTTON_DOWN)
    selection++;
    
  if (selection % 3 == 0){ // easy mode
    tft.setCursor(40, 80);
    tft.fillRect(25, 80, 10, 8, ST7735_GREEN);
    tft.print("Easy");
    tft.setCursor(40, 100);
    tft.fillRect(25, 100, 10, 8, ST7735_BLACK);
    tft.print("Medium");
    tft.setCursor(40, 120);
    tft.fillRect(25, 120, 10, 8, ST7735_BLACK);
    tft.print("Hard");
    difficulty = 90;
  } else if (selection % 3 == 1){ // normal mode
    tft.setCursor(40, 80);
    tft.fillRect(25, 80, 10, 8, ST7735_BLACK);
    tft.print("Easy");  
    tft.setCursor(40, 100);
    tft.fillRect(25, 100, 10, 8, ST7735_GREEN);
    tft.print("Medium");
    tft.setCursor(40, 120);
    tft.fillRect(25, 120, 10, 8, ST7735_BLACK);
    tft.print("Hard");
    difficulty = 65;
  } else { // hard mode
    tft.setCursor(40, 80);
    tft.fillRect(25, 80, 10, 8, ST7735_BLACK);
    tft.print("Easy");  
    tft.setCursor(40, 100);
    tft.fillRect(25, 100, 10, 8, ST7735_BLACK);
    tft.print("Medium");
    tft.setCursor(40, 120);
    tft.fillRect(25, 120, 10, 8, ST7735_GREEN);
    tft.print("Hard");
    difficulty = 40;
  }
  
  if(titleButton == BUTTON_SELECT){
    showTitle = false;
    return;
  }
  
  delay(100);
  titleSelection();
}

void drawBoard(){
  tft.drawRect(0, 9, tft.width(), tft.height() - 9, ST7735_WHITE);
  tft.setTextSize(1);
  tft.setTextColor(ST7735_WHITE);
  tft.setCursor(3, 0);
  tft.print("Score:");
}

void updateScore(int score){
  tft.fillRect(38, 0, tft.width() - 75, 9, ST7735_BLACK);
  tft.setTextSize(1);
  tft.setCursor(38, 0);
  tft.setTextColor(ST7735_WHITE);
  tft.print(score);
}

void updateSnakePosition(uint8_t buttonPressed){

  if (buttonPressed == BUTTON_UP){
    if(currentDirection != BUTTON_DOWN){
      pixelLocationYAxis -= 3;
      currentDirection = BUTTON_UP;
    } else
      pixelLocationYAxis += 3;
  }
  if (buttonPressed == BUTTON_DOWN){
    if(currentDirection != BUTTON_UP){
      pixelLocationYAxis += 3;
      currentDirection = BUTTON_DOWN;
    } else
      pixelLocationYAxis -= 3;
  }  
  if (buttonPressed == BUTTON_LEFT){
    if(currentDirection != BUTTON_RIGHT){
      pixelLocationXAxis -= 3;
      currentDirection = BUTTON_LEFT;
    } else
      pixelLocationXAxis += 3;
  }  
  if (buttonPressed == BUTTON_RIGHT){
    if(currentDirection != BUTTON_LEFT){
      pixelLocationXAxis += 3;
      currentDirection = BUTTON_RIGHT;
    } else
      pixelLocationXAxis -= 3;
  }
    
  if (pixelLocationYAxis < 10)
    pixelLocationYAxis = 10;
  if (pixelLocationYAxis > tft.height() - 3)
    pixelLocationYAxis = tft.height() - 3;
  if (pixelLocationXAxis < 1)
    pixelLocationXAxis = 1;
  if (pixelLocationXAxis > tft.width() - 3)
    pixelLocationXAxis = tft.width() - 3;
    
  drawSnake(pixelLocationXAxis, pixelLocationYAxis, pixelColor);
  x[head] = pixelLocationXAxis;
  y[head] = pixelLocationYAxis;
  collisionCheck();
  head++;
  if (head >= 300){
    removeLastFromTail();
    resetArray();
    head = snakeSize + 1;
  }
}

void resetArray(){
  for(int j = 1; j < snakeSize; j++){
    x[snakeSize - j] = x[head - j];
    x[head - j] = 0;
    y[snakeSize - j] = y[head - j];
    y[head - j] = 0;
  }
  x[0] = 0;
  y[0] = 0;
}

void drawSnake(int pixelLocationXAxis, int pixelLocationYAxis, uint16_t pixelColor){

  tft.drawPixel(pixelLocationXAxis - 1, pixelLocationYAxis - 1, pixelColor);  //top left
  tft.drawPixel(pixelLocationXAxis, pixelLocationYAxis - 1, pixelColor);      //top
  tft.drawPixel(pixelLocationXAxis + 1, pixelLocationYAxis - 1, pixelColor);  //top right
  tft.drawPixel(pixelLocationXAxis - 1, pixelLocationYAxis, pixelColor);      //left
  tft.drawPixel(pixelLocationXAxis, pixelLocationYAxis, pixelColor);          //middle
  tft.drawPixel(pixelLocationXAxis + 1, pixelLocationYAxis, pixelColor);      //right
  tft.drawPixel(pixelLocationXAxis - 1, pixelLocationYAxis + 1, pixelColor);  //bottom left
  tft.drawPixel(pixelLocationXAxis, pixelLocationYAxis + 1, pixelColor);      //bottom
  tft.drawPixel(pixelLocationXAxis + 1, pixelLocationYAxis + 1, pixelColor);  //bottom right
}

void removeLastFromTail(){
  drawSnake(x[head - snakeSize], y[head - snakeSize], ST7735_BLACK);
  x[head - snakeSize] = 0;
  y[head - snakeSize] = 0;
}

void collisionCheck(){
  if (x[head] < 2 || x[head] > tft.width() - 2 || y[head] <= 11 || y[head] >= tft.height()){
    pixelColor = ST7735_RED;
    collision = true;
    drawSnake(pixelLocationXAxis, pixelLocationYAxis, ST7735_RED);
    displayEnd = true;
  }
  
  for (int z = 0; z < head; z++) {
    if ((x[head] == x[z]) && (y[head] == y[z])) {
      pixelColor = ST7735_RED;
      collision = true;
      drawSnake(pixelLocationXAxis, pixelLocationYAxis, ST7735_RED);
      displayEnd = true;
    }
  }
}

void appleLogic(){
  if (!appleOn){
    appleX = TrueRandom.random(3, tft.width() - 6);
    appleY = 10;
    yMultiplier = TrueRandom.random(1, 49);
    if (appleX % 3 == 1)
      appleX--;
    else if (appleX % 3 == 2)
      appleX++;
    appleY += (3 * yMultiplier);
    drawApple(appleX, appleY, ST7735_GREEN);
    appleOn = true;
  }
}

void checkIfAppleGot(){
  if ((x[head - 1] == appleX)){
    if ((y[head - 1] == appleY)){
      appleOn = false;
      snakeSize += 2;
      score += 10;
      updateScore(score);
      allowSpecial = true;
    }
  } else
    drawApple(appleX, appleY, ST7735_GREEN);
}

void specialApple(){
  if (!specialOn && (score % 100 == 0) && allowSpecial){
    specialX = TrueRandom.random(2, tft.width() - 5);
    specialY = 10;
    yMultiplier = TrueRandom.random(1, 49);
    if (specialX % 3 == 1)
      specialX--;
    else if (specialX % 3 == 2)
      specialX++;
    specialY += (3 * yMultiplier);
    drawApple(specialX, specialY, ST7735_RED);
    specialTime = 0;
    specialOn = true;
    allowSpecial = false;
  }
}

void checkIfSpecialGot(){
  if (specialOn){
    if ((x[head - 1] == specialX)){
      if ((y[head - 1] == specialY)){
        specialSelection();
      }
    } else
      drawApple(specialX, specialY, ST7735_RED);
  }
}

void specialSelection(){
  int removeSize = TrueRandom.random(1, 5);
  int addedScore = TrueRandom.random(1, 11) * 100;
  snakeSize -= removeSize;
  for (int i = 0; i < removeSize; i++){
    drawSnake(x[head - snakeSize - i], y[head - snakeSize - i], ST7735_BLACK);
    x[head - snakeSize - i] = 0;
    y[head - snakeSize - i] = 0;
  }
  score += addedScore;
  updateScore(score);
  specialOn = false;
}

void specialTimer(){
  specialTime++;
  if (specialTime > 150){
    drawApple(specialX, specialY, ST7735_BLACK);
    specialOn = false;
    allowSpecial == false;
    specialTime = 0;
  }
}

void drawApple(int pixelLocationXAxis, int pixelLocationYAxis, uint16_t pixelColor){
  tft.drawPixel(pixelLocationXAxis, pixelLocationYAxis - 1, pixelColor);      //top
  tft.drawPixel(pixelLocationXAxis - 1, pixelLocationYAxis, pixelColor);      //left
  tft.drawPixel(pixelLocationXAxis, pixelLocationYAxis, pixelColor);          //middle
  tft.drawPixel(pixelLocationXAxis + 1, pixelLocationYAxis, pixelColor);      //right
  tft.drawPixel(pixelLocationXAxis, pixelLocationYAxis + 1, pixelColor);      //bottom
}

void displayEndingScreen(){
  tft.fillRect(25, 65, tft.width() - 50, 30, ST7735_BLUE);
  tft.setTextSize(3);
  tft.setCursor(35, 70);
  tft.setTextColor(ST7735_WHITE);
  tft.print(score);
  displayEnd = false;
}
