uniform sampler2DArray curTex;
uniform float colorAlpha;

in vec3 texCoord;
in vec3 lightness;

out vec4 fragColor;

void main() {
	vec3 texColor = texture(curTex, texCoord).rgb;
	fragColor = vec4(texColor * lightness,colorAlpha);
}
