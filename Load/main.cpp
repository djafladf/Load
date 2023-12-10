#include <vgl.h>
#include <InitShader.h>
#include "MyCube.h"
#include "Obj.h"

#include <vec.h>
#include <mat.h>
#define ROTATEX 0
#define ROTATEY 1
#define ROTATEZ 2

MyCube cube;
MyObj Obj;

GLuint program;
GLuint prog_phong;

GLuint uMat;
GLuint uColor;
mat4 g_Mat = mat4(1.0f);

int winWidth = 500;
int winHeight = 500;

bool play = false;
int phong = 1;

float g_Spec = 0.3;
float g_Shine = 50;

float Ex = 0;
float Ey = 0;
float Ez = 0;

int RT = 0;

mat4 myLookAt(vec3 eye, vec3 at, vec3 up)
{
	// Implement your own look-at function
	mat4 V(1.0f);
	vec3 n = at - eye;
	n /= length(n);

	float a = dot(up, n);
	vec3 v = up - a * n;
	v /= length(v);

	vec3 w = cross(n, v);

	mat4 Rw(1.0f);

	Rw[0][0] = w.x;	Rw[0][1] = v.x; Rw[0][2] = -n.x;
	Rw[1][0] = w.y;	Rw[1][1] = v.y; Rw[1][2] = -n.y;
	Rw[2][0] = w.z;	Rw[2][1] = v.z; Rw[2][2] = -n.z;

	mat4 Rc(1.0f);
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			Rc[i][j] = Rw[j][i];

	mat4 Tc = Translate(-eye.x, -eye.y, -eye.z);

	V = Rc * Tc;

	return V;
}

mat4 myOrtho(float l, float r, float b, float t, float zNear, float zFar)
{
	// Implement your own Ortho function
	mat4 V(1.0f);

	V[0][0] = 2 / (r - l);
	V[1][1] = 2 / (t - b);
	V[2][2] = 2 / (zFar - zNear);
	V[0][3] = -(r + l) / (r - l);
	V[1][3] = -(t + b) / (t - b);
	V[2][3] = (zNear) / (zFar - zNear);

	return V;
}

mat4 myPerspective(float fovy, float aspectRatio, float zNear, float zFar)
{
	mat4 P(1.0f);

	float rad = fovy * 3.141592 / 180.0;

	float sz = 1 / zFar;
	float h = zFar * tan(rad / 2);

	float sy = 1 / h;
	float w = h * aspectRatio;
	float sx = 1 / w;

	mat4 S = Scale(sx, sy, sz);
	mat4 M(1.0f);

	float c = -zNear / zFar;
	M[2][2] = 1 / (c + 1);
	M[2][3] = -c / (c + 1);
	M[3][2] = -1;
	M[3][3] = 0;

	P = M * S;

	return P;
}

void myInit(FILE * file)
{
	cube.Init();
	Obj.Init(file);
	//bj.Init("Cube.obj");

	program = InitShader("vshader.glsl", "fshader.glsl");
	prog_phong = InitShader("vphong.glsl", "fphong.glsl");
}

void DrawAxis()
{
	glUseProgram(program);
	mat4 x_a = Translate(0.5, 0, 0) * Scale(1, 0.05, 0.05);
	glUniformMatrix4fv(uMat, 1, GL_TRUE, g_Mat * x_a);
	glUniform4f(uColor, 1, 0, 0, 1);
	cube.Draw(program);

	mat4 y_a = Translate(0, 0.5, 0) * Scale(0.05, 1, 0.05);
	glUniformMatrix4fv(uMat, 1, GL_TRUE, g_Mat * y_a);
	glUniform4f(uColor, 0, 1, 0, 1);
	cube.Draw(program);

	mat4 z_a = Translate(0, 0, 0.5) * Scale(0.05, 0.05, 1);
	glUniformMatrix4fv(uMat, 1, GL_TRUE, g_Mat * z_a);
	glUniform4f(uColor, 0, 0, 1, 1);
	cube.Draw(program);
}

void DrawGrid()
{
	glUseProgram(program);
	float n = 40;
	float w = 10;
	float h = 10;

	for (int i = 0; i < n; i++)
	{
		mat4 m = Translate(0, -0.5, -h + 2 * h / n * i) * Scale(w * 2, 0.02, 0.02);
		glUniformMatrix4fv(uMat, 1, GL_TRUE, g_Mat * m);
		glUniform4f(uColor, 1, 1, 1, 1);
		cube.Draw(program);
	}
	for (int i = 0; i < n; i++)
	{
		mat4 m = Translate(-w + 2 * w / n * i, -0.5, 0) * Scale(0.02, 0.02, h * 2);
		glUniformMatrix4fv(uMat, 1, GL_TRUE, g_Mat * m);
		glUniform4f(uColor, 1, 1, 1, 1);
		cube.Draw(program);
	}
}

float g_Time = 0;

void display()
{
	glEnable(GL_DEPTH_TEST);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);


	mat4 ViewMat = myLookAt(vec3(1,0,1) * 3, vec3(0, 0, 0), vec3(0, 1, 0));
	//if(RT == ROTATEX) ViewMat = myLookAt(vec3(sin(Ex), sin(Ey), sin(Ez)) * 3, vec3(0, 0, 0), vec3(0, 1, 0));

	float aspect = winWidth / (float)winHeight;
	float h = 1;

	//mat4 ProjMat = myOrtho(-h*aspect,h*aspect,-h,h,0,100);
	mat4 ProjMat = myPerspective(40, aspect, 0.1, 100);

	g_Mat = ProjMat * ViewMat;

	glUseProgram(program);
	uMat = glGetUniformLocation(program, "uMat");
	uColor = glGetUniformLocation(program, "uColor");

	DrawAxis();

	glUseProgram(prog_phong);
	GLuint uProjMat = glGetUniformLocation(prog_phong, "uProjMat");
	GLuint uModelMat = glGetUniformLocation(prog_phong, "uModelMat");

	mat4 ModelMat = RotateX(Ex * 90) * RotateY(Ey * 90) * RotateZ(Ez * 90) * Scale(Obj.SCALE);
	glUniformMatrix4fv(uProjMat, 1, GL_TRUE, ProjMat);
	glUniformMatrix4fv(uModelMat, 1, GL_TRUE, ViewMat * ModelMat);

	//vec4 LPos = vec4(2*sin(g_Time*3.14/2), 2, -2+2*cos(g_Time * 3.14 / 2), 1);
	vec4 LPos = vec4(2, 2, 0, 1);
	vec4 LCol = vec4(1, 1, 1, 1);
	vec4 KAmb = vec4(0.1, 0.1, 0.2, 1);
	vec4 KDif = vec4(1.0, 0.8, 0.5, 1);
	vec4 KSpc = vec4(1, 1, 1, 1) * g_Spec;

	GLuint uLPos = glGetUniformLocation(prog_phong, "uLPos");
	GLuint uLCol = glGetUniformLocation(prog_phong, "uLCol");
	GLuint uKAmb = glGetUniformLocation(prog_phong, "uKAmb");
	GLuint uKDif = glGetUniformLocation(prog_phong, "uKDif");
	GLuint uKSpc = glGetUniformLocation(prog_phong, "uKSpc");
	GLuint uShine = glGetUniformLocation(prog_phong, "uShine");
	GLuint uPhong = glGetUniformLocation(prog_phong, "uPhong");

	glUniform4f(uLPos, LPos[0], LPos[1], LPos[2], LPos[3]);
	glUniform4f(uLCol, LCol[0], LCol[1], LCol[2], LCol[3]);
	glUniform4f(uKAmb, KAmb[0], KAmb[1], KAmb[2], KAmb[3]);
	glUniform4f(uKDif, KDif[0], KDif[1], KDif[2], KDif[3]);
	glUniform4f(uKSpc, KSpc[0], KSpc[1], KSpc[2], KSpc[3]);
	glUniform1f(uShine, g_Shine);
	glUniform1i(uPhong, phong);


	//cube.Draw(prog_phong);
	Obj.Draw(prog_phong);

	glutSwapBuffers();
}


void idle()
{
	if (play) 
	{
		g_Time += 0.016;
		if (RT == ROTATEX) Ex += 0.016;
		else if (RT == ROTATEY) Ey += 0.016;
		else Ez += 0.016;
	}
	Sleep(16);					// for vSync
	glutPostRedisplay();
}

void keyboard(unsigned char ch, int x, int y)
{

	switch (ch) 
	{
	case ' ': play = !play; break;
	case '1': phong = 1; break;
	case '2': phong = 0; break;
	case '3': if (g_Spec < 1) g_Spec += 0.1f; break;
	case '4': if (g_Spec > 0.1) g_Spec -= 0.1f; break;
	case '5': if (g_Shine > 10) g_Shine -= 10; break;
	case '6': if (g_Shine < 100) g_Shine += 10; break;
	}
}

void mouse(int button, int state, int x, int y) 
{
	if (button == GLUT_LEFT_BUTTON) RT = ROTATEX;
	else if (button == GLUT_MIDDLE_BUTTON) RT = ROTATEY;
	else if (button == GLUT_RIGHT_BUTTON) RT = ROTATEZ;
}

void reshape(int w, int h)
{
	winWidth = w;
	winHeight = h;
	glViewport(0, 0, w, h);
	glutPostRedisplay();
}

#include<cstdio>
using namespace std;
int main(int argc, char** argv)
{
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(winWidth, winHeight);
	string path;
	getline(cin, path);
	FILE* file;
	while (true)
	{
		file = fopen(path.c_str(), "r");
		if (file) break;
		cout << "Can't find File : '" + path + "'!\n";
		getline(cin, path);
	}

	glutCreateWindow("OpenGL");

	glewExperimental = true;
	glewInit();

	myInit(file);
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutMouseFunc(mouse);
	glutKeyboardFunc(keyboard);
	glutReshapeFunc(reshape);
	glutMainLoop();

	return 0;
}
