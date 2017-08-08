#include <algorithms/unode_algorithms.h>
#include <algorithms/util_algorithms.h>

#include <CGAL/aff_transformation_tags.h>

#include <stdexcept>

#include <iterator>
#include <algorithm>
#include <numeric>

#include <cmath>

namespace urban
{
    double border_length(scene::UNode & unode)
    {
        return std::accumulate(
            unode.border_halfedges_begin(),
            unode.halfedges_end(),
            .0,
            [](double & length, const Polyhedron::Halfedge & halfedge)
            {
                return length + std::sqrt(to_double(Vector_3(halfedge.next()->vertex()->point(), halfedge.vertex()->point()) * Vector_3(halfedge.next()->vertex()->point(), halfedge.vertex()->point())));
            }
        );
    }

    scene::UNode & affine_transform(scene::UNode & unode, const Affine_transformation_3 & affine_transformation)
    {
        std::transform(
            unode.points_begin(),
            unode.points_end(),
            unode.points_begin(),
            [& affine_transformation](Point_3 & point)
            {
                return affine_transformation.transform(point);
            }
        );
        return unode;
    }

    scene::UNode & translate(scene::UNode & unode, const Vector_3 & offset)
    {
        Affine_transformation_3 translation(CGAL::TRANSLATION, offset);
        return affine_transform(unode, translation);
    }

    scene::UNode & scale(scene::UNode & unode, double scale)
    {
        Affine_transformation_3 scaling(CGAL::SCALING, scale);
        return affine_transform(unode, scaling);
    }

    scene::UNode & rotate(scene::UNode & unode, const Vector_3 & axis, double angle)
    {
        std::map<double, Vector_3> _rotation{{angle, axis}};
        Affine_transformation_3 rotation(rotation_transform(_rotation));
        return affine_transform(unode, rotation);
    }

    scene::UNode & rotate(scene::UNode & unode, const std::map<double, Vector_3> & _rotations)
    {
        Affine_transformation_3 rotation(rotation_transform(_rotations));
        return affine_transform(unode, rotation);
    }


    void plane_equations(scene::UNode& unode)
    {
        std::transform(
            unode.facets_begin(),
            unode.facets_end(),
            unode.planes_begin(),
            [](scene::UNode::Facet & facet)
            {
                scene::UNode::Halfedge_handle halfedge = facet.halfedge();
                return scene::UNode::Facet::Plane_3(
                    halfedge->vertex()->point(),
                    halfedge->next()->vertex()->point(),
                    halfedge->next()->next()->vertex()->point()
                );
            }
        );
    }

    scene::UNode & prune(scene::UNode & unode)
    {
        std::vector<scene::UNode::Halfedge_handle> prunables = unode.pruning_halfedges();
        for(auto prunable : prunables)
        {
            unode = unode.join_facet(prunable);
        }
        return unode;
    }

    double area(scene::UNode& unode)
    {
        return std::accumulate(
            unode.facets_begin(),
            unode.facets_end(),
            .0,
            [&unode](double & area, scene::UNode::Facet & facet)
            {
                return area + unode.area(facet);
            }
        );
    }
}
