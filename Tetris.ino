//========================================================================
// TETRIS with M5STACK : 2018.01.20 Transplant by macsbug
//                       2018.05.03 Modified by @shiakrunochi
//                       2018.05.20 Modified by @rimksky
// Controller : Buttons A = LEFT, B = RIGHT, C = START, ROTATE
// Display    : Left = 100x240, Center = 120x240, Right = 100x240
// Block      : 8ea, 12x12 pixel
// SD         : tetris.jpg : BackGround Image : R.G.B 320x240 pixel
// Github     : (Original) https://macsbug.wordpress.com/2018/01/20/tetris-with-m5stack/
//========================================================================
#include <M5Stack.h>                                       // M5STACK
#ifdef USE_SD_UPDATER
    #include "M5StackUpdater.h"
    SDUpdater sdUpdater;
#endif

uint16_t BlockImage[8][12][12];                            // Block
uint16_t backBuffer[240][120];                             // GAME AREA
uint16_t nextBlockBuffer[60][48];                             // NEXT BLOCK AREA
const int Length = 12;     // the number of pixels for a side of a block
const int Width  = 10;     // the number of horizontal blocks
const int Height = 20;     // the number of vertical blocks
int screen[Width][Height] = {0}; //it shows color-numbers of all positions
struct Point {int X, Y;};
struct Block {Point square[4][4]; int numRotate, color;};
Point pos; Block block;
int nextBlockType = -1;
long score = 0;
Block nextBlock;
int rot, fall_cnt = 0;
bool started = false, gameover = false;
boolean but_A_pre = false, but_A = false, but_LEFT = false, but_RIGHT = false, but_DOWN = false;
int game_speed = 25; // 25msec
Block blocks[7] = {
  {{{{-1,0},{0,0},{1,0},{2,0}},{{0,-1},{0,0},{0,1},{0,2}},
  {{0,0},{0,0},{0,0},{0,0}},{{0,0},{0,0},{0,0},{0,0}}},2,1},
  {{{{0,-1},{1,-1},{0,0},{1,0}},{{0,0},{0,0},{0,0},{0,0}},
  {{0,0},{0,0},{0,0},{0,0}},{{0,0},{0,0},{0,0},{0,0}}},1,2},
  {{{{-1,-1},{-1,0},{0,0},{1,0}},{{-1,1},{0,1},{0,0},{0,-1}},
  {{-1,0},{0,0},{1,0},{1,1}},{{1,-1},{0,-1},{0,0},{0,1}}},4,3},
  {{{{-1,0},{0,0},{0,1},{1,1}},{{0,-1},{0,0},{-1,0},{-1,1}},
  {{0,0},{0,0},{0,0},{0,0}},{{0,0},{0,0},{0,0},{0,0}}},2,4},
  {{{{-1,0},{0,0},{1,0},{1,-1}},{{-1,-1},{0,-1},{0,0},{0,1}},
  {{-1,1},{-1,0},{0,0},{1,0}},{{0,-1},{0,0},{0,1},{1,1}}},4,5},
  {{{{-1,1},{0,1},{0,0},{1,0}},{{0,-1},{0,0},{1,0},{1,1}},
  {{0,0},{0,0},{0,0},{0,0}},{{0,0},{0,0},{0,0},{0,0}}},2,6},
  {{{{-1,0},{0,0},{1,0},{0,-1}},{{0,-1},{0,0},{0,1},{-1,0}},
  {{-1,0},{0,0},{1,0},{0,1}},{{0,-1},{0,0},{0,1},{1,0}}},4,7}
};
extern uint8_t tetris_img[];

//========================================================================
void setup(void) {
  Serial.begin(115200);         // SERIAL
  M5.begin();                   // M5STACK INITIALIZE
  
#ifdef USE_SD_UPDATER
  if(digitalRead(BUTTON_A_PIN) == 0) {
    Serial.println("Will Load menu binary");
    sdUpdater.updateFromFS(SD);
    ESP.restart();
  }
#endif
 
  M5.Lcd.setBrightness(200);    // BRIGHTNESS = MAX 255
  M5.Lcd.fillScreen(BLACK);     // CLEAR SCREEN
//  M5.Lcd.setRotation(1);        // SCREEN ROTATION = 0
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(2);
  //----------------------------// Make Block ----------------------------
  make_block( 0, BLACK);        // Type No, Color
  make_block( 1, 0x00F0);       // DDDD     RED
  make_block( 2, 0xFBE4);       // DD,DD    PUPLE 
  make_block( 3, 0xFF00);       // D__,DDD  BLUE
  make_block( 4, 0xFF87);       // DD_,_DD  GREEN 
  make_block( 5, 0x87FF);       // __D,DDD  YELLO
  make_block( 6, 0xF00F);       // _DD,DD_  LIGHT GREEN
  make_block( 7, 0xF8FC);       // _D_,DDD  PINK
  //----------------------------------------------------------------------
  // M5.Lcd.drawJpgFile(SD, "/tetris.jpg");     // Load background from SD
  M5.Lcd.drawJpg(tetris_img, 34215); // Load background from file data
  PutStartPos();                             // Start Position
  for (int i = 0; i < 4; ++i) screen[pos.X + 
   block.square[rot][i].X][pos.Y + block.square[rot][i].Y] = block.color;
  Draw();                                    // Draw block
  DrawNextBlock();
}
//========================================================================
void loop() {
  if (gameover) return;
  Point next_pos;
  int next_rot = rot;
  GetNextPosRot(&next_pos, &next_rot);
  ReviseScreen(next_pos, next_rot);
  M5.update();
  delay(game_speed);                                      // SPEED ADJUST
}
//========================================================================
void Draw() {                               // Draw 120x240 in the center
  for (int i = 0; i < Width; ++i) for (int j = 0; j < Height; ++j)
   for (int k = 0; k < Length; ++k) for (int l = 0; l < Length; ++l)
    backBuffer[j * Length + l][i * Length + k] = BlockImage[screen[i][j]][k][l];
    M5.Lcd.drawBitmap(100, 0, 120, 240, (uint16_t *)backBuffer);
}
void DrawNextBlock() {  
  for(int x = 0; x < 48;x++) {
    for(int y = 0; y < 60;y++){
      nextBlockBuffer[y][x]=0;
    }
  }
  nextBlock = blocks[nextBlockType];
  int offset = 6 + 12;
  
  for (int i = 0; i < 4; ++i) {
      for (int k = 0; k < Length; ++k) for (int l = 0; l < Length; ++l){
        nextBlockBuffer[60 - (nextBlock.square[0][i].X * Length + l + offset)][nextBlock.square[0][i].Y * Length + k + offset] = BlockImage[nextBlockType + 1][k][l];
      }
  }
  M5.Lcd.drawBitmap(26, 100, 48, 60, (uint16_t *)nextBlockBuffer);
  M5.Lcd.fillRect(2, 76, 96, 19, BLACK);
  M5.Lcd.setCursor(10, 78);
  M5.Lcd.printf("%7d",score);
}

//========================================================================
void PutStartPos() {
  pos.X = 4; pos.Y = 1;
  if (nextBlockType == -1){
    block = blocks[random(7)];
  }else{
    block = blocks[nextBlockType];
  }
  nextBlockType = random(7);
  rot = random(block.numRotate);
}
//========================================================================
bool GetSquares(Block block, Point pos, int rot, Point* squares) {
  bool overlap = false;
  for (int i = 0; i < 4; ++i) {
    Point p;
    p.X = pos.X + block.square[rot][i].X;
    p.Y = pos.Y + block.square[rot][i].Y;
    overlap |= p.X < 0 || p.X >= Width || p.Y < 0 || p.Y >= 
      Height || screen[p.X][p.Y] != 0;
    squares[i] = p;
  }
  return !overlap;
}
//========================================================================
void GameOver() {
  for (int i = 0; i < Width; ++i) for (int j = 0; j < Height; ++j)
  if (screen[i][j] != 0) screen[i][j] = 4;
  gameover = true;
}
//========================================================================
void ClearKeys() { but_A_pre = false; but_A=false; but_LEFT=false; but_RIGHT=false; but_DOWN=false;}
//========================================================================
bool KeyPadLoop(){
  /*
  if( M5.BtnC.pressedFor(50) ){
    Serial.println("pressedFor(50):true");
  }
  else{
    Serial.println("pressedFor(50):false");    
  }
  if( M5.BtnC.pressedFor(200) ){
    Serial.println("pressedFor(200):true");
  }
  else{
    Serial.println("pressedFor(200):false");    
  }
  if( M5.BtnC.wasReleased() ){
    Serial.println("wasReleased():true");
  }
  else{
    Serial.println("wasReleased():false");    
  }  
  Serial.println("----------------------------");
  */
        
  if(M5.BtnA.pressedFor(100) || M5.BtnA.wasPressed()){ClearKeys();but_LEFT =true;return true;}
  if(M5.BtnB.pressedFor(100) || M5.BtnB.wasPressed()){ClearKeys();but_RIGHT=true;return true;}
  if(M5.BtnC.pressedFor(30) && !M5.BtnC.pressedFor(200) ){ClearKeys();but_A_pre   =true;return false;}
  if(but_A_pre && M5.BtnC.wasReleased() ){ClearKeys();but_A    =true;return true;}
  if(M5.BtnC.pressedFor(200)){ClearKeys();but_DOWN    =true;return true;}
  
  if (Serial.available()) {
    char r = Serial.read();
    while(Serial.read() != -1);
    if (r == 'z') { ClearKeys();  but_A=true; } //else but_A=false;
    if (r == '2') { ClearKeys();  but_DOWN=true; }  //else but_DOWN=false;
    if (r == '4') { ClearKeys();  but_LEFT=true; }  // else but_LEFT=false;
    if (r == '6') { ClearKeys();  but_RIGHT=true; }  //else but_RIGHT=false;
    return true;
  }
  
  return false;
}
//========================================================================
void GetNextPosRot(Point* pnext_pos, int* pnext_rot) {
  bool received = KeyPadLoop();
  if (but_A) started = true;
  if (!started) return;
  pnext_pos->X = pos.X;
  pnext_pos->Y = pos.Y;
  if ((fall_cnt = (fall_cnt + 1) % 10) == 0) pnext_pos->Y += 1;
  else if (received) {
    if (but_LEFT) { but_LEFT = false; pnext_pos->X -= 1;}
    else if (but_RIGHT) { but_RIGHT = false; pnext_pos->X += 1;}
    else if (but_DOWN) { but_DOWN = false; pnext_pos->Y += 1;}
    else if (but_A) { but_A = false;
      *pnext_rot = (*pnext_rot + block.numRotate - 1)%block.numRotate; 
    }
  }
}
//========================================================================
void DeleteLine() {
  int deleteCount = 0;
  for (int j = 0; j < Height; ++j) {
    bool Delete = true;
    for (int i = 0; i < Width; ++i) if (screen[i][j] == 0) Delete = false;
    if (Delete) {
      for (int k = j; k >= 1; --k) {
        for (int i = 0; i < Width; ++i) {
          screen[i][k] = screen[i][k - 1];
        }
      }
      deleteCount++;
    }
  }
  switch (deleteCount){
    case 1:score = score + 40;break;
    case 2:score = score + 100;break;
    case 3:score = score + 300;break;
    case 4:score = score + 1200;break;
  }
  if(score > 9999999){
    score = 9999999;
  }
}
//========================================================================
void ReviseScreen(Point next_pos, int next_rot) {
  if (!started) return;
  Point next_squares[4];
  for (int i = 0; i < 4; ++i) screen[pos.X + 
    block.square[rot][i].X][pos.Y + block.square[rot][i].Y] = 0;
  if (GetSquares(block, next_pos, next_rot, next_squares)) {
   for (int i = 0; i < 4; ++i){
     screen[next_squares[i].X][next_squares[i].Y] = block.color;
   }
   pos = next_pos; rot = next_rot;
  }
  else {
   for (int i = 0; i < 4; ++i) screen[pos.X + 
    block.square[rot][i].X][pos.Y + block.square[rot][i].Y] = block.color;
   if (next_pos.Y == pos.Y + 1) {
    DeleteLine(); PutStartPos();DrawNextBlock();
    if (!GetSquares(block, pos, rot, next_squares)) {
     for (int i = 0; i < 4; ++i) screen[pos.X + 
      block.square[rot][i].X][pos.Y + block.square[rot][i].Y] = block.color;
      GameOver();
    }
   }
  }
  Draw();
}
//========================================================================
void make_block( int n , uint16_t color ){            // Make Block color       
  for ( int i =0 ; i < 12; i++ ) for ( int j =0 ; j < 12; j++ ){
    BlockImage[n][i][j] = color;                           // Block color
    if ( i == 0 || j == 0 ) BlockImage[n][i][j] = 0;       // BLACK Line
  } 
}
//========================================================================
