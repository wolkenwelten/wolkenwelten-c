uniform sampler2D curTex;

varying vec2 multiTexCoord;
varying float lightness;

void main() {
	gl_FragColor = texture2D(curTex, multiTexCoord) * vec4(1.0,1.0,1.0,lightness);
}
