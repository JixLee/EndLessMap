#include "MapManager.h"
#include <iostream>

MapManager::MapManager(Ogre::SceneManager* sceneMgr, Ogre::SceneNode* playerNode)
{
	mSceneMgr = sceneMgr;
	//create my map node.
	mMapNode = sceneMgr->getRootSceneNode()->createChildSceneNode();
	
	mPlayerNode = playerNode;
	init();
}

MapManager::~MapManager(void)
{
	Ogre::deletePtr(mMapNode);
	Ogre::deletePtr(mPlayerNode);
	mMapList.clear();
	mFullyLoadedAreaList.clear();
	mLoadingAreaList.clear();
	tempAreaList.clear();
	mSurroundingAreaList.clear();
	mRigidBodyList.clear();
}

void MapManager::init(void)
{
	mNumEntitiesInstanced = 0;

	//lets create a floor
	Ogre::Plane plane(Ogre::Vector3::UNIT_Y, 0);
	Ogre::MeshManager::getSingleton().createPlane("floor", 
		Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
													plane,
													MAP_SIZE/10,
													MAP_SIZE/10,
													1,
													1,
													true,
													1,
													5,
													5,
													Ogre::Vector3::UNIT_Z);

	//create the map tree
	mMapTree = new MapTree(mSceneMgr);

	//find out what object is around us and add them into the node.
	FirstLoadObjects();
}

void MapManager::AddObstacle(MapArea* area, Objects* input)
{
	if(input->mLoaded)
		return;

	char areaString[30];
	sprintf(areaString, "map_area_%d", area->getIndex());

	//get the area node from map node.
	Ogre::SceneNode* areaNode = (Ogre::SceneNode*)mMapNode->getChild(Ogre::String(areaString));

	char cubeString[30];
	sprintf(cubeString, "cube_%d", input->index);
	
	Ogre::Entity* cubeEntity = mSceneMgr->createEntity(Ogre::String(cubeString),"cube.mesh");
			
	cubeEntity->setMaterialName("Cube");

	//cubeEntity->getMesh()->setAutoBuildEdgeLists(true);
	//cubeEntity->getMesh()->buildEdgeList();
	
	cubeEntity->setCastShadows(false);

	//create a node for the cube
	Ogre::SceneNode* cubeNode = areaNode->createChildSceneNode(Ogre::String(cubeString));

	//attach the cube to the node
	cubeNode->attachObject(cubeEntity);

	//move the cube to it's position
	cubeNode->setPosition(input->mPosition);
	
	//cube entity actual size = 100 by 100 by 100

	Ogre::Vector3 scale;
	scale.x = Ogre::Real(0.01f)*input->sizeWidth;
	scale.y = 0.1f;
	scale.z = Ogre::Real(0.01f)*input->sizeHeight;
	cubeNode->setScale(scale);

	input->mLoaded = true;

	Ogre::Vector3 size = cubeEntity->getBoundingBox().getSize();
	size /= 2.f;
	size *= 0.96f;
	//size rescale. refer to prac07 for full calculation
	size.x *= Ogre::Real(0.01f)*input->sizeWidth;
	size.y *= Ogre::Real(0.1f);
	size.z *= Ogre::Real(0.01f)*input->sizeHeight;

	OgreBulletCollisions::BoxCollisionShape *sceneBoxShape = new OgreBulletCollisions::BoxCollisionShape(size);
	OgreBulletDynamics::RigidBody *boxBody = new OgreBulletDynamics::RigidBody("BoxRigid" + Ogre::StringConverter::toString(input->index), 
		PhysicManager::getSingletonPtr()->getPhysicWorld(),
		PhysicManager::COL_OBSTACLE,
		PhysicManager::mObstacleCollideWith);

	boxBody->setStaticShape(cubeNode, sceneBoxShape,
		1.f,
		1.f,
		input->mPosition,
		Ogre::Quaternion(0,0,0,1));

	mRigidBodyList.push_back(boxBody);
	
	mNumEntitiesInstanced++;
}

void MapManager::RemoveObstacle(MapArea* area, Objects* input)
{
	char areaString[30];
	sprintf(areaString, "map_area_%d", area->getIndex());

	//get the area node from map node.
	Ogre::SceneNode* areaNode = (Ogre::SceneNode*)mMapNode->getChild(Ogre::String(areaString));

	char cubeString[30];
	sprintf(cubeString, "cube_%d", input->index);

	Ogre::SceneNode* objectNode = (Ogre::SceneNode*)areaNode->getChild(Ogre::String(cubeString));

	objectNode->detachAllObjects();

	mSceneMgr->destroyEntity(Ogre::String(cubeString));

	//remove the object node from areanode
	areaNode->removeAndDestroyChild(Ogre::String(cubeString));

	input->mLoaded = false;

	//we also need to remove rigid body
	
	std::vector<OgreBulletDynamics::RigidBody*>::iterator rigidBodyIterator;
	for(rigidBodyIterator = mRigidBodyList.begin(); rigidBodyIterator != mRigidBodyList.end(); rigidBodyIterator++)
	{
		//is this the rigid body for this cube?
		if((*rigidBodyIterator)->getName().compare("BoxRigid" + Ogre::StringConverter::toString(input->index)))
		{
			//yes!
			//get out of this loop
			break;
		}
	}

	//remove it from the physic manager
	PhysicManager::getSingletonPtr()->removeRigidBody((*rigidBodyIterator), BODIES_NONE);

	//remove from the list
	mRigidBodyList.erase(rigidBodyIterator);
	
	mNumEntitiesInstanced--;
}

void MapManager::AddEnemy(Objects* input)
{
	EnemyAI* enemyAI = new EnemyAI("Enemy " + Ogre::StringConverter::toString(input->index),
		input->mPosition,
		input->index,
		mSceneMgr);

	enemyAI->setSpawnPoint(input);

	input->mLoaded = true;

	mEnemyList.push_back(enemyAI);
}

void MapManager::AddFlameThrower(Objects* input)
{
	FlameThrower* flameThrower;

	switch(input->direction)
	{
	case 0://up
		flameThrower = new FlameThrower("Flame " + Ogre::StringConverter::toString(input->index), 
			input->mPosition, 
		Ogre::Vector3::NEGATIVE_UNIT_Z,
		10.f,
		5000.f,
		2000.f);
		break;
	case 1: //left
		flameThrower = new FlameThrower("Flame " + Ogre::StringConverter::toString(input->index), 
			input->mPosition, 
			Ogre::Vector3::NEGATIVE_UNIT_X,
		10.f,
		5000.f,
		2000.f);
		break;
	case 2: //down
		flameThrower = new FlameThrower("Flame " + Ogre::StringConverter::toString(input->index), 
			input->mPosition, 
			Ogre::Vector3::UNIT_Z,
		10.f,
		5000.f,
		2000.f);
		break;
	case 3: //right
		flameThrower = new FlameThrower("Flame " + Ogre::StringConverter::toString(input->index), 
			input->mPosition, 
		Ogre::Vector3::UNIT_X,
		10.f,
		5000.f,
		2000.f);
		break;
	}

	input->mLoaded = true;

	mFlameThrowerList.push_back(flameThrower);
	
}
/*
Find out if the input object is inside the load raidus
*/
bool MapManager::IsObjectInsideRadius(Objects* input, int radius)
{
	//in case input is invalid, return
	if (!input)
	{
		return false;
	}

	//use object's 4 boundaries to check with the player position, see if it is inside the radius or not.
	if((Ogre::Math::Abs(input->mPosition.x - mPlayerNode->getPosition().x) < radius && Ogre::Math::Abs(input->mPosition.z - mPlayerNode->getPosition().z) < radius)
		|| (Ogre::Math::Abs(input->mPosition.x + input->sizeWidth - mPlayerNode->getPosition().x) < radius && Ogre::Math::Abs(input->mPosition.z  + input->sizeHeight - mPlayerNode->getPosition().z) < radius)
		|| (Ogre::Math::Abs(input->mPosition.x + input->sizeWidth - mPlayerNode->getPosition().x) < radius && Ogre::Math::Abs(input->mPosition.z - mPlayerNode->getPosition().z) < radius)
		|| (Ogre::Math::Abs(input->mPosition.x - mPlayerNode->getPosition().x) < radius && Ogre::Math::Abs(input->mPosition.z - mPlayerNode->getPosition().z  + input->sizeHeight) < radius))
		return true;
	
	return false;
}

//find out if the input area is inside the load radius
bool MapManager::IsAreaInsideLoadRadius(MapArea* input)
{
	if((Ogre::Math::Abs(input->getStartingPosition().x - mPlayerNode->getPosition().x) < LOADRADIUS && Ogre::Math::Abs(input->getStartingPosition().z - mPlayerNode->getPosition().z) < LOADRADIUS)
		|| (Ogre::Math::Abs(input->getEndingPosition().x - mPlayerNode->getPosition().x) < LOADRADIUS && Ogre::Math::Abs(input->getStartingPosition().z - mPlayerNode->getPosition().z) < LOADRADIUS)
		|| (Ogre::Math::Abs(input->getStartingPosition().x - mPlayerNode->getPosition().x) < LOADRADIUS && Ogre::Math::Abs(input->getEndingPosition().z - mPlayerNode->getPosition().z) < LOADRADIUS)
		|| (Ogre::Math::Abs(input->getEndingPosition().x - mPlayerNode->getPosition().x) < LOADRADIUS && Ogre::Math::Abs(input->getEndingPosition().z - mPlayerNode->getPosition().z) < LOADRADIUS))
			return true;
	return false;
}

bool MapManager::IsAreaInsideHoldRadius(MapArea* input)
{
	if((Ogre::Math::Abs(input->getStartingPosition().x - mPlayerNode->getPosition().x) < HOLDRADIUS && Ogre::Math::Abs(input->getStartingPosition().z - mPlayerNode->getPosition().z) < HOLDRADIUS)
		|| (Ogre::Math::Abs(input->getEndingPosition().x - mPlayerNode->getPosition().x) < HOLDRADIUS && Ogre::Math::Abs(input->getStartingPosition().z - mPlayerNode->getPosition().z) < HOLDRADIUS)
		|| (Ogre::Math::Abs(input->getStartingPosition().x - mPlayerNode->getPosition().x) < HOLDRADIUS && Ogre::Math::Abs(input->getEndingPosition().z - mPlayerNode->getPosition().z) < HOLDRADIUS)
		|| (Ogre::Math::Abs(input->getEndingPosition().x - mPlayerNode->getPosition().x) < HOLDRADIUS && Ogre::Math::Abs(input->getEndingPosition().z - mPlayerNode->getPosition().z) < HOLDRADIUS))
			return true;
	
	return false;
}
/*
First time load the objects, only run once when we first create the map.
It will load all the objects that is around the player position.
Let's just assume we start at the center of a map.

If load game is to be implemented, TODO check surrounding maps using current position
*/
void MapManager::FirstLoadObjects(void)
{
	//first tell the map where we are and ask for the areas that is possibily affected

	//then we take the list of areas, add all the objects into our scene
	mFullyLoadedAreaList = mMapTree->getCurrentMap()->GetAllFullAffectingArea(mPlayerNode->getPosition());

	//add the objects in the list into the scene.
	std::vector<MapArea*>::iterator loadedIterator;
	for(loadedIterator = mFullyLoadedAreaList.begin(); loadedIterator != mFullyLoadedAreaList.end(); loadedIterator++)
	{
		char areaString[20];
		sprintf(areaString, "map_area_%d", (*loadedIterator)->getIndex());

		//create a node for every area
		Ogre::SceneNode* areaNode = mMapNode->createChildSceneNode(Ogre::String(areaString));

		(*loadedIterator)->setLoaded(true);

		//add the ground into the node.
		char groundString[30];
		sprintf(groundString, "map_area_ground_%d", (*loadedIterator)->getIndex());
		Ogre::SceneNode* groundNode = areaNode->createChildSceneNode(groundString);
		groundNode->attachObject((*loadedIterator)->getGroundEntity());
		groundNode->setPosition((*loadedIterator)->getCenterPosition());

		//run through the obstacle list
		std::vector<Objects*>::iterator obstacleIterator;
		for(obstacleIterator = (*loadedIterator)->_mObstacleList.begin(); obstacleIterator!=(*loadedIterator)->_mObstacleList.end(); obstacleIterator++)
		{
			//no
			//is this object inside the load radius?
			if(!IsObjectInsideRadius((*obstacleIterator), LOADRADIUS))
			{
				//no
				//skip this object
				continue;
			}
			//yes continue whatever we need to do
	
			AddObstacle((*loadedIterator), (*obstacleIterator));
		}

		std::vector<Objects*>::iterator enemyIterator;
		for(enemyIterator = (*loadedIterator)->_mEnemySpawnPointList.begin(); enemyIterator!=(*loadedIterator)->_mEnemySpawnPointList.end(); enemyIterator++)
		{
			//no
			//is this object inside the load radius?
			if(!IsObjectInsideRadius((*enemyIterator), ENEMY_LOADRADIUS))
			{
				//no
				//skip this object
				continue;
			}
			//yes continue whatever we need to do
	
			AddEnemy((*enemyIterator));
		}

		std::vector<Objects*>::iterator flamethrowerIterator;
		for(flamethrowerIterator = (*loadedIterator)->_mFlameThrowerList.begin(); flamethrowerIterator!=(*loadedIterator)->_mFlameThrowerList.end(); flamethrowerIterator++)
		{
			//no
			//is this object inside the load radius?
			if(!IsObjectInsideRadius((*flamethrowerIterator), ENEMY_LOADRADIUS))
			{
				//no
				//skip this object
				continue;
			}
			//yes continue whatever we need to do
	
			AddFlameThrower((*flamethrowerIterator));
		}
	}
}

//update that checks the loaded list for adding or removing objects inside
void MapManager::UpdateLoadedList(std::vector<MapArea*> *input)
{
	//run through our current list
	//see if there is anything in the list to be updated
	std::vector<MapArea*>::iterator loadedIterator;
	for(loadedIterator = input->begin(); loadedIterator != input->end(); loadedIterator++)
	{
		//No matter if the area is fully inside or not, let's check if the area is now outside of the HOLDRADIUS
		if(!IsAreaInsideLoadRadius((*loadedIterator)))
		{
			if(!IsAreaInsideHoldRadius((*loadedIterator)))
			{
				//now, time to remove the area node
				//std::cout << "REMOVING AREA NODE!\n";
				char areaString[30];
				sprintf(areaString, "map_area_%d", (*loadedIterator)->getIndex());

				//get the area node from map node.
				Ogre::SceneNode* areaNode = (Ogre::SceneNode*)mMapNode->getChild(Ogre::String(areaString));

				//delete ground node
				char groundString[30];
				sprintf(groundString, "map_area_ground_%d", (*loadedIterator)->getIndex());

				Ogre::SceneNode* groundNode = (Ogre::SceneNode*)areaNode->getChild(Ogre::String(groundString));

				groundNode->removeAndDestroyAllChildren();
				groundNode->detachAllObjects();

				areaNode->removeAndDestroyChild(groundString);

				areaNode->removeAndDestroyAllChildren();
				areaNode->detachAllObjects();
				
				mMapNode->removeAndDestroyChild(areaString);

				//set the map area to be unloaded
				(*loadedIterator)->setLoaded(false);

				//remove it from the list
				input->erase(loadedIterator);
				return;
			}
		}

		//since this area is not fully inside, lets check if this object is currently inside the scene
		std::vector<Objects*>::iterator obstacleIterator;
		for(obstacleIterator = (*loadedIterator)->_mObstacleList.begin(); obstacleIterator!=(*loadedIterator)->_mObstacleList.end(); obstacleIterator++)
		{
			//is this object in the load radius?
			if(IsObjectInsideRadius((*obstacleIterator), LOADRADIUS))
			{
				//yes
				//is this object already loaded in the game?
				if((*obstacleIterator)->mLoaded)
				{
					//yes
					//skip
					continue;
				}
				else
				{
					//no
					//add it into the game
					AddObstacle((*loadedIterator), (*obstacleIterator));
				}
			}
			//is this object out of hold radius?
			else if(!IsObjectInsideRadius((*obstacleIterator), HOLDRADIUS))
			{
				//no
				//is this object already loaded in the game?
				if((*obstacleIterator)->mLoaded)
				{
					//yes
					//remove it
					RemoveObstacle((*loadedIterator),(*obstacleIterator));
				}
				else
				{
					//no
					//skip
					continue;
				}
			}
		}

		std::vector<Objects*>::iterator enemyIterator;
		for(enemyIterator = (*loadedIterator)->_mEnemySpawnPointList.begin(); enemyIterator!=(*loadedIterator)->_mEnemySpawnPointList.end(); enemyIterator++)
		{
			//is this object inside the load radius?
			if(IsObjectInsideRadius((*enemyIterator), ENEMY_LOADRADIUS))
			{
				//yes
				//is this enemy spawn point already loaded?
				if(!(*enemyIterator)->mLoaded)
				{
					//no
					//add enemy
					AddEnemy((*enemyIterator));
				}
			}
		}

		std::vector<Objects*>::iterator flamethrowerIterator;
		for(flamethrowerIterator = (*loadedIterator)->_mFlameThrowerList.begin(); flamethrowerIterator!=(*loadedIterator)->_mFlameThrowerList.end(); flamethrowerIterator++)
		{
			//no
			//is this object inside the load radius?
			if(!IsObjectInsideRadius((*flamethrowerIterator), ENEMY_LOADRADIUS))
			{
				//no
				//skip this object
				continue;
			}
			//yes continue whatever we need to do

			if(!(*flamethrowerIterator)->mLoaded)
				AddFlameThrower((*flamethrowerIterator));
		}
	}
}

//combine two vector together, taking source and putting it into result
void MapManager::CombineVectorList(std::vector<MapArea*> *source, std::vector<MapArea*> *result)
{
	std::vector<MapArea*>::iterator sourceIterator;
	for(sourceIterator = source->begin(); sourceIterator != source->end(); sourceIterator++)
	{
		result->push_back((*sourceIterator));
	}
}
/*
updateScene checks all the object that is inside the scene and updates the scene manager accordingly.
The idea behind it is to use spatial partition to manage scene nodes and objects in the game.
Load the objects when they come into load radius, unload them when they get out of the hold radius.
*/
void MapManager::UpdateScene(void)
{
	//first of all update our loaded list
	UpdateLoadedList(&mFullyLoadedAreaList);

	//go through the enemy list, see if we need to remove any it from the scene
	std::vector<EnemyAI*>::iterator enemyIterator;
	for(enemyIterator = mEnemyList.begin(); enemyIterator != mEnemyList.end(); enemyIterator++)
	{
		//check if this enemy position is too far from our player
		Ogre::Real displacement = (*enemyIterator)->mEnemy->getPosition().distance(mPlayerNode->getPosition());
		if(displacement > ENEMY_HOLDRADIUS)
		{
			//the enemy is outside hold radius
			//remove it
			if((*enemyIterator)->mSpells.empty() == false)
			{
				unsigned int i = 0;
				while(i < (*enemyIterator)->mSpells.size())
				{
					//spell died
					BaseSpell *PointSpell = (*enemyIterator)->mSpells[i];
					PointSpell->kill(Ogre::Root::getSingletonPtr()->getSceneManager("JixLeePuff"));
					(*enemyIterator)->mSpells.erase((*enemyIterator)->mSpells.begin() + i);
					delete PointSpell;

					i++;
				}
			}
			(*enemyIterator)->mEnemy->kill();
			mEnemyList.erase(enemyIterator);
			break;
		}
	}

	//then check if map are still at the safe zone of the map.
	//meaning we do not worry about moving into a new map which requires a lot of things to do

	//has our character cross over to another map?
	//North
	if(mPlayerNode->getPosition().z < mMapTree->getCurrentMap()->getStartingPosition().z)
	{
		mMapTree->traverse(NORTH);
	}
	//South
	if(mPlayerNode->getPosition().z > mMapTree->getCurrentMap()->getEndingPosition().z)
	{
		mMapTree->traverse(SOUTH);
	}
	//East
	if(mPlayerNode->getPosition().x > mMapTree->getCurrentMap()->getEndingPosition().x)
	{
		mMapTree->traverse(EAST);
	}
	//West
	if(mPlayerNode->getPosition().x < mMapTree->getCurrentMap()->getStartingPosition().x)
	{
		mMapTree->traverse(WEST);
	}

	//clear surrounding area list if it is not empty
	if(!mSurroundingAreaList.empty())
		mSurroundingAreaList.clear();

	//first check to see if the player's load radius has reach beyond the borders of the current map
	//check one by one

	//North
	if((static_cast<int>(mPlayerNode->getPosition().z) - LOADRADIUS) < mMapTree->getCurrentMap()->getStartingPosition().z)
	{
		//yes!
		//get the list of areas been affected in that map
		Map* temp = mMapTree->getDirectionedMap(NORTH);
		tempAreaList = temp->GetAllFullAffectingArea(mPlayerNode->getPosition());
		//combine the temp area list into surrounding area list
		CombineVectorList(&tempAreaList, &mSurroundingAreaList);
	}
	//South
	if((static_cast<int>(mPlayerNode->getPosition().z) + LOADRADIUS) > mMapTree->getCurrentMap()->getEndingPosition().z)
	{
		//yes!
		Map* temp = mMapTree->getDirectionedMap(SOUTH);
		tempAreaList = temp->GetAllFullAffectingArea(mPlayerNode->getPosition());
		CombineVectorList(&tempAreaList, &mSurroundingAreaList);
	}
	//West
	if((static_cast<int>(mPlayerNode->getPosition().x) - LOADRADIUS) < mMapTree->getCurrentMap()->getStartingPosition().x)
	{
		//yes!
		Map* temp = mMapTree->getDirectionedMap(WEST);
		tempAreaList = temp->GetAllFullAffectingArea(mPlayerNode->getPosition());
		CombineVectorList(&tempAreaList, &mSurroundingAreaList);
	}
	//East
	if((static_cast<int>(mPlayerNode->getPosition().x) + LOADRADIUS) > mMapTree->getCurrentMap()->getEndingPosition().x)
	{
		//yes!
		Map* temp = mMapTree->getDirectionedMap(EAST);
		tempAreaList = temp->GetAllFullAffectingArea(mPlayerNode->getPosition());
		CombineVectorList(&tempAreaList, &mSurroundingAreaList);
	}
	
	//second check if we need to add or remove our objects in the scene

	//check if there are new areas come into load radius
	//get a new list
	//update the loading list
	mLoadingAreaList = mMapTree->getCurrentMap()->GetAllFullAffectingArea(mPlayerNode->getPosition());

	//combine the surrounding area list into loading area list
	CombineVectorList(&mSurroundingAreaList, &mLoadingAreaList);

	//check if the list is empty
	if(!mLoadingAreaList.empty())
	{
		//no we receive a new list of areas
		//first add those objects inside
		std::vector<MapArea*>::iterator newAreaIterator;

		for(newAreaIterator=mLoadingAreaList.begin(); newAreaIterator!=mLoadingAreaList.end(); newAreaIterator++)
		{
			//just incase this area is actually already loaded
			if((*newAreaIterator)->isLoaded())
				continue;

			//create a scene node for the area
			char areaString[20];
			sprintf(areaString, "map_area_%d", (*newAreaIterator)->getIndex());

			//create a node for every area
			Ogre::SceneNode* areaNode = mMapNode->createChildSceneNode(Ogre::String(areaString));

			(*newAreaIterator)->setLoaded(true);

			//add the ground into the node.
			char groundString[30];
			sprintf(groundString, "map_area_ground_%d", (*newAreaIterator)->getIndex());
			Ogre::SceneNode* groundNode = areaNode->createChildSceneNode(groundString);
			groundNode->attachObject((*newAreaIterator)->getGroundEntity());
			groundNode->setPosition((*newAreaIterator)->getCenterPosition());

			//now we need to go through the objects to find out which object has to be removed from it
			std::vector<Objects*>::iterator obstacleIterator;
			for(obstacleIterator = (*newAreaIterator)->_mObstacleList.begin(); obstacleIterator!=(*newAreaIterator)->_mObstacleList.end(); obstacleIterator++)
			{
				//is this object inside the load radius?
				if(IsObjectInsideRadius((*obstacleIterator), LOADRADIUS))
				{
					AddObstacle((*newAreaIterator), (*obstacleIterator));
				}
			}

			std::vector<Objects*>::iterator enemyIterator;
			for(enemyIterator = (*newAreaIterator)->_mEnemySpawnPointList.begin(); enemyIterator!=(*newAreaIterator)->_mEnemySpawnPointList.end(); enemyIterator++)
			{
				//no
				//is this object inside the load radius?
				if(IsObjectInsideRadius((*enemyIterator), ENEMY_LOADRADIUS))
				{
					AddEnemy((*enemyIterator));
				}
			}

			std::vector<Objects*>::iterator flamethrowerIterator;
			for(flamethrowerIterator = (*newAreaIterator)->_mFlameThrowerList.begin(); flamethrowerIterator!=(*newAreaIterator)->_mFlameThrowerList.end(); flamethrowerIterator++)
			{
				//no
				//is this object inside the load radius?
				if(IsObjectInsideRadius((*flamethrowerIterator), ENEMY_LOADRADIUS))
				{
					AddFlameThrower((*flamethrowerIterator));
				}
			}

			//then we add the areas inside the list to our fully loaded list
			mFullyLoadedAreaList.push_back((*newAreaIterator));
		}
		
	}
}