#include "cinder\app\AppBasic.h"
#include "DAMECamera.h"

using namespace ci;

DAMECamera::DAMECamera(void){
	position = Vec3f(0,0,0);
	movePerSecond = Vec3f(0,0,0);
}

DAMECamera::DAMECamera(Vec3f pos){
	position = pos;
	movePerSecond = Vec3f(0,0,0);
}

DAMECamera::~DAMECamera(void){}

void DAMECamera::update(float elapsedSeconds){
	if(movePerSecond.length() > 0){
		Vec3f distanceToDestination = destination - position;
		if(distanceToDestination.length() > movePerSecond.length()*elapsedSeconds){
			position += movePerSecond*elapsedSeconds;
		} else {
			position = destination;
			movePerSecond = Vec3f(0,0,0);
		}
	}
}

void DAMECamera::setPosition(Vec3f pos){
	position = pos;
}

Vec3f DAMECamera::getPosition(){
	return position;
}

void DAMECamera::moveTo(Vec3f dest, float durationInSeconds){
	movePerSecond = (dest - position) / durationInSeconds;
	destination = dest;
}