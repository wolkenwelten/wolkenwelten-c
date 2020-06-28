uniform sampler2D curTex;
uniform float colorAlpha;

varying vec2 multiTexCoord;
varying float lightness;

void main() {
	vec2 texCoord = (fract(multiTexCoord) + floor(multiTexCoord / 256.0))/16.0;
	gl_FragColor = vec4(texture2D(curTex, texCoord).rgb * vec3(lightness,lightness,lightness),colorAlpha);
}
