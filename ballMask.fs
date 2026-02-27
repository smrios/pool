
#version 330

uniform sampler2D uBackground; // stacking texture
uniform sampler2D uMask;       // balls mask

in vec2 fragTexCoord;
out vec4 fragColor;

void main() {
    vec4 bg = texture(uBackground, fragTexCoord);
    float maskAlpha = texture(uMask, fragTexCoord).a; // only alpha channel

    // Overlay only where mask is opaque
    fragColor = vec4(bg.rgb * maskAlpha, maskAlpha);
}
