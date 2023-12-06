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
	deque<string> res;
	deque<vec3> vecs;
	list<deque<int>> vecInfs;

	float minX = 1000, maxX = -1000;
	float minY = 1000, maxY = -1000;
	float minZ = 1000, maxZ = -1000;
	vec3 MID = vec3(0, 0, 0);
	while (getline(file, line)) 
	{
		if (line == "") continue;
		// read Line
		res.clear();
		stringstream ss(line);
		while (getline(ss, temp, ' ')) res.push_back(temp);
		string first = res.front(); res.pop_front();
		// Decode
		if (first == "v") for (int i = 0; i < res.size(); i += 3) 
		{
			float x = stof(res[i]); minX = min(x, minX); maxX = max(x, maxX);
			float y = stof(res[i + 1]); minY = min(y, minY); maxY = max(y, maxY);
			float z = stof(res[i + 2]);	minZ = min(z, minZ); maxZ = max(z, maxZ);
			MID += vec3(x,y,z);
			vecs.push_back(vec3(x,y,z));
		}
		else if (first == "f") for (int i = 0; i < res.size(); i += 3) 
		{
			vecInfs.push_back({ stoi(res[i]), stoi(res[i + 1]), stoi(res[i + 2]) });
		}
	}
	cout << "Load End" << endl;
	NumVertices = vecInfs.size() * 3;
	Vertices = new MyObjVertex [NumVertices];

	int cur = 0;
	auto it = vecInfs.begin();

	int i = 0;
	MID = MID / vecs.size();
	vec3 MIN = vec3(minX, minY, minZ);
	vec3 MAX = vec3(maxX, maxY, maxZ) - MIN; MAX = vec3(1 / MAX[0], 1 / MAX[1], 1 / MAX[2]);
	MID = (MID - MIN) * MAX;
	while (it != vecInfs.end()) 
	{
		auto inf = *it;
		vec3 a = (vecs[inf[0] - 1] - MIN) * MAX - MID;
		vec3 b = (vecs[inf[1] - 1] - MIN) * MAX - MID;
		vec3 c = (vecs[inf[2] - 1] - MIN) * MAX - MID;
		vec3 n = normalize(cross(b - a, c - a));
		Vertices[cur].position = a;	Vertices[cur].normal = n; cur++;
		Vertices[cur].position = b;	Vertices[cur].normal = n; cur++;
		Vertices[cur].position = c;	Vertices[cur].normal = n; cur++;
		it++;
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