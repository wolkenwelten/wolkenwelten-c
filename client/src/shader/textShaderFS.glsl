uniform sampler2D curTex;

varying vec2 multiTexCoord;
varying vec4 frontColor;

void main() {
	vec4 pixel;
	pixel = texture2D(curTex, multiTexCoord);
	if (pixel.a == 0.0) { discard; }
	gl_FragColor = frontColor * pixel;
}
