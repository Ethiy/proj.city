#include "brick_projection.h"

#include "../../algorithms/projection/projection_algorithms.h"
#include "../../algorithms/ogr/ogr_algorithms.h"

#include <CGAL/Boolean_set_operations_2.h>

#include <list>
#include <iterator>
#include <algorithm>

#include <stdexcept>

namespace urban
{
    namespace projection
    {
        BrickPrint::BrickPrint(void):name("N/A"), projected_surface(), bounding_box(){}
        BrickPrint::BrickPrint(const std::string & _name, const Bbox_3 & _bounding_box):name(_name + "_projected_xy"), bounding_box(Bbox_2(_bounding_box.xmin(), _bounding_box.ymin(), _bounding_box.xmax(), _bounding_box.ymax())){}
        BrickPrint::BrickPrint(const std::string & _name, OGRLayer* projection_layer): name(_name), projected_surface(), bounding_box()
        {
            projection_layer->ResetReading();

            OGRFeature* ogr_facet;
            while((ogr_facet = projection_layer->GetNextFeature()) != NULL)
            {
                FacePrint facet(ogr_facet, projection_layer->GetLayerDefn());
                projected_facets.push_back(facet);
                OGRFeature::DestroyFeature(ogr_facet);
                projected_surface.join(facet.get_polygon());
                bounding_box += facet.bbox();
            }
        }
        BrickPrint::BrickPrint(const FacePrint & face_projection):name("contains_only_one_facet"), projected_facets(std::list<FacePrint>{{face_projection}}), projected_surface(Polygon_set(face_projection.get_polygon())) , bounding_box(face_projection.bbox()) {}
        BrickPrint::BrickPrint(const BrickPrint & other):name(other.name), projected_facets(other.projected_facets), projected_surface(other.projected_surface), bounding_box(other.bounding_box){}
        BrickPrint::BrickPrint(BrickPrint && other):name(std::move(other.name)), projected_facets(std::move(other.projected_facets)), projected_surface(std::move(other.projected_surface)), bounding_box(std::move(other.bounding_box)){}
        BrickPrint::~BrickPrint(void){}

        void BrickPrint::swap(BrickPrint & other)
        {
            using std::swap;
            swap(name, other.name);
            swap(projected_facets, other.projected_facets);
            swap(projected_surface, other.projected_surface);
            swap(bounding_box, other.bounding_box);
        }
            
        BrickPrint & BrickPrint::operator=(const BrickPrint & other)
        {
            name = other.name;
            projected_facets = std::move(other.projected_facets);
            projected_surface = std::move(other.projected_surface);
            bounding_box = std::move(other.bounding_box);
            return *this;
        }

        BrickPrint & BrickPrint::operator=(BrickPrint && other)
        {
            name = std::move(other.name);
            projected_facets.resize(other.projected_facets.size());
            std::copy(std::begin(other.projected_facets), std::end(other.projected_facets), std::begin(projected_facets));
            projected_surface = std::move(other.projected_surface);
            bounding_box = std::move(other.bounding_box);
            return *this;
        }

        BrickPrint & BrickPrint::operator+=(const BrickPrint & other)
        {
            name += "_" + other.name;
            if(projected_surface.do_intersect(other.projected_surface))
            {
                std::for_each(
                    std::begin(other.projected_facets),
                    std::end(other.projected_facets),
                    [this](const FacePrint & facet)
                    {
                        insert(facet);
                    }
                );
            }
            else
            {
                std::copy(std::begin(other.projected_facets), std::end(other.projected_facets), std::back_inserter(projected_facets));
            }
            projected_surface.join(other.projected_surface);
            bounding_box += other.bounding_box;
            return *this;
        }


        Bbox_2 BrickPrint::bbox(void) const noexcept
        {
            return bounding_box;
        }
        
        std::string BrickPrint::get_name(void) const noexcept
        {
            return name;
        }

        size_t BrickPrint::size(void) const noexcept
        {
            return projected_facets.size();
        }


        BrickPrint::iterator BrickPrint::begin(void) noexcept
        {
            return projected_facets.begin();
        }
        BrickPrint::iterator BrickPrint::end(void) noexcept
        {
            return projected_facets.end();
        }
        BrickPrint::const_iterator BrickPrint::cbegin(void) const noexcept
        {
            return projected_facets.cbegin();
        }
        BrickPrint::const_iterator BrickPrint::cend(void) const noexcept
        {
            return projected_facets.cend();
        }


        bool BrickPrint::contains(const Point_2 & point) const
        {
            return projected_surface.oriented_side(point) != CGAL::ON_NEGATIVE_SIDE;
        }

        bool BrickPrint::contains(const InexactPoint_2 & inexact_point) const
        {
            InexactToExact to_exact;
            return contains(to_exact(inexact_point));
        }

        bool BrickPrint::in_domain(const Point_2 & point) const
        {
            return CGAL::do_overlap(bounding_box, point.bbox());
        }

        bool BrickPrint::contains(const FacePrint & facet) const
        {
            Polygon_set shallow_copy(projected_surface);
            shallow_copy.intersection(facet.get_polygon());
            std::list<Polygon_with_holes> _inter;
            shallow_copy.polygons_with_holes(std::back_inserter(_inter));
            return _inter.size() == 1 && _inter.front() == facet.get_polygon();
        }

        bool BrickPrint::overlaps(const Polygon & polygon) const
        {
            Polygon_set shallow_copy(projected_surface);
            shallow_copy.intersection(polygon);
            return !shallow_copy.is_empty();
        }
        
        bool BrickPrint::overlaps(const Polygon_with_holes & polygon) const
        {
            Polygon_set shallow_copy(projected_surface);
            shallow_copy.intersection(polygon);
            return !shallow_copy.is_empty();
        }

        bool BrickPrint::overlaps(const FacePrint & facet) const
        {
            return overlaps(facet.get_polygon());
        }

        bool BrickPrint::is_under(const FacePrint & facet) const
        {
            bool under(false);

            if(facet.is_perpendicular() || facet.is_degenerate())
                under = true;
            else
            {
                Point_2 point(
                    CGAL::centroid(
                        facet.outer_boundary()[0],
                        facet.outer_boundary()[1],
                        facet.outer_boundary()[2]
                    )
                );
                under = contains(facet) && facet.get_height(point) <= get_height(point); /** there are no intersections */
            }
            return under;
        }
        
        bool BrickPrint::check_integrity(void) const
        {
            Polygon_set diff(projected_surface);
            std::for_each(
                std::begin(projected_facets),
                std::end(projected_facets),
                [&diff](const FacePrint & facet)
                {
                    return diff.difference(facet.get_polygon());
                }
            );
            return diff.is_empty();
        }

        bool BrickPrint::has_same_footprint(const BrickPrint & other) const
        {
            Polygon_set l_copy(projected_surface),
                        r_copy(other.projected_surface);
            l_copy.symmetric_difference(r_copy);
            return l_copy.is_empty();
        }
        bool BrickPrint::has_same_facets(const BrickPrint & other) const
        {
            bool result(false);
            if(projected_facets.size() == other.projected_facets.size())
            {
                std::vector<bool> results(projected_facets.size());
                std::transform(
                    std::begin(projected_facets),
                    std::end(projected_facets),
                    std::begin(other.projected_facets),
                    std::begin(results),
                    [](const FacePrint & l_face, const FacePrint & r_face)
                    {
                        return l_face == r_face;
                    }
                );
                result = std::accumulate(
                    std::begin(results),
                    std::end(results),
                    true,
                    [](bool & all, const bool r)
                    {
                        return all && r;
                    }
                );
            }
            return result;
        }

        void BrickPrint::insert(const FacePrint & new_facet)
        {
            if(projected_facets.empty())
                projected_facets.push_back(new_facet);
            else
            {
                /* If new_facet does not intersect the surface we push it directly*/
                if(!overlaps(new_facet))
                    projected_facets.push_back(new_facet);
                else
                {
                    /* If new_facet is under the surface we loose it*/
                    if(!is_under(new_facet))
                    {
                        std::list<FacePrint> result;
                        std::list<FacePrint> new_facets{new_facet};
                        std::for_each(
                            std::begin(projected_facets),
                            std::end(projected_facets),
                            [&result, &new_facets](FacePrint & facet)
                            {
                                std::list<FacePrint> occlusion_results(occlusion(facet, new_facets));
                                result.splice(std::end(result), occlusion_results);
                            }
                        );
                        result.splice(std::end(result), new_facets);
                        projected_facets = std::move(result);
                    }
                }
            }
            projected_surface.join(new_facet.get_polygon());
        }


        double BrickPrint::get_height(const Point_2 & point) const
        {
            double height(0);
            if(contains(point))
                height  = std::accumulate(
                    std::begin(projected_facets),
                    std::end(projected_facets),
                    .0,
                    [point](double & result_height, const FacePrint & facet)
                    {
                        return result_height + facet.get_height(point);
                    }
                );
            return height;
        }

        double BrickPrint::get_height(const InexactPoint_2 & inexact_point) const
        {
            double height(0);
            if(contains(inexact_point))
                height  = std::accumulate(
                    std::begin(projected_facets),
                    std::end(projected_facets),
                    .0,
                    [inexact_point](double & result_height, const FacePrint & facet)
                    {
                        return result_height + facet.get_height(inexact_point);
                    }
                );
            return height;
        }

        double BrickPrint::get_mean_height(const Polygon & window) const
        {
            double height(0);
            if(overlaps(window))
            {
                std::for_each(
                    std::begin(projected_facets),
                    std::end(projected_facets),
                    [&height, &window](const FacePrint & facet)
                    {
                        std::list<Polygon_with_holes> _inter;
                        CGAL::intersection(facet.get_polygon(), window, std::back_inserter(_inter));
                        height += 0; // how to compute integral in general?
                        throw std::logic_error("Not yet implemented.");
                    }
                );
            }
            return height;
        }

        void BrickPrint::to_ogr(GDALDataset* file) const
        {
            OGRLayer* projection_layer = file->CreateLayer("projection_xy", NULL, wkbPolygon, NULL);
            if(projection_layer == NULL)
                throw std::runtime_error("GDAL could not create a projection layer");
            
            OGRFieldDefn* plane_coefficient_a = new OGRFieldDefn("coeff_a", OFTReal);
            projection_layer->CreateField(plane_coefficient_a);
            OGRFieldDefn* plane_coefficient_b = new OGRFieldDefn("coeff_b", OFTReal);
            projection_layer->CreateField(plane_coefficient_b);
            OGRFieldDefn* plane_coefficient_c = new OGRFieldDefn("coeff_c", OFTReal);
            projection_layer->CreateField(plane_coefficient_c);
            OGRFieldDefn* plane_coefficient_d = new OGRFieldDefn("coeff_d", OFTReal);
            projection_layer->CreateField(plane_coefficient_d);
            
            std::for_each(
                std::begin(projected_facets),
                std::end(projected_facets),
                [projection_layer](const projection::FacePrint & facet)
                {
                    OGRFeature* ogr_facet = facet.to_ogr(projection_layer->GetLayerDefn());
                    if(projection_layer->CreateFeature(ogr_facet) != OGRERR_NONE)
                        throw std::runtime_error("GDAL could not insert the facet in shapefile");
                    OGRFeature::DestroyFeature(ogr_facet);
                }
            );
        }

        std::ostream & operator<<(std::ostream & os, const BrickPrint & brick_projection)
        {
            os << "Name: " << brick_projection.name << std::endl
               << "Bounding box: " << brick_projection.bounding_box << std::endl
               << "Face Projections: " << brick_projection.projected_facets.size() << std::endl;
            std::copy(std::begin(brick_projection.projected_facets), std::end(brick_projection.projected_facets), std::ostream_iterator<FacePrint>(os, "\n"));
            std::vector<Polygon_with_holes> copy;
            brick_projection.projected_surface.polygons_with_holes(std::back_inserter(copy));
            os << "Projected surface: " << std::endl;
            std::copy(std::begin(copy), std::end(copy), std::ostream_iterator<Polygon_with_holes>(os, "\n"));
            return os;
        }

        BrickPrint & operator+(BrickPrint & lhs, const BrickPrint & rhs)
        {
            return lhs += rhs;
        }

        bool operator==(const BrickPrint & lhs, const BrickPrint & rhs)
        {
            return  lhs.has_same_footprint(rhs) && lhs.has_same_facets(rhs);
        }

        bool operator!=(const BrickPrint & lhs, const BrickPrint & rhs)
        {
            return !(lhs == rhs);
        }
    }

    void swap(projection::BrickPrint & lhs, projection::BrickPrint & rhs)
    {
        lhs.swap(rhs);
    }
}