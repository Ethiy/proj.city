#include <io/io_obj.h>

#include <io/Obj_stream/obj_stream.h>

#include <fstream>
#include <sstream>

namespace city
{
    namespace io
    {
        WaveObjHandler::WaveObjHandler(boost::filesystem::path const& _filepath, std::map<std::string, bool> const& _modes)
            : FileHandler(_filepath, _modes)
        {}
        WaveObjHandler::WaveObjHandler(boost::filesystem::path const& _filepath, std::vector<shadow::Mesh> const& _meshes, std::map<std::string, bool> const& _modes)
            : FileHandler(_filepath, _modes), meshes(_meshes)
        {}
        WaveObjHandler::WaveObjHandler(boost::filesystem::path const& _filepath, scene::Scene const& scene, std::map<std::string, bool> const& _modes)
            : WaveObjHandler(_filepath, std::vector<shadow::Mesh>(scene.size() + 1), _modes)
        {
            std::transform(
                std::begin(scene),
                std::end(scene),
                std::begin(meshes),
                [](scene::UNode const& unode)
                {
                    return shadow::Mesh(unode);
                }
            );
            meshes.back() = shadow::Mesh(scene.get_terrain());
        }
        WaveObjHandler::~WaveObjHandler(void) {}

        std::vector<shadow::Mesh> const& WaveObjHandler::data(void) const noexcept
        {
            return meshes;
        }

        WaveObjHandler& WaveObjHandler::read(void)
        {
            std::ostringstream error_message;

            if (modes["read"])
            {
                if (boost::filesystem::is_regular_file(filepath))
                {
                    std::fstream obj_file(filepath.string(), std::ios::in);
                    Obj_stream object_stream(obj_file);
                    object_stream >> meshes;
                }
                else
                {
                    error_message << "This file: " << filepath.string() << " cannot be found! You should check the file path.";
                    boost::system::error_code ec(boost::system::errc::no_such_file_or_directory, boost::system::system_category());
                    throw boost::filesystem::filesystem_error(error_message.str(), ec);
                }
            }
            else
            {
                error_message << std::boolalpha << "The read mode is set to:" << modes["read"] << "! You should set it as follows: \'modes[\"read\"] = true\'";
                boost::system::error_code ec(boost::system::errc::io_error, boost::system::system_category());
                throw boost::filesystem::filesystem_error(error_message.str(), ec);
            }
            return *this;
        }

        void WaveObjHandler::write(void)
        {
            if (modes["write"])
            {
                    std::fstream obj_file(filepath.string(), std::ios::out);
                    Obj_stream object_stream(obj_file);
                    object_stream << meshes;
            }
            else
            {
                std::ostringstream error_message;
                error_message << std::boolalpha << "The write mode is set to:" << modes["write"] << "! You should set it as follows: \'modes[\"write\"] = true\'";
                boost::system::error_code ec(boost::system::errc::io_error, boost::system::system_category());
                throw boost::filesystem::filesystem_error(error_message.str(), ec);
            }
        }

        shadow::Mesh WaveObjHandler::exclude_mesh(std::string const& excluded)
        {
            auto part = std::stable_partition(
                std::begin(meshes),
                std::end(meshes),
                [excluded](shadow::Mesh const& mesh)
                {
                    return mesh.get_name() != excluded;
                }
            );
            std::vector<shadow::Mesh> excluded_meshes(
                std::make_move_iterator(part),
                std::make_move_iterator(std::end(meshes))
            );
            meshes.erase(part, std::end(meshes));
            return std::accumulate(
                std::begin(excluded_meshes),
                std::end(excluded_meshes),
                shadow::Mesh()
            ).set_name(excluded);
        }
        void WaveObjHandler::add_mesh(shadow::Mesh const& mesh)
        {
            meshes.push_back(mesh);
        }

        scene::Scene WaveObjHandler::get_scene(void)
        {
            auto terrain = read().exclude_mesh("terrain");
            return scene::Scene(
                meshes,
                terrain
            );
        }
    }
}
