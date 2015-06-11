#include <camoto/stream_file.hpp>
#include <camoto/util.hpp>
#include <camoto/gamemaps.hpp>
#include <iostream>

using namespace camoto;
using namespace camoto::gamemaps;

int main(void)
{
	// Use the manager to look up a particular map format
	auto mapType = MapManager::byCode("map-xargon");

	// Open a map file on disk (false means don't create the file if it doesn't
	// exist)
	auto file = std::make_unique<stream::file>("board_01.xr1", false);

	// We cheat here - we should check and load any supplementary files, but
	// for the sake of keeping this example simple we know this format doesn't
	// need any supps.
	camoto::SuppData supps;

	// Use the map format handler to read in the file we opened as a map
	std::shared_ptr<Map> map;
	try {
		map = mapType->open(std::move(file), supps);
	} catch (const stream::open_error& e) {
		std::cerr << "Error opening map: " << e.what() << std::endl;
		return 1;
	}

	// See if the map is a 2D grid-based one
	auto map2d = std::dynamic_pointer_cast<Map2D>(map);
	if (map2d) {

		// It is, print the number of layers
		std::cout << "This map has " << map2d->layers().size() << " layers."
			<< std::endl;

	} else {
		std::cout << "This map was not a 2D map." << std::endl;
	}

	// No cleanup required because all the Ptr variables are shared pointers,
	// which get destroyed automatically when they go out of scope (if nobody
	// else is using them!)

	return 0;
}
