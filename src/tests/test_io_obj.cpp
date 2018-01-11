#include <io/io_obj.h>
#include <io/Line/line.h>

#include <boost/filesystem.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp> 

#include <string>
#include <vector>
#include <algorithm>
#include <iterator>
#include <fstream>
#include <streambuf>

#include <catch.hpp>

SCENARIO("Input/Output from obj file:")
{
    GIVEN("An existing obj file")
    {
        boost::filesystem::path filepath("../../ressources/3dModels/OBJ/scene.obj");

        WHEN("reading in text mode")
        {
            std::vector<urban::shadow::Mesh> meshes = urban::io::FileHandler<urban::io::Obj_stream>(filepath, std::map<std::string, bool>{{"read", true}}).read();

            THEN("the output checks")
            {
                std::ostringstream auxilary, out;
                std::copy(
                    std::begin(meshes),
                    std::end(meshes),
                    std::ostream_iterator<urban::shadow::Mesh>(std::cout, "\n")
                );

                std::istringstream _auxilary(auxilary.str());
                std::list<std::string> lines;
                urban::io::readlines(_auxilary, std::back_inserter(lines));
                std::copy(
                    std::begin(lines),
                    std::end(lines),
                    std::ostream_iterator<std::string>(out, "\n")
                );

                std::ifstream tmp("../../ressources/tests/hammerhead_shadow_mesh.txt");
                std::string tmp_str((std::istreambuf_iterator<char>(tmp)), std::istreambuf_iterator<char>());
                // REQUIRE(out.str() == tmp_str);
            }
        }

        WHEN("the reading mode is not chosen")
        {
            urban::io::FileHandler<urban::io::Obj_stream> handler(filepath, std::map<std::string, bool>{{}});

            THEN("the reader throws")
            {
                REQUIRE_THROWS(handler.read());
            }
        }
    }

    GIVEN("A wrong file path")
    {
        boost::filesystem::path filepath("../ressources/3dModels/OBJ/scene.obj");

        WHEN("the reading mode is chosen")
        {
            urban::io::FileHandler<urban::io::Obj_stream> handler(filepath, std::map<std::string, bool>{{"read", true}});

            THEN("the reader throws")
            {
                REQUIRE_THROWS(handler.read());
            }
        }

        WHEN("the reading mode is not chosen")
        {
            urban::io::FileHandler<urban::io::Obj_stream> handler(filepath, std::map<std::string, bool>{{}});

            THEN("the reader throws")
            {
                REQUIRE_THROWS(handler.read());
            }
        }
    }

    GIVEN("An existing set of objects")
    {
        std::vector<urban::shadow::Mesh> meshes = urban::io::FileHandler<urban::io::Obj_stream>(
            boost::filesystem::path("../../ressources/3dModels/OFF/hammerhead.off"),
            std::map<std::string, bool>{{"read", true}}
        ).read();

        WHEN("the writing mode is chosen")
        {
            std::ostringstream file_name;
            file_name << boost::uuids::random_generator()() << ".off";

            urban::io::FileHandler<urban::io::Obj_stream> handler(
                boost::filesystem::path(file_name.str()),
                std::map<std::string, bool>{{"write", true}}
            );
            handler.write(meshes);

            THEN("the input should check")
            {
                std::vector<urban::shadow::Mesh> written_meshes = urban::io::FileHandler<urban::io::Obj_stream>(
                    boost::filesystem::path(file_name.str()),
                    std::map<std::string, bool>{{"read", true}}
                ).read();

                std::ostringstream auxilary, out;
                std::copy(
                    std::begin(written_meshes),
                    std::end(written_meshes),
                    std::ostream_iterator<urban::shadow::Mesh>(auxilary, "\n")
                );

                std::istringstream _auxilary(auxilary.str());
                std::vector<std::string> lines;
                urban::io::readlines(_auxilary, std::back_inserter(lines));
                std::copy(std::next(std::begin(lines)), std::end(lines), std::ostream_iterator<std::string>(out, "\n"));

                std::ifstream tmp("../../ressources/tests/hammerhead_shadow_mesh.txt");
                std::string tmp_str((std::istreambuf_iterator<char>(tmp)), std::istreambuf_iterator<char>());
                REQUIRE(out.str() == tmp_str);
            }
        }
        
        WHEN("the writing mode is not chosen")
        {
            urban::io::FileHandler<urban::io::Obj_stream> handler("./hammerhead.off", std::map<std::string, bool>{{"write", false}});

            THEN("the writter throws")
            {
                REQUIRE_THROWS(handler.write(meshes));
            }
        }
    }
}
