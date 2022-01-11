uniform sampler2D curTex;

in vec2 multiTexCoord;

out vec4 fragColor;

void main() {
	fragColor = texture(curTex, multiTexCoord);
}
