uniform mat4 matMVP;
uniform vec3 transPos;

in uint packedData;

out vec3 texCoord;
out float lightValue;

void main(){
	/* First we unpack the position out of the lowest 3x5-bits */
	uvec4 pos      = uvec4(packedData & 0x1Fu, (packedData >> 5) & 0x1Fu, (packedData >> 10) & 0x1Fu, 1);
	/* Then we use the positional data as texture coordinates, since our
	 | textures wrap around we get a tiling effect, as well as different
	 | textures for each block depending on world position which should
	 | help make things look slightly more interesting.
	 */
	uvec2 taxis[3] = uvec2[3](pos.xy, pos.xz, pos.zy);
	/* Now we unpack the textureIndex and side of the current vertex, the
	 | side is used select the correct spatial axes for usage as texture
	 | coordinates. The textureIndex chooses which texture to draw.
	 */
	uint textureIndex = (packedData >> 16) & 0xFFu;
	uint side      = (packedData >> 24) & 0x7u;
	/* We divide the uv coordinates by 2 since we have 4 variations per
	 | texture right now. If we ever add more we have to increase this value.
	 */
	texCoord = vec3(uvec3(taxis[side >> 1], textureIndex)) / vec3(2.0,2.0,1.0);
	/* Finally we extract the 4-bit lightness value, turn it into a float
	 | in the range of 0.0-1.0 and then multiply it by itself to make theater
	 | lightness curve look non-linear
	 */
	float lightRaw = float(packedData >> 28) * (1.0 / 16.0);
	lightValue = lightRaw * lightRaw;

	/* To determine the position we multiply by our MVP matrix after adding
	 | our transPos uniform value, this is done so that our position within
	 | a chunk can fit in 5-bits, without this step we would need 16-bit
	 | values, per axis...
	 */
	gl_Position = matMVP * (vec4(pos) + vec4(transPos,0.0));
}
