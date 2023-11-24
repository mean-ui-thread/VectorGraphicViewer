#ifndef VECTOR_GRAPHIC_H
#define VECTOR_GRAPHIC_H

#include <array>
#include <cfloat>
#include <cstdint>
#include <vector>

#include <glm/vec2.hpp>

#include "BitMask.h"
#include "VertexData.h"

enum class LineCap : uint8_t
{
    butt = 0, // The ends of lines are squared off at the endpoints.
    round, // The ends of lines are rounded.
    square, // he ends of lines are squared off by adding a box with an equal width and half the height of the line's thickness.
};

enum class LineJoin : uint8_t
{
    bevel = 0, //! Fills an additional triangular area between the common endpoint of connected segments, and the separate outside rectangular corners of each segment.
    round, // Rounds off the corners of a shape by filling an additional sector of disc centered at the common endpoint of connected segments. The radius for these rounded corners is equal to the line width.
    miter, // Connected segments are joined by extending their outside edges to connect at a single point, with the effect of filling an additional lozenge-shaped area. This setting is affected by the miterLimit property.
};

enum class FillRule : uint8_t
{
    nonZero, //! The non-zero winding rule, which is the default rule.
    evenOdd, //! The even-odd winding rule.
};

enum class PointProperties : uint8_t
{
    none = 0,
    corner = 0x01,
    leftTurn = 0x02,
    rightTurn = 0x04,
    bevel = 0x08,
    sharp = 0x10,
};

struct ContourPoint {
    glm::vec2 pos;
    glm::vec2 dir;
    glm::vec2 norm;
    float length = 0;
    BitMask<PointProperties> properties;
};

struct SubPath2D
{
    std::vector<ContourPoint> points;
    std::vector<glm::vec2> outerPoints;
    std::vector<glm::vec2> innerPoints;
    bool closed = false; 
};

struct Mesh {
    std::vector<PositionVertex> vertices;
    std::vector<uint16_t> indices;
};

class Path2D {
public:

    friend class VectorGraphic;

    void reset();
    void closePath();
    void moveTo(float x, float y);
    void lineTo(float x, float y);
    void bezierCurveTo(float cp1x, float cp1y, float cp2x, float cp2y, float x, float y);
    void quadraticCurveTo(float cpx, float cpy, float x, float y);
    void arc(float x, float y, float radius, float startAngle, float endAngle, bool anticlockwise = false);
    void arcTo(float x1, float y1, float x2, float y2, float radius);
    void ellipse(float x, float y, float radiusX, float radiusY, float rotation, float startAngle, float endAngle, bool anticlockwise = false);
    void rect(float x, float y, float width, float height);

    std::vector<Mesh> fill();
    std::vector<Mesh> stroke(float lineWidth = 1.0f, LineJoin lineJoin = LineJoin::miter, LineCap lineCap = LineCap::butt, float miterLimit = 10.0f);

private:

    void calculateSegmentDirection();
    
    SubPath2D &getCurrentSubPath(bool addDefaultStartingPointIfCreated = true);
    SubPath2D &createSubPath();

    std::vector<SubPath2D> subPaths;
};



#endif // VECTOR_GRAPHIC_H