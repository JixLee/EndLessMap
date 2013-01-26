#ifndef _MAP_TREE_H_
#define _MAP_TREE_H_

#include "Map.h"
#include "Common/MACRO.h"

struct MapNode
{
	Map* m_Map;			//current map
	MapNode* m_NorthNode;	//map at the north of this node
	MapNode* m_SouthNode;	//map at the south of this node
	MapNode* m_WestNode;	//map at the west of this node
	MapNode* m_EastNode;	//map at the east of this node
};

/*
A list of map nodes, linked together with the 4 direction.
*/
class MapTree
{
	int mMapIndex;	//map index a unique number for every map.
	Ogre::SceneManager* mSceneMgr;
	MapNode* currentMap;	//the current map
	MapNode* newMap;		//use for creating a new map
	MapNode* nextMap;
	
	void createMap(int direction);		//create a new map
public:
	unsigned int mDirection;	//trace which direction to traverl to

	MapTree(Ogre::SceneManager* sceneMgr);
	~MapTree(void);

	void traverse(int direction);		//traverse map to a specific direciton
	Map* getDirectionedMap(int direction);
	Map* getCurrentMap(void)	{	return currentMap->m_Map;	}
};
#endif