//#version 330
//
//in vec2 fragTexCoord;
//out vec4 finalColor;
//
//uniform sampler2D texture0;
//uniform float time;
//
//const vec2 resolution = vec2(512.0, 512.0);
//
//// Size of the fake vertex grid
//const float blockSize = 6.0;
//
//// Max movement in pixels
//const float wobble = 0.25;
//
//// Speed of the motion
//const float speed = 0.4;
//
//void main()
//{
//    // UV → pixel space
//    vec2 pixel = fragTexCoord * resolution;
//
//    // Identify block
//    vec2 block = floor(pixel / blockSize);
//
//    // Base snapped position (this is the stable grid)
//    vec2 snappedPixel = block * blockSize;
//
//    // Stable per-block phase
//    float phase = sin(
//        block.x * 11.7 +
//        block.y * 5.3 +
//        time * speed
//    );
//
//    // Block-level offset (this is the movement)
//    vec2 offset = vec2(
//        phase,
//        cos(phase + 1.3)
//    ) * wobble;
//
//    // Apply motion AFTER snapping
//    vec2 finalPixel = snappedPixel + offset;
//
//    vec2 uv = finalPixel / resolution;
//    finalColor = texture(texture0, uv);
//}
//

//---------------//
//WORKING "WAVEY"//
//---------------//

//#version 330
//
//in vec2 fragTexCoord;
//out vec4 finalColor;
//
//uniform sampler2D texture0;
//uniform float time;
//
//const float wobble = 0.005; // Strength in UV units
//const float speed = 2.0;
//
//void main()
//{
//    // Use a very small multiplier on fragTexCoord to create wide, smooth waves
//    float phaseX = fragTexCoord.x * 2.0 + time * speed;
//    float phaseY = fragTexCoord.y * 2.0 + time * speed;
//
//    vec2 offset = vec2(
//        sin(phaseY), 
//        cos(phaseX)
//    ) * wobble;
//
//    vec2 uv = fragTexCoord + offset;
//    finalColor = texture(texture0, uv);
//}

//#version 330
//
//in vec2 fragTexCoord;
//out vec4 finalColor;
//
//uniform sampler2D texture0;
//uniform float time;
//
//// ADJUST THESE
//const float strength = 0.008; // How far it drifts
//const float speed = 1.5;      // Overall movement speed
//
//void main()
//{
//    vec2 uv = fragTexCoord;
//    
//    // Layer 1: Large, slow movement
//    float driftX = sin(uv.y * 2.0 + time * speed) * 0.5;
//    driftX += sin(uv.y * 5.0 + time * speed * 0.8) * 0.25;
//    
//    // Layer 2: Medium, faster movement for variety
//    float driftY = cos(uv.x * 3.0 + time * speed * 1.1) * 0.5;
//    driftY += cos(uv.x * 7.0 + time * speed * 0.6) * 0.25;
//
//    // Combine them into a final offset
//    vec2 offset = vec2(driftX, driftY) * strength;
//
//    finalColor = texture(texture0, uv + offset);
//}

#version 330

in vec2 fragTexCoord;
out vec4 finalColor;

uniform sampler2D texture0;
uniform float time;

// ADJUST THESE
const float strength = 0.01; 
const float speed = 0.8;      
const float targetFPS = 8.0; // The "snappiness" factor (try 8.0 to 15.0)

void main()
{
    vec2 uv = fragTexCoord;
    
    // 1. "Snap" the time variable to a lower frequency
    // This makes the 't' value stay the same for several frames
    float snappedTime = floor(time * targetFPS) / targetFPS;

    // 2. Use snappedTime instead of the raw time
    // Layer 1: Large, slow movement
    float driftX = sin(uv.y * 2.0 + snappedTime * speed) * 0.5;
    driftX += sin(uv.y * 5.0 + snappedTime * speed * 0.8) * 0.25;
    
    // Layer 2: Medium, faster movement
    float driftY = cos(uv.x * 3.0 + snappedTime * speed * 1.1) * 0.5;
    driftY += cos(uv.x * 7.0 + snappedTime * speed * 0.6) * 0.25;

    vec2 offset = vec2(driftX, driftY) * strength;

    finalColor = texture(texture0, uv + offset);
}
