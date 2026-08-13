VARYING vec2 vTex;

#ifdef VERTEX
void main() {
    gl_Position = vec4(aTexcoord * 0.5f, 0.0f, 1.0f);
    vTex = aTexcoord;
}
#endif

#ifdef FRAGMENT
out vec4 oColor;
void main() {
    oColor = vec4(vTex * 0.5f, 0.0f, 1.0f);
}
#endif
