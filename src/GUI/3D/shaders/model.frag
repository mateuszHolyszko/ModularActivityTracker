precision mediump float;
varying vec3 v_normal;

void main() {
    vec3 light_dir = normalize(vec3(0.5, 1.0, 0.8));
    float diff = max(dot(normalize(v_normal), light_dir), 0.0);
    
    // Use alpha based on the diffuse lighting
    float alpha = smoothstep(0.1, 0.5, diff);
    
    // Light gray color
    vec3 gray_color = vec3(0.7);
    gl_FragColor = vec4(gray_color * diff, alpha);
}