#include "cinder\app\AppBasic.h"

using namespace ci;

class DAMECamera
{
private:
	Vec3f position;
	Vec3f destination;
	float duration;
	Vec3f movePerSecond;
public:
	DAMECamera(void);
	DAMECamera(Vec3f pos);
	~DAMECamera(void);
	void update(float elapsedSeconds);
	void setPosition(Vec3f pos);
	Vec3f getPosition();
	void moveTo(Vec3f dest, float durationInSeconds);
};