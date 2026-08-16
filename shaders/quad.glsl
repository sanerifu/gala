VARYING vec2 vTex;

#ifdef VERTEX
void main() {
    vec2 tex = vec2(float((gl_VertexID << 1) & 2), float(gl_VertexID & 2));
    vTex = tex;
    gl_Position = vec4(tex * 2.0f - 1.0f, 0.0f, 1.0f);
}
#endif

#ifdef FRAGMENT
out vec4 oColor;
void main() {
    oColor = texture(tColor, vec2(vTex.s, 1.0f - vTex.t));
}
#endif
