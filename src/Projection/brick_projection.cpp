#include "brick_projection.h"

#include <stdexcept>

namespace urban
{
    BrickProjection::BrickProjection(void){}
    BrickProjection::BrickProjection(const std::string & _name, const std::map<size_t, FaceProjection> & _facets_xy, const Bbox_2 & _bounding_box):name(_name), facets_xy(_facets_xy), bounding_box(_bounding_box){}
    BrickProjection::BrickProjection(const std::string & _name, const Bbox & _bounding_box):name(_name), bounding_box(Bbox_2(_bounding_box.xmin(), _bounding_box.ymin(), _bounding_box.xmax(), _bounding_box.ymax())){}
    BrickProjection::BrickProjection(BrickProjection & other):name(other.name), facets_xy(other.facets_xy), bounding_box(other.bounding_box){}
    BrickProjection::~BrickProjection(void){}

    void BrickProjection::set_name(const std::string & _name)
    {
        name = _name;
    }

    void BrickProjection::set_Bbox(const Bbox & _bounding_box)
    {
        bounding_box = Bbox_2(_bounding_box.xmin(), _bounding_box.ymin(), _bounding_box.xmax(), _bounding_box.ymax());
    }


    bool BrickProjection::in_domain(const Point_2 & point)
    {
        return to_double(point.x()) <= bounding_box.xmax() && to_double(point.x()) >= bounding_box.xmin() && to_double(point.y()) <= bounding_box.ymax() && to_double(point.y()) <= bounding_box.ymin();
    }

    double BrickProjection::get_height(const Point_2 & point)
    {
        if(in_domain(point))
            return std::accumulate(
                std::begin(facets_xy),
                std::end(facets_xy),
                .0,
                [point](double & height, const std::pair<size_t, FaceProjection> & p)
                {
                    FaceProjection facet(p.second);
                    return height + facet.get_height(point);
                }
            );
        else
            throw new std::out_of_range("The point is not insinde the bounding box");
    }
}