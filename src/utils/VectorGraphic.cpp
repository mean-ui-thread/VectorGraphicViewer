#include "VectorGraphic.h"

#include <fstream>
#include <sstream>

#define MPE_POLY2TRI_IMPLEMENTATION
#include <MPE_fastpoly2tri.h>



#include <SDL2/SDL.h>

#include <glm/glm.hpp>
#include <glm/gtc/epsilon.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/exterior_product.hpp>

#include "StringUtils.h"
#include "ViewerApp.h"

#include <rapidxml/rapidxml.hpp>

using namespace rapidxml;

static constexpr float distTol = 0.01f; // tolerance for points being added too closely from each other
static constexpr size_t BEZIER_RECURSION_LIMIT = 128;

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
                                float tesselationTolerance,
                                int32_t level = 0)
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
                    if(d2 < tesselationTolerance)
                    {
                        addPoint(points, glm::vec2(x2, y2), PointProperties::none);
                        return;
                    }
                }
                else
                {
                    if(d3 < tesselationTolerance)
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
                if(d3 * d3 <= tesselationTolerance * (dx*dx + dy*dy))
                {
                    addPoint(points, glm::vec2(x23, y23), PointProperties::none);
                    return;
                }
                break;

            case 2:
                // p1,p3,p4 are collinear, p2 is significant
                //----------------------
                if(d2 * d2 <= tesselationTolerance * (dx*dx + dy*dy))
                {
                    addPoint(points, glm::vec2(x23, y23), PointProperties::none);
                    return;
                }
                break;

            case 3:
                // Regular case
                //-----------------
                if((d2 + d3)*(d2 + d3) <= tesselationTolerance * (dx*dx + dy*dy))
                {
                    addPoint(points, glm::vec2(x23, y23), PointProperties::none);
                    return;
                }
                break;
        }

        // Continue subdivision
        //----------------------
        recursiveBezier(points, x1, y1, x12, y12, x123, y123, x1234, y1234, tesselationTolerance, level + 1);
        recursiveBezier(points, x1234, y1234, x234, y234, x34, y34, x4, y4, tesselationTolerance, level + 1);
    }

    template<typename PointArray>
    inline void bezierTo(PointArray &points,
                            float x1, float y1,
                            float x2, float y2,
                            float x3, float y3,
                            float x4, float y4,
                            float tesselationTolerance)
    {
        recursiveBezier(points, x1, y1, x2, y2, x3, y3, x4, y4, tesselationTolerance);
        addPoint(points, glm::vec2(x4, y4), PointProperties::corner);
    }

    template <typename PointArray>
    inline void arc(PointArray &points, glm::vec2 center, float radius,
                    float startAngle, float endAngle, bool anticlockwise, float tesselationTolerance)
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
                        pos.x, pos.y, tesselationTolerance); // end point

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
    inline void arcTo(PointArray &points, glm::vec2 p0, glm::vec2 p1, glm::vec2 p2, float radius, float tesselationTolerance)
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

        arc(points, c, radius, a0, a1, anticlockwise, tesselationTolerance);
    }
}

std::vector<Path2D> Path2D::fromSVGFile(const std::string &filePath, Unit unit, float dpi, float tesselationTolerance)
{
    std::ifstream ifs;
    ifs.open(filePath);
    if (!ifs.is_open())
    {
        SDL_LogCritical(0, "Could not open %s", filePath.c_str());
        return {};
    }

    std::stringstream svgStream;
    svgStream << ifs.rdbuf();

    std::string svgBuffer = svgStream.str();

    ifs.close();

    return fromSVGBuffer(svgBuffer, unit, dpi, tesselationTolerance);

}

struct SVGState {

    std::string id;

    Color fillStyle = Transparent;
    Color strokeStyle = Transparent;
    float lineWidth = 1.0f;
    float miterLimit = 10.0f;
    LineJoin lineJoin = LineJoin::miter;
    LineCap lineCap = LineCap::butt;

    // from path
    std::string d;

    // from circle
    float cx = 0.0f;
    float cy = 0.0f;
    float r = 0.0f;

    inline void apply(xml_node<> *node) {
        for(xml_attribute<> *attr = node->first_attribute(); attr != nullptr; attr = attr->next_attribute()) {
            if (strncmp(attr->name(), "id", 3) == 0) {
                id = attr->value();
            } else if (strncmp(attr->name(), "fill", 5) == 0) {
                fillStyle = attr->value();
            } else if (strncmp(attr->name(), "stroke", 7) == 0) {
                strokeStyle = attr->value();
            } else if (strncmp(attr->name(), "stroke-width", 13) == 0) {
                lineWidth = atof(attr->value());
            } else if (strncmp(attr->name(), "stroke-miterlimit", 18) == 0) {
                miterLimit = atof(attr->value());
            } else if (strncmp(attr->name(), "stroke-linejoin", 16) == 0) {
                if (strncmp(attr->value(), "bevel", 6) == 0) {
                    lineJoin = LineJoin::bevel;

                } else if (strncmp(attr->value(), "round", 6) == 0) {
                    lineJoin = LineJoin::round;

                } else if (strncmp(attr->value(), "miter", 6) == 0) {
                    lineJoin = LineJoin::miter;

                }
            } else if (strncmp(attr->name(), "stroke-linecap", 16) == 0) {
                if (strncmp(attr->value(), "butt", 5) == 0) {
                    lineCap = LineCap::butt;

                } else if (strncmp(attr->value(), "round", 6) == 0) {
                    lineCap = LineCap::round;

                } else if (strncmp(attr->value(), "square", 7) == 0) {
                    lineCap = LineCap::square;
                }
            } else if (strncmp(attr->name(), "d", 2) == 0) {
                std::string value = attr->value();
                d = trim(value);

            } else if (strncmp(attr->name(), "cx", 3) == 0) {
                cx = atof(attr->value());
            } else if (strncmp(attr->name(), "cy", 3) == 0) {
                cy = atof(attr->value());
            } else if (strncmp(attr->name(), "r", 2) == 0) {
                r = atof(attr->value());
            } else {
                SDL_LogCritical(0, "Unsupported SVG <%s> Attribute '%s': '%s'", node->name(), attr->name(), attr->value());
            }
        }
    }
};

void processSvgChildrenNodes(xml_node<> *node, std::vector<SVGState> &stateStack, std::vector<Path2D> &paths, float tesselationTolerance);

void processSvgGroup(xml_node<> *groupNode, std::vector<SVGState> &stateStack, std::vector<Path2D> &paths, float tesselationTolerance) {
    stateStack.push_back(stateStack.back());
    SVGState &state = stateStack.back();
    state.apply(groupNode);
    processSvgChildrenNodes(groupNode, stateStack, paths, tesselationTolerance);
    stateStack.pop_back();
}

void processSvgPath(xml_node<> *pathNode, std::vector<SVGState> &stateStack, std::vector<Path2D> &paths, float tesselationTolerance) {
    stateStack.push_back(stateStack.back());
    SVGState &state = stateStack.back();
    state.apply(pathNode);

    paths.push_back(Path2D(tesselationTolerance, state.d));
    auto &path = paths.back();
    path.fillStyle = state.fillStyle;
    path.strokeStyle = state.strokeStyle;
    path.lineJoin = state.lineJoin;
    path.lineCap = state.lineCap;
    path.miterLimit = state.miterLimit;
    path.lineWidth = state.lineWidth;
    
    stateStack.pop_back();
}

void processSvgCircle(xml_node<> *circleNode, std::vector<SVGState> &stateStack, std::vector<Path2D> &paths, float tesselationTolerance) {
    stateStack.push_back(stateStack.back());
    SVGState &state = stateStack.back();
    state.apply(circleNode);

    paths.push_back(Path2D(tesselationTolerance));
    auto &path = paths.back();
    path.arc(state.cx, state.cy, state.r, 0.0f, M_PI * 2.0f, true);
    path.fillStyle = state.fillStyle;
    path.strokeStyle = state.strokeStyle;
    path.lineJoin = state.lineJoin;
    path.lineCap = state.lineCap;
    path.miterLimit = state.miterLimit;
    path.lineWidth = state.lineWidth;
    
    stateStack.pop_back();
}

void processSvgChildrenNodes(xml_node<> *node, std::vector<SVGState> &stateStack, std::vector<Path2D> &paths, float tesselationTolerance) {
    for (xml_node<> *childNode = node->first_node(); childNode != nullptr; childNode = childNode->next_sibling()) {
        if (strncmp(childNode->name(), "g", 2) == 0) {
            processSvgGroup(childNode, stateStack, paths, tesselationTolerance);
        } else if (strncmp(childNode->name(), "path", 5) == 0) {
            processSvgPath(childNode, stateStack, paths, tesselationTolerance);
        } else if (strncmp(childNode->name(), "circle", 5) == 0) {
            processSvgCircle(childNode, stateStack, paths, tesselationTolerance);
        } else {
            SDL_LogCritical(0, "Unsupported SVG node <%s>", childNode->name());
        }
    }
}

std::vector<Path2D> Path2D::fromSVGBuffer(const std::string &buffer, Unit unit, float dpi, float tesselationTolerance)
{

    std::vector<Path2D> paths;

    std::vector<SVGState> stateStack;
    stateStack.push_back(SVGState()); // default

    xml_document<> doc;    // character type defaults to char
    doc.parse<0>((char*)buffer.c_str());    // 0 means default parse flags

    xml_node<> *svg = doc.first_node("svg");
    processSvgChildrenNodes(svg, stateStack, paths, tesselationTolerance);

    return paths;
}




Path2D::Path2D(float tesselationFactor, const std::string &svgData) : tesselationTolerance(1.0f / tesselationFactor) {

    struct Command {
        char id;
        std::vector<float> params;

        void addParam(std::string paramStr) {
            trim(paramStr);
            if (paramStr.size() > 0) {
                params.push_back(atof(paramStr.c_str()));
            }
        }
    };

    std::vector<Command> commands;

    size_t paramStart = 0;
    for (size_t i = 0; i < svgData.size(); ++i) {
        if (isalpha(svgData[i]) || svgData[i] == ',' || svgData[i] == ' ' || svgData[i] == '-') {
            commands.back().addParam(svgData.substr(paramStart, i - paramStart));
            // Ok. I hate SVG for real now.
            if (svgData[i] == '-') {
                paramStart = i;
            } else {
                paramStart = i+1;
            }
        }

        if (isalpha(svgData[i])) {
            commands.resize(commands.size() + 1);
            commands.back().id = svgData[i];
        }
    }
    if (paramStart < svgData.size()) {
        commands.back().addParam(svgData.substr(paramStart, svgData.size() - paramStart));
    }

#if 0
    for (size_t i = 0; i < commands.size(); ++i) {
        Command &command = commands[i];
        printf("%c(", command.id);
        for(size_t j = 0; j < command.params.size(); ++ j) {
            printf("%3.2ff", command.params[j]);
            if (j+1 < command.params.size()) printf(", ");
        }
        printf(")\n");
    }
#endif

    // reused for relative position from lower-cased commands
    glm::vec2 prevPoint = glm::vec2(0, 0); 

    // reused for smooth bezier
    glm::vec2 prevControlPoint = glm::vec2(0, 0);

    for (size_t i = 0; i < commands.size(); ++i) {
        const Command &command = commands[i];

        switch(command.id) {
            case 'M': {
                float x = command.params[0];
                float y = command.params[1];
                moveTo(x, y);
                prevPoint.x = x;
                prevPoint.y = y;
                prevControlPoint = prevPoint;
                break;
            }
            case 'm': {
                float x = prevPoint.x + command.params[0];
                float y = prevPoint.y + command.params[1];
                moveTo(x, y);
                prevPoint.x = x;
                prevPoint.y = y;
                prevControlPoint = prevPoint;
                break;
            }
            case 'L': {
                float x = command.params[0];
                float y = command.params[1];
                lineTo(x, y);
                prevPoint.x = x;
                prevPoint.y = y;
                prevControlPoint = prevPoint;
                break;
            }
            case 'l': {
                float x = prevPoint.x + command.params[0];
                float y = prevPoint.y + command.params[1];
                lineTo(x, y);
                prevPoint.x = x;
                prevPoint.y = y;
                prevControlPoint = prevPoint;
                break;
            }
            case 'H':{
                float x = command.params[0];
                float y = prevPoint.y;
                lineTo(x, y);
                prevPoint.x = x;
                prevControlPoint.x = prevPoint.x;
                break;
            }
            case 'h':{
                float x = prevPoint.x + command.params[0];
                float y = prevPoint.y;
                lineTo(x, y);
                prevPoint.x = x;
                prevControlPoint.x = prevPoint.x;
                break;
            }
            case 'V':{
                float x = prevPoint.x; 
                float y = command.params[0];
                lineTo(x, y);
                prevPoint.y = y;
                prevControlPoint.y = prevPoint.y;
                break;
            }
            case 'v': {
                float x = prevPoint.x; 
                float y = prevPoint.y + command.params[0];
                lineTo(x, y);
                prevPoint.y = y;
                prevControlPoint.y = prevPoint.y;
                break;
            }
            case 'C': {
                float cp1x = command.params[0];
                float cp1y = command.params[1];
                float cp2x = command.params[2];
                float cp2y = command.params[3];
                float x = command.params[4];
                float y = command.params[5];
                bezierCurveTo(cp1x, cp1y, cp2x, cp2y, x, y);
                prevPoint.x = x;
                prevPoint.y = y;
                prevControlPoint.x = cp2x;
                prevControlPoint.x = cp2y;
                break;
            }
            case 'c': {
                float cp1x = prevPoint.x + command.params[0];
                float cp1y = prevPoint.y + command.params[1];
                float cp2x = prevPoint.x + command.params[2];
                float cp2y = prevPoint.y + command.params[3];
                float x = prevPoint.x + command.params[4];
                float y = prevPoint.y + command.params[5];
                bezierCurveTo(cp1x, cp1y, cp2x, cp2y, x, y);
                prevPoint.x = x;
                prevPoint.y = y;
                prevControlPoint.x = cp2x;
                prevControlPoint.x = cp2y;
                break;
            }
            case 'S': {
                float cp1x = 2.0 * prevPoint.x - prevControlPoint.x;
                float cp1y = 2.0 * prevPoint.y - prevControlPoint.y;
                float cp2x = command.params[0];
                float cp2y = command.params[1];
                float x = command.params[2];
                float y = command.params[3];
                bezierCurveTo(cp1x, cp1y, cp2x, cp2y, x, y);
                prevPoint.x = x;
                prevPoint.y = y;
                prevControlPoint.x = cp2x;
                prevControlPoint.x = cp2y;
                break;
            }
            case 's': {
                float cp1x = 2.0 * prevPoint.x - prevControlPoint.x;
                float cp1y = 2.0 * prevPoint.y - prevControlPoint.y;
                float cp2x = prevPoint.x + command.params[0];
                float cp2y = prevPoint.x + command.params[1];
                float x = prevPoint.x + command.params[2];
                float y = prevPoint.x + command.params[3];
                bezierCurveTo(cp1x, cp1y, cp2x, cp2y, x, y);
                prevPoint.x = x;
                prevPoint.y = y;
                prevControlPoint.x = cp2x;
                prevControlPoint.x = cp2y;
                break;
            }
            case 'Q': {
                float cpx = command.params[0];
                float cpy = command.params[1];
                float x = command.params[2];
                float y = command.params[3];
                quadraticCurveTo(cpx, cpy, x, y);
                prevControlPoint.x = cpx;
                prevControlPoint.y = cpy;
                prevPoint.x = x;
                prevPoint.y = y;
                break;
            }
            case 'q': {
                float cpx = prevPoint.x + command.params[0];
                float cpy = prevPoint.y + command.params[1];
                float x = prevPoint.x + command.params[2];
                float y = prevPoint.y + command.params[3];
                quadraticCurveTo(cpx, cpy, x, y);
                prevControlPoint.x = cpx;
                prevControlPoint.y = cpy;
                prevPoint.x = x;
                prevPoint.y = y;
                break;
            }
            case 'z':
                closePath();
                break;
            default:
                SDL_LogCritical(0, "Command '%c' not supported!!!\n", command.id);
                break;
        }
    }


}

void Path2D::beginPath() {
    subPaths.clear();
}

void Path2D::closePath() {
    if (subPaths.size() > 0) {
        auto &subPath = subPaths.back();
        // we need at least 3 points to form a closed shape. Otherwise it is a line or a dot and we can't close that.
        if (subPath.points.size() >= 3) {
            subPath.closed = true;
        }
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
    detail::bezierTo(points, prevPoint.x, prevPoint.y, cp1x, cp1y, cp2x, cp2y, x, y, tesselationTolerance);
}

void Path2D::quadraticCurveTo(float cpx, float cpy, float x, float y) {
    SubPath2D &subPath = getCurrentSubPath();
    auto &points = subPath.points;
    auto &prevPoint = points.back().pos;
    float c1x = prevPoint.x + 2.0f/3.0f*(cpx - prevPoint.x);
    float c1y = prevPoint.y + 2.0f/3.0f*(cpy - prevPoint.y);
    float c2x = x + 2.0f/3.0f*(cpx - x);
    float c2y = y + 2.0f/3.0f*(cpy - y);
    detail::bezierTo(points, prevPoint.x, prevPoint.y, c1x, c1y, c2x, c2y, x, y, tesselationTolerance);
}

void Path2D::arc(float x, float y, float radius, float startAngle, float endAngle, bool anticlockwise) {
    SubPath2D &subPath = getCurrentSubPath(false);
    detail::arc(subPath.points, glm::vec2(x, y), radius, startAngle, endAngle, anticlockwise, tesselationTolerance);
}

void Path2D::arcTo(float x1, float y1, float x2, float y2, float radius) {
    SubPath2D &subPath = getCurrentSubPath();
    auto &points = subPath.points;
    auto &prevPoint = points.back().pos;
    detail::arcTo(points, prevPoint, glm::vec2(x1, y1), glm::vec2(x2, y2), radius, tesselationTolerance);
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

void Path2D::fill(Mesh &mesh) {

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

            uint16_t offset = mesh.vertices.size();

            // populate the vertices
            for (size_t vid = 0; vid < polyContext.PointPoolCount; ++vid) {
                MPEPolyPoint &point = polyContext.PointsPool[vid];
                mesh.vertices.push_back({{point.X, point.Y, 0.0f}, fillStyle});
            }

            // populate the indices
            for (size_t tid = 0; tid < polyContext.TriangleCount; ++tid) {
                MPEPolyTriangle* triangle = polyContext.Triangles[tid];

                // get the array index by pointer address arithmetic.
                uint16_t p0 = static_cast<uint16_t>(triangle->Points[0] - polyContext.PointsPool);
                uint16_t p1 = static_cast<uint16_t>(triangle->Points[1] - polyContext.PointsPool);
                uint16_t p2 = static_cast<uint16_t>(triangle->Points[2] - polyContext.PointsPool);
                mesh.indices.push_back(offset+p2);
                mesh.indices.push_back(offset+p1);
                mesh.indices.push_back(offset+p0);
            }
        }
    }
    subPaths.clear();
}

void Path2D::fillRect(Mesh &mesh, float x, float y, float width, float height) {
    rect(x, y, width, height);
    fill(mesh);
    subPaths.clear();
}

void Path2D::stroke(Mesh &mesh) {

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


            // create inner and outer contour
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
                                    halfLineWidth,
                                    tesselationTolerance);
                        }
                        else
                        {
                            detail::addPoint(innerPoints, points[p1].pos + ext0);
                            detail::arcTo(innerPoints,
                                    points[p1].pos + ext0,
                                    points[p1].pos + ext1,
                                    points[p1].pos + ext2,
                                    halfLineWidth,
                                    tesselationTolerance);
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
                        detail::arcTo(outerPoints, v0, v1, v2, halfLineWidth, tesselationTolerance);
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
                    detail::arcTo(outerPoints, p0, p1, p2, halfLineWidth, tesselationTolerance);

                    // then arc 90 degrees from p3 to p5
                    detail::arcTo(outerPoints, p2, p3, p4, halfLineWidth, tesselationTolerance);
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
                        detail::arcTo(outerPoints, v0, v1, v2, halfLineWidth, tesselationTolerance);
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
                    detail::arcTo(outerPoints, p0, p1, p2, halfLineWidth, tesselationTolerance);

                    // then arc 90 degrees from p2 to p0
                    detail::arcTo(outerPoints, p2, p3, p4, halfLineWidth, tesselationTolerance);

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

    // Triangulate the fills (closed subpaths) if the color isn't transparent
    // also skip if the strokeStyle and the fillStyle are the same for closed
    // paths since we can optimized a bit by producing fewer triangles
    if (fillStyle.a > 0 && strokeStyle != fillStyle) {
        for (size_t id = 0; id < subPaths.size(); ++id) {
            auto &subPath = subPaths[id];

            if (!subPath.closed) continue; // skip. it's not an outline.

            auto &points = subPaths[id].innerPoints;

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
                    glm::vec2 &point = points[j];

                    polyPoints[j].X = point.x;
                    polyPoints[j].Y = point.y;
                }

                MPE_PolyAddEdge(&polyContext);

                MPE_PolyTriangulate(&polyContext);

                uint32_t vertexCount = polyContext.PointPoolCount;
                uint16_t indexCount = polyContext.TriangleCount*3;

                uint16_t offset = mesh.vertices.size();

                // populate the vertices
                for (size_t vid = 0; vid < polyContext.PointPoolCount; ++vid) {
                    MPEPolyPoint &point = polyContext.PointsPool[vid];
                    mesh.vertices.push_back({{point.X, point.Y, 0.0f}, fillStyle});
                }

                // populate the indices
                for (size_t tid = 0; tid < polyContext.TriangleCount; ++tid) {
                    MPEPolyTriangle* triangle = polyContext.Triangles[tid];

                    // get the array index by pointer address arithmetic.
                    uint16_t p0 = static_cast<uint16_t>(triangle->Points[0] - polyContext.PointsPool);
                    uint16_t p1 = static_cast<uint16_t>(triangle->Points[1] - polyContext.PointsPool);
                    uint16_t p2 = static_cast<uint16_t>(triangle->Points[2] - polyContext.PointsPool);
                    mesh.indices.push_back(offset+p2);
                    mesh.indices.push_back(offset+p1);
                    mesh.indices.push_back(offset+p0);
                }
            }
        }
    }



    // Triangulate the lines (outterPoints that doesn't have inner points) and outlines (outter poitns that has inner points)
    for (size_t id = 0; id < subPaths.size(); ++id) {
        auto &subPath = subPaths[id];
        auto &outerPoints = subPath.outerPoints;
        auto &innerPoints = subPath.innerPoints;

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
        }

        // Only cut out a hole if the fillStyle is different than the stroke style
        if (fillStyle != strokeStyle && innerPoints.size() >= 3) {
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


        MPE_PolyTriangulate(&polyContext);

        uint32_t vertexCount = polyContext.PointPoolCount;
        uint16_t indexCount = polyContext.TriangleCount*3;

        uint16_t offset = mesh.vertices.size();

        // populate the vertices
        for (size_t vid = 0; vid < polyContext.PointPoolCount; ++vid) {
            MPEPolyPoint &point = polyContext.PointsPool[vid];
            mesh.vertices.push_back({{point.X, point.Y, 0.0f}, strokeStyle});
        }

        // populate the indices
        for (size_t tid = 0; tid < polyContext.TriangleCount; ++tid) {
            MPEPolyTriangle* triangle = polyContext.Triangles[tid];

            // get the array index by pointer address arithmetic.
            uint16_t p0 = static_cast<uint16_t>(triangle->Points[0] - polyContext.PointsPool);
            uint16_t p1 = static_cast<uint16_t>(triangle->Points[1] - polyContext.PointsPool);
            uint16_t p2 = static_cast<uint16_t>(triangle->Points[2] - polyContext.PointsPool);
            mesh.indices.push_back(offset+p2);
            mesh.indices.push_back(offset+p1);
            mesh.indices.push_back(offset+p0);
        }
        
    }

    subPaths.clear();
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