#include <fstream>
#include <camoto/gamemaps.hpp>

using namespace camoto::gamemaps;

int main(void)
{
	// Get hold of the Manager class
	ManagerPtr manager = getManager();

	// Use the manager to look up a particular map format
	MapTypePtr mapType = manager->getMapTypeByCode("map-xargon");

	// Open an map file on disk
	camoto::iostream_sptr file(new std::fstream("board_01.xr1"));

	// We cheat here - we should check and load any supplementary files, but
	// for the sake of keeping this example simple we know this format doesn't
	// need any supps.
	MP_SUPPDATA supps;

	// Use the map format handler to read in the file we opened as a map
	MapPtr map;
	try {
		map = mapType->open(file, supps);
	} catch (const std::ios::failure& e) {
		std::cerr << "Error opening map: " << e.what() << std::endl;
		return 1;
	}

	// See if the map is a 2D grid-based one
	Map2DPtr map2d = boost::dynamic_pointer_cast<Map2D>(map);
	if (map2d) {

		// It is, print the number of layers
		std::cout << "This map has " << map2d->getLayerCount() << " layers."
			<< std::endl;

	} else {
		std::cout << "This map was not a 2D map." << std::endl;
	}

	// No cleanup required because all the Ptr variables are shared pointers,
	// which get destroyed automatically when they go out of scope (and nobody
	// else is using them!)

	return 0;
}
