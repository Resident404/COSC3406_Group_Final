#version 130

// Vertex buffer
in vec3 vertex;
in vec3 normal;
in vec3 color;

// Uniform (global) buffer
uniform mat4 world_mat;
uniform mat4 view_mat;
uniform mat4 projection_mat;

// Attributes forwarded to the fragment shader
out vec2 uv_interp;
out vec3 normal_interp;
out vec4 color_interp;


void main()
{
    gl_Position = projection_mat * view_mat * world_mat * vec4(vertex, 1.0);

    // Generate texture coordinates from vertex position
    // Simple planar mapping
    uv_interp = vertex.xy * 2.0;

    // Forward normal for lighting (simplified - no normal matrix for now)
    normal_interp = normal;

    // Forward color
    color_interp = vec4(color, 1.0);
}
