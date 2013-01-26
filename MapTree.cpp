#include "MapTree.h"
#include <iostream>

MapTree::MapTree(Ogre::SceneManager* sceneMgr)
:mDirection(NORTH)
{
	mSceneMgr = sceneMgr;

	currentMap = new MapNode();

	mMapIndex = MAP_INDEX_MULTIPLIER;

	//create the free with a starting map.
	currentMap->m_Map = new Map(mSceneMgr, Ogre::Vector3(0,0,0), mMapIndex);

	//increment the map index
	mMapIndex += MAP_INDEX_MULTIPLIER;

	//set the surrounding node to none
	currentMap->m_NorthNode = NULL;
	currentMap->m_SouthNode = NULL;
	currentMap->m_EastNode = NULL;
	currentMap->m_WestNode = NULL;
}

MapTree::~MapTree(void)
{
}

void MapTree::createMap(int direction)
{
	//a new map node
	MapNode* temp = new MapNode();
	
	//point the new map node's mapnode pointer to the current node
	//the position of the new mapnode is based on the direction, with direction
	//we know where is this new map center located.
	Ogre::Vector3 newMapCenter = currentMap->m_Map->getCenterPosition();

	switch(mDirection)
	{
	case NORTH: newMapCenter.z -= MAP_SIZE;
		temp->m_SouthNode = currentMap;
		currentMap->m_NorthNode = temp;
			break;
	case SOUTH: newMapCenter.z += MAP_SIZE; 
		temp->m_NorthNode = currentMap;
		currentMap->m_SouthNode = temp;
			break;
	case EAST:  newMapCenter.x += MAP_SIZE;
		temp->m_WestNode = currentMap;
		currentMap->m_EastNode = temp;
			break;
	case WEST: newMapCenter.x -= MAP_SIZE;
		temp->m_EastNode = currentMap;
		currentMap->m_WestNode = temp;
			break;
	}

	//create a new map in this map node
	temp->m_Map = new Map(mSceneMgr, newMapCenter, mMapIndex);

	//increment the map index
	mMapIndex += MAP_INDEX_MULTIPLIER;

	//point the next map node to this new map node
	nextMap = temp;
}

void MapTree::traverse(int direction)
{	
	switch(direction)
	{
	case NORTH: nextMap = currentMap->m_NorthNode;
			break;
	case SOUTH: nextMap = currentMap->m_SouthNode;
			break;
	case EAST: nextMap = currentMap->m_EastNode;
			break;
	case WEST: nextMap = currentMap->m_WestNode;
			break;
	}

	//now we know the next map exists, traverse to it
	currentMap = nextMap;
}

/*
Get the map in the direciton it wanted.
If the map doesn't exist, create the map before returning.
*/
Map* MapTree::getDirectionedMap(int direction)
{
	mDirection = direction;
	switch(direction)
	{
	case NORTH: nextMap = currentMap->m_NorthNode;
			break;
	case SOUTH: nextMap = currentMap->m_SouthNode;
			break;
	case EAST: nextMap = currentMap->m_EastNode;
			break;
	case WEST: nextMap = currentMap->m_WestNode;
			break;
	}

	if(nextMap==NULL)
	{
		//map doesn't exist
		//create the map
		createMap(direction);
	}

	//return it after creating the map
	return nextMap->m_Map;
}