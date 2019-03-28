uniform mat4 u_MVP;
uniform mat4 u_MV;

attribute vec4 position;
attribute vec2 textureCoords;
attribute vec3 normal;

varying vec3 v_Position;
varying vec3 v_Normal;


void main() {
    v_Position = vec3(u_MV * position);
    v_Normal = vec3(u_MV * vec4(normal, 0.0));

    gl_Position = u_MVP * position;
}
