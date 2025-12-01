#include "obstacle.h"
#include <iostream>

namespace game {

	Obstacle::Obstacle(const std::string name, const Resource* geometry, const Resource* material) : SceneNode(name, geometry, material) {
		//Set up geometry and material for the obstacle.
	}
	Obstacle::~Obstacle() {};

	float Obstacle::GetHealth() { return health_; }
	void Obstacle::SetHealth(int hp) { health_ += hp; }

	void Obstacle::SetStartPoint(glm::vec3 startPoint) { startPoint_ = startPoint; }
	glm::vec3 Obstacle::GetStartPoint(glm::vec3 startPoint) { return startPoint_; }

	void Obstacle::SetEndPoint(glm::vec3 endPoint) { endPoint_ = endPoint; }
	glm::vec3 Obstacle::GetEndPoint(glm::vec3 endPoint) { return endPoint_; }

	void Obstacle::Update(float deltaTime) {

		//Logic for controlling the obstacles position.
			//Interpolate position between start and end.

		// lifeTime_ += deltaTime
		lifeTime_ += deltaTime;
		if (lifeTime_ > 10.0) { lifeTime_ = 0.0; }

		// t = lifeTime_/timeLimit
		float t = lifeTime_ / 10;
		//float t = 0.5 * glm::sin(lifeTime_ - (glm::pi<float>()/2.0)) + 1;
		glm::clamp(t, 0.0f, 1.0f);

		// currentPosition = startPosition*(1-t) + endPosition*t
		//currentPosition_ = (startPoint_ * (1 - t)) + (endPoint_ * t);
		//currentPosition_ = startPoint_ + (endPoint_ - startPoint_) * t;
		currentPosition_ = glm::mix(startPoint_, endPoint_, t);

		//std::cout << "attempting to update obstacle position to: (" << currentPosition_[0] << ", " << currentPosition_[1] << ", " << currentPosition_[2] << ")\n";

		SetPosition(currentPosition_);
	}
}

/*
AABB
return
	a.max.x > b.min.x &&
	a.min.x < b.max.x &&
	a.max.y > b.min.y &&
	a.min.y < b.max.y &&
	a.max.z > b.min.z &&
	a.min.z < b.max.z


Give an AABB to the player and each obstacle.
*/