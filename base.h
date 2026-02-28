#include <string>
#include <vector>
#include <algorithm>
#include <raylib.h>
#include <raymath.h>

using namespace std;

typedef enum gamePhase{
  shooting,
  postGame
}gamePhase;
typedef struct ball{
  float radius;
  Vector2 pos, vel;
  bool sunk = false;
  bool cue = false;
  float sinceSunk; //1 = complete
  int sunkHole = -1;
  bool eightball = false;
}ball;
typedef struct shotInfo{
  int cueBounces;
  int cueRailHits;
  int totalBounces;
  int totalRailHits;
  float initSpeed;
  float power;
  int sinks;
  int initSinks;
}shotInfo;
typedef struct gameState{
  int shots;
  int sunk;
  int score;
  int shotMultiplier;
  shotInfo currentShot;
  int currentShotScore = 0;
  bool sunkEightEarly = false;
  gamePhase gp = shooting;
}gameState;
typedef struct ballHit{
  ball* hitBall;
  float tHit;
  Vector2 ghostPos;
}ballHit;
typedef struct resources{
  Texture tx_background;
  Texture tx_table;
  Texture tx_ballBackground;
  Texture tx_ballOverlay;
  RenderTexture2D rt_rackBallMask;
  Font f_ming32;
  Shader sh_tablewobble;
    int tw_resloc;
    int tw_timeloc;
    int tw_wobloc;
  Shader sh_ballMask;
    int bm_bgLoc;
    int bm_maskLoc;
} resources;

    const Color DARKTEAL = {17, 75, 95, 255};
    const Color SEAGREEN = {26, 147, 111, 255};
    const Color CELADON  = {136, 212, 152, 255};
    const Color TEAGREEN = {198, 218, 191, 255};
    const Color VANCREAM = {243, 233, 210, 255};

    const float sinkAnimationSpeed = 10.0f;
    const int hintDots = 8;
    const float dotSpacing = 32;

    const float HOLE_SIZE = 40;
    const Vector2 TABLE_SIZE = {1024, 512};
    const float ballRadius = 18;

    const float maxDist = 256;

    const int cueBounceMulti = 3;
    const int totalBounceMulti = 1;
    const int cueRailHitMulti = 4;
    const int totalRailHitMulti = 2;
    const float minMovingShotSpeed = 50.0f;
    const int movingShotMulti = 4;

  extern Rectangle playingSurface;
  extern resources res;
  extern vector<Vector2> Holes;
