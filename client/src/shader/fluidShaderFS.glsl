uniform sampler2DArray curTex;
uniform float colorAlpha;

in vec3 texCoord;
in vec3 lightColor;
out vec4 fragColor;

void main() {
	fragColor = vec4(texture(curTex, texCoord).rgb * lightColor, colorAlpha);
}
