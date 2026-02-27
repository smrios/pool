#include "base.h"
#include "phys.h"
#include <string>
#include <vector>
#include <algorithm>
#include <raylib.h>
#include <raymath.h>

using namespace std;

vector<Vector2> Holes { 
  {TABLE_SIZE.x / 2, 15 + TABLE_SIZE.y},
  {TABLE_SIZE.x / 2,-15},
  {-(5*0.71f),             -(5 * 0.71f)},
  {-(5*0.71f),              TABLE_SIZE.y+(5 * 0.71f)},
  {TABLE_SIZE.x+(5*0.71f), -(5 * 0.71f)},
  {TABLE_SIZE.x+(5*0.71f), TABLE_SIZE.y+(5 * 0.71f)},

};
ballHit FindFirstBallHit(
  const Vector2& cuePos,
  const Vector2& cueDir,
  std::vector<ball>& balls,
  float radius) {

  ballHit result = {};
  float closestT = std::numeric_limits<float>::max();

  float expandedRadius = radius * 2.0f;
  float expandedRadiusSq = expandedRadius * expandedRadius;

  for (auto& b : balls) {
    Vector2 m = b.pos - cuePos;
    float t = Vector2DotProduct(m, cueDir);

    if(b.sunk)
      continue;
    if (t < 0.0f)
      continue;

    float mLenSq = Vector2DotProduct(m, m);
    float dSq = mLenSq - t * t;

    if (dSq > expandedRadiusSq)
      continue;

    float thc = std::sqrt(expandedRadiusSq - dSq);
    float tHit = t - thc;

    if (tHit > 0.0f && tHit < closestT) {
      closestT = tHit;
      result.hitBall = &b;
      result.tHit = tHit;
    }
  }

  if (result.hitBall) {
    result.ghostPos = cuePos + cueDir * result.tHit;
  }

  return result;
}
void RackBallsTriangle(vector<ball> &balls, int count, Vector2 apexPos) {
  if (count < 15) return;

  float r = balls[0].radius;
  float rowSpacing = std::sqrt(3.0f) * r;   // vertical distance between rows
  float colSpacing = 2.0f * r;              // horizontal spacing

  int index = 0;

  // 5 rows: 1, 2, 3, 4, 5 balls
  for (int row = 0; row < 5; ++row) {
    int ballsInRow = row + 1;

    // Center row horizontally around apex X
    float rowWidth = (ballsInRow - 1) * colSpacing;
    float startY = apexPos.y - rowWidth * 0.5f;
    float x = apexPos.x + row * rowSpacing;

    for (int col = 0; col < ballsInRow; ++col) {
      balls[index].pos.y = startY + col * colSpacing;
      balls[index].pos.x = x;

      balls[index].vel = {0.0f, 0.0f};

      index++;
    }
  }
}

void ResolveCollision(ball& a, ball& b) {
  Vector2 delta = b.pos - a.pos;
  float dist = Vector2Length(delta);
  float minDist = a.radius + b.radius;

  // No collision
  if (dist >= minDist || dist == 0.0f)
      return;

  Vector2 normal = Vector2Normalize(delta);

  // --- Position correction (separate overlapping balls) ---
  float penetration = minDist - dist;
  Vector2 correction = normal * (penetration * 0.5f);
  a.pos = a.pos - correction;
  b.pos = b.pos + correction;

  // --- Relative velocity ---
  Vector2 relativeVel = b.vel - a.vel;

  float velAlongNormal = Vector2DotProduct(relativeVel, normal);

  // Balls are separating
  if (velAlongNormal > 0)
      return;

  // --- Elastic collision (equal mass) ---
  float restitution = 1.0f; // perfectly elastic

  float impulseMag = -(1.0f + restitution) * velAlongNormal / 2.0f;

  Vector2 impulse = normal * impulseMag;


  a.vel = a.vel - impulse;
  b.vel = b.vel + impulse;
}
bool ResolveCollisionTOI(ball& a, ball& b, float dt, gameState &gs) {
  Vector2 dp = Vector2Subtract(b.pos, a.pos);
  Vector2 dv = Vector2Subtract(b.vel, a.vel);
  float radiusSum = a.radius + b.radius;
  
  // Safety check: If they are ALREADY overlapping, resolve them instantly
  float distSq = Vector2DotProduct(dp, dp);
  if (distSq < radiusSum * radiusSum) {
    ResolveCollision(a, b); // Use your static resolver to push them apart
    return true; 
  }

  float A = Vector2DotProduct(dv, dv);
  if (A <= 0.000001f) return false; // Not moving relative to each other

  float B = 2.0f * Vector2DotProduct(dp, dv);
  float C = distSq - radiusSum * radiusSum;

  float discriminant = B * B - 4 * A * C;
  if (discriminant < 0.0f) return false;

  float sqrtDisc = sqrtf(discriminant);
  float t0 = (-B - sqrtDisc) / (2 * A);

  // If t0 is between 0 and dt, we have a collision this frame
  if (t0 >= 0.0f && t0 <= dt) {
    // 1. Move to impact
    a.pos = Vector2Add(a.pos, Vector2Scale(a.vel, t0));
    b.pos = Vector2Add(b.pos, Vector2Scale(b.vel, t0));

    // 2. Physics Response
    Vector2 normal = Vector2Normalize(Vector2Subtract(b.pos, a.pos));
    float velAlongNormal = Vector2DotProduct(Vector2Subtract(b.vel, a.vel), normal);

    if (velAlongNormal < 0) {
      float restitution = 1.0f;
      float impulseMag = -(1.0f + restitution) * velAlongNormal / 2.0f;
      Vector2 impulse = Vector2Scale(normal, impulseMag);
      a.vel = Vector2Subtract(a.vel, impulse);
      b.vel = Vector2Add(b.vel, impulse);
      
      gs.currentShot.totalBounces++;
      if(a.cue || b.cue)
        gs.currentShot.cueBounces++;
    }

    // 3. Move remaining time
    float remaining = dt - t0;
    a.pos = Vector2Add(a.pos, Vector2Scale(a.vel, remaining));
    b.pos = Vector2Add(b.pos, Vector2Scale(b.vel, remaining));
    
    return true;
  }

  return false;
}
void collideAllBalls(vector<ball*> &balls){
  for(int x = 1; x < balls.size(); x++){
    for(int y = 0; y < x; y++){
      if(balls[x]->sunk || balls[y]->sunk)
        continue;
      ResolveCollision(*balls[x], *balls[y]);
    }
  }
}
void checkPockets(ball &b){
  if(b.cue)
    return;
  for(int i = 0; i < Holes.size(); i++){
    if(Vector2Length(b.pos - Holes[i]) >= HOLE_SIZE + (b.radius / 2))
      continue;
    b.sunk = true;
    b.sunkHole = i;
    return;
  }
}
bool ResolveWallCollision(ball &b){
  float bumperRes = -0.8;
  bool ret = false;
  if(b.pos.x - b.radius < 0){
    b.vel.x *= bumperRes;
    b.pos.x = b.radius;
    ret = true;
  }
  if(b.pos.x + b.radius > TABLE_SIZE.x){
    b.vel.x *= bumperRes;
    b.pos.x = TABLE_SIZE.x - b.radius;
    ret = true;
  }
  if(b.pos.y - b.radius < 0){
    b.vel.y *= bumperRes;
    b.pos.y = b.radius;
    ret = true;
  }
  if(b.pos.y + b.radius > TABLE_SIZE.y){
    b.vel.y *= bumperRes;
    b.pos.y = TABLE_SIZE.y - b.radius;
    ret = true;
  }
  return ret;
}
void updateBalls(vector<ball*> &balls, gameState &gs, float timeScale){
  float dt = GetFrameTime() * timeScale;
  int sinks = 0;

  //Check pockets first

  bool nonEightRemaining = false;
  bool eightSunk = false;
  for (auto* b : balls) {
    if (b->cue) continue; // cue ball never sinks here
    if(b->eightball && b->sunk) { sinks++; eightSunk = true; continue;}
    if (b->sunk) { sinks++; continue; }
    checkPockets(*b);
    if(b->eightball && b->sunk) { sinks++; eightSunk = true; continue;}
    if (b->sunk) { sinks++; continue; }
    nonEightRemaining = true;
  }
  if(nonEightRemaining && eightSunk)
    gs.sunkEightEarly = true;
  if(!nonEightRemaining && eightSunk){
    gs.gp = postGame;
  }
  gs.currentShot.sinks = sinks-gs.currentShot.initSinks;
  gs.sunk = sinks;

  //Apply friction only to non-sunk balls
  for (auto* b : balls) {
    if (b->sunk) continue;
    float friction = 0.983f;
    b->vel *= powf(friction, dt * 60.0f);
    if (Vector2Length(b->vel) < 5.0f)
      b->vel = {0,0};
  }

  //Solve TOI collisions only for non-sunk balls
  for (int i = 0; i < balls.size(); i++) {
    if (balls[i]->sunk) continue;
    for (int j = i + 1; j < balls.size(); j++) {
      if (balls[j]->sunk) continue;
      ResolveCollisionTOI(*balls[i], *balls[j], dt, gs);
    }
  }

  //Move remaining balls
  for (auto* b : balls) {
    if (b->sunk) continue;
    b->pos += b->vel * dt;
  }

  //Wall collisions
  for (auto* b : balls) {
    if (b->sunk) continue;
    if(ResolveWallCollision(*b)){
      if(b->cue)
        gs.currentShot.cueRailHits++;
      gs.currentShot.totalRailHits++;
    }
  }
}

int analyzeShot(shotInfo &si){
  int score = 0; 
  score += 
    (si.cueBounces * cueBounceMulti) + 
    (si.totalBounces * totalBounceMulti) + 
    (si.cueRailHits * cueRailHitMulti) + 
    (si.totalRailHits * totalRailHitMulti);
  score *= si.sinks * ((si.initSpeed >= minMovingShotSpeed) ? movingShotMulti : 1);
  return score;
}
Rectangle playingSurface = {
  ((float)GetScreenWidth()/2)-(TABLE_SIZE.x/2), 
  ((float)GetScreenHeight()/2)-(TABLE_SIZE.y/2),
  TABLE_SIZE.x, TABLE_SIZE.y};
void resetGame(gameState &gs, vector<ball> &rack, ball &cue){
  gs = {
    .shots = 0,
    .sunk = 0,
    .score = 0,
    .shotMultiplier = 0,
    .currentShot = {
      .cueBounces = 0,
      .cueRailHits = 0,
      .totalBounces = 0,
      .totalRailHits = 0,
      .initSpeed = 0,
      .power = 0,
      .sinks = 0,
      .initSinks = 0,
    },
    .currentShotScore = 0,
    .sunkEightEarly = false,
  };
  RackBallsTriangle(rack, 15, {playingSurface.width * 0.6f, playingSurface.height/2});
  cue = { ballRadius, {playingSurface.width*0.2f, playingSurface.height/2}, {0,0}, false, true};
}
void hitCueBall(ball &cue, gameState& gs, const Vector2 &dir, const float &magnitude){
  gs.score += gs.currentShotScore;
  gs.currentShot = {
    .cueBounces = 0,
    .cueRailHits = 0,
    .totalBounces = 0,
    .totalRailHits = 0,
    .initSpeed = Vector2Length(cue.vel),
    .power = magnitude/maxDist,
    .sinks = 0,
    .initSinks = gs.sunk,
  };
  cue.vel = Vector2Normalize(dir) * pow((magnitude / maxDist),2) * 5000;
  gs.shots++;
  gs.currentShotScore = 0;
}
