#ifndef _MAP_H_
#define _MAP_H_

#include <OgreLogManager.h>
#include <OgreEntity.h>
#include <OgreRoot.h>
#include <OgreSceneManager.h>

#include <vector>

#include "OgreBulletDynamicsRigidBody.h"
#include "Shapes/OgreBulletCollisionsStaticPlaneShape.h"
#include "Shapes/OgreBulletCollisionsBoxShape.h"
#include "Shapes/OgreBulletCollisionsCapsuleShape.h"
#include "MapArea.h"
#include "Common/MACRO.h"
#include "Linked_List.h"

/*
This is one map, containning everything should be in one map.
My map manager needs to access all the objects inside the class, so everything is public

Author: Li Jixiang
*/
class Map
{
private:
	unsigned int mIndex;							//the index of this map
	bool	mGroundLoaded;							//trace if the ground has been loaded
	MapArea* mMapArea[10][10];						//divide the map into 10 by 10 areas
	Ogre::Real offset;

	Ogre::SceneManager* mSceneMgr;
	Ogre::Vector3 mCenterPosition;				
	Ogre::Vector3 mStartingPosition;				//map starting position
	Ogre::Vector3 mEndingPosition;					//map ending position

	int FindCurrentArea(Ogre::Real currentPosition, bool X);
public:
	Map(Ogre::SceneManager* sceneMgr, Ogre::Vector3 centerPosition, unsigned int index);
	~Map(void);

	void Generate(void);		//generate the map
	std::vector<MapArea*> GetAllFullAffectingArea(Ogre::Vector3 currentPosition);
	
	unsigned int getIndex(void)				{ return mIndex; }
	bool		 isGroundLoaded(void)		{ return mGroundLoaded; }
	
	Ogre::Vector3 getCenterPosition(void)	{ return mCenterPosition; }
	Ogre::Vector3 getStartingPosition(void)	{ return mStartingPosition; }
	Ogre::Vector3 getEndingPosition(void)	{ return mEndingPosition; }
};
#endif