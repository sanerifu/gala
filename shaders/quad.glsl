VARYING vec2 vTex;

#ifdef VERTEX
void main() {
    vec2 tex = vec2(float((gl_VertexID << 1) & 2), float(gl_VertexID & 2));
    vTex = tex;
    gl_Position = vec4(tex * 0.5f, 0.0f, 1.0f);
}
#endif

#ifdef FRAGMENT
out vec4 oColor;
void main() {
    oColor = vec4(vTex * 0.5f, 0.0f, 1.0f);
}
#endif
