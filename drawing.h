#ifndef DRAWING
#define DRAWING
#include <string>
#include <vector>
#include <raylib.h>
#include <raymath.h>

using namespace std;

typedef struct gameState gameState;
typedef struct ball ball;

float easeOutCubic(float x);
void drawTextAlignRight(const Font &f, const string &text, float size, Vector2 pos, Color c);
void drawTableOfContents(
    const Font &f, 
    const vector<pair<string, string>> &text,
    int minDots,
    float size,
    Vector2 pos,
    Color c);
void drawInfoText(const gameState &gs);
void drawTable(const float &time);
void drawHoles();
void drawBallMask(RenderTexture2D &rt, vector<ball> &rack);
void drawBalls(const vector<ball> &rack);
void drawTextFlash(
  RenderTexture2D &rt, 
  const Font &f, 
  string txt, 
  float size,
  Color bg, Color fg);
void drawFlashOverTable(const RenderTexture2D &flash, float prog);
void loadResources();
void unloadResources();
#endif
