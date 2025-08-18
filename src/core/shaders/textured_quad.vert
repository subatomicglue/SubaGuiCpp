#version 450
layout(location=0) in vec2 inPos;
layout(location=1) in vec2 inUV;
layout(location=2) in uint inColor;

layout(push_constant) uniform PC {
  mat3 m; // 2D affine as mat3 (clip space transform)
} pc;

layout(location=0) out vec2 vUV;
layout(location=1) flat out uint vColor;

void main(){
  vec3 p = pc.m * vec3(inPos, 1.0);
  gl_Position = vec4(p.xy, 0.0, 1.0);
  vUV = inUV;
  vColor = inColor;
}
