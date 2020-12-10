uniform sampler2D curTex;

in vec2 multiTexCoord;
in float lightness;

out vec4 fragColor;

void main() {
	fragColor = texture2D(curTex, multiTexCoord) * vec4(lightness,lightness,lightness,1.0);
}
