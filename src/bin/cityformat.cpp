#include <config.h>

#include <urban.h>

#include <docopt.h>

#include <boost/filesystem.hpp>

static const char USAGE[]=
R"(cityformat.

    Usage:
      cityformat <scene> --input-format=<input_frmt> [--prune --graphs --terrain] [output <path> --output-format=<output_format>]
      cityformat --formats
      cityformat (-h | --help)
      cityformat --version
    Options:
      -h --help                             Show this screen.
      --version                             Show version.
      --prune                               Prune building faces.
      --input-format=<input_frmt>           Specify input format.
      --graphs                              Save the building facets dual graph.
      --terrain                             Taking care of terrain.
      --output-format=<output_frmt>         Specify output format.
      --formats                             Give all possible formats.
)";

struct Arguments
{
    struct SceneArguments
    {
        boost::filesystem::path input_path;
        std::string input_format;
        bool prune = false;
        bool graphs = false;
        bool terrain = false;
    };
    struct SaveArguments
    {
        boost::filesystem::path output_path;
        std::string output_format;
    };

    Arguments(std::map<std::string, docopt::value> const& docopt_args)
        : formats(docopt_args.at("--formats").asBool())
    {
        std::cout << "Parsing arguments... " << std::flush;
        
        if(! formats)
        {
            scene_args.input_path = docopt_args.at("<scene>").asString();
            scene_args.input_format = docopt_args.at("--input-format").asString();
            scene_args.prune = docopt_args.at("--prune").asBool();
            scene_args.graphs = docopt_args.at("--graphs").asBool();
            scene_args.terrain = docopt_args.at("--terrain").asBool();
            
            save_args.output_path = docopt_args.at("<path>").asString();
            save_args.output_format = docopt_args.at("--output-format").asString();
        }

        std::cout << "Done." << std::flush << std::endl;
    }
    ~Arguments(void)
    {}

    bool formats;
    SceneArguments scene_args;
    SaveArguments save_args;
};

inline std::ostream & operator <<(std::ostream & os, Arguments & arguments)
{
    if(arguments.formats)
        os << "Arguments:" << std::endl
           << "  Possible formats: " << arguments.formats << std::endl;
    else
        os << "Arguments:" << std::endl
           << "  Input path: " << arguments.scene_args.input_path << std::endl
           << "  Input format: " << arguments.scene_args.input_format << std::endl
           << "  Pruning faces: " << arguments.scene_args.prune << std::endl
           << "  Taking care of terrain: " << arguments.scene_args.terrain << std::endl
           << "  Saving dual graphs: " << arguments.scene_args.graphs << std::endl
           << "  Output path: " << arguments.save_args.output_path << std::endl
           << "  Output format: " << arguments.save_args.output_format << std::endl;

    return os;
}

int main(int argc, const char** argv)
{
    try
    {
        Arguments arguments(
            docopt::docopt(
                USAGE,
                { argv + 1, argv + argc },
                true,
                "orthoproject " + std::string(VERSION)
            )
        );
        std::cout << std::boolalpha << arguments << std::endl;

        if(arguments.formats)
        {
            std::cout << "Possible formats are: ";
            std::copy(
                std::begin(city::io::SceneHandler::supported_formats),
                std::prev(std::end(city::io::SceneHandler::supported_formats)),
                std::ostream_iterator<std::string>(std::cout, ", ")
            );
            std::cout << city::io::SceneHandler::supported_formats.back() << std::endl;
        }
        else
        {
            auto scene = city::io::SceneHandler(
                arguments.scene_args.input_path,
                std::map<std::string, bool>{{"read", true}},
                arguments.scene_args.input_format
            ).read();

            if(arguments.scene_args.prune)
                scene = scene.prune(arguments.scene_args.terrain);
            
            if(arguments.scene_args.graphs)
                city::save_building_duals(
                    arguments.scene_args.input_path.parent_path(),
                    scene
                );

            city::io::SceneHandler scene_writer(
                arguments.save_args.output_path,
                std::map<std::string, bool>{{"write", true}},
                arguments.save_args.output_format
            );
            scene_writer.write(scene);
        }
    }
    catch(std::exception const& except)
    {
        std::cerr << except.what() << std::flush << std::endl;
    }
    return EXIT_SUCCESS;
}
