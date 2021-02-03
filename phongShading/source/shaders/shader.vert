#version 450
#extension GL_ARB_separate_shader_objects : enable

// the uniform buffer object
layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

// inputs specified in the vertex buffer attributes
layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;

// the shader output, goes to the next stage in the pipeline (for our pipeline goes to the fragment stage)
layout(location = 0) out vec3 fragColor;

// main function, entry point to the shader
void main() {
    // gl_position is a keyword 
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 0.0, 1.0);
    fragColor = inColor;
}