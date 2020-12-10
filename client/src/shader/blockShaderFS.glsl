uniform sampler2DArray curTex;
uniform float colorAlpha;

in vec3 texCoord;
in float lightness;

out vec4 fragColor;

void main() {
	vec3 texColor = texture(curTex, texCoord).rgb;
	fragColor = vec4(lightness,lightness,lightness,1.f);
	//fragColor = vec4(texColor,1.f);
	//fragColor = vec4(texColor * vec3(lightness,lightness,lightness),colorAlpha);
}
