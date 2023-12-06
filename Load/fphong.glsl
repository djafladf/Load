#version 330

in vec4 color;
in vec3 N3;
in vec3 L3;
in vec3 V3;

uniform mat4 uProjMat;
uniform mat4 uModelMat;
uniform vec4 uLPos;
uniform vec4 uLCol;
uniform vec4 uKAmb;
uniform vec4 uKDif;
uniform vec4 uKSpc;
uniform float uShine;

out vec4 fColor;

void main()
{
	vec3 N = normalize(N3);
	vec3 L = normalize(L3);
	vec3 V = normalize(V3);

	// R = 2*dot(N,L)*N - L
	vec3 R = 2*dot(N, L)*N - L;

	vec4 amb = uKAmb*uLCol;

	float d = max(dot(L,N),0);

	vec4 dif = uKDif*uLCol*d;

	float s = pow(max(dot(R,V),0), uShine);
	vec4 spec = uKSpc*uLCol*s;


	fColor = amb + dif + spec;					// phong illumination model
	fColor.w = 1.0f;
}
