#version 330 core

out vec4 FragColor;

uniform float time;
uniform vec2 resolution;
uniform sampler2D tex0;

void main() {

  vec2 uv = gl_FragCoord.xy / max(resolution.x, resolution.y);

  vec4 color = texture2D(tex0, uv + sin(time/4.0));

  FragColor = color * vec4(sin(time/2.0), 0.75f, 0.4f, 1.0f);
}
