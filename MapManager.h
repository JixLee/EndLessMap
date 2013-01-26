#ifndef _MAP_MANAGER_H_
#define _MAP_MANAGER_H_

#include <OgreLogManager.h>
#include <OgreRoot.h>
#include <OgreSceneManager.h>
#include <OgreCamera.h>

#include <vector>

#include "OgreBulletDynamicsRigidBody.h"
#include "Shapes/OgreBulletCollisionsStaticPlaneShape.h"
#include "Shapes/OgreBulletCollisionsBoxShape.h"

#include "Common/MACRO.h"
#include "Linked_List.h"
#include "MapArea.h"
#include "../Physics/PhysicManager.h"
#include "MapTree.h"
#include "Enemy/EnemyAI.h"
#include "Obstacles/FlameThrower.h"

/*
This is the map manager that handles everything in the game maps

Author: Li Jixiang
*/
class MapManager
{
private:
	int mNumEntitiesInstanced;
	Ogre::SceneManager* mSceneMgr;
	Ogre::SceneNode* mMapNode;	//main scene node for the map.
	std::vector<Map*> mMapList;	//a list of maps
	std::vector<MapArea*> mFullyLoadedAreaList;		//keep track of the list that is already been loaded in the game
	std::vector<MapArea*> mLoadingAreaList;
	std::vector<MapArea*> tempAreaList;
	std::vector<MapArea*> mSurroundingAreaList;
	std::vector<OgreBulletDynamics::RigidBody*> mRigidBodyList;

	MapTree *mMapTree;

	Ogre::SceneNode* mPlayerNode;
	std::vector<OgreBulletDynamics::RigidBody*>			mBodies;

	void init(void);
	void AddObstacle(MapArea* area, Objects* input);
	void RemoveObstacle(MapArea* area, Objects* input);
	void AddEnemy(Objects* input);
	void AddFlameThrower(Objects* input);
	void FirstLoadObjects(void);
	bool IsObjectInsideRadius(Objects* input, int radius);
	bool IsAreaInsideLoadRadius(MapArea* input);
	bool IsAreaInsideHoldRadius(MapArea* input);
	void UpdateLoadedList(std::vector<MapArea*> *input);
	void CombineVectorList(std::vector<MapArea*> *source, std::vector<MapArea*> *result);
public:
	std::vector<EnemyAI*> mEnemyList;					//actual enemy list that has been added into the scene
	std::vector<FlameThrower*>	mFlameThrowerList;

	MapManager(Ogre::SceneManager* sceneMgr, Ogre::SceneNode* playerNode);
	~MapManager(void);

	void UpdateScene(void);
	std::vector<OgreBulletDynamics::RigidBody*> getBodies(void)		{ return mBodies; }

};
#endif