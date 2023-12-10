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

	MyObjVertex* Vertices;
	GLuint Init(string path);
	void SetPositionAndOtherAttributes(GLuint program);

	GLuint vao;
	GLuint buffer;
	vec3 SCALE;
	bool bInitialized;
	void Draw(GLuint program);
private:
};



MyObj::MyObj(void)
{
	bInitialized = false;
	NumVertices = 0;
	Vertices = NULL;
}

MyObj::~MyObj(void)
{
	if (Vertices != NULL)
		delete[] Vertices;
}

#include <time.h>
#include <thread>
#include <chrono>
using std::thread;

deque<vec3> vecs;
deque<deque<int>> Config;
deque<deque<vec3>> Norms;
deque<vec3> Smooth;
bool* Locks;
vec3 MID;

void CalcNorms(int st, int ed)
{
	for (int i = st; i < ed; i++)
	{
		if (i >= Config.size()) break;
		auto& inf = Config[i];
		int x = inf[0] - 1, y = inf[1] - 1, z = inf[2] - 1;
		vec3 a = (vecs[x] - MID);
		vec3 b = (vecs[y] - MID);
		vec3 c = (vecs[z] - MID);
		vec3 n = normalize(cross(b - a, c - a));

		if (!Locks[x])
		{
			Locks[x] = true;
			Norms[x].push_back(n);
			Locks[x] = false;
		}
		else
		{
			while (Locks[x]) this_thread::sleep_for(chrono::milliseconds(1));
			Locks[x] = true;
			Norms[x].push_back(n);
			Locks[x] = false;
		}

		if (!Locks[y])
		{
			Locks[y] = true;
			Norms[y].push_back(n);
			Locks[y] = false;
		}
		else
		{
			while (Locks[y]) this_thread::sleep_for(chrono::milliseconds(1));
			Locks[y] = true;
			Norms[y].push_back(n);
			Locks[y] = false;
		}

		if (!Locks[z])
		{
			Locks[z] = true;
			Norms[z].push_back(n);
			Locks[z] = false;
		}
		else
		{
			while (Locks[z]) this_thread::sleep_for(chrono::milliseconds(1));
			Locks[z] = true;
			Norms[z].push_back(n);
			Locks[z] = false;
		}
	}
}
void CalcSmooths(int st, int ed)
{
	for (int i = st; i < ed; i++)
	{
		if (i >= Norms.size()) break;
		auto& inf = Norms[i];
		vec3 cnt = vec3(0);
		for (auto& inff : inf) cnt += inff;
		Smooth[i] = normalize(cnt);
	}
}
GLuint MyObj::Init(string path)
{
	time_t start, end;
	if (bInitialized == true) return vao;
	int threadNum = 10;

	FILE* file = fopen(path.c_str(), "r");
	if (file == NULL) return  vao;
	start = clock();

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
	std::cout << "Take " << (double)(clock() - start) / CLOCKS_PER_SEC << "...";
	std::cout << "Load End\n";
	fclose(file);
	MID /= vecs.size();
	float xsub = maxX - minX, ysub = maxY - minY, zsub = maxZ - minZ;
	float sub = min(xsub, min(ysub, zsub));
	SCALE = vec3(1 / sub);
	Locks = new bool[vecs.size()];
	for (int i = 0; i < vecs.size();i++) Locks[i] = false;
	Norms.clear(); Smooth.clear();
	start = clock();
	for (int i = 0; i < vecs.size(); i++) Norms.push_back({});

	int delta = Config.size() / threadNum;
	thread* tr = new thread[threadNum + 1];
	for (int i = 0; i < threadNum + 1; i++)
	{
		tr[i] = thread(CalcNorms, i * delta, (i + 1) * delta);
	}

	for (int i = 0; i < threadNum + 1;i++)
	{
		tr[i].join();
	}
	delete[] tr;
	delta = Norms.size() / threadNum;

	thread* tr2 = new thread[threadNum + 1];
	for (int i = 0; i < Config.size(); i++) Smooth.push_back(0);
	for (int i = 0; i < threadNum + 1; i++)
	{
		tr2[i] = thread(CalcSmooths, i * delta, (i + 1) * delta);
	}
	for (int i = 0; i < threadNum + 1;i++)
	{
		tr2[i].join();
	}
	delete[] tr2;
	std::cout << "Take " << (double)(clock() - start) / CLOCKS_PER_SEC << "...";
	std::cout << "Calc Smooth End\n";
	NumVertices = Config.size() * 3;
	Vertices = new MyObjVertex[NumVertices];
	int cur = 0;
	int i = 0;
	start = clock();
	for (auto& inf : Config)
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
	glBufferData(GL_ARRAY_BUFFER, sizeof(MyObjVertex) * NumVertices, Vertices, GL_STATIC_DRAW);
	bInitialized = true;
	std::cout << "Take " << (clock() - start) / CLOCKS_PER_SEC << "...";
	std::cout << "Send Data End\n";
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
	if (!bInitialized) return;			// check whether it is initiazed or not. 

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	SetPositionAndOtherAttributes(program);

	glDrawArrays(GL_TRIANGLES, 0, NumVertices);
}