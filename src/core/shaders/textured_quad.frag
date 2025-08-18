#version 450
layout(location=0) in vec2 vUV;
layout(location=1) flat in uint vColor;

layout(location=0) out vec4 outColor;

layout(set=0, binding=0) uniform sampler2D tex0;

vec4 abgr(uint c){
  float a = float((c >> 24) & 255) / 255.0;
  float b = float((c >> 16) & 255) / 255.0;
  float g = float((c >>  8) & 255) / 255.0;
  float r = float((c >>  0) & 255) / 255.0;
  return vec4(r,g,b,a);
}

void main(){
  vec4 base = texture(tex0, vUV);
  vec4 modC = abgr(vColor);
  outColor = base * modC;
}
