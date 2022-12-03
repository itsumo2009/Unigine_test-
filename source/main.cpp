#include "framework/engine.h"
#include "framework/utils.h"
#include "main.h"

using namespace std;
using namespace glm;

const float railwayWidth = 0.1f;
const float railwaySleeperWidth = 0.15f;
const float railwaySleeperStep = 0.3f;

/*
* Coordinate system:
* x - right    
* y - up
* z - backward
*/
glm::vec3 lerp(const glm::vec3& first, const glm::vec3& second, float t)
{
	return first * (1 - t) + second * t;
}

std::vector<float> calculateSegmentLength(const std::vector<glm::vec3>& points)
{
	std::vector<float> result;
	for (size_t i = 0; i < points.size(); ++i)
	{
		size_t j = i + 1;
		if (j == points.size())
		{
			j = 0;
		}

		result.push_back(glm::distance(points[j], points[i]));
	}
	return result;
}

std::vector<glm::vec3> createSleeperDirections(const std::vector<glm::vec3>& directions)
{
	vector<glm::vec3> result_directions;
	for (const auto& direction : directions)
	{
		result_directions.push_back(glm::cross(direction, glm::vec3(0, 1, 0)));
	}

	return result_directions;
}

std::vector<glm::vec3> createPathDirections(const std::vector<glm::vec3>& points)
{
	vector<glm::vec3> result_directions;
	const float dt = 0.01f;

	for (size_t i = 0; i < points.size(); ++i)
	{
		size_t j = i + 1;
		if (j == points.size())
		{
			j = 0;
		}

		result_directions.push_back(glm::normalize(points[j] - points[i]));
	}
	return result_directions;
}

std::vector<glm::vec3> createSmoothPathPoints(const std::vector<Object*>& points)
{
	vector<glm::vec3> result_points;
	const float dt = 0.01f;

	for (size_t i = 0; i < points.size(); ++i)
	{
		size_t j = i + 1;
		if (j == points.size())
		{
			j = 0;
		}

		size_t k = j + 1;
		if (k == points.size())
		{
			k = 0;
		}

		glm::vec3 centerFirst = 0.5f * (points[i]->getPosition() + points[j]->getPosition());
		glm::vec3 centerSecond = 0.5f * (points[j]->getPosition() + points[k]->getPosition());

		for (float t = 0; t < 1.0f; t += dt)
		{
			auto firstPoint = lerp(centerFirst, points[j]->getPosition(), t);
			auto secondPoint = lerp(points[j]->getPosition(), centerSecond, t);
			result_points.push_back(lerp(firstPoint, secondPoint, t));
		}
	}
	return result_points;
}

Mesh createMeshPath(const vector<Object*>& points)
{
	
	vector<Vertex>       vertices;
	vector<unsigned int> indices;
	
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
	vector<Object*> points;
	for (int i = 0; i < 8; i++)
	{
		Object *sphere = engine->createObject(&sphere_mesh);
		sphere->setColor(1, 0, 0);
		sphere->setPosition(path[i*3], path[i*3+1], path[i*3+2]);
		sphere->setScale(0.25f);
		points.push_back(sphere);
	}
	LineDrawer path_drawer(path, points.size(), true);
	vector<glm::vec3> result_points = createSmoothPathPoints(points);
	vector<float> segment_length = calculateSegmentLength(result_points);
	vector<glm::vec3> result_directions = createPathDirections(result_points);
	vector<glm::vec3> sleeper_directions = createSleeperDirections(result_directions);

	LineDrawer smooth_path_driver(result_points, true);
	smooth_path_driver.setColor(1.0f, 0.5f, 0.5f);

	float current_length = 0.0f;
	for (int i = 0; i < segment_length.size(); ++i)
	{
		int j = i + 1;
		if (j == segment_length.size())
		{
			j = 0;
		}

		if (current_length + segment_length[i] > railwaySleeperStep || i == 0)
		{
			float t = i ==0 ? 0 : railwaySleeperStep - current_length / segment_length[i];
			Object* sphere = engine->createObject(&sphere_mesh);
			sphere->setColor(0, i * 1.0f / segment_length.size(), 0);
			sphere->setPosition(lerp(result_points[i], result_points[j], t) + lerp(sleeper_directions[i], sleeper_directions[j], t) * railwaySleeperWidth * 0.5f);
			sphere->setScale(0.15f);

			sphere = engine->createObject(&sphere_mesh);
			sphere->setColor(0, 0, i * 1.0f / segment_length.size());
			sphere->setPosition(lerp(result_points[i], result_points[j], t) - lerp(sleeper_directions[i], sleeper_directions[j], t) * railwaySleeperWidth * 0.5f);
			sphere->setScale(0.15f);
			current_length -= railwaySleeperStep;
		}

		current_length += segment_length[i];
	}

	// main loop
	while (!engine->isDone())
	{
		engine->update();
		engine->render();

		smooth_path_driver.draw();
		path_drawer.draw();
		
		engine->swap();
	}

	engine->shutdown();
	return 0;
}
