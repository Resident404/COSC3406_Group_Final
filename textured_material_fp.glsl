#version 130

// Attributes passed from the vertex shader
in vec2 uv_interp;
in vec3 normal_interp;
in vec4 color_interp;

// Texture sampler
uniform sampler2D texture_map;

void main()
{
    // Sample texture
    vec4 tex_color = texture(texture_map, uv_interp);

    // Simple lighting
    vec3 light_dir = normalize(vec3(0.5, 0.8, 0.6));
    vec3 normal = normalize(normal_interp);
    float diffuse = max(dot(normal, light_dir), 0.3);

    // Combine texture and lighting
    gl_FragColor = vec4(tex_color.rgb * diffuse, tex_color.a);
}
