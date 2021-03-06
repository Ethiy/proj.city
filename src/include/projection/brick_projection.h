#pragma once

#include <geometry_definitions.h>
#include <shadow/point.h>
#include <projection/face_projection.h>

#include <ogrsf_frmts.h>

#include <string>
#include <vector>

#include <ostream>

namespace city
{
    namespace projection
    {
        class BrickPrint
        {
        public:
            BrickPrint(void);
            BrickPrint(FacePrint const& face_projection);
            BrickPrint(OGRLayer* projection_layer);
            BrickPrint(BrickPrint const& other);
            BrickPrint(BrickPrint && other);
            ~BrickPrint(void);

            void swap(BrickPrint & other);
            BrickPrint & operator =(BrickPrint const& other);
            BrickPrint & operator =(BrickPrint && other);

            Bbox_2 const& bbox(void) const noexcept;
            std::size_t size(void) const noexcept;

            using iterator = std::vector<FacePrint>::iterator;
            using const_iterator = std::vector<FacePrint>::const_iterator;
            iterator begin(void) noexcept;
            iterator end(void) noexcept;
            const_iterator begin(void) const noexcept;
            const_iterator end(void) const noexcept;
            const_iterator cbegin(void) const noexcept;
            const_iterator cend(void) const noexcept;
            bool is_empty(void) const noexcept;

            bool in_domain(Point_2 const& point) const;
            bool contains(Point_2 const& point) const;
            bool contains(InexactPoint_2 const& inexact_point) const;
            bool contains(FacePrint const& facet) const;
            bool overlaps(Polygon const& polygon) const;
            bool overlaps(Polygon_with_holes const& polygon) const;
            bool overlaps(FacePrint const& facet) const;
            bool is_under(FacePrint const&) const;

            double get_height(Point_2 const&) const;
            double get_height(InexactPoint_2 const& inexact_point) const;

            std::vector<double> areas(void) const;
            double area(void) const;
            std::vector<double> edge_lengths(void) const;
            double circumference(void) const;
            
            void filter(void);

            BrickPrint & operator +=(FacePrint const& lfacet);
            BrickPrint & operator +=(BrickPrint const& other);

            std::vector<FacePrint> occlusion(FacePrint const& lfacet);

            void to_ogr(OGRLayer* projection_layer, shadow::Point const& reference_point, bool labels) const;
        private:
            Bbox_2 bounding_box;
            std::vector<FacePrint> projected_facets;

            bool equal_facets(BrickPrint const& other) const;

            friend std::ostream & operator <<(std::ostream & os, BrickPrint const& brick_projection);
            friend bool operator ==(BrickPrint const& lhs, BrickPrint const& rhs);
            friend bool operator !=(BrickPrint const& lhs, BrickPrint const& rhs);
        };

        BrickPrint operator +(BrickPrint const& lhs, BrickPrint const& rhs);
    }
    void swap(projection::BrickPrint & lhs, projection::BrickPrint & rhs);

    std::vector<double> areas(projection::BrickPrint const& brick_projection);
    double area(projection::BrickPrint const& brick_projection);
    std::vector<double> edge_lengths(projection::BrickPrint const& brick_projection);
    double circumference(projection::BrickPrint const& brick_projection);
}
