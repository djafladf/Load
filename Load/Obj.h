#pragma once

#include <vgl.h>
#include <vec.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <deque>
#include<list>
using namespace std;

struct MyObjVertex
{
	vec4 position;
	vec3 normal;
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

GLuint MyObj::Init(string path)
{
	if(bInitialized == true) return vao;
	ifstream file(path);
	string line,temp;
	deque<vec3> vecs;
	deque<deque<int>> vecInfs;

	bool Init = true;
	float minX = 0, maxX = 0;
	float minY = 0, maxY = 0;
	float minZ = 0, maxZ = 0;
	while (getline(file, line)) 
	{
		if (line.size() == 0 || line.at(0) == '#') continue;
		istringstream is(line);
		string sub; is >> sub;
		if (sub == "v")
		{
			vec3 cnt;
			is >> cnt.x >> cnt.y >> cnt.z;
			if (Init) 
			{
				Init = false; 
				minX = maxX = cnt.x;
				minY = maxY = cnt.y;
				minZ = maxZ = cnt.z;
			}
			else 
			{
				minX = min(cnt.x, minX); maxX = max(cnt.x, maxX);
				minY = min(cnt.y, minY); maxY = max(cnt.y, maxY);
				minZ = min(cnt.z, minZ); maxZ = max(cnt.z, maxZ);
			} 
			vecs.push_back(cnt);
		}
		else if (sub == "f") 
		{
			int x, y, z;
			is >> x >> y >> z;
			vecInfs.push_back({x,y,z});
		}
	}
	cout << "Load End\n";
	NumVertices = vecInfs.size() * 3;
	Vertices = new MyObjVertex [NumVertices];

	int cur = 0;

	MID = vec3((maxX + minX) * 0.5, (maxY + minY) * 0.5, (maxZ + minZ) * 0.5);
	SCALE = vec3(1 / (maxX - minX), 1 / (maxY - minY), 1 / (maxZ - minZ)) * 0.5;
	MID = MID* SCALE;
	for (int i = 0; i < vecInfs.size(); i++) 
	{
		auto inf = vecInfs[i];
		vec3 a = vecs[inf[0] - 1];
		vec3 b = vecs[inf[1] - 1];
		vec3 c = vecs[inf[2] - 1];
		vec3 n = normalize(cross(b - a, c - a));
		Vertices[cur].position = a;	Vertices[cur].normal = n; cur++;
		Vertices[cur].position = b;	Vertices[cur].normal = n; cur++;
		Vertices[cur].position = c;	Vertices[cur].normal = n; cur++;
	}

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(MyObjVertex)*NumVertices, Vertices, GL_STATIC_DRAW);
	
	bInitialized = true;
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
}


void MyObj::Draw(GLuint program)
{
	if(!bInitialized) return;			// check whether it is initiazed or not. 
	
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	SetPositionAndOtherAttributes(program);
	
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);
}