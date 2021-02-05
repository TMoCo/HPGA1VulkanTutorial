#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D texSampler; // equivalent sampler1D and sampler3D

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec3 fragColor;
layout(location = 3) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

// light position
// NB we assume the light is white:
// const vec3 lightColour = vec3(1.0, 1.0, 1.0);
const vec3 lightPos = vec3(0, -30, 50);

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float specularExponent;
};

// material properties found in .mtl file
const Material mallard = Material(vec3(0.1, 0.1, 0.1), vec3(0.5, 0.5, 0.5), vec3(0.7, 0.7, 0.7), 38.0);

void main()
{
    // the colour of the model without lighting
    vec3 color = texture(texSampler, fragTexCoord).rgb;
    // view direction, assumes eye is at the origin (which is the case)
    vec3 viewDir = normalize(-fragPos);
    // light direction from fragment to light
    vec3 lightDir = normalize(lightPos - fragPos);
    // reflect direction, reflection of the light direction by the fragment normal
    vec3 reflectDir = reflect(-lightDir, fragNormal);


    // ambient
    vec3 ambient = mallard.ambient;

    // diffuse (lambertian)
    float diff = max(dot(lightDir, fragNormal), 0.0);
    vec3 diffuse = mallard.diffuse * diff;

    // specular
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), mallard.specularExponent);
    vec3 specular = mallard.specular * spec;

    // vector multiplication is element wise <3
    outColor = vec4((ambient + diffuse + specular) * color, 1.0);
    //outColor = vec4(ambient + diffuse + specular, 1.0);
}