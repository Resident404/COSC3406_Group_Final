#version 130

// Attributes passed from the vertex shader
in vec3 position_interp;
in vec3 normal_interp;
in vec3 light_pos;

// Material attributes - BRIGHT WHITE
vec4 ambient_color = vec4(0.8, 0.8, 0.8, 1.0);
vec4 diffuse_color = vec4(1.0, 1.0, 1.0, 1.0);  // Pure white
vec4 specular_color = vec4(1.0, 1.0, 1.0, 1.0);
float phong_exponent = 64.0;


void main()
{
    // Blinn-Phong shading
    vec3 N, // Interpolated normal for fragment
         L, // Light-source direction
         V, // View direction
         H; // Half-way vector

    // Compute Lambertian lighting Id
    N = normalize(normal_interp);

    L = (light_pos - position_interp);
    L = normalize(L);

    float Id = max(dot(N, L), 0.0);

    // Compute specular term for Blinn-Phong shading
    V = - position_interp; // Eye position is (0, 0, 0) in view coordinates
    V = normalize(V);

    H = (V + L); // Halfway vector
    H = normalize(H);

    float spec_angle_cos = max(dot(N, H), 0.0);
    float Is = pow(spec_angle_cos, phong_exponent);

    // Assign light to the fragment
    gl_FragColor = ambient_color + Id*diffuse_color + Is*specular_color;
}
