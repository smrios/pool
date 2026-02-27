#include "base.h"
#include "drawing.h"
#include <string>
#include <vector>
#include <raylib.h>
#include <raymath.h>

using namespace std;

resources res;

float easeOutCubic(float x){
  return 1 - pow(1-x,1.2);
}
void drawTextAlignRight(const Font &f, const string &text, float size, Vector2 pos, Color c){
  Vector2 ts = MeasureTextEx(f, text.c_str(), size, 0);
  float xpos = pos.x - ts.x;
  DrawTextEx(f, text.c_str(), {xpos, pos.y}, size, 0, c);
}
void drawTableOfContents(
    const Font &f, 
    const vector<pair<string, string>> &text,
    int minDots,
    float size,
    Vector2 pos,
    Color c){
  int mostChars = 0;
  for(int i = 0; i < text.size(); i++){
    if(mostChars < text[i].first.length() + text[i].second.length()) 
      mostChars = text[i].first.length() + text[i].second.length();
  }
  float ypos = pos.y;
  for(int i = 0; i < text.size(); i++){
    int totalChars = text[i].first.length() + text[i].second.length();
    int numDots = minDots + (mostChars - totalChars);
    string dots = ""; 
    for(int i = 0; i < numDots; i++)
      dots += ".";
    string txt = text[i].first + dots + text[i].second;
    //DrawTextEx(f, txt.c_str(), {pos.x, ypos}, size, 0, c);
    drawTextAlignRight(f, txt, size, {pos.x, ypos}, c);
    ypos += MeasureTextEx(f, txt.c_str(), size, 0).y;
  }
}
void drawInfoText(const gameState &gs){
  drawTableOfContents(res.f_ming32, { 
      {"shots", to_string(gs.shots).c_str()},
      {"sunk", to_string(gs.sunk).c_str()},
      {"early 8", (gs.sunkEightEarly) ? "true" : "false"},
      {"s/s", to_string((gs.shots != 0) ? (float)gs.sunk / gs.shots : 0.0f).c_str()},
      {"trh", to_string(gs.currentShot.totalRailHits).c_str()},
      {"crh", to_string(gs.currentShot.cueRailHits).c_str()},
      {"cb", to_string(gs.currentShot.cueBounces).c_str()},
      {"tb", to_string(gs.currentShot.totalBounces).c_str()},
      {"sunk", to_string(gs.currentShot.sinks).c_str()},
      {"moving", (gs.currentShot.initSpeed > minMovingShotSpeed) ? "true" : "false"},
      {"shot score", to_string(gs.currentShotScore).c_str()},
      {"total score", to_string((int)roundf(gs.score * ((gs.shots != 0) ? (float)gs.sunk / gs.shots : 0.0f) * ((gs.sunkEightEarly) ? 0.1f : 1))).c_str()},
    },
    2, 32, {playingSurface.x - 64, playingSurface.y}, WHITE);
}
void drawTable(const float &time){
  SetShaderValue(res.sh_tablewobble, res.tw_timeloc, &time, SHADER_UNIFORM_FLOAT);
  BeginShaderMode(res.sh_tablewobble);
  DrawTexture(res.tx_table, playingSurface.x, playingSurface.y, WHITE);
  EndShaderMode();
}
void drawHoles(){
  for(int i = 0; i < Holes.size(); i++){
    DrawCircleV(Holes[i] + (Vector2){playingSurface.x, playingSurface.y}, HOLE_SIZE, BLACK);
  }
}
void drawBallMask(RenderTexture2D &rt, vector<ball> &rack){
  BeginTextureMode(rt);

  ClearBackground({0,0,0,0});
  for(int i = 0; i < 15; i++){
    if(!rack[i].sunk){
      DrawCircleV(Vector2Add({playingSurface.x, playingSurface.y}, rack[i].pos), rack[i].radius, WHITE);
      continue;
    }
    if(rack[i].sinceSunk > 1)
      continue;
    rack[i].sinceSunk += GetFrameTime() * sinkAnimationSpeed;
    Vector2 pos = Vector2Lerp(rack[i].pos, Holes[rack[i].sunkHole], easeOutCubic(rack[i].sinceSunk));
    float radius = rack[i].radius * (1 - easeOutCubic(rack[i].sinceSunk));
    DrawCircleV(Vector2Add({playingSurface.x, playingSurface.y}, pos), radius, WHITE);
  }

  EndTextureMode();
}
void drawBalls(const vector<ball> &rack){
  BeginShaderMode(res.sh_ballMask);
    SetShaderValueTexture(res.sh_ballMask, res.bm_bgLoc, res.tx_ballBackground);
    SetShaderValueTexture(res.sh_ballMask, res.bm_maskLoc, res.rt_rackBallMask.texture);

    DrawTexturePro(res.tx_ballBackground, {0,0, (float)res.tx_ballBackground.width, -(float)res.tx_ballBackground.height}, {0,0,1920,1080}, {0,0}, 0, WHITE);
  EndShaderMode();
    for(int i = 0; i < 15; i++){
      if(rack[i].sunk)
        continue;
      DrawTextureV(
          res.tx_ballOverlay, 
          ((Vector2){playingSurface.x, playingSurface.y} + rack[i].pos) - (Vector2){(float)res.tx_ballOverlay.width / 2, (float)res.tx_ballOverlay.height / 2},
          ((rack[i].eightball) ? (Color){50, 50, 50, 180} : (Color){164,180,252, 100})); 
    }
}
void drawTextFlash(
  RenderTexture2D &rt, 
  const Font &f, 
  string txt, 
  float size,
  Color bg, Color fg){
  UnloadRenderTexture(rt);
  Vector2 ts = MeasureTextEx(f, txt.c_str(), size, 0);
  rt = LoadRenderTexture(ts.x, ts.y);
  BeginTextureMode(rt);
  ClearBackground(bg);
  DrawTextEx(f, txt.c_str(), {0,0}, size, 0, fg);
  EndTextureMode();
}
void drawFlashOverTable(const RenderTexture2D &flash, float prog){
  DrawTexturePro(
    flash.texture, 
    {0,0,(float)flash.texture.width, -(float)flash.texture.height}, 
    playingSurface, 
    {0,0}, 0, {255, 255, 255, (unsigned char)floor((1-prog) * 255)});
}
void loadResources(){
  res.tx_background = LoadTexture("res/grime.png");
  res.tx_ballOverlay = LoadTexture("res/ballover.png");
  res.tx_ballBackground = LoadTexture("res/balltexture.png");
  res.rt_rackBallMask = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());

  res.tx_table = LoadTexture("res/pooltable.png");
    SetTextureWrap(res.tx_table, TEXTURE_WRAP_REPEAT);
    GenTextureMipmaps(&res.tx_table);
    SetTextureFilter(res.tx_table, TEXTURE_FILTER_TRILINEAR);
  Texture ballTexture = LoadTexture("res/balltexture.png");

  res.f_ming32 = LoadFontEx("res/ming.ttf", 32, 0, 255);

  res.sh_tablewobble = LoadShader(0, "res/psx_bg.fs");
    res.tw_resloc = GetShaderLocation(res.sh_tablewobble, "resolution");
    res.tw_timeloc = GetShaderLocation(res.sh_tablewobble, "time");
    res.tw_wobloc = GetShaderLocation(res.sh_tablewobble, "wobble");
    float wobble = 0.08f;
    Vector2 tres = {(float)res.tx_table.width, (float)res.tx_table.height};
    SetShaderValue(res.sh_tablewobble, res.tw_resloc, &tres, SHADER_UNIFORM_VEC2);
    SetShaderValue(res.sh_tablewobble, res.tw_wobloc, &wobble, SHADER_UNIFORM_FLOAT);
  res.sh_ballMask = LoadShader(0, "res/ballMask.fs");
    res.bm_bgLoc = GetShaderLocation(res.sh_ballMask, "uBackground");
    res.bm_maskLoc = GetShaderLocation(res.sh_ballMask, "uMask");
}
void unloadResources(){
  UnloadTexture(res.tx_table);  
  UnloadTexture(res.tx_ballOverlay);  
  UnloadTexture(res.tx_ballBackground);
  UnloadTexture(res.tx_background);  
  UnloadRenderTexture(res.rt_rackBallMask);
  UnloadShader(res.sh_tablewobble);
  UnloadShader(res.sh_ballMask);
  UnloadFont(res.f_ming32);
}
