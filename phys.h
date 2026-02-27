#ifndef PHYS
#define PHYS
#include <string>
#include <vector>
#include <algorithm>
#include <raylib.h>
#include <raymath.h>

using namespace std;

typedef struct ballHit ballHit;
typedef struct ball ball;
typedef struct gameState gameState;
typedef struct shotInfo shotInfo;

ballHit FindFirstBallHit(
  const Vector2& cuePos,
  const Vector2& cueDir,
  std::vector<ball>& balls,
  float radius);
void RackBallsTriangle(vector<ball> &balls, int count, Vector2 apexPos);

void ResolveCollision(ball& a, ball& b);
bool ResolveCollisionTOI(ball& a, ball& b, float dt, gameState &gs);
void collideAllBalls(vector<ball*> &balls);
void checkPockets(ball &b);
bool ResolveWallCollision(ball &b);
void updateBalls(vector<ball*> &balls, gameState &gs, float timeScale);

int analyzeShot(shotInfo &si);
void resetGame(gameState &gs, vector<ball> &rack, ball &cue);
void hitCueBall(ball &cue, gameState& gs, const Vector2 &dir, const float &magnitude);
#endif
