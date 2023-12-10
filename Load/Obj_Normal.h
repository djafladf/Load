#pragma once

#include <vgl.h>
#include <vec.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <deque>
#include<list>
#pragma warning(disable : 4996)
using namespace std;

struct MyObjVertex
{
	vec4 position;
	vec3 normal;
	vec3 smooth;
};

class MyObj
{
public:
	MyObj(void);
	~MyObj(void);

	int NumVertices;

	MyObjVertex * Vertices;
	GLuint Init(string path);
	void SetPositionAndOtherAttributes(GLuint program);

	GLuint vao;
	GLuint buffer;
	vec3 MID, SCALE;
	bool bInitialized;
	void Draw(GLuint program);
};



MyObj::MyObj(void)
{
	bInitialized = false;
	NumVertices = 0;
	Vertices = NULL;
}

MyObj::~MyObj(void)
{
	if(Vertices != NULL) 
		delete [] Vertices;
}

#include <stdio.h>
#include <time.h>

GLuint MyObj::Init(string path)
{
	time_t start, end;
	if(bInitialized == true) return vao;
	FILE* file = fopen(path.c_str(), "r");
	if (file == NULL) return  vao;
	start = clock();
	deque<vec3> vecs;
	deque<deque<int>> Config;
	MID = vec3(0, 0, 0);
	bool Init = true;
	float minX = 0, maxX = 0;
	float minY = 0, maxY = 0;
	float minZ = 0, maxZ = 0;
	while (true) 
	{
		char lineHeader[256];
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF) break;
		if (strcmp(lineHeader, "v") == 0) 
		{
			vec3 cnt; fscanf(file, "%f %f %f\n", &cnt.x, &cnt.y, &cnt.z);
			vecs.push_back(cnt);
			if (Init)
			{
				Init = false;
				minX = cnt.x; maxX = cnt.x;
				minY = cnt.y; maxY = cnt.y;
				minZ = cnt.z; maxZ = cnt.z;
			}
			else
			{
				minX = min(cnt.x, minX); maxX = max(cnt.x, maxX);
				minY = min(cnt.y, minY); maxY = max(cnt.y, maxY);
				minZ = min(cnt.z, minZ); maxZ = max(cnt.z, maxZ);
			}
			MID += cnt;
		}
		else if (strcmp(lineHeader, "f") == 0) 
		{
			float x, y, z; fscanf(file, "%f %f %f\n", &x, &y, &z);
			Config.push_back({ (int)x,(int)y,(int)z });
		}
	}
	cout << "Take " << (double)(clock() - start)/CLOCKS_PER_SEC << "...";
	cout << "Load End\n";
	fclose(file);
	MID /= vecs.size();
	float xsub = maxX - minX, ysub = maxY - minY, zsub = maxZ - minZ;
	float sub = min(xsub, min(ysub, zsub)); 
	SCALE = vec3(1/sub);
	
	start = clock();
	deque<deque<vec3>> Norms; for (int i = 0; i < vecs.size(); i++) Norms.push_back({});
	for (auto inf : Config) 
	{
		vec3 a = (vecs[inf[0] - 1] - MID);
		vec3 b = (vecs[inf[1] - 1] - MID);
		vec3 c = (vecs[inf[2] - 1] - MID);
		vec3 n = normalize(cross(b - a, c - a));
		Norms[inf[0] - 1].push_back(n);
		Norms[inf[1] - 1].push_back(n);
		Norms[inf[2] - 1].push_back(n);
	}

	deque<vec3> Smooth;
	for (auto &inf : Norms) 
	{
		vec3 cnt = vec3(0);
		for (auto& inff : inf) cnt += inff;
		Smooth.push_back(normalize(cnt));
	}
	cout << "Take " << (double)(clock() - start)/CLOCKS_PER_SEC << "...";
	cout << "Calc Smooth End\n";
	NumVertices = Config.size() * 3;
	Vertices = new MyObjVertex[NumVertices];
	int cur = 0;
	int i = 0;
	start = clock();
	for (auto &inf : Config) 
	{
		vec3 a = (vecs[inf[0] - 1] - MID);
		vec3 b = (vecs[inf[1] - 1] - MID);
		vec3 c = (vecs[inf[2] - 1] - MID);
		vec3 n = normalize(cross(b - a, c - a));
		Vertices[cur].position = a; Vertices[cur].smooth = Smooth[inf[0] - 1]; Vertices[cur].normal = n; cur++;
		Vertices[cur].position = b;	Vertices[cur].smooth = Smooth[inf[1] - 1]; Vertices[cur].normal = n; cur++;
		Vertices[cur].position = c;	Vertices[cur].smooth = Smooth[inf[2] - 1]; Vertices[cur].normal = n; cur++;
	}
	
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(MyObjVertex)* NumVertices, Vertices, GL_STATIC_DRAW);
	bInitialized = true;
	cout << "Take " << (clock() - start)/CLOCKS_PER_SEC << "...";
	cout << "Send Data End\n";
	return vao;
}

void MyObj::SetPositionAndOtherAttributes(GLuint program)
{
	GLuint vPosition = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, sizeof(MyObjVertex), BUFFER_OFFSET(0));

	GLuint vNormal = glGetAttribLocation(program, "vNormal");
	glEnableVertexAttribArray(vNormal);
	glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, sizeof(MyObjVertex), BUFFER_OFFSET(sizeof(vec4)));

	GLuint vSmooth = glGetAttribLocation(program, "vSmooth");
	glEnableVertexAttribArray(vSmooth);
	glVertexAttribPointer(vSmooth, 3, GL_FLOAT, GL_FALSE, sizeof(MyObjVertex), BUFFER_OFFSET(sizeof(vec4) + sizeof(vec3)));
}


void MyObj::Draw(GLuint program)
{
	if(!bInitialized) return;			// check whether it is initiazed or not. 
	
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	SetPositionAndOtherAttributes(program);
	
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);
}