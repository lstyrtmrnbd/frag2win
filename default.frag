#version 330 core

out vec4 FragColor;

uniform float time;

void main() {

  FragColor = vec4(sin(time/2.0), 0.75f, 0.4f, 1.0f);
}