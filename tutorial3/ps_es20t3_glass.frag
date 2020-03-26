precision highp float;

varying vec3 texCoord0;				// normal
varying vec3 texCoord1;				// view vector (eye - pixel position)
uniform vec3 my_EyePos;
uniform samplerCube samplerCb;

void main (void)
{
	vec3 normal = normalize(texCoord0);
	vec3 viewVector = normalize(texCoord1);

	float NV = dot(normal, viewVector);

	vec3 reflectionVector = 2.0 * NV * normal - viewVector;

	vec4 reflectionColor = textureCube(samplerCb, reflectionVector);

	/* sample using reflection vector */
	gl_FragColor = reflectionColor * 0.9999;
}
