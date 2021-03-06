#include <io/io_off.h>
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

SCENARIO("Input/Output from OFF file:")
{
    GIVEN("An existing OFF file")
    {
        boost::filesystem::path filepath("../../ressources/3dModels/OFF/hammerhead.off");

        WHEN("reading in text mode")
        {
            city::shadow::Mesh mesh = city::io::OFFHandler(filepath, std::map<std::string, bool>{{"read", true}}).read();

            THEN("the output checks")
            {
                std::ostringstream auxilary, out;
                auxilary << mesh;

                std::istringstream _auxilary(auxilary.str());
                std::vector<std::string> lines;
                city::io::readlines(_auxilary, std::back_inserter(lines));
                std::copy(std::next(std::begin(lines), 1), std::end(lines), std::ostream_iterator<std::string>(out, "\n"));

                std::ifstream tmp("../../ressources/tests/hammerhead_shadow_mesh.txt");
                std::string tmp_str((std::istreambuf_iterator<char>(tmp)), std::istreambuf_iterator<char>());
                REQUIRE(out.str() == tmp_str);
            }
        }

        WHEN("the reading mode is not chosen")
        {
            city::io::OFFHandler handler(filepath, std::map<std::string, bool>{{}});

            THEN("the reader throws")
            {
                REQUIRE_THROWS(handler.read());
            }
        }
    }

    GIVEN("A wrong file path")
    {
        boost::filesystem::path filepath("../ressources/3dModels/OFF/hammerhead.off");

        WHEN("the reading mode is chosen")
        {
            city::io::OFFHandler handler(filepath, std::map<std::string, bool>{{"read", true}});

            THEN("the reader throws")
            {
                REQUIRE_THROWS(handler.read());
            }
        }

        WHEN("the reading mode is not chosen")
        {
            city::io::OFFHandler handler(filepath, std::map<std::string, bool>{{}});

            THEN("the reader throws")
            {
                REQUIRE_THROWS(handler.read());
            }
        }
    }

    GIVEN("An existing a city::shadow::Mesh")
    {
        city::shadow::Mesh mesh = city::io::OFFHandler(
            boost::filesystem::path("../../ressources/3dModels/OFF/hammerhead.off"),
            std::map<std::string, bool>{{"read", true}}
        ).read();

        WHEN("the writing mode is chosen")
        {
            std::ostringstream file_name;
            file_name << boost::uuids::random_generator()() << ".off";

            city::io::OFFHandler handler(boost::filesystem::path(file_name.str()), std::map<std::string, bool>{{"write", true}});
            handler.write(mesh);

            THEN("the input should check")
            {
                city::shadow::Mesh written_mesh = city::io::OFFHandler(
                    boost::filesystem::path(file_name.str()),
                    std::map<std::string, bool>{{"read", true}}
                ).read();

                std::ostringstream auxilary, out;
                auxilary << written_mesh;

                std::istringstream _auxilary(auxilary.str());
                std::vector<std::string> lines;
                city::io::readlines(_auxilary, std::back_inserter(lines));
                std::copy(std::next(std::begin(lines)), std::end(lines), std::ostream_iterator<std::string>(out, "\n"));

                std::ifstream tmp("../../ressources/tests/hammerhead_shadow_mesh.txt");
                std::string tmp_str((std::istreambuf_iterator<char>(tmp)), std::istreambuf_iterator<char>());
                REQUIRE(out.str() == tmp_str);
            }
        }
        
        WHEN("the writing mode is not chosen")
        {
            city::io::OFFHandler handler("./hammerhead.off", std::map<std::string, bool>{{"write", false}});

            THEN("the writter throws")
            {
                REQUIRE_THROWS(handler.write(mesh));
            }
        }
    }
}
