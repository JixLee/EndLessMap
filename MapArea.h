#ifndef _MAPAREA_H_
#define _MAPAREA_H_

#include <OgreLogManager.h>
#include <OgreEntity.h>
#include <OgreRoot.h>
#include <OgreSceneManager.h>
#include <vector>
#include <string>
#include "Common\XML\tinyxml.h"
#include "Common\MACRO.h"

enum AREAOBJECT
{
	OBSTACLE,
	SPAWN_POINT,
	POSITION,
	X,
	Z,
	SCALE_WIDTH,
	SCALE_HEIGHT,
	DIRECTION,
	ENEMY,
	FLAME_THROWER,
	OBJECT_TOTAL
};

//this stores the string I need for reading and understanding the xml
static const std::string areaObjectStr[OBJECT_TOTAL] = {
	"Obstacle",
	"SpawnPoint",
	"Position",
	"x",
	"z",
	"scaleWidth",
	"scaleHeight",
	"facing",
	"Enemy",
	"FlameThrower"
};

//data structer that holds the object data.
struct Objects{
	bool mLoaded;	//check if this object has already been loaded to the scene
	int sizeWidth;
	int sizeHeight;
	int type;		//the type of this object
	unsigned int index;
	Ogre::Vector3 mPosition;		//left top corner position.
	int direction;
};

/*
Area is the sub division of the map, a map is divided into sub areas
Each area contains it's position it's offsets, a list of objects and AIs that created in this area.

Author: Li Jixiang
*/
class MapArea
{
private:
	//this stores the area array position in the map
	int mArrayPositionX;		
	int mArrayPositionZ;

	Ogre::SceneManager* mSceneMgr;
	Ogre::Real mOffsetX;
	Ogre::Real mOffsetZ;

	Ogre::Vector3 mEndingPosition;
	Ogre::Vector3 mCenterPosition;
	Ogre::Vector3 mPosition;

	unsigned int mIndex;				//map area index
	Objects* dummy;
	Objects* dummyE;

	bool	mLoaded;

	unsigned int mObjectNumber;			//trace how many object are there in this area
	unsigned int mEnemyNumber;			//trace how many enemy arte there in this area
	unsigned int mFlameThrowerNumber;
	Ogre::Entity *mGroundEntity;		//create a Entity for the ground

	void GenerateEnemy(Objects* spawnPoint);
	void loadXMLData(const char* pFilename);

public:
	std::vector<Objects*> _mObstacleList;			//a list of obstacle in this area
	std::vector<Objects*> _mEnemySpawnPointList;				//a list of enemy spawn point in this area, one spawn = one enemy
	std::vector<Objects*> _mFlameThrowerList;

	MapArea(){};
	MapArea(Ogre::SceneManager* sceneMgr, Ogre::Vector3 position, Ogre::Real offsetX, Ogre::Real offsetZ, unsigned int index, int arrayPositionX, int arrayPositionZ);
	~MapArea(void);

	void Generate(void);
	Ogre::Vector3 getEndingPosition(void)		{ return mEndingPosition; }
	Ogre::Vector3 getCenterPosition(void)		{ return mCenterPosition; }
	Ogre::Vector3 getStartingPosition(void)		{ return mPosition; }

	unsigned int getIndex(void)					{ return mIndex; }
	bool	isLoaded(void)						{ return mLoaded; }
	void	setLoaded(bool loaded)				{ mLoaded = loaded; }

	unsigned int getTotalObjectNumber(void)		{ return mObjectNumber; }
	Ogre::Entity* getGroundEntity(void)			{ return mGroundEntity; }

	void CreateBorder(void);				//create border if this area is at the edge of the map
};
#endif
