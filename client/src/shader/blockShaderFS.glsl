out vec3 lightness;

uniform sampler2DArray curTex;
uniform float colorAlpha;

in vec3 texCoord;
in float lightValue;
out vec4 fragColor;

void main() {
	/* Very simple shader, we look up the currents pixel color according to
	 | the texCoord passed to us, and then multiply in order to darken the
	 | color according to the current lightness level.  The alpha value is
	 | stored as a uniform because we only fadeIn entire chunks just after
	 | they have been generated so their sudden appearance is less jarring.
	 */
	vec3 lightColor = vec3(lightValue, lightValue, lightValue);
	fragColor = vec4(texture(curTex, texCoord).rgb * lightColor, colorAlpha);
}
