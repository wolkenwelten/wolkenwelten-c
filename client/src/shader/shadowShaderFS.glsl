uniform sampler2D curTex;

in vec2 multiTexCoord;
in float lightness;

out vec4 fragColor;

void main() {
	fragColor = texture(curTex, multiTexCoord) * vec4(1.0,1.0,1.0,lightness);
}
