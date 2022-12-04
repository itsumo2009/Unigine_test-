#include "framework/engine.h"
#include "framework/utils.h"

#include "Path.h"

using namespace std;
using namespace glm;

const float railwayWidth = 0.1f;
const float railWidth = 0.0125f / 2;
const float railwaySleeperWidth = 0.15f;
const float railwaySleeperStep = 0.3f;
const float trainInterval = 0.2f;
const int trainLength = 6;

enum Inn
{
	Inner,
	Outer
};

/*
* Coordinate system:
* x - right    
* y - up
* z - backward
*/

Mesh createMeshPath(const Path& path, Inn inn)
{
	Path::IntervalIterator it(path, railWidth);

	vector<Vertex>       vertices;
	vector<unsigned int> indices;

	for (int i = 0; i < path.length() / railWidth; ++i)
	{
		glm::vec3 pos = it.position;
		Vertex v;
		if (inn == Inner)
		{
			v.position = pos - it.normal * (railwayWidth * 0.5f + railWidth);
			vertices.push_back(v);
			v.position = pos - it.normal * railwayWidth * 0.5f;
			vertices.push_back(v);
		}
		else
		{
			v.position = pos + it.normal * railwayWidth * 0.5f;
			vertices.push_back(v);
			v.position = pos + it.normal * (railwayWidth * 0.5f + railWidth);
			vertices.push_back(v);
		}

		it.advance();
	}
	
	for (int i = 0; i < vertices.size(); i += 2)
	{
		int j = (i + 1) % vertices.size();
		int k = (i + 2) % vertices.size();
		int q = (i + 3) % vertices.size();
		indices.push_back(j);
		indices.push_back(i);
		indices.push_back(k);

		indices.push_back(q);
		indices.push_back(j);
		indices.push_back(k);
	}
	return Mesh(vertices, indices);
}

int main()
{
	// initialization
	Engine *engine = Engine::get();
	engine->init(1600, 900, "UNIGINE Test Task");

	// set up camera
	Camera &cam = engine->getCamera();
	cam.Position = vec3(0.0f, 12.0f, 17.0f);
	cam.Yaw = -90.0f;
	cam.Pitch = -45.0f;
	cam.UpdateCameraVectors();

	// create shared meshes
	Mesh plane_mesh = createPlane();
	Mesh sphere_mesh = createSphere();
	Mesh cube_mesh = createCube();

	// create background objects
	Object *plane = engine->createObject(&plane_mesh);
	plane->setColor(0.2f, 0.37f, 0.2f); // green
	plane->setPosition(0, -0.5f, 0);
	plane->setRotation(-90.0f, 0.0f, 0.0f);
	plane->setScale(20.0f);

	// path
	const float path[] = {
		 0.0f, -0.375f,  7.0f, // 1
		-6.0f, -0.375f,  5.0f, // 2
		-8.0f, -0.375f,  1.0f, // 3
		-4.0f, -0.375f, -6.0f, // 4
		 0.0f, -0.375f, -7.0f, // 5
		 1.0f, -0.375f, -4.0f, // 6
		 4.0f, -0.375f, -3.0f, // 7
		 8.0f, -0.375f,  7.0f  // 8
	};

	vector<Object*> train;
	for (int i = 0; i < trainLength; ++i)
	{
		Object* cube = engine->createObject(&cube_mesh);
		cube->setColor(0.75, 0.75, 0.75);
		cube->setScale(railwaySleeperWidth);
		train.push_back(cube);
	}

	vector<vec3> positions;
	for (int i = 0; i < 8; i++)
	{
		positions.push_back(glm::vec3(path[i * 3], path[i * 3 + 1], path[i * 3 + 2]));
	}

	Path smoothPath(positions);

	Mesh railwayMeshInner(createMeshPath(smoothPath, Inn::Inner));
	Object* railwayInner = engine->createObject(&railwayMeshInner);
	railwayInner->setColor(0.75, 0.75, 0.75);
	railwayInner->setPosition(0, 0.0005f, 0);

	Mesh railwayMeshOuter(createMeshPath(smoothPath, Inn::Outer));
	Object* railwayOuter = engine->createObject(&railwayMeshOuter);
	railwayOuter->setColor(0.75, 0.75, 0.75);
	railwayOuter->setPosition(0, 0.0005f, 0);

	std::vector<Path::VelocityIterator> its;
	for (int i = 0; i < trainLength; ++i)
	{
		its.emplace_back(smoothPath, 1.2f, -i * trainInterval);
	}

	Path::IntervalIterator intervalIterator(smoothPath, railwaySleeperStep);
	for (int i = 0; i < smoothPath.length() / railwaySleeperStep; ++i)
	{
		Object* sleeper = engine->createObject(&cube_mesh);
		sleeper->setColor(185 / 255.0f, 122 / 255.0f, 87 / 255.0f);
		glm::vec3 pos = intervalIterator.position;
		pos.y -= 0.0025f;
		sleeper->setPosition(pos);
		sleeper->setRotation(0, glm::atan(intervalIterator.direction.x, intervalIterator.direction.z) * 180 / glm::pi<float>(), 0);
		sleeper->setScale(railwaySleeperWidth, 0.005f, 0.005f);
		intervalIterator.advance();
	}

	// main loop
	while (!engine->isDone())
	{
		for (int i = 0; i < 6; ++i)
		{
			auto& it = its[i];
			it.advance(engine->getDeltaTime());
			glm::vec3 pos = it.position;
			pos.y += railwaySleeperWidth / 2;
			train[i]->setPosition(pos);
			train[i]->setRotation(0, glm::atan(it.direction.x, it.direction.z) * 180 / glm::pi<float>(), 0);
		}
		
		engine->update();
		engine->render();
		engine->swap();
	}

	engine->shutdown();
	return 0;
}
