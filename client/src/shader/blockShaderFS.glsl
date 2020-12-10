uniform sampler2D curTex;
uniform float colorAlpha;

in vec2 multiTexCoord;
in float lightness;

out vec4 fragColor;

void main() {
	vec2 texCoord = (fract(multiTexCoord) + floor(multiTexCoord / 256.0))/16.0;
	fragColor = vec4(texture2D(curTex, texCoord).rgb * vec3(lightness,lightness,lightness),colorAlpha);
}
