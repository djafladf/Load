#version 330

in  vec4 vPosition;
in  vec3 vNormal;

out vec3 L3;
out vec3 N3;
out vec3 V3;

uniform mat4 uProjMat;
uniform mat4 uModelMat;
uniform vec4 uLPos;
uniform vec4 uLCol;
uniform vec4 uKAmb;
uniform vec4 uKDif;
uniform vec4 uKSpc;
uniform float uShine;
uniform int uPhong;

void main()
{
	gl_Position  = uProjMat * (uModelMat * vPosition);
	gl_Position *= vec4(1,1,-1,1);	// z축 방향이 반대임

	vec4 P = uModelMat*vPosition;				// camera coord
	vec4 L = uLPos - P;
	L3 = normalize(L.xyz);
	vec4 N;
	if(uPhong == 1) N = uModelMat*vec4(vPosition.xyz,0);
	else N = uModelMat*vec4(vNormal,0);
	
	N3 = normalize(N.xyz);					// camera coord.
	vec4 V = vec4(0,0,0,1) - P;
	V3 = normalize(V.xyz);
}
