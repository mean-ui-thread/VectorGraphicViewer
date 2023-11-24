#include "VectorGraphic.h"

#define MPE_POLY2TRI_IMPLEMENTATION
#include <MPE_fastpoly2tri.h>


#include <glm/glm.hpp>
#include <glm/gtc/epsilon.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/exterior_product.hpp>

static constexpr float distTol = 0.01f;
static constexpr float tessTol = 0.25f;
static constexpr size_t BEZIER_RECURSION_LIMIT = 32;

namespace detail
{

    inline void addPoint(std::vector<glm::vec2> &points, glm::vec2 pos, PointProperties = PointProperties::none)
    {
        if (points.size() > 0)
        {
            if (glm::all(glm::epsilonEqual(points.back(), pos, distTol)))
            {
                return;
            }
        }

        points.push_back(pos);
    }

    inline void addPoint(std::vector<ContourPoint> &points, glm::vec2 pos, PointProperties type)
    {
        if (points.size() > 0)
        {
            if (glm::all(glm::epsilonEqual(points.back().pos, pos, distTol)))
            {
                return;
            }
        }
        points.push_back({pos, {}, {}, 0.0f, type});
    }


    // based of http://antigrain.com/__code/src/agg_curves.cpp.html by Maxim Shemanarev


    inline float calcSqrtDistance(float x1, float y1, float x2, float y2)
    {
        float dx = x2-x1;
        float dy = y2-y1;
        return dx * dx + dy * dy;
    }

    template<typename PointArray>
    inline void recursiveBezier(PointArray &points,
                                float x1, float y1,
                                float x2, float y2,
                                float x3, float y3,
                                float x4, float y4,
                                int32_t level)
    {
        if(level > BEZIER_RECURSION_LIMIT)
        {
            return;
        }

        // Calculate all the mid-points of the line segments
        //----------------------
        float x12   = (x1 + x2) / 2;
        float y12   = (y1 + y2) / 2;
        float x23   = (x2 + x3) / 2;
        float y23   = (y2 + y3) / 2;
        float x34   = (x3 + x4) / 2;
        float y34   = (y3 + y4) / 2;
        float x123  = (x12 + x23) / 2;
        float y123  = (y12 + y23) / 2;
        float x234  = (x23 + x34) / 2;
        float y234  = (y23 + y34) / 2;
        float x1234 = (x123 + x234) / 2;
        float y1234 = (y123 + y234) / 2;

        // Try to approximate the full cubic curve by a single straight line
        //------------------
        float dx = x4-x1;
        float dy = y4-y1;

        float d2 = glm::abs(((x2 - x4) * dy - (y2 - y4) * dx));
        float d3 = glm::abs(((x3 - x4) * dy - (y3 - y4) * dx));

        switch((int(d2 > glm::epsilon<float>()) << 1) + int(d3 > glm::epsilon<float>()))
        {
            case 0:
            {
                // All collinear OR p1==p4
                //----------------------
                float k = dx*dx + dy*dy;
                if(glm::epsilonEqual(k, 0.0f, glm::epsilon<float>()))
                {
                    d2 = calcSqrtDistance(x1, y1, x2, y2);
                    d3 = calcSqrtDistance(x4, y4, x3, y3);
                }
                else
                {
                    k   = 1 / k;
                    float da1 = x2 - x1;
                    float da2 = y2 - y1;
                    d2  = k * (da1*dx + da2*dy);
                    da1 = x3 - x1;
                    da2 = y3 - y1;
                    d3  = k * (da1*dx + da2*dy);
                    if(d2 > 0 && d2 < 1 && d3 > 0 && d3 < 1)
                    {
                        // Simple collinear case, 1---2---3---4
                        // We can leave just two endpoints
                        return;
                    }
                    if(d2 <= 0) d2 = calcSqrtDistance(x2, y2, x1, y1);
                    else if(d2 >= 1) d2 = calcSqrtDistance(x2, y2, x4, y4);
                    else             d2 = calcSqrtDistance(x2, y2, x1 + d2*dx, y1 + d2*dy);

                    if(d3 <= 0) d3 = calcSqrtDistance(x3, y3, x1, y1);
                    else if(d3 >= 1) d3 = calcSqrtDistance(x3, y3, x4, y4);
                    else             d3 = calcSqrtDistance(x3, y3, x1 + d3*dx, y1 + d3*dy);
                }
                if(d2 > d3)
                {
                    if(d2 < tessTol)
                    {
                        addPoint(points, glm::vec2(x2, y2), PointProperties::none);
                        return;
                    }
                }
                else
                {
                    if(d3 < tessTol)
                    {
                        addPoint(points, glm::vec2(x3, y3), PointProperties::none);
                        return;
                    }
                }
                break;
            }
            case 1:
                // p1,p2,p4 are collinear, p3 is significant
                //----------------------
                if(d3 * d3 <= tessTol * (dx*dx + dy*dy))
                {
                    addPoint(points, glm::vec2(x23, y23), PointProperties::none);
                    return;
                }
                break;

            case 2:
                // p1,p3,p4 are collinear, p2 is significant
                //----------------------
                if(d2 * d2 <= tessTol * (dx*dx + dy*dy))
                {
                    addPoint(points, glm::vec2(x23, y23), PointProperties::none);
                    return;
                }
                break;

            case 3:
                // Regular case
                //-----------------
                if((d2 + d3)*(d2 + d3) <= tessTol * (dx*dx + dy*dy))
                {
                    addPoint(points, glm::vec2(x23, y23), PointProperties::none);
                    return;
                }
                break;
        }

        // Continue subdivision
        //----------------------
        recursiveBezier(points, x1, y1, x12, y12, x123, y123, x1234, y1234, level + 1);
        recursiveBezier(points, x1234, y1234, x234, y234, x34, y34, x4, y4, level + 1);
    }

    template<typename PointArray>
    inline void bezierTo(PointArray &points,
                            float x1, float y1,
                            float x2, float y2,
                            float x3, float y3,
                            float x4, float y4)
    {
        recursiveBezier(points, x1, y1, x2, y2, x3, y3, x4, y4, 0);
        addPoint(points, glm::vec2(x4, y4), PointProperties::corner);
    }

    template <typename PointArray>
    inline void arc(PointArray &points, glm::vec2 center, float radius,
                    float startAngle, float endAngle, bool anticlockwise)
    {
        float deltaAngle = endAngle - startAngle;

        if (anticlockwise)
        {
            if (glm::abs(deltaAngle) < glm::two_pi<float>())
            {
                while (deltaAngle > 0.0f)
                {
                    deltaAngle -= glm::two_pi<float>();
                }
            }
            else
            {
                deltaAngle = -glm::two_pi<float>();
            }
        }
        else
        {
            if (glm::abs(deltaAngle) < glm::two_pi<float>())
            {
                while (deltaAngle < 0.0f)
                {
                    deltaAngle += glm::two_pi<float>();
                }
            }
            else
            {
                deltaAngle = glm::two_pi<float>();
            }
        }

        int32_t segmentCount = static_cast<int32_t>(glm::ceil( glm::abs(deltaAngle) / glm::half_pi<float>()));

        float midAngle = (deltaAngle / segmentCount) * 0.5f;
        float tangentFactor = glm::abs(4.0f / 3.0f * (1.0f - glm::cos(midAngle)) / glm::sin(midAngle));

        if (anticlockwise)
        {
            tangentFactor = -tangentFactor;
        }

        glm::vec2 dir(glm::cos(startAngle), glm::sin(startAngle));
        glm::vec2 prev = center + dir * radius;
        glm::vec2 prevTan(-dir.y * tangentFactor * radius, dir.x * tangentFactor * radius);
        addPoint(points, prev, PointProperties::corner);

        for (int32_t segment = 1; segment <= segmentCount; ++segment)
        {
            float angle = startAngle + deltaAngle * (static_cast<float>(segment)/static_cast<float>(segmentCount));
            dir = glm::vec2(glm::cos(angle), glm::sin(angle));
            glm::vec2 pos = center + dir * radius;
            glm::vec2 tan(-dir.y * tangentFactor * radius, dir.x * tangentFactor * radius);

            bezierTo(points,
                        prev.x, prev.y, // start point
                        prev.x + prevTan.x, prev.y + prevTan.y, // control point 1
                        pos.x - tan.x, pos.y - tan.y, // control point 2
                        pos.x, pos.y); // end point

            prev = pos;
            prevTan = tan;
        }
    }

    inline float distPtSeg(const glm::vec2 &c, const glm::vec2 &p, const glm::vec2 &q)
    {
        glm::vec2 pq = q - p;
        glm::vec2 pc = c - p;
        float d = glm::dot(pq, pq);
        float t = glm::dot(pq, pc);

        if (d > 0)
        {
            t /= d;
        }

        t = glm::clamp(t, 0.0f, 1.0f);

        pc = p + (t*pq) - c;

        return glm::dot(pc, pc);
    }

    template <typename PointArray>
    inline void arcTo(PointArray &points, glm::vec2 p0, glm::vec2 p1, glm::vec2 p2, float radius)
    {
        if (glm::all(glm::epsilonEqual(p0, p1, distTol)))
        {
            addPoint(points, p1, PointProperties::corner);
            return;
        }

        if (glm::all(glm::epsilonEqual(p1, p2, distTol)))
        {
            addPoint(points, p1, PointProperties::corner);
            return;
        }

        if (distPtSeg(p1, p0, p2) < (distTol * distTol))
        {
            addPoint(points, p1, PointProperties::corner);
            return;
        }

        if (radius < distTol)
        {
            addPoint(points, p1, PointProperties::corner);
            return;
        }

        glm::vec2 d0 = glm::normalize(p0 - p1);
        glm::vec2 d1 = glm::normalize(p2 - p1);
        float a = glm::acos(glm::dot(d0, d1));
        float d = radius / glm::tan(a * 0.5f);

        if (d > 10000.0f)
        {
            addPoint(points, p1, PointProperties::corner);
            return;
        }

        float cp = d1.x * d0.y - d0.x * d1.y;

        glm::vec2 c;
        float a0, a1;
        bool anticlockwise;
        if (cp > 0.0f)
        {
            c.x = p1.x + d0.x * d + d0.y * radius;
            c.y = p1.y + d0.y * d + -d0.x * radius;
            a0 = glm::atan(d0.x, -d0.y);
            a1 = glm::atan(-d1.x, d1.y);
            anticlockwise = false;
        }
        else
        {
            c.x = p1.x + d0.x * d + -d0.y * radius;
            c.y = p1.y + d0.y * d + d0.x * radius;
            a0 = glm::atan(-d0.x, d0.y);
            a1 = glm::atan(d1.x, -d1.y);
            anticlockwise = true;
        }

        arc(points, c, radius, a0, a1, anticlockwise);
    }
}

void Path2D::reset() {
    subPaths.resize(0);
}

void Path2D::closePath() {
    if (subPaths.size() > 0) {
        subPaths.back().closed = true;
    }
}

void Path2D::moveTo(float x, float y) {
    SubPath2D &subPath = createSubPath();
    detail::addPoint(subPath.points, glm::vec2(x, y), PointProperties::corner);
}

void Path2D::lineTo(float x, float y) {
    SubPath2D &subPath = getCurrentSubPath();
    detail::addPoint(subPath.points, glm::vec2(x, y), PointProperties::corner);
}

void Path2D::bezierCurveTo(float cp1x, float cp1y, float cp2x, float cp2y, float x, float y) {
    SubPath2D &subPath = getCurrentSubPath();
    auto &points = subPath.points;
    auto &prevPoint = points.back().pos;
    detail::bezierTo(points, prevPoint.x, prevPoint.y, cp1x, cp1y, cp2x, cp2y, x, y);
}

void Path2D::quadraticCurveTo(float cpx, float cpy, float x, float y) {
    SubPath2D &subPath = getCurrentSubPath();
    auto &points = subPath.points;
    auto &prevPoint = points.back().pos;
    float c1x = prevPoint.x + 2.0f/3.0f*(cpx - prevPoint.x);
    float c1y = prevPoint.y + 2.0f/3.0f*(cpy - prevPoint.y);
    float c2x = x + 2.0f/3.0f*(cpx - x);
    float c2y = y + 2.0f/3.0f*(cpy - y);
    detail::bezierTo(points, prevPoint.x, prevPoint.y, c1x, c1y, c2x, c2y, x, y);
}

void Path2D::arc(float x, float y, float radius, float startAngle, float endAngle, bool anticlockwise) {
    SubPath2D &subPath = getCurrentSubPath(false);
    detail::arc(subPath.points, glm::vec2(x, y), radius, startAngle, endAngle, anticlockwise);
}

void Path2D::arcTo(float x1, float y1, float x2, float y2, float radius) {
    SubPath2D &subPath = getCurrentSubPath();
    auto &points = subPath.points;
    auto &prevPoint = points.back().pos;
    detail::arcTo(points, prevPoint, glm::vec2(x1, y1), glm::vec2(x2, y2), radius);
}

void Path2D::ellipse(float x, float y, float radiusX, float radiusY, float rotation, float startAngle, float endAngle, bool anticlockwise) {
    // TODO
    assert(false);
}

void Path2D::rect(float x, float y, float width, float height) {
    closePath();
    SubPath2D &subPath = createSubPath();
    auto &points = subPath.points;
    detail::addPoint(points, glm::vec2(x, y), PointProperties::corner);
    detail::addPoint(points, glm::vec2(x, y+height), PointProperties::corner);
    detail::addPoint(points, glm::vec2(x+width, y+height), PointProperties::corner);
    detail::addPoint(points, glm::vec2(x+width, y), PointProperties::corner);
    subPath.closed = true;
}

std::vector<Mesh> Path2D::fill() {

    std::vector<Mesh> result;

    // close circular paths.
    for (size_t id = 0; id < subPaths.size(); ++id) {
        auto &subPath = subPaths[id];
        auto &points = subPath.points;

        // Check if the first and last point are the same. Get rid of
        // the last point if that is the case, and close the subpath.
        if (points.size() >= 2 && glm::all(glm::epsilonEqual(points[0].pos, points.back().pos, distTol)))
        {
            points.resize(points.size()-1);
            subPath.closed = true;
        }
    }

    for (size_t id = 0; id < subPaths.size(); ++id) {
        auto &points = subPaths[id].points;

        // we need at least 3 points to make a shape. Otherwise it is a line or a point or nothing at all :-) 
        if (points.size() >= 3) {

            MPEPolyContext polyContext;
            
            // Request how much memory (in bytes) you should
            // allocate for the library
            size_t memoryRequired = MPE_PolyMemoryRequired(points.size());

            // Allocate a memory block of size MemoryRequired
            // IMPORTANT: The memory must be zero initialized
            std::vector<uint8_t> mempool;
            mempool.resize(memoryRequired, 0);

            // Initialize the poly context by passing the memory pointer,
            // and max number of points from before
            MPE_PolyInitContext(&polyContext, mempool.data(), points.size());

            MPEPolyPoint* polyPoints = MPE_PolyPushPointArray(&polyContext, points.size());
            for(size_t j = 0; j < points.size(); ++j)
            {
                glm::vec2 &point = points[j].pos;

                polyPoints[j].X = point.x;
                polyPoints[j].Y = point.y;
            }

            MPE_PolyAddEdge(&polyContext);

            MPE_PolyTriangulate(&polyContext);

            uint32_t vertexCount = polyContext.PointPoolCount;
            uint16_t indexCount = polyContext.TriangleCount*3;

            result.resize(result.size() + 1);
            Mesh &mesh = result.back();

            mesh.vertices.reserve(vertexCount);
            mesh.indices.reserve(indexCount);

            // populate the vertices
            for (size_t vid = 0; vid < polyContext.PointPoolCount; ++vid) {
                MPEPolyPoint &point = polyContext.PointsPool[vid];
                mesh.vertices.push_back({{point.X, point.Y, 0.0f}});
            }

            // populate the indices
            for (size_t tid = 0; tid < polyContext.TriangleCount; ++tid) {
                MPEPolyTriangle* triangle = polyContext.Triangles[tid];

                // get the array index by pointer address arithmetic.
                uint16_t p0 = static_cast<uint16_t>(triangle->Points[0] - polyContext.PointsPool);
                uint16_t p1 = static_cast<uint16_t>(triangle->Points[1] - polyContext.PointsPool);
                uint16_t p2 = static_cast<uint16_t>(triangle->Points[2] - polyContext.PointsPool);
                mesh.indices.push_back(p2);
                mesh.indices.push_back(p1);
                mesh.indices.push_back(p0);
            }
        }
    }


    return result;
}

std::vector<Mesh> Path2D::stroke(float lineWidth, LineJoin lineJoin, LineCap lineCap, float miterLimit) {

    float halfLineWidth = lineWidth * 0.5f;

    // close circular paths.
    for (size_t id = 0; id < subPaths.size(); ++id) {
        auto &subPath = subPaths[id];
        auto &points = subPath.points;

        // Check if the first and last point are the same. Get rid of
        // the last point if that is the case, and close the subpath.
        if (points.size() >= 2 && glm::all(glm::epsilonEqual(points[0].pos, points.back().pos, distTol)))
        {
            points.resize(points.size()-1);
            subPath.closed = true;
        }
    }

    // Calculate direction vectors for each points of each subpaths
    for(size_t id = 0; id < subPaths.size(); ++id) {
        SubPath2D &subPath = subPaths[id];

        size_t p0, p1;

        if (subPath.closed)
        {
            p0 = subPath.points.size() - 1;
            p1 = 0;
        }
        else
        {
            p0 = 0;
            p1 = 1;
        }

        while(p1 < subPath.points.size())
        {
            glm::vec2 delta = subPath.points[p1].pos - subPath.points[p0].pos;
            float length = glm::length(delta);
            subPath.points[p0].length = length;
            subPath.points[p0].dir = delta / length;
            p0 = p1++;
        }

        if (!subPath.closed)
        {
            // last point should have the same direction than its
            // previous point.
            subPath.points[p0].dir = subPath.points[p0-1].dir;
        }

    }

    for(size_t id = 0; id < subPaths.size(); ++id) {

        SubPath2D &subPath = subPaths[id];

        auto &points = subPath.points;
        auto &outerPoints = subPath.outerPoints;
        auto &innerPoints = subPath.innerPoints;

        if(subPath.closed) {

            // Calculate normal vectors
            for (size_t p0 = points.size() - 1, p1 = 0; p1 < points.size(); p0 = p1++)
            {
                // rotate direction vector by 90degree CW
                glm::vec2 dir0 = glm::vec2(points[p0].dir.y, -points[p0].dir.x);
                glm::vec2 dir1 = glm::vec2(points[p1].dir.y, -points[p1].dir.x);
                glm::vec2 norm = (dir0 + dir1) * 0.5f;
                float dot = glm::dot(norm, norm);
                if (dot > glm::epsilon<float>())
                {
                    norm *= glm::clamp(1.0f / dot, 0.0f, 1000.0f);
                }
                points[p1].norm = norm;

                float cross = glm::cross(points[p1].dir, points[p0].dir);
                points[p1].properties.set(cross > 0.0f ? PointProperties::leftTurn : PointProperties::rightTurn);

                float sharpnessLimit = glm::min(points[p0].length, points[p1].length) * (1.0f / halfLineWidth);
                if (dot * sharpnessLimit * sharpnessLimit > 1.0f)
                {
                    points[p1].properties.set(PointProperties::sharp);
                }

                if (points[p1].properties.test(PointProperties::corner))
                {
                    if (lineJoin == LineJoin::bevel || lineJoin == LineJoin::round ||
                        dot * miterLimit * miterLimit < 1.0f)
                    {
                        points[p1].properties.set(PointProperties::bevel);
                    }
                }
            }


            // create inner and outer contour.
            for (size_t p0 = points.size() - 1, p1 = 0; p1 < points.size(); p0 = p1++) {
                if (points[p1].properties.test(PointProperties::bevel))
                {
                    if (lineJoin == LineJoin::round)
                    {
                        glm::vec2 ext0 = glm::vec2(points[p0].dir.y, -points[p0].dir.x) * halfLineWidth;
                        glm::vec2 ext1 = points[p1].norm * halfLineWidth;
                        glm::vec2 ext2 = glm::vec2(points[p1].dir.y, -points[p1].dir.x) * halfLineWidth;

                        if (points[p1].properties.test(PointProperties::leftTurn))
                        {
                            detail::addPoint(innerPoints, points[p1].pos + ext1);
                            detail::addPoint(outerPoints, points[p1].pos - ext0);
                            detail::arcTo(outerPoints,
                                    points[p1].pos - ext0,
                                    points[p1].pos - ext1,
                                    points[p1].pos - ext2,
                                    halfLineWidth);
                        }
                        else
                        {
                            detail::addPoint(innerPoints, points[p1].pos + ext0);
                            detail::arcTo(innerPoints,
                                    points[p1].pos + ext0,
                                    points[p1].pos + ext1,
                                    points[p1].pos + ext2,
                                    halfLineWidth);
                            detail::addPoint(outerPoints, points[p1].pos - ext1);
                        }
                    }
                    else
                    {
                        if (points[p1].properties.test(PointProperties::leftTurn))
                        {
                            detail::addPoint(innerPoints, points[p1].pos + points[p1].norm * halfLineWidth);
                            detail::addPoint(outerPoints, points[p1].pos - glm::vec2(points[p0].dir.y, -points[p0].dir.x) * halfLineWidth);
                            detail::addPoint(outerPoints, points[p1].pos - glm::vec2(points[p1].dir.y, -points[p1].dir.x) * halfLineWidth);
                        }
                        else
                        {
                            detail::addPoint(innerPoints, points[p1].pos + glm::vec2(points[p0].dir.y, -points[p0].dir.x) * halfLineWidth);
                            detail::addPoint(innerPoints, points[p1].pos + glm::vec2(points[p1].dir.y, -points[p1].dir.x) * halfLineWidth);
                            detail::addPoint(outerPoints, points[p1].pos - points[p1].norm * halfLineWidth);

                        }

                    }
                }
                else
                {
                    glm::vec2 extrusion = points[p1].norm * halfLineWidth;
                    detail::addPoint(innerPoints, points[p1].pos + extrusion);
                    detail::addPoint(outerPoints, points[p1].pos - extrusion);
                }

            }
        } else {
            // Calculate normal vectors
            points[0].norm = glm::vec2(points[0].dir.y, -points[0].dir.x); // first point
            points.back().norm = glm::vec2(points.back().dir.y, -points.back().dir.x); // last point
            for (size_t p0 = 0, p1 = 1; p0 < points.size()-2; p0++, p1++)
            {
                // rotate direction vector by 90degree CW
                glm::vec2 dir0 = glm::vec2(points[p0].dir.y, -points[p0].dir.x);
                glm::vec2 dir1 = glm::vec2(points[p1].dir.y, -points[p1].dir.x);
                glm::vec2 norm = (dir0 + dir1) * 0.5f;
                float dot = glm::dot(norm, norm);
                if (dot > glm::epsilon<float>())
                {
                    norm *= glm::clamp(1.0f / dot, 0.0f, 1000.0f);
                }
                points[p1].norm = norm;

                float cross = glm::cross(points[p1].dir, points[p0].dir);
                points[p1].properties.set(cross > 0.0f ? PointProperties::leftTurn : PointProperties::rightTurn);

                float sharpnessLimit = glm::max(1.0f, glm::min(points[p0].length, points[p1].length) * (1.0f / halfLineWidth));
                if (dot * sharpnessLimit * sharpnessLimit > 1.0f)
                {
                    points[p1].properties.set(PointProperties::sharp);
                }

                if (points[p1].properties.test(PointProperties::corner))
                {
                    if (lineJoin == LineJoin::bevel ||
                        lineJoin == LineJoin::round ||
                        dot * miterLimit * miterLimit < 1.0f)
                    {
                        points[p1].properties.set(PointProperties::bevel);
                    }
                }
            }

            // extrude our points
            for (size_t p0 = points.size() - 1, p1 = 0; p1 < points.size(); p0 = p1++)
            {
                if (points[p1].properties.test(PointProperties::bevel) && points[p1].properties.test(PointProperties::leftTurn))
                {
                    if (lineJoin == LineJoin::round)
                    {
                        glm::vec2 v0 = points[p1].pos - glm::vec2(points[p1-1].dir.y, -points[p1-1].dir.x) * halfLineWidth;
                        glm::vec2 v1 = points[p1].pos - points[p1].norm * halfLineWidth;
                        glm::vec2 v2 = points[p1].pos - glm::vec2(points[p1].dir.y, -points[p1].dir.x) * halfLineWidth;
                        detail::addPoint(outerPoints, v0);
                        detail::arcTo(outerPoints, v0, v1, v2, halfLineWidth);
                    }
                    else
                    {
                        glm::vec2 v0, v1;
                        if (points[p1].properties.test(PointProperties::sharp))
                        {
                            // rotate direction vectors by 90degree CW
                            glm::vec2 dir0 = glm::vec2(points[p0].dir.y, -points[p0].dir.x);
                            glm::vec2 dir1 = glm::vec2(points[p1].dir.y, -points[p1].dir.x);

                            v0 = points[p1].pos - dir0 * halfLineWidth;
                            v1 = points[p1].pos - dir1 * halfLineWidth;
                        }
                        else
                        {
                            v0 = points[p1].pos - points[p0].norm * halfLineWidth;
                            v1 = points[p1].pos - points[p1].norm * halfLineWidth;
                        }

                        detail::addPoint(outerPoints, v0);
                        detail::addPoint(outerPoints, v1);
                    }
                }
                else
                {
                    detail::addPoint(outerPoints, points[p1].pos - points[p1].norm * halfLineWidth);
                }
            }

            // add the end cap
            if (lineCap != LineCap::butt)
            {
                glm::vec2 dir = points.back().dir * halfLineWidth;
                glm::vec2 ext = points.back().norm * halfLineWidth;

                /*
                    ...>>>>>>>>>(p0)---[+dir]-->(p1)
                    ...-----------------         |
                                        ---      |
                                            -- [+ext]
                                            -   |
                                            -  V
                                            - (p2)
                                            -  |
                                            -   |
                                            -- [+ext]
                                        ---      |
                    ...-----------------         V
                    ...<<<<<<<<<(p4)<--[-dir]---(p3)
                    */

                const glm::vec2 &p0 = outerPoints[outerPoints.size()-1];
                glm::vec2 p1 = p0 + dir;
                glm::vec2 p2 = p1 + ext;
                glm::vec2 p3 = p2 + ext;
                glm::vec2 p4 = p3 - dir;

                if (lineCap == LineCap::round)
                {
                    // then arc 90 degrees from p1 to p3
                    detail::arcTo(outerPoints, p0, p1, p2, halfLineWidth);

                    // then arc 90 degrees from p3 to p5
                    detail::arcTo(outerPoints, p2, p3, p4, halfLineWidth);
                }
                else // square
                {
                    detail::addPoint(outerPoints, p1);
                    detail::addPoint(outerPoints, p2);
                    detail::addPoint(outerPoints, p3);
                    detail::addPoint(outerPoints, p4);
                }
            }

            // extrude the 'other' side of our points in reverse.
            for (size_t p0 = points.size(), p1 = points.size() - 1; p0 > 0; p0 = p1--)
            {
                if (points[p1].properties.test(PointProperties::bevel) && points[p1].properties.test(PointProperties::rightTurn))
                {
                    if (lineJoin == LineJoin::round)
                    {
                        glm::vec2 v0 = points[p1].pos + glm::vec2(points[p1].dir.y, -points[p1].dir.x) * halfLineWidth;
                        glm::vec2 v1 = points[p1].pos + points[p1].norm * halfLineWidth;
                        glm::vec2 v2 = points[p1].pos + glm::vec2(points[p1+1].dir.y, -points[p1+1].dir.x) * halfLineWidth;
                        detail::addPoint(outerPoints, v0);
                        detail::arcTo(outerPoints, v0, v1, v2, halfLineWidth);
                    }
                    else
                    {
                        glm::vec2 v0, v1;
                        if (points[p1].properties.test(PointProperties::sharp))
                        {
                            // rotate direction vectors by 90degree CW
                            glm::vec2 dir0 = glm::vec2(points[p0].dir.y, -points[p0].dir.x);
                            glm::vec2 dir1 = glm::vec2(points[p1].dir.y, -points[p1].dir.x);

                            v0 = points[p1].pos + dir0 * halfLineWidth;
                            v1 = points[p1].pos + dir1 * halfLineWidth;
                        }
                        else
                        {
                            v0 = points[p1].pos + points[p0].norm * halfLineWidth;
                            v1 = points[p1].pos + points[p1].norm * halfLineWidth;
                        }

                        detail::addPoint(outerPoints, v1);
                        detail::addPoint(outerPoints, v0);
                    }
                }
                else
                {
                    detail::addPoint(outerPoints, points[p1].pos + points[p1].norm * halfLineWidth);
                }
            }

            // add the front cap
            if (lineCap != LineCap::butt)
            {
                glm::vec2 dir = points[0].dir * halfLineWidth;
                glm::vec2 ext = points[0].norm * halfLineWidth;

                /*
                        (p3)---[+dir]-->(p4)>>>>>>>>>...
                        ^         -----------------...
                        |      ---
                    [-ext]  --
                        |   -
                        |  -
                        (p2) -
                        ^  -
                        |   -
                    [-ext]  --
                        |      ---
                        |         -----------------...
                        (p1)<--[-dir]---(p0)<<<<<<<<<...
                    */

                const glm::vec2 &p0 = outerPoints[outerPoints.size()-1];
                glm::vec2 p1 = p0 - dir;
                glm::vec2 p2 = p1 - ext;
                glm::vec2 p3 = p2 - ext;
                glm::vec2 p4 = p3 + dir;

                if (lineCap == LineCap::round)
                {
                    // arc 90 degrees from p4 to p2
                    detail::arcTo(outerPoints, p0, p1, p2, halfLineWidth);

                    // then arc 90 degrees from p2 to p0
                    detail::arcTo(outerPoints, p2, p3, p4, halfLineWidth);

                    // remove duplicate p4 point since it is already
                    // in outerPoints as the outerPoints[0]
                    outerPoints.resize(outerPoints.size()-1);
                }
                else // square
                {
                    detail::addPoint(outerPoints, p1);
                    detail::addPoint(outerPoints, p2);
                    detail::addPoint(outerPoints, p3);
                    // don't add p4 point since it is already
                    // in outerPoints as the outerPoints[0]
                }
            }
        }
    }

    std::vector<Mesh> result;

    for (size_t id = 0; id < subPaths.size(); ++id) {
        auto &outerPoints = subPaths[id].outerPoints;
        auto &innerPoints = subPaths[id].innerPoints;

        uint32_t maxPointCount = static_cast<uint32_t>(outerPoints.size() + innerPoints.size());

        MPEPolyContext polyContext;
        
        // Request how much memory (in bytes) you should
        // allocate for the library
        size_t memoryRequired = MPE_PolyMemoryRequired(maxPointCount);

        // Allocate a memory block of size MemoryRequired
        // IMPORTANT: The memory must be zero initialized
        std::vector<uint8_t> mempool;
        mempool.resize(memoryRequired, 0);

        // Initialize the poly context by passing the memory pointer,
        // and max number of points from before
        MPE_PolyInitContext(&polyContext, mempool.data(), maxPointCount);

        if (outerPoints.size() >= 3) {
            // fill outer polyPoints buffer.
            MPEPolyPoint* polyPoints = MPE_PolyPushPointArray(&polyContext, outerPoints.size());
            for(size_t j = 0; j < outerPoints.size(); ++j) {
                glm::vec2 &point = outerPoints[j];

                polyPoints[j].X = point.x;
                polyPoints[j].Y = point.y;
            }
            MPE_PolyAddEdge(&polyContext);

            if (innerPoints.size() >= 3) {
                // fill inner polyPoints buffer.
                MPEPolyPoint* polyHoles = MPE_PolyPushPointArray(&polyContext, innerPoints.size());
                for(size_t j = 0; j < innerPoints.size(); ++j)
                {
                    glm::vec2 &point = innerPoints[j];

                    polyHoles[j].X = point.x;
                    polyHoles[j].Y = point.y;
                }
                MPE_PolyAddHole(&polyContext);
            }
        }

        MPE_PolyTriangulate(&polyContext);

        uint32_t vertexCount = polyContext.PointPoolCount;
        uint16_t indexCount = polyContext.TriangleCount*3;

        result.resize(result.size() + 1);
        Mesh &mesh = result.back();

        mesh.vertices.reserve(vertexCount);
        mesh.indices.reserve(indexCount);

        // populate the vertices
        for (size_t vid = 0; vid < polyContext.PointPoolCount; ++vid) {
            MPEPolyPoint &point = polyContext.PointsPool[vid];
            mesh.vertices.push_back({{point.X, point.Y, 0.0f}});
        }

        // populate the indices
        for (size_t tid = 0; tid < polyContext.TriangleCount; ++tid) {
            MPEPolyTriangle* triangle = polyContext.Triangles[tid];

            // get the array index by pointer address arithmetic.
            uint16_t p0 = static_cast<uint16_t>(triangle->Points[0] - polyContext.PointsPool);
            uint16_t p1 = static_cast<uint16_t>(triangle->Points[1] - polyContext.PointsPool);
            uint16_t p2 = static_cast<uint16_t>(triangle->Points[2] - polyContext.PointsPool);
            mesh.indices.push_back(p2);
            mesh.indices.push_back(p1);
            mesh.indices.push_back(p0);
        }

        
    }

    return result;
}


SubPath2D &Path2D::getCurrentSubPath(bool addDefaultStartingPointIfCreated) {

    if (subPaths.size() == 0) { 
        SubPath2D &subPath = createSubPath();
        if (addDefaultStartingPointIfCreated) detail::addPoint(subPath.points, glm::vec2(), PointProperties::corner);
        return subPath;
    }

    SubPath2D &currentSubPath = subPaths.back();
    if (currentSubPath.closed)
    {
        SubPath2D &subPath = createSubPath();
        if (addDefaultStartingPointIfCreated) detail::addPoint(subPath.points, glm::vec2(), PointProperties::corner);
        return subPath;
    }

    return subPaths.back();
}


SubPath2D &Path2D::createSubPath() {
    subPaths.resize(subPaths.size() + 1);
    return subPaths.back();
}