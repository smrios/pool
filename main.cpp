#include "base.h"
#include "drawing.h"
#include "phys.h"
#include <string>
#include <vector>
#include <algorithm>
#include <raylib.h>
#include <raymath.h>

using namespace std;

void initScreen(){
  SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_HIGHDPI);
  InitWindow(1920, 1080, "poollike");
  ToggleFullscreen();
  SetTargetFPS(120);
}

int main(){
  initScreen();
  playingSurface = {
  ((float)GetScreenWidth()/2)-(TABLE_SIZE.x/2), 
  ((float)GetScreenHeight()/2)-(TABLE_SIZE.y/2),
  TABLE_SIZE.x, TABLE_SIZE.y};
  loadResources();

  Texture ballOverlay = LoadTexture("ballover.png");
  
  vector<ball*> balls;  
  ball cue = {ballRadius, {playingSurface.width*0.2f,playingSurface.height/2},{0,0}, false, true};
  vector<ball> rack;
  rack.reserve(15);

  for(int i = 0; i < 15; i++){
    rack.push_back({
      .radius = ballRadius,
      .pos = {0,0},
      .vel = {0,0},
      .eightball = i == 4
    });
    rack[i].sunk = false;
    balls.push_back(&rack[i]);
  }

  RackBallsTriangle(rack, 15, {playingSurface.width * 0.6f, playingSurface.height/2});
  balls.push_back(&cue);

  float time=0;

  int sinks = 0;
  int shots = 0;

  RenderTexture2D ballMask = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
  Vector2 cursorPos = {0,0};
  DisableCursor();
  gameState gs = {
    .shots = 0,
    .sunk = 0,
    .score = 0
  };

  RenderTexture2D flash = LoadRenderTexture(1, 1);
  float flashProg;
  const float flashSpeed = 0.5f;

  while(!WindowShouldClose()){
    time += GetFrameTime();
    switch(gs.gp){
      case shooting:{
        cursorPos+= GetMouseDelta() * ((IsKeyDown(KEY_LEFT_SHIFT)) ? 0.1f : 0.5f);
        //cue shot angle calcs
          Vector2 mp = cursorPos;
          Vector2 cp = (Vector2){playingSurface.x, playingSurface.y} + cue.pos; 
          Vector2 dir = cp - mp;

          float magnitude = clamp(Vector2Length(dir), 0.0f, maxDist);
          Vector2 ghostPos = cp - (Vector2Normalize(dir) * magnitude);
          if(IsMouseButtonPressed(0)){
            //if it is the second shot, and nothing has been sunk, invalid break
            if(gs.sunk == 0 && gs.shots == 1){
              drawTextFlash(flash, res.f_ming32, "INVALID BREAK", 64, BLACK, WHITE);
              flashProg = 0;
              resetGame(gs, rack, cue);
            }
            else{
              hitCueBall(cue, gs, dir, magnitude);
            }
          }
        updateBalls(balls, gs, 1.0f);
        gs.currentShotScore = analyzeShot(gs.currentShot);

        ballHit firstBall = FindFirstBallHit(cue.pos, Vector2Normalize(dir), rack, ballRadius);

        drawBallMask(res.rt_rackBallMask, rack);
        if(flashProg != -1){
          flashProg += GetFrameTime() * flashSpeed;
          if(flashProg > 1) {flashProg = -1; continue;}
        }
        BeginDrawing();
          ClearBackground(DARKTEAL);

            DrawTexture(res.tx_background, 0, 0, WHITE);
            drawInfoText(gs);
            drawTable(time);

            drawHoles();

            if(firstBall.hitBall != nullptr){
              DrawCircleV(firstBall.ghostPos + (Vector2){playingSurface.x, playingSurface.y}, ballRadius, {0,0,0,70});
              Vector2 dir = firstBall.hitBall->pos - firstBall.ghostPos;
              DrawLineEx(
                  firstBall.hitBall->pos + (Vector2){playingSurface.x, playingSurface.y}, 
                  firstBall.hitBall->pos + (Vector2){playingSurface.x, playingSurface.y} + (Vector2Normalize(dir) * 64), 5, {0,0,0,70});
            }

            DrawCircleV(Vector2Add({playingSurface.x, playingSurface.y}, cue.pos), cue.radius, (Vector2Length(cue.vel) > minMovingShotSpeed) ? RED : VANCREAM);

            drawBalls(rack);

            DrawCircleV(ghostPos, 8, VANCREAM);
            for(int i = 0; i < hintDots; i++){
              Vector2 pos = (cp + (Vector2Normalize(dir) * cue.radius)) + Vector2Normalize(dir) * (i+1) * pow((magnitude / maxDist), 2) * dotSpacing;
              DrawCircleV({pos.x, pos.y}, 6-(4*((float)i/hintDots)), VANCREAM);
            }
          if(flashProg != -1) drawFlashOverTable(flash, flashProg);
          DrawCircleV(cursorPos, 4, VANCREAM);
        EndDrawing();
      }
      break;
      case postGame:{
        updateBalls(balls, gs, 1.0f);
        BeginDrawing();
        ClearBackground(BLACK);
        DrawTexture(res.tx_background, 0,0, WHITE);
        drawTable(time);
        drawHoles();
        DrawCircleV(Vector2Add({playingSurface.x, playingSurface.y}, cue.pos), cue.radius, (Vector2Length(cue.vel) > minMovingShotSpeed) ? RED : VANCREAM);
        DrawRectangleRec(playingSurface, {0,0,0, 170});
        EndDrawing();
      }
        break;
    }

  }
  unloadResources();
  CloseWindow();
}
