#include "Window.cpp"
#include <variant>

#ifndef Renderer_cpp
#define Renderer_cpp

struct Point3d {
    uint8_t r, g, b;
    int16_t x, y, z;
    char c;

    Point3d(uint8_t r, uint8_t g, uint8_t b, int16_t x, int16_t y, int16_t z): 
        r(r), g(g), b(b), x(x), y(y), z(z), c('@') {}
    
    Point3d(uint8_t r, uint8_t g, uint8_t b, int16_t x, int16_t y, int16_t z, char c): 
        r(r), g(g), b(b), x(x), y(y), z(z), c(c) {}
    
    Point3d(const Point3d& other) = default;
    ~Point3d() = default;
    Point3d() = default;
};

struct Edge3d {
public:
    Point3d a, b;

    Edge3d(const Point3d& a, const Point3d& b): a(a), b(b) {}
};

template<size_t S>
struct Face3d {
public:
    std::array<Point3d, S> points;

    template<typename... Args>
    Face3d(Args&... args): points{Point3d(std::forward<Args>(args))...} {}
    
    template<typename... Args>
    Face3d(Args&&... args): points{Point3d(std::forward<Args>(args))...} {}
    
    const Point3d& operator[](size_t i) const {
        return points[i];
    }
};

template<size_t S, size_t... FaceSizes>
struct Obj3d {
public:
    using FaceType = std::variant<Face3d<FaceSizes>...>;
private:
    std::array<FaceType, S> points;
public:
    template<typename... Args>
    Obj3d(Args& ...args): points{FaceType(std::forward<Args>(args))...} {}

public:
    Point3d& operator[] (size_t i){ return points[i]; }
    auto begin() const { return points.begin(); }
    auto end()   const { return points.end();   }
};

class Renderer {
private:
    Window& window;
public:
    Renderer(Window& window): window(window) {}
    ~Renderer() = default;
public:
    void render(const Point3d& p){
        window.drawPoint(project(p));
    }

    void render(const Edge3d& e){
        window.drawLine(
            project(e.a),
            project(e.b)
        );
    }

    template<size_t S>
    void render(const Face3d<S>& face, bool fill){
        render(face, std::make_index_sequence<S>{}, fill);
    }

private:
    template<size_t S, size_t... Indices>
    void render(const Face3d<S>& f, std::index_sequence<Indices...>, bool fill){
        window.drawPoly(fill, project(f[Indices])...);
    }

public:
    template<size_t S, size_t... Sizes>
    void render(const Obj3d<S, Sizes...>& obj, bool fill){

        using FaceTypes = std::variant<Face3d<Sizes>...>;

        for (const FaceTypes& face : obj){
            std::visit([this, fill](auto&& f){
                render(f, fill);
            }, face);
        }
    }

public:
    template<size_t S>
    static std::array<Point3d, S> rotateXAxis(float angleInRadians, const std::array<Point3d, S>& buff) {
        std::array<Point3d, S> res{};
        
        for (uint64_t i = 0; i < S; ++i) {
            float newY = roundf((float)buff[i].y * (float)cosf(angleInRadians) - (float)buff[i].z * (float)sinf(angleInRadians));
            float newZ = roundf((float)buff[i].y * (float)sinf(angleInRadians) + (float)buff[i].z * (float)cosf(angleInRadians));
            res[i] = {
                buff[i].r, buff[i].g, buff[i].b,
                buff[i].x,
                (int16_t)newY,
                (int16_t)newZ
            };
        }

        return res;
    }

    template<size_t S>
    static std::array<Point3d, S> rotateZAxis(float angleInRadians, const std::array<Point3d, S>& buff) {
        std::array<Point3d, S> res{};
        
        for (uint64_t i = 0; i < S; ++i) {
            float newX = roundf(buff[i].x * (float)cosf(angleInRadians) - (float)buff[i].y * (float)sinf(angleInRadians));
            float newY = roundf((float)buff[i].x * (float)sinf(angleInRadians) + (float)buff[i].y * (float)cosf(angleInRadians));
            res[i] = {
                buff[i].r, buff[i].g, buff[i].b,
                (int16_t)newX,
                (int16_t)newY,
                buff[i].z
            };
        }

        return res;
    }

    template<size_t S>
    static std::array<Point3d, S> rotateYAxis(float angleInRadians, const std::array<Point3d, S>& buff) {
        std::array<Point3d, S> res{};
        
        for (uint64_t i = 0; i < S; ++i) {
            float newX = roundf((float)buff[i].x * (float)cosf(angleInRadians) + (float)buff[i].z * (float)sinf(angleInRadians));
            float newZ = roundf((float)-buff[i].x * (float)sinf(angleInRadians) + (float)buff[i].z * (float)cosf(angleInRadians));
            
            res[i] = {
                buff[i].r, buff[i].g, buff[i].b,
                (int16_t)newX,
                buff[i].y,
                (int16_t)newZ
            };
        }
        return res;
    }

/*
    render functions that take in a buffer
*/
public:
    void renderPoint(Point3d* buff, uint64_t i){
        window.drawPoint(project(buff[i]));
    }

    void renderEdge(Point3d* buff, uint64_t a, uint64_t b){
        window.drawLine(
            project(buff[a]),
            project(buff[b])
        );
    }

    template<typename... Args>
    void renderFace(Point3d* buff, bool fill, Args... indices) {
        window.drawPoly(
            fill,
            (
                project(buff[(uint64_t)indices])
            )...
        );
    }

    template<typename... Args>
    void renderTriObj(Point3d* buff, bool fill, Args... args){
        static_assert(sizeof...(args) % 3 == 0, "Number of Indices Must be a Multiple of 3");
        
        std::array<uint64_t, sizeof...(args)> indices = {((uint64_t)args)...};
        for (uint64_t i = 0; i < indices.size(); i += 3){
            window.drawTri(
                project(buff[indices[i]]),
                project(buff[indices[i + 1]]),
                project(buff[indices[i + 2]]),
                fill
            );
        }
    }
    
    template<uint64_t PointsPerFace, typename... Args>
    void renderRegObj(Point3d* buff, bool fill, Args... args){
        static_assert(sizeof...(args) % PointsPerFace == 0, "Number of Indicies Must be a Multiple of Number of Points Per Face");

        std::array<uint64_t, sizeof...(args)> indices = {((uint64_t)args)...};

        for (uint64_t i = 0; i < indices.size(); i += PointsPerFace){
            call_drawPoly<indices.size(), PointsPerFace>(indices, i, fill, buff, std::make_integer_sequence<uint64_t, PointsPerFace>{});
        }
    }

private:
    template<size_t S, uint64_t PointsPerFace, uint64_t... Indices>
    void call_drawPoly(const std::array<uint64_t, S>& indices, uint64_t start, bool fill, Point3d* buff, std::integer_sequence<uint64_t, Indices...>) {
        window.drawPoly(fill, project(buff[indices[start + Indices]])...);
    }

private:
    Point2d project(const Point3d& p){
        uint16_t x = roundf(((float)p.x * 50.0f) / ((float)p.z + 50.0f) + (float)(window.width() / 2));
        uint16_t y = (float)(window.height() / 2) - roundf(((float)p.y * 50.0f) / ((float)p.z + 50.0f));

        return {
            p.r, p.g, p.b, 
            x,
            y,
            p.c
        };
    }
};

#endif