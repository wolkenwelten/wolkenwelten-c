uniform sampler2D curTex;

in vec2 multiTexCoord;
in vec4 color;

out vec4 fragColor;

void main() {
	fragColor = texture(curTex, multiTexCoord) * color;
}
