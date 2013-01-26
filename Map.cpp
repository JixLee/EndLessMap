#include "Map.h"
#include <iostream>

Map::Map(Ogre::SceneManager* sceneMgr, Ogre::Vector3 centerPosition, unsigned int index)
{
	mSceneMgr = sceneMgr;
	mCenterPosition = centerPosition;
	mIndex = index;

	//calculate our starting and ending position for the map.
	mStartingPosition.x = centerPosition.x - (MAP_SIZE/2);
	mStartingPosition.y = 0;
	mStartingPosition.z = centerPosition.z - (MAP_SIZE/2);

	mEndingPosition.x = centerPosition.x + (MAP_SIZE/2);
	mEndingPosition.y = 0;
	mEndingPosition.z = centerPosition.z + (MAP_SIZE/2);

	mGroundLoaded = false;

	Generate();
}

Map::~Map(void)
{
	DELETEPTR(mSceneMgr);
}

void Map::Generate(void)
{
	offset = MAP_SIZE/10;
	
	unsigned int areaIndex = mIndex;

	for(int z = 0; z < 10; z++)
		for(int x = 0; x < 10; x++)
		{

			mMapArea[z][x] = new MapArea(mSceneMgr,Ogre::Vector3(mCenterPosition.x + (offset*x-MAP_SIZE/2),0,mCenterPosition.z + (offset*z-MAP_SIZE/2)),offset,offset,areaIndex, z, x);
			areaIndex += 1000;
		}
}

int Map::FindCurrentArea(Ogre::Real currentPosition, bool X)
{
	int result = 0;
	bool found = false;

	while(!found)
	{
		if(result>=10)
			break;

		if(X)
		{
			if(currentPosition >= mMapArea[0][result]->getStartingPosition().x && currentPosition <= mMapArea[0][result]->getEndingPosition().x)
			{
				found = true;
			}
		}
		else
		{
			if(currentPosition >= mMapArea[result][0]->getStartingPosition().z && currentPosition <= mMapArea[result][0]->getEndingPosition().z)
			{
				found = true;
			}
		}

		if(!found)
			result++;
	}

	return result;
	
}

/*
Function that takes in the position of the character, and return an array of all the areas that will get affected for loading.
Only returns area that is inside the load radius.

For now, I am assuming that we always start off at the center of a area. Need improvements when doing loading of a game.
*/
std::vector<MapArea*> Map::GetAllFullAffectingArea(Ogre::Vector3 currentPosition)
{
	//first find out which area is the the current position at.
	int currentRow;
	int currentColumn;

	//loop through x positions to find which row are we in.
	currentRow = FindCurrentArea(currentPosition.x, true);

	//loop through z positions to find which column are we in.
	currentColumn = FindCurrentArea(currentPosition.z, false);

	//find out how many we need to return
	//using load radius we will know how many area around it will get affected
	int rowsOfAreasAffected = int(LOADRADIUS/(int)(offset));
	
	//find out our remainder, which is the areas that are inside load radius but not fully inside.
	int remainder = LOADRADIUS%(int)(offset);

	if(remainder > 0)
		rowsOfAreasAffected++;

	//now we should know our starting row/column and ending row/column we need to return
	int startRow = currentRow - rowsOfAreasAffected;
	int endRow =  currentRow + rowsOfAreasAffected;
	int startColumn = currentColumn - rowsOfAreasAffected;
	int endColumn  = currentColumn + rowsOfAreasAffected;

	//but we need to make sure the rows and columns is a valid one
	if(startRow<0)
		//start row shound't be less than 0
		//set it to 0
		startRow = 0;

	if(endRow>=10)
		//end row shouldn't be more or equals to 10
		//set it back to 9
		endRow = 9;

	//same to column
	if(startColumn<0)
		startColumn = 0;
	if(endColumn>=10)
		endColumn = 9;

	//now we know the number we can create our array
	std::vector<MapArea*> tempArea;

	//now we need to fill up our array with actual map area.
	for(int row = startRow; row<=endRow; row++)
		for(int column = startColumn; column<=endColumn; column++)
		{
			//has this area already loaded?
			if(mMapArea[column][row]->isLoaded())
				//yes, skip
				continue;

			//make sure the area is actually inside load radius
			if((Ogre::Math::Abs(mMapArea[column][row]->getStartingPosition().x - currentPosition.x) < LOADRADIUS && Ogre::Math::Abs(mMapArea[column][row]->getStartingPosition().z - currentPosition.z) < LOADRADIUS)
				|| (Ogre::Math::Abs(mMapArea[column][row]->getEndingPosition().x - currentPosition.x) < LOADRADIUS && Ogre::Math::Abs(mMapArea[column][row]->getStartingPosition().z - currentPosition.z) < LOADRADIUS)
				|| (Ogre::Math::Abs(mMapArea[column][row]->getStartingPosition().x - currentPosition.x) < LOADRADIUS && Ogre::Math::Abs(mMapArea[column][row]->getEndingPosition().z - currentPosition.z) < LOADRADIUS)
				|| (Ogre::Math::Abs(mMapArea[column][row]->getEndingPosition().x - currentPosition.x) < LOADRADIUS && Ogre::Math::Abs(mMapArea[column][row]->getEndingPosition().z - currentPosition.z) < LOADRADIUS))
			{
				//add it into the list
				tempArea.push_back(mMapArea[column][row]);
			}
			else
				//skip
				continue;
		}

	return tempArea;
}