attribute vec3 position;

uniform mat4 mvp;

varying vec3 normal;

void main()
{
	gl_Position = mvp * vec4(position.xyz, 1.0);

        normal = position;
}
