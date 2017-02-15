#include "../Shadow/mesh.h"
#include "../UrbanObject/brick.h"
#include "../IO/io.h"
#include "../IO/io_3ds.h"

#include "../Projection/brick_projection.h"
#include "../Algorithms/algorithms.h"

#include <boost/filesystem.hpp>

#include <vector>
#include <map>
#include <algorithm>
#include <iterator>

int main(int, char **)
{
    try
    {
        boost::filesystem::path filepath("../../ressources/3dModels/3DS/Toy/Toy Santa Claus N180816.3DS");
        std::map<std::string, bool> modes{{"read", true}};
        urban::io::FileHandler<Lib3dsFile> handler(filepath, modes);
        std::vector<urban::shadow::Mesh> meshes = handler.read();

        std::vector<urban::Brick> urban_objects(meshes.size());
        std::transform(
            std::begin(meshes),
            std::end(meshes),
            std::begin(urban_objects),
            [](const urban::shadow::Mesh & mesh)
            {
                return urban::Brick(mesh);
            }
        );

        std::vector<urban::projection::BrickPrint> projections_xy(urban_objects.size());
        std::transform(
            std::begin(meshes),
            std::end(meshes),
            std::begin(projections_xy),
            [](const urban::Brick & brick)
            {
                return urban::project(brick);
            }
        );
    }
    catch (const std::exception& except)
    {
        std::cerr << except.what() << std::endl;
    }
    return EXIT_SUCCESS;
}
