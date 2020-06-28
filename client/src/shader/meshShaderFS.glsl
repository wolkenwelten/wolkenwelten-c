uniform sampler2D curTex;

varying vec2 multiTexCoord;
varying float lightness;

void main() {
	gl_FragColor = texture2D(curTex, multiTexCoord) * vec4(lightness,lightness,lightness,1.0);
}
