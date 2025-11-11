#version 100
precision highp float;

uniform sampler2D tex;
uniform float time;
uniform float intensity;

varying vec2 uv;

// Random number generator
float rand(vec2 co) {
    return fract(sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453);
}

void main() {
    vec2 uv0 = uv; // Keep original UVs for distortion effects

    // === Simple per-row sync jitter ===
    float rowIndex = floor(uv0.y * 240.0);
    float tStep = floor(time * 10.0);
    vec2 seed = vec2(rowIndex, tStep);

    if (rand(seed) < 0.02) {
        float dir = rand(seed + 10.0) < 0.5 ? -1.0 : 1.0;
        uv0.x += dir / 320.0;
    }

    // === VHS glitch ===
    if (rand(vec2(floor(time * 12.0), 0.0)) > 0.95) {
        float glitch_y = step(0.3, fract(uv0.y * 10.0));
        float glitch_offset = (rand(vec2(time, uv0.y)) - 0.5) * 0.2 * intensity;
        uv0.x += glitch_offset * glitch_y;
    }

    // === Chromatic aberration ===
    float shift = 0.015 * intensity;
    float r = texture2D(tex, uv0 + vec2(shift, 0.0)).r;
    float g = texture2D(tex, uv0).g;
    float b = texture2D(tex, uv0 - vec2(shift, 0.0)).b;
    vec3 color = vec3(r, g, b);

    // === Noise ===
    float noise = (rand(uv0 * time) - 0.5) * 0.5 * intensity;
    color += noise;

    // === Very Thick TV-Style Scanlines ===
    float lines = 240.0; // Matches 480i resolution
    float thickness = 0.3;
    float darkness = 0.3;

    float scanline = cos(uv0.y * lines * 3.14159);
    scanline = clamp(scanline * thickness + (1.0 - thickness), 0.0, 1.0);
    color *= mix(darkness, 1.0, scanline);

    // Add subtle phosphor glow between lines
    vec3 glow = texture2D(tex, uv0 + vec2(0.0, 0.01)).rgb * 0.2;
    color = max(color, glow);

    gl_FragColor = vec4(clamp(color, 0.0, 1.0), 1.0);
}