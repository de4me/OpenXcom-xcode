/*
 * Copyright 2010-2016 OpenXcom Developers.
 *
 * This file is part of OpenXcom.
 *
 * OpenXcom is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenXcom is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenXcom.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "RuleGlobe.h"
#ifndef SDL_FRAMEWORK
#include <SDL_endian.h>
#else
#include <SDL/SDL_endian.h>
#endif
#include <fstream>
#include "../Engine/Exception.h"
#include "Polygon.h"
#include "Polyline.h"
#include "Texture.h"
#include "../Engine/Palette.h"
#include "../Geoscape/Globe.h"
#include "../Engine/FileMap.h"
#include "../fmath.h"

namespace OpenXcom
{

/**
 * Creates a blank ruleset for globe contents.
 */
RuleGlobe::RuleGlobe()
{
}

/**
 *
 */
RuleGlobe::~RuleGlobe()
{
	for (std::list<Polygon*>::iterator i = _polygons.begin(); i != _polygons.end(); ++i)
	{
		delete *i;
	}
	for (std::list<Polyline*>::iterator i = _polylines.begin(); i != _polylines.end(); ++i)
	{
		delete *i;
	}
	for (std::map<int, Texture*>::iterator i = _textures.begin(); i != _textures.end(); ++i)
	{
		delete i->second;
	}
}

/**
 * Loads the globe from a YAML file.
 * @param node YAML node.
 */
void RuleGlobe::load(const YAML::Node &node)
{
	if (node["data"])
	{
		for (std::list<Polygon*>::iterator i = _polygons.begin(); i != _polygons.end(); ++i)
		{
			delete *i;
		}
		_polygons.clear();
		loadDat(FileMap::getFilePath(node["data"].as<std::string>()));
	}
	if (node["polygons"])
	{
		for (std::list<Polygon*>::iterator i = _polygons.begin(); i != _polygons.end(); ++i)
		{
			delete *i;
		}
		_polygons.clear();
		for (YAML::const_iterator i = node["polygons"].begin(); i != node["polygons"].end(); ++i)
		{
			Polygon *polygon = new Polygon(3);
			polygon->load(*i);
			_polygons.push_back(polygon);
		}
	}
	if (node["polylines"])
	{
		for (std::list<Polyline*>::iterator i = _polylines.begin(); i != _polylines.end(); ++i)
		{
			delete *i;
		}
		_polylines.clear();
		for (YAML::const_iterator i = node["polylines"].begin(); i != node["polylines"].end(); ++i)
		{
			Polyline *polyline = new Polyline(3);
			polyline->load(*i);
			_polylines.push_back(polyline);
		}
	}
	for (YAML::const_iterator i = node["textures"].begin(); i != node["textures"].end(); ++i)
	{
		if ((*i)["id"])
		{
			int id = (*i)["id"].as<int>();
			std::map<int, Texture*>::const_iterator j = _textures.find(id);
			Texture *texture;
			if (j != _textures.end())
			{
				texture = j->second;
			}
			else
			{
				texture = new Texture(id);
				_textures[id] = texture;
			}
			texture->load(*i);
		}
		else if ((*i)["delete"])
		{
			int id = (*i)["delete"].as<int>();
			std::map<int, Texture*>::iterator j = _textures.find(id);
			if (j != _textures.end())
			{
				_textures.erase(j);
			}
		}
	}

	Globe::COUNTRY_LABEL_COLOR = node["countryColor"].as<int>(Globe::COUNTRY_LABEL_COLOR);
	Globe::CITY_LABEL_COLOR = node["cityColor"].as<int>(Globe::CITY_LABEL_COLOR);
	Globe::BASE_LABEL_COLOR = node["baseColor"].as<int>(Globe::BASE_LABEL_COLOR);
	Globe::LINE_COLOR = node["lineColor"].as<int>(Globe::LINE_COLOR);
	if (node["oceanPalette"])
	{
		Globe::OCEAN_COLOR = Palette::blockOffset(node["oceanPalette"].as<int>(Globe::OCEAN_COLOR));
	}
	Globe::OCEAN_SHADING = node["oceanShading"].as<bool>(Globe::OCEAN_SHADING);
}

/**
 * Returns the list of polygons in the globe.
 * @return Pointer to the list of polygons.
 */
std::list<Polygon*> *RuleGlobe::getPolygons()
{
	return &_polygons;
}

/**
 * Returns the list of polylines in the globe.
 * @return Pointer to the list of polylines.
 */
std::list<Polyline*> *RuleGlobe::getPolylines()
{
	return &_polylines;
}

/**
 * Loads a series of map polar coordinates in X-Com format,
 * converts them and stores them in a set of polygons.
 * @param filename Filename of the DAT file.
 * @sa http://www.ufopaedia.org/index.php?title=WORLD.DAT
 */
void RuleGlobe::loadDat(const std::string &filename)
{
	// Load file
	std::ifstream mapFile (filename.c_str(), std::ios::in | std::ios::binary);
	if (!mapFile)
	{
		throw Exception(filename + " not found");
	}

	short value[10];

	while (mapFile.read((char*)&value, sizeof(value)))
	{
		Polygon* poly;
		int points;

		for (int i = 0; i < 10; ++i)
		{
			value[i] = SDL_SwapLE16(value[i]);
		}

		if (value[6] != -1)
		{
			points = 4;
		}
		else
		{
			points = 3;
		}
		poly = new Polygon(points);

		for (int i = 0, j = 0; i < points; ++i)
		{
			// Correct X-Com degrees and convert to radians
			double lonRad = Xcom2Rad(value[j++]);
			double latRad = Xcom2Rad(value[j++]);

			poly->setLongitude(i, lonRad);
			poly->setLatitude(i, latRad);
		}
		poly->setTexture(value[8]);

		_polygons.push_back(poly);
	}

	if (!mapFile.eof())
	{
		throw Exception("Invalid globe map");
	}

	mapFile.close();
}

/**
 * Returns the rules for the specified texture.
 * @param id Texture ID.
 * @return Rules for the texture.
 */
Texture *RuleGlobe::getTexture(int id) const
{
	std::map<int, Texture*>::const_iterator i = _textures.find(id);
	if (_textures.end() != i) return i->second; else return 0;
}

/**
 * Returns a list of all globe terrains associated with this deployment.
 * @param deployment Deployment name.
 * @return List of terrains.
 */
std::vector<std::string> RuleGlobe::getTerrains(const std::string &deployment) const
{
	std::vector<std::string> terrains;
	for (std::map<int, Texture*>::const_iterator i = _textures.begin(); i != _textures.end(); ++i)
	{
		if ((deployment.empty() && i->second->getDeployments().empty()) || i->second->getDeployments().find(deployment) != i->second->getDeployments().end())
		{
			for (std::vector<TerrainCriteria>::const_iterator j = i->second->getTerrain()->begin(); j != i->second->getTerrain()->end(); ++j)
			{
				terrains.push_back(j->name);
			}
		}
	}
	return terrains;
}

}
