#include "MapArea.h"
#include <string>
#include <math.h>
#include <iostream>

MapArea::MapArea(Ogre::SceneManager* sceneMgr, Ogre::Vector3 position, Ogre::Real offsetX, Ogre::Real offsetZ, unsigned int index, int arrayPositionX, int arrayPositionZ)
:mLoaded(false),
mObjectNumber(0),
mEnemyNumber(0),
mFlameThrowerNumber(0),
mArrayPositionX(arrayPositionX),
mArrayPositionZ(arrayPositionZ)
{
	mPosition = position;
	mOffsetX = offsetX;
	mOffsetZ = offsetZ;

	mCenterPosition.x = mPosition.x + (offsetX/2);
	mCenterPosition.y = 0;
	mCenterPosition.z = mPosition.z + (offsetZ/2);

	mEndingPosition.x = mPosition.x + mOffsetX-1;
	mEndingPosition.y = 0;
	mEndingPosition.z = mPosition.z + mOffsetZ-1;

	mIndex = index;

	char groundString[30];
	sprintf(groundString, "map_area_ground_%d", mIndex);
 
	mSceneMgr = sceneMgr;
	mGroundEntity = mSceneMgr->createEntity(groundString, "floor");
	//testing adding material to new entity.
	mGroundEntity->setMaterialName("ground");
	mGroundEntity->setCastShadows(false);

	Generate();
}

MapArea::~MapArea(void)
{
	_mObstacleList.clear();
	_mEnemySpawnPointList.clear();
	_mFlameThrowerList.clear();

	DELETEPTR(mGroundEntity);
	DELETEPTR(dummy);
	DELETEPTR(dummyE);
}

//take the spawn point, and randomly generate enemy inside it
void MapArea::GenerateEnemy(Objects* spawnPoint)
{
	//spawn 1 to 3 enemies in this area
	int enemyNumber = 1;//static_cast<int>(Ogre::Math::RangeRandom(1,3));
	while(enemyNumber>0)
	{
		//now genereate a individual spawn point for the enemy
		dummyE = new Objects();
		dummyE->type = ENEMY;
		dummyE->mLoaded = false;
		dummyE->index = mIndex + mEnemyNumber + 1;
		dummyE->sizeWidth = 1;
		dummyE->sizeHeight = 1;
		//random x and random z
		dummyE->mPosition.x = Ogre::Math::RangeRandom(spawnPoint->mPosition.x, spawnPoint->mPosition.x + spawnPoint->sizeWidth);
		dummyE->mPosition.z = Ogre::Math::RangeRandom(spawnPoint->mPosition.z, spawnPoint->mPosition.z + spawnPoint->sizeHeight);
		dummyE->mPosition.y = 5;

		_mEnemySpawnPointList.push_back(dummyE);

		mEnemyNumber++;

		enemyNumber--;
	}
}

void MapArea::loadXMLData(const char* pFilename)
{
	TiXmlDocument doc(pFilename);
	bool loadOkay = doc.LoadFile();
	if (loadOkay)
	{
		TiXmlNode* pParent = &doc;
		TiXmlNode* pChild;
		for ( pChild = pParent->FirstChild(); pChild != 0; pChild = pChild->NextSibling())
		{
			//this is a obstacle
			TiXmlNode* pNode;
			int tempX = 0;
			int tempZ = 0;
			int tempScaleWidth = 0;
			int tempScaleHeight = 0;
			int tempDirection = 0;

			//run through what is inside out obstacle
			for ( pNode = pChild->FirstChild(); pNode != 0; pNode = pNode->NextSibling())
			{
				TiXmlElement* pObstacleElement = pNode->ToElement();
				TiXmlAttribute* pAttrib=pObstacleElement->FirstAttribute();

				//run through all of the attribution in this element
				while(pAttrib)
				{
					int ival;
					//convert the attribution into integer
					if (pAttrib->QueryIntValue(&ival)!=TIXML_SUCCESS)
						return;

					//find out what attribution are we looking at
					const char* attributeName = pAttrib->Name();

					if(!strcmp(attributeName,areaObjectStr[X].c_str()))
					{
						//this is the x value of the position
						tempX = ival;
					}
					else if(!strcmp(attributeName,areaObjectStr[Z].c_str()))
					{
						//this is the z value of the position
						tempZ = ival;
					}
					else if(!strcmp(attributeName,areaObjectStr[SCALE_WIDTH].c_str()))
					{
						//this is the x value of the position
						tempScaleWidth = ival;
					}
					else if(!strcmp(attributeName,areaObjectStr[SCALE_HEIGHT].c_str()))
					{
						//this is the x value of the position
						tempScaleHeight = ival;
					}
					else if(!strcmp(attributeName,areaObjectStr[DIRECTION].c_str()))
					{
						//this is the x value of the position
						tempDirection = ival;
					}

					pAttrib=pAttrib->Next();
				}
			}

			//after we get all the attributions, time to create the obstacle
			dummy = new Objects();
			dummy->mLoaded = false;
			//conver the position.
			//position in xml is based on percentage.
			//find a multiplier, convert current map size into a percentage multiplier
			int multiplier = MAP_SIZE/10/100;	//map size divide by 10 areas, then convert to percentage
			
			//y is always 5. for now
			dummy->mPosition.y = 5;
			dummy->sizeWidth = tempScaleWidth*multiplier;
			dummy->sizeHeight = tempScaleHeight*multiplier;

			dummy->mPosition.x = mPosition.x + (tempX*multiplier) + (dummy->sizeWidth/2);
			dummy->mPosition.z = mPosition.z + (tempZ*multiplier) + (dummy->sizeHeight/2);
			dummy->direction = 0;

			//find out what object is this
			const char* name = pChild->Value();
			if(!strcmp(name,areaObjectStr[OBSTACLE].c_str()))
			{
				dummy->type = OBSTACLE;
				dummy->index = mIndex + mObjectNumber + 1;
				//push it into the vector list
				_mObstacleList.push_back(dummy);

				mObjectNumber++;
			}
			else if(!strcmp(name,areaObjectStr[SPAWN_POINT].c_str()))
			{
				//this is a spawn point
				//a large spawn point that can be used to genereate several enemy
				GenerateEnemy(dummy);
			}
			else if(!strcmp(name,areaObjectStr[FLAME_THROWER].c_str()))
			{
				dummy->index = mIndex + mFlameThrowerNumber + 1;
				dummy->type = FLAME_THROWER;
				dummy->direction = tempDirection;
				_mFlameThrowerList.push_back(dummy);

				mFlameThrowerNumber++;
			}
		}
	}
	else
	{
		printf("Failed to load file \"%s\"\n", pFilename);
	}
}
void MapArea::CreateBorder(void)
{
	//check this area's array position
	if(mArrayPositionX==0 || mArrayPositionX==9)
	{
		//this is either at the top or the bottom of the map, prepare to add a border to the top of the area
		//left the middle 2 area out, so it will use as an entrance into the other map
		if(mArrayPositionZ!=4 && mArrayPositionZ!=5)
		{
			//now add the area into it.
			dummy = new Objects();
			dummy->mLoaded = false;
			//find a multiplier, convert current map size into a percentage multiplier
			int multiplier = MAP_SIZE/10/100;	//map size divide by 10 areas, then convert to percentage
			
			dummy->mPosition.y = 5;
			dummy->sizeWidth = MAP_SIZE/10;		//the width will cover the whole area, so use the area size
			dummy->sizeHeight = 5*multiplier;	//use default size for the height, because the border in the map editor is done this way.

			//as the top area border, position is the start of the area position.
			if(mArrayPositionX == 0)
				dummy->mPosition.z = mPosition.z;
			//else the bottom of the area border, position z have to be adjusted.
			else
				dummy->mPosition.z = mPosition.z - (5*multiplier);

			dummy->mPosition.x = mPosition.x;
			dummy->direction = 0;

			dummy->type = OBSTACLE;
			dummy->index = mIndex + mObjectNumber + 1;
			//push it into the vector list
			_mObstacleList.push_back(dummy);

			mObjectNumber++;
		}
	}

	//do the same for the z
	if(mArrayPositionZ==0 || mArrayPositionZ==9)
	{
		//this is either at the top or the bottom of the map, prepare to add a border to the top of the area
		//left the middle 2 area out, so it will use as an entrance into the other map
		if(mArrayPositionX!=4 && mArrayPositionX!=5)
		{
			//now add the area into it.
			dummy = new Objects();
			dummy->mLoaded = false;
			//find a multiplier, convert current map size into a percentage multiplier
			int multiplier = MAP_SIZE/10/100;	//map size divide by 10 areas, then convert to percentage
			
			//something extra we need to do here, because the top and bottom border covers the whole line,
			//some place the left and right border will intercep with that
			dummy->mPosition.y = 5;
			dummy->sizeWidth = 5*multiplier;	//use default size for the width, because the border in the map editor is done this way.

			if(mArrayPositionX==0 || mArrayPositionX==9)
				dummy->sizeHeight = MAP_SIZE/10 - (5*multiplier);	//this place will intercep with the top and down border, so need to reduce the size to avoid that happening
			else
				dummy->sizeHeight = MAP_SIZE/10;

			//as the left area border, position is the start of the area position.
			if(mArrayPositionZ == 0)
				dummy->mPosition.x = mPosition.x;
			//else the right of the area border, position x have to be adjusted.
			else
				dummy->mPosition.x = mPosition.x - (5*multiplier);

			if(mArrayPositionX==0)
				dummy->mPosition.z = mPosition.z + (5*multiplier);
			else
				dummy->mPosition.z = mPosition.z;
			
			dummy->direction = 0;

			dummy->type = OBSTACLE;
			dummy->index = mIndex + mObjectNumber + 1;
			//push it into the vector list
			_mObstacleList.push_back(dummy);

			mObjectNumber++;
		}
	}
}
void MapArea::Generate(void)
{
	//random between maps
	int randomNum = (int)Ogre::Math::RangeRandom(0,5);
//	randomNum = 0;

	//load the map porpoties.
	switch(randomNum)
	{
	case 0:
		loadXMLData("MapArea/area_0.xml");
		break;
	case 1:
		loadXMLData("MapArea/area_1.xml");
		break;
	case 2:
		loadXMLData("MapArea/area_2.xml");
		break;
	case 3:
		loadXMLData("MapArea/area_3.xml");
		break;
	case 4:
		loadXMLData("MapArea/area_4.xml");
		break;
	case 5:
		loadXMLData("MapArea/area_5.xml");
		break;
	default:
		break;
	}

	CreateBorder();
}