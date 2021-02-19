#version 450
#extension GL_ARB_separate_shader_objects : enable

//
// Uniform
//

// the uniform buffer object
layout(binding = 0, std140) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec3 ambient;
    int uvToRgb;
    vec3 diffuse;
    int useTexture;
    vec4 specular;
    vec3 lightPos;
} ubo;

layout(binding = 1) uniform sampler2D texSampler; // equivalent sampler1D and sampler3D

//
// Input from previous stage
//

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec4 fragMaterial;
layout(location = 3) in vec2 fragTexCoord;

//
// Output
//

layout(location = 0) out vec4 outColor;

// light position
// NB we assume the light is white:
//const vec3 lightPos = vec3(0, 0, 1);

void main()
{
    if (ubo.uvToRgb == 1) {
        outColor = vec4(fragTexCoord, 0.0, -10.0);
    }
    else {
        // the colour of the model without lighting
        vec3 color = vec3(0.5, 0.5, 0.5);
        if (ubo.useTexture == 1) {
            color = texture(texSampler, fragTexCoord).rgb;
        }
        // view direction, assumes eye is at the origin (which is the case)
        vec3 viewDir = normalize(-fragPos);
        // light direction from fragment to light
        vec3 lightDir = normalize(ubo.lightPos - fragPos);
        // reflect direction, reflection of the light direction by the fragment normal
        vec3 reflectDir = reflect(-lightDir, fragNormal);

        // ambient
        vec3 ambient = ubo.ambient;

        // diffuse (lambertian)
        float diff = max(dot(lightDir, fragNormal), 0.0f);
        vec3 diffuse = ubo.diffuse * diff;

        // specular (glossy)
        float spec = pow(max(dot(viewDir, reflectDir), 0.0f), ubo.specular.w);
        vec3 specular = ubo.specular.xyz * spec;


        // vector multiplication is element wise <3
        outColor = vec4((ambient + diffuse + specular) * color, 1.0f);
    }
}