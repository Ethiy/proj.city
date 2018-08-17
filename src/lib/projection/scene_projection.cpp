#include <projection/scene_projection.h>
#include <projection/raster_projection.h>

#include <io/io_raster.h>

#include <projection/utilities.h>

#include <algorithm>
#include <numeric>
#include <iterator>

#include <tbb/parallel_for.h>
#include <tbb/parallel_reduce.h>
#include <tbb/blocked_range.h>


namespace city
{
    namespace projection
    {
        FootPrint::FootPrint(void)
        {}
        FootPrint::FootPrint(scene::UNode const& unode)
            : FootPrint(unode, orthobbox(unode.bbox()))
        {}
        FootPrint::FootPrint(scene::UNode const& unode, Bbox_2 const& mask)
            : name(unode.get_name()), reference_point(unode.get_reference_point()), epsg_index(unode.get_epsg())
        {
            std::cout << "Projecting node: " << unode.get_name() << std::endl;
            std::vector<FacePrint> prints = orthoprint(unode, mask);
            try
            {
                projection = std::accumulate(
                    std::begin(prints),
                    std::end(prints),
                    projection,
                    [](BrickPrint const& proj, FacePrint const& face_print)
                    {
                        return proj + face_print;
                    }
                );
            }
            catch(const std::exception& e)
            {
                std::cerr << e.what() << std::endl;
            }
        }
        FootPrint::FootPrint(std::string const& _name, OGRLayer* projection_layer)
            : name(_name), projection(projection_layer)
        {
            auto epsg_buffer = projection_layer->GetSpatialRef()->GetEPSGGeogCS();
            epsg_buffer > 0 ? epsg_index =  static_cast<unsigned short>(epsg_buffer) : epsg_index = 2154;
        }
        FootPrint::FootPrint(FootPrint const& other)
            : name(other.name), reference_point(other.reference_point), epsg_index(other.epsg_index), projection(other.projection)
        {}
        FootPrint::FootPrint(FootPrint && other)
            : name(std::move(other.name)), reference_point(std::move(other.reference_point)), epsg_index(std::move(other.epsg_index)), projection(std::move(other.projection))
        {}
        FootPrint::~FootPrint(void)
        {}

        void FootPrint::swap(FootPrint & other)
        {
            using std::swap;

            swap(name, other.name);
            swap(reference_point, other.reference_point);
            swap(epsg_index, other.epsg_index);
            swap(projection, other.projection);
        }
        FootPrint & FootPrint::operator =(FootPrint const& other)
        {
            name = other.name;
            reference_point = other.reference_point;
            epsg_index = other.epsg_index;
            projection = other.projection;

            return *this;
        }
        FootPrint & FootPrint::operator =(FootPrint && other)
        {
            name = std::move(other.name);
            reference_point = std::move(other.reference_point);
            epsg_index = std::move(other.epsg_index);
            projection = std::move(other.projection);

            return *this;
        }

        std::string const& FootPrint::get_name(void) const noexcept
        {
            return name;
        }
        shadow::Point const& FootPrint::get_reference_point(void) const noexcept
        {
            return reference_point;
        }
        unsigned short FootPrint::get_epsg(void) const noexcept
        {
            return epsg_index;
        }
        Bbox_2 const& FootPrint::bbox(void) const noexcept
        {
            return projection.bbox();
        }
        BrickPrint const& FootPrint::data(void) const noexcept
        {
            return projection;
        }

        std::vector<double> FootPrint::areas(void) const
        {
            return projection.areas();
        }
        double FootPrint::area(void) const
        {
            return projection.area();
        }
        std::vector<double> FootPrint::edge_lengths(void) const
        {
            return projection.edge_lengths();
        }        
        double FootPrint::circumference(void) const
        {
            return projection.circumference();
        }

        FootPrint::iterator FootPrint::begin(void) noexcept
        {
            return projection.begin();
        }
        FootPrint::iterator FootPrint::end(void) noexcept
        {
            return projection.end();
        }
        FootPrint::const_iterator FootPrint::begin(void) const noexcept
        {
            return projection.begin();
        }
        FootPrint::const_iterator FootPrint::end(void) const noexcept
        {
            return projection.end();
        }
        FootPrint::const_iterator FootPrint::cbegin(void) const noexcept
        {
            return projection.cbegin();
        }
        FootPrint::const_iterator FootPrint::cend(void) const noexcept
        {
            return projection.cend();
        }

        FootPrint & FootPrint::operator +=(FootPrint const& other)
        {
            if(projection.empty())
                *this = other;
            else
            {
                if(reference_point != other.reference_point || epsg_index != other.epsg_index)
                    throw std::logic_error("Feature not supported");
                
                projection += other.projection;
            }
            return *this;
        }

        void FootPrint::to_ogr(GDALDataset* file, bool labels) const
        {
            OGRSpatialReference spatial_reference_system;
            spatial_reference_system.importFromEPSG(epsg_index);

            OGRLayer* projection_layer = file->CreateLayer(name.c_str(), &spatial_reference_system, wkbPolygon, nullptr);

            if(projection_layer == nullptr)
                throw std::runtime_error("GDAL could not create a projection layer!");
            projection.to_ogr(projection_layer, reference_point, labels);            
        }
        
        std::vector<double> FootPrint::rasterize(std::vector<double> const& image, shadow::Point const& top_left, std::size_t const height, std::size_t const width, double const pixel_size) const
        {
            return std::accumulate(
                std::begin(projection),
                std::end(projection),
                image,
                [top_left, height, width, pixel_size](std::vector<double> const& _image, projection::FacePrint const& face_projection)
                {
                    return face_projection.rasterize(_image, top_left, height, width, pixel_size);
                }
            );
        }

        std::ostream & operator <<(std::ostream & os, FootPrint const& footprint)
        {
            os << "Name: " << footprint.name << std::endl
               << "Reference Point: " << footprint.reference_point << std::endl
               << "EPSG index: " << footprint.epsg_index << std::endl
               << footprint.projection;
            
            return os;
        }
        bool operator ==(FootPrint const& lhs, FootPrint const& rhs)
        {
            return  lhs.reference_point == rhs.reference_point
                    &&
                    lhs.epsg_index == rhs.epsg_index
                    &&
                    lhs.projection == rhs.projection;
        }
        bool operator !=(FootPrint const& lhs, FootPrint const& rhs)
        {
            return !(lhs == rhs);
        }

        FootPrint operator +(FootPrint const& lhs, FootPrint const& rhs)
        {
            FootPrint result(lhs);
            return result += rhs;
        }


        ScenePrint::ScenePrint(void)
        {}
        ScenePrint::ScenePrint(scene::Scene const& scene)
            : pivot(scene.get_pivot()),
              epsg_index(scene.get_epsg()),
              buildings(scene::orthoproject(scene)),
              terrain(scene.get_terrain())
        {}
        ScenePrint::ScenePrint(ScenePrint const& other)
            : pivot(other.pivot), epsg_index(other.epsg_index), buildings(other.buildings), terrain(other.terrain)
        {}
        ScenePrint::ScenePrint(ScenePrint && other)
            : pivot(std::move(other.pivot)), epsg_index(std::move(other.epsg_index)), buildings(std::move(other.buildings)), terrain(std::move(other.terrain))
        {}
        ScenePrint::~ScenePrint(void)
        {}

        void ScenePrint::swap(ScenePrint & other)
        {
            using std::swap;

            swap(pivot, other.pivot);
            swap(epsg_index, other.epsg_index);
            swap(buildings, other.buildings);
            swap(terrain, other.terrain);
        }
        ScenePrint& ScenePrint::operator =(ScenePrint const& other)
        {
            pivot = other.pivot;
            epsg_index = other.epsg_index;
            buildings = other.buildings;
            terrain = other.terrain;

            return *this;
        }
        ScenePrint& ScenePrint::operator =(ScenePrint && other)
        {
            pivot = std::move(other.pivot);
            epsg_index = std::move(other.epsg_index);
            buildings = std::move(other.buildings);
            terrain = std::move(other.terrain);

            return *this;
        }

        Bbox_2 ScenePrint::bbox(void) const
        {
            return std::accumulate(
                std::begin(buildings),
                std::end(buildings),
                Bbox_2(),
                [](Bbox_2 const& bb, FootPrint const& nodeprint)
                {
                    return bb + nodeprint.bbox();
                }
            );
        }

        std::vector<double> ScenePrint::areas(void) const
        {
            std::vector<double> _areas(buildings.size());
            std::transform(
                std::begin(buildings),
                std::end(buildings),
                std::begin(_areas),
                [](FootPrint const& nodeprint)
                {
                    return nodeprint.area();
                }
            );
            return _areas;
        }
        std::vector<double> ScenePrint::circumferences(void) const
        {
            std::vector<double> _circumferences(buildings.size());
            std::transform(
                std::begin(buildings),
                std::end(buildings),
                std::begin(_circumferences),
                [](FootPrint const& nodeprint)
                {
                    return nodeprint.circumference();
                }
            );
            return _circumferences;
        }

        std::vector<RasterPrint> rasterize(ScenePrint const& scene_projection, double const pixel_size)
        {
            std::cout << "rasterizing projections... " << std::flush;
            std::vector<RasterPrint> raster_projections(scene_projection.size());
            std::transform(
                std::begin(scene_projection),
                std::end(scene_projection),
                std::begin(raster_projections),
                [pixel_size, &scene_projection](FootPrint const& building)
                {
                    return RasterPrint(
                        building,
                        pixel_size,
                        FootPrint(scene_projection.get_terrain(), building.bbox())
                    );
                }
            );
            std::cout << "Done." << std::flush << std::endl;
            return raster_projections;
        }

        void rasterize_and_save(ScenePrint const& scene_projection, double const pixel_size, boost::filesystem::path const& root_path)
        {
            std::cout << "rasterizing projections... " << std::endl;
            boost::filesystem::path raster_dir(root_path / "rasters");
            boost::filesystem::create_directory(raster_dir);
            for(auto const& building : scene_projection)
            {
                RasterPrint raster(
                    building,
                    pixel_size,
                    FootPrint(scene_projection.get_terrain(), building.bbox())
                );
                city::io::RasterHandler(
                    boost::filesystem::path(raster_dir / (raster.get_name() + ".tiff")),
                    std::map<std::string,bool>{{"write", true}}
                ).write(raster);
            }
            std::cout << "Done." << std::flush << std::endl;
        }
    }

    void swap(projection::FootPrint & lhs, projection::FootPrint & rhs)
    {
        lhs.swap(rhs);
    }

    std::vector<double> areas(projection::FootPrint const& footprint)
    {
        return footprint.areas();
    }
    double area(projection::FootPrint const& footprint)
    {
        return footprint.area();
    }
    std::vector<double> edge_lengths(projection::FootPrint const& footprint)
    {
        return footprint.edge_lengths();
    }
    double circumference(projection::FootPrint const& footprint)
    {
        return footprint.circumference();
    }
}
