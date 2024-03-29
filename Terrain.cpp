#include "Terrain.h"

Terrain::Terrain(Ogre::SceneManager *sceneMgr)
	:mTerrainsImported(false),
	mTerrainGlobals(0),
	mTerrainGroup(0)
{
	mSceneMgr = sceneMgr;
}

Terrain::~Terrain(void)
{
	OGRE_DELETE mTerrainGroup;
    OGRE_DELETE mTerrainGlobals;
}

void Terrain::createTerrain(void)
{
	Ogre::MaterialManager::getSingleton().setDefaultTextureFiltering(Ogre::TFO_ANISOTROPIC);
    Ogre::MaterialManager::getSingleton().setDefaultAnisotropy(7);
 
	Ogre::Vector3 lightdir(Ogre::Real(0.55), Ogre::Real(-0.3), Ogre::Real(0.75));
    lightdir.normalise();

    Ogre::Light* light = mSceneMgr->createLight("tstLight");
    light->setType(Ogre::Light::LT_DIRECTIONAL);
    light->setDirection(lightdir);
    light->setDiffuseColour(Ogre::ColourValue::White);
    light->setSpecularColour(Ogre::ColourValue(0.4f, 0.4f, 0.4f));
 
    mSceneMgr->setAmbientLight(Ogre::ColourValue(0.2f, 0.2f, 0.2f));
 
    mTerrainGlobals = OGRE_NEW Ogre::TerrainGlobalOptions();

    mTerrainGroup = OGRE_NEW Ogre::TerrainGroup(mSceneMgr, Ogre::Terrain::ALIGN_X_Z, 513, 1200.0f);
    mTerrainGroup->setFilenameConvention(Ogre::String("myFirstOgreTerrain"), Ogre::String("dat"));
    mTerrainGroup->setOrigin(Ogre::Vector3::ZERO);
 
    configureTerrainDefaults(light);
 
    for (long x = 0; x <= 0; ++x)
        for (long y = 0; y <= 0; ++y)
            defineTerrain(x, y);
 
    // sync load since we want everything in place when we start
    mTerrainGroup->loadAllTerrains(true);
 
    if (mTerrainsImported)
    {
        Ogre::TerrainGroup::TerrainIterator ti = mTerrainGroup->getTerrainIterator();
        while(ti.hasMoreElements())
        {
            Ogre::Terrain* t = ti.getNext()->instance;
            initBlendMaps(t);
        }
    }
 
    mTerrainGroup->freeTemporaryResources();
	
}

void Terrain::configureTerrainDefaults(Ogre::Light* light)
{
    // Configure global
    mTerrainGlobals->setMaxPixelError(8);
    // testing composite map
    mTerrainGlobals->setCompositeMapDistance(3000);
 
    // Important to set these so that the terrain knows what to use for derived (non-realtime) data
    mTerrainGlobals->setLightMapDirection(light->getDerivedDirection());
    mTerrainGlobals->setCompositeMapAmbient(mSceneMgr->getAmbientLight());
    mTerrainGlobals->setCompositeMapDiffuse(light->getDiffuseColour());
 
    // Configure default import settings for if we use imported image
    Ogre::Terrain::ImportData& defaultimp = mTerrainGroup->getDefaultImportSettings();
    defaultimp.terrainSize = 513;
    defaultimp.worldSize = 1200.0f;
    defaultimp.inputScale = 60;
    defaultimp.minBatchSize = 33;
    defaultimp.maxBatchSize = 65;
//	defaultimp.pos = Ogre::Vector3(1000000,0,0);

    // textures
    defaultimp.layerList.resize(3);
    defaultimp.layerList[0].worldSize = 10;
    defaultimp.layerList[0].textureNames.push_back("Terrain/dirt_grayrocky_diffusespecular.dds");
    defaultimp.layerList[0].textureNames.push_back("Terrain/dirt_grayrocky_normalheight.dds");
    defaultimp.layerList[1].worldSize = 3;
    defaultimp.layerList[1].textureNames.push_back("Terrain/grass_green-01_diffusespecular.dds");
    defaultimp.layerList[1].textureNames.push_back("Terrain/grass_green-01_normalheight.dds");
    defaultimp.layerList[2].worldSize = 20;
    defaultimp.layerList[2].textureNames.push_back("Terrain/growth_weirdfungus-03_diffusespecular.dds");
    defaultimp.layerList[2].textureNames.push_back("Terrain/growth_weirdfungus-03_normalheight.dds");
	
}

void getTerrainImage(bool flipX, bool flipY, Ogre::Image& img)
{
    img.load("Terrain/terrain.png", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    if (flipX)
        img.flipAroundY();
    if (flipY)
        img.flipAroundX();

}
void Terrain::defineTerrain(long x, long y)
{
    Ogre::String filename = mTerrainGroup->generateFilename(x, y);
    if (Ogre::ResourceGroupManager::getSingleton().resourceExists(mTerrainGroup->getResourceGroup(), filename))
    {
        mTerrainGroup->defineTerrain(x, y);
    }
    else
    {
        Ogre::Image img;
        getTerrainImage(x % 2 != 0, y % 2 != 0, img);
        mTerrainGroup->defineTerrain(x, y, &img);
        mTerrainsImported = true;
    }
}

void Terrain::initBlendMaps(Ogre::Terrain* terrain)
{
    Ogre::TerrainLayerBlendMap* blendMap0 = terrain->getLayerBlendMap(1);
    Ogre::TerrainLayerBlendMap* blendMap1 = terrain->getLayerBlendMap(2);
    Ogre::Real minHeight0 = 70;
    Ogre::Real fadeDist0 = 40;
    Ogre::Real minHeight1 = 70;
    Ogre::Real fadeDist1 = 15;
    float* pBlend0 = blendMap0->getBlendPointer();
    float* pBlend1 = blendMap1->getBlendPointer();
    for (Ogre::uint16 y = 0; y < terrain->getLayerBlendMapSize(); ++y)
    {
        for (Ogre::uint16 x = 0; x < terrain->getLayerBlendMapSize(); ++x)
        {
            Ogre::Real tx, ty;
 
            blendMap0->convertImageToTerrainSpace(x, y, &tx, &ty);
            Ogre::Real height = terrain->getHeightAtTerrainPosition(tx, ty);
            Ogre::Real val = (height - minHeight0) / fadeDist0;
            val = Ogre::Math::Clamp(val, (Ogre::Real)0, (Ogre::Real)1);
            *pBlend0++ = val;
 
            val = (height - minHeight1) / fadeDist1;
            val = Ogre::Math::Clamp(val, (Ogre::Real)0, (Ogre::Real)1);
            *pBlend1++ = val;
        }
    }
    blendMap0->dirty();
    blendMap1->dirty();
    blendMap0->update();
    blendMap1->update();
}