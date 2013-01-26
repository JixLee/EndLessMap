This is a feature I did for a school project.

The project is using Ogre3D engine, and the feature I did was a map that is ever expanding.

Here is the source codes for the map system.

The source contains those files:

Map.cpp
Map.h

MapArea.cpp
MapArea.h

MapManager.cpp
MapManager.h

MapTree.cpp
MapTree.h


The basic idea of the map works like this:
MapManager is the main class, it contains a MapTree and is doing all the management stuff.
MapTree contains Maps that are link to each other, and is expanding whenever the player is reaching one end of the map.
Each Map contains 10 MapArea in an array.
Each MapArea reads a xml format file that contains every object porpoties in that area.

I created another very simple map editor that lets player create and export xml files in an other application.