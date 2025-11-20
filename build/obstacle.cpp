#include "obstacle.h"
#include <iostream>

namespace game {

	Obstacle::Obstacle(const std::string name, const Resource* geometry, const Resource* material) : SceneNode(name, geometry, material) {
		//Set up geometry and material for the obstacle.
	}
	Obstacle::~Obstacle() {};

	float Obstacle::GetHealth() { return health_; }
	void Obstacle::SetHealth(int hp) { health_ += hp; }


	void Obstacle::Update(double deltaTime) {
		//Interpolate position between start and end.
	}
}