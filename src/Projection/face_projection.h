#pragma once

#include "../geometry_definitions.h"

namespace urban
{
    class FaceProjection : Polygon
    {
    public:
        Plane get_plane(void) const noexcept;
        Vector get_normal(void) const noexcept;
        double get_height(const Point_2 &); // problem: degenerate faces and intersections

        bool is_inside(const Point_2 &);

        void set_plane(const Plane & _plane) noexcept;
    private:
        Plane original_plane;
    };
}