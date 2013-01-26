#ifndef _TERRAIN_H_
#define _TERRAIN_H_

#include <OgreRoot.h>
#include <OgreSceneManager.h>
#include <OgreTerrain.h>
#include <OgreTerrainGroup.h>
#include <OgreMaterialManager.h>

/*
Generate the terrain
*/

class Terrain
{
private:
	Ogre::SceneManager* mSceneMgr;
    Ogre::TerrainGlobalOptions* mTerrainGlobals;
    
	
    void defineTerrain(long x, long y);
    void initBlendMaps(Ogre::Terrain* terrain);
    void configureTerrainDefaults(Ogre::Light* light);
public:
	bool mTerrainsImported;
	Ogre::TerrainGroup* mTerrainGroup;
	Terrain(Ogre::SceneManager *sceneMgr);
	~Terrain(void);

	void createTerrain(void);
};
#endif