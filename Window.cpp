#include <array>
#include <vector>

#ifndef Window_cpp
#define Window_cpp

#if defined(_WIN32) || defined(WIN32)
#   include <windows.h>
#else
#   include <termios.h>
#   include <unistd.h>
#endif

#include "Screen.cpp"

class Window {
private:
    uint16_t W;
    uint16_t H;
    Screen screen;
    struct Point;
    struct Triangle;
    struct Vector2;
    struct Vector3;
    struct Vector3d;

public:
    Window(uint16_t width, uint16_t height): W(width), H(height), screen(width, height) {
        io_write(1, ANSI::SCREEN::PUSH.data, 9);
    }

    ~Window(){
        io_write(1, ANSI::SCREEN::POP.data, 9);
    }
public:
    uint16_t width(){
        return W;
    }
    uint16_t height(){
        return H;
    }
private:
public: /* draw functions */ 
    void drawPoint(const Point2d& point){
        screen[point.x][point.y] = point;
    }

    void drawXLine(Point2d a, Point2d b){
        if (a.x > b.x){ auto temp = a; a = b; b = temp; } // sort them

        const float diffX = (float)b.x - (float)a.x;
       
        const float diffR = (float)b.r - (float)a.r;
        const float diffG = (float)b.g - (float)a.g;
        const float diffB = (float)b.b - (float)a.b;

        if (!diffX){
            screen[a.x][a.y] = Pixel (
                a.c,
                (float)(a.r + b.r) / 2.0f,
                (float)(a.g + b.g) / 2.0f,
                (float)(a.b + b.b) / 2.0f
            );
        }

        else {
            for (uint16_t i = a.x; i <= b.x; ++i)
                screen[i][a.y] = Pixel(
                    a.c,
                    a.r + (((float)i - (float)a.x) / (float)diffX) * (float)diffR,
                    a.g + (((float)i - (float)a.x) / (float)diffX) * (float)diffG,
                    a.b + (((float)i - (float)a.x) / (float)diffX) * (float)diffB
                );
        }
    }

    void drawYLine(Point2d a, Point2d b){
        if (a.y > b.y){ auto temp = a; a = b; b = temp; } // sort the points
        
        const float diffY = (float)b.y - (float)a.y;
        const float diffR = (float)b.r - (float)a.r;
        const float diffG = (float)b.g - (float)a.g;
        const float diffB = (float)b.b - (float)a.b;

        for (uint16_t i = a.y; i <= b.y; ++i){
            const float lerpFactor = (((float)i - (float)a.x) / (float)diffY);
            screen[a.x][i] = Pixel (
                a.c,
                a.r + (float)lerpFactor * (float)diffR,
                a.g + (float)lerpFactor * (float)diffG,
                a.b + (float)lerpFactor * (float)diffB
            );
        }
    }

    void drawLine(const Point2d& a, const Point2d& b){ 
        uint16_t x = a.x;
        uint16_t y = a.y;
        
        const float distance = sqrtf((float)pow((float)b.x - (float)a.x, 2) + (float)pow((float)b.y - (float)a.y, 2));
        
        const float diffR = (float)b.r - (float)a.r;
        const float diffG = (float)b.g - (float)a.g;
        const float diffB = (float)b.b - (float)a.b;


        int dX = abs((long double)b.x - a.x);
        int dY = abs((long double)b.y - a.y);

        int sX = (a.x < b.x) ? 1 : -1;
        int sY = (a.y < b.y) ? 1 : -1;

        int err = dX - dY;

        while (true) {

            float lerpFactor = sqrtf((float)pow((float)x - (float)a.x, 2) + (float)pow((float)y - (float)a.y, 2)) / distance;
            float r = (float)a.r + ((float)lerpFactor * (float)diffR);
            float g = (float)a.g + ((float)lerpFactor * (float)diffG);
            float b_ = (float)a.b + ((float)lerpFactor * (float)diffB);
            drawPoint({
                (uint8_t)(r),
                (uint8_t)(g),
                (uint8_t)(b_),
                x, y, a.c
            });
            
            if (x == b.x && y == b.y) break;

            int e2 = 2 * err;

            if (e2 > -dY) {
                err -= dY;
                x += sX;
            }

            if (e2 < dX) {
                err += dX;
                y += sY;
            }
        }
    }
    /* 
        assumes that points are in clockwise order 
        a.y == b.y && b.x == c.x && c.y = d.y && d.x == a.x
    */
    void drawRect(const Point2d& a, const Point2d& b, const Point2d& c, const Point2d& d, bool fill){
        if (fill){  
            const float diffY = (float)d.y - (float)a.y;

            // vertical A to D lerp constants
            const float ADdiffR = (float)d.r - (float)a.r;
            const float ADdiffG = (float)d.g - (float)a.g;
            const float ADdiffB = (float)d.b - (float)a.b;

            // vertical B to C lerp constants
            const float BCdiffR = (float)c.r - (float)b.r;
            const float BCdiffG = (float)c.g - (float)b.g;
            const float BCdiffB = (float)c.b - (float)b.b;

            for (uint16_t i = a.y; i <= d.y; ++i){
                float lerpFactorA = ((float)i - (float)a.y) / (float)diffY;
                float lerpFactorB = ((float)i - (float)b.y) / (float)diffY;
                drawXLine(
                { // left interpolated pixel
                    /* r */ (uint8_t)(a.r + (float)lerpFactorA * (float)ADdiffR),
                    /* g */ (uint8_t)(a.g + (float)lerpFactorA * (float)ADdiffG),
                    /* b */ (uint8_t)(a.b + (float)lerpFactorA * (float)ADdiffB),
                    /* x */ a.x,
                    /* y */ i
                },
                { // right interpolated pixel
                    /* r */ (uint8_t)(b.r + (float)lerpFactorB * (float)BCdiffR),
                    /* g */ (uint8_t)(b.g + (float)lerpFactorB * (float)BCdiffG),
                    /* b */ (uint8_t)(b.b + (float)lerpFactorB * (float)BCdiffB),
                    /* x */ b.x,
                    /* y */i
                }); 
            }

        }
        else {
            // since points are already in order, we can just draw lines
            drawXLine(a, b);
            drawXLine(c, d);

            drawYLine(b, c);
            drawYLine(d, a);
        }   
    }

    void drawRect(const Point2d& a, const Point2d& b, bool fill){
        if (fill){
            const float diffY = (float)b.y - (float)a.y;
            // vertical A to B lerp constants
            const float ABdiffR = (float)b.r - (float)a.r;
            const float ABdiffG = (float)b.g - (float)a.g;
            const float ABdiffB = (float)b.b - (float)a.b;

            for (uint16_t i = a.y; i <= b.y; ++i){
                float lerpFactorA = ((float)i - (float)a.y) / (float)diffY;
                float lerpFactorB = ((float)i - (float)b.y) / (float)diffY;
                drawXLine(
                { // left interpolated pixel
                    /* r */ (uint8_t)(a.r + (float)lerpFactorA * (float)ABdiffR),
                    /* g */ (uint8_t)(a.g + (float)lerpFactorA * (float)ABdiffG),
                    /* b */ (uint8_t)(a.b + (float)lerpFactorA * (float)ABdiffB),
                    /* x */ a.x,
                    /* y */ i
                },
                { // right interpolated pixel
                    /* r */ (uint8_t)(b.r + (float)lerpFactorB * (float)ABdiffR),
                    /* g */ (uint8_t)(b.g + (float)lerpFactorB * (float)ABdiffG),
                    /* b */ (uint8_t)(b.b + (float)lerpFactorB * (float)ABdiffB),
                    /* x */ b.x,
                    /* y */i
                }); 
            }
        }
        else{
            drawXLine({
                a.r, a.g, a.b,
                a.x, a.y
            }, {
                b.r, b.g, b.b,
                b.x, a.y
            });

            drawXLine({
                a.r, a.g, a.b,
                a.x, b.y
            }, {
                b.r, b.g, b.b,
                b.x, b.y
            });

            drawYLine({
                a.r, a.g, a.b,
                a.x, a.y
            }, {
                a.r, a.g, a.b,
                a.x, b.y
            });
            
            drawYLine({
                a.r, a.g, a.b,
                b.x, a.y
            }, {
                a.r, a.g, a.b,
                b.x, b.y
            });
        }
    }

    void drawTri(Point2d a, Point2d b, Point2d c, bool fill){
        if (!fill){
            drawLine(a, b);
            drawLine(b, c);
            drawLine(c, a);
        }
        else {
            // sort by y coordinate
            if (a.y > b.y){ auto temp = a; a = b; b = temp; }
            if (a.y > c.y){ auto temp = a; a = c; c = temp; }
            if (b.y > c.y){ auto temp = b; b = c; c = temp; }

            if (a.y == c.y) return;

            // flat top
            if (a.y == b.y)
                fillFlatTop(c, b, a);
            else
            // flat bottom
            if (b.y == c.y)
                fillFlatBottom(a, b, c);
            else
            // bend on left
            if (b.x < c.x){
                float lerpFactor = ((float)b.y - (float)a.y) / ((float)c.y - (float)a.y);
                uint16_t x = (float)a.x + (float)lerpFactor * ((float)c.x - (float)a.x);

                uint8_t r = (float)a.r + (float)lerpFactor * ((float)c.r - (float)a.r);
                uint8_t g = (float)a.g + (float)lerpFactor * ((float)c.g - (float)a.g);
                uint8_t b_ = (float)a.b + (float)lerpFactor * ((float)c.b - (float)a.b);

                fillFlatTop(c, {r, g, b_, x, b.y}, b);
                fillFlatBottom(a, {r, g, b_, x, b.y}, b);
            } else
            // bend on right
            if (b.x > c.x){
                float lerpFactor = ((float)b.y - (float)a.y) / ((float)c.y - (float)a.y);
                uint16_t x = (float)a.x + (float)lerpFactor * ((float)c.x - (float)a.x);

                uint8_t r = (float)a.r + (float)lerpFactor * ((float)c.r - (float)a.r);
                uint8_t g = (float)a.g + (float)lerpFactor * ((float)c.g - (float)a.g);
                uint8_t b_ = (float)a.b + (float)lerpFactor * ((float)c.b - (float)a.b);

                fillFlatTop(c, {r, g, b_, x, b.y}, b);
                fillFlatBottom(a, {r, g, b_, x, b.y}, b);
            }
        }
    }

    template<typename ... Args>
    void drawPoly(bool fill, Args&&... args){
        std::array<Point2d, (sizeof ...(args))> points = { (Point2d)(args)... };
        if (fill){
            for (Vector3& tri : polyTriSplit(points)){
                drawTri(points[tri.a], points[tri.b], points[tri.c], true);
            }
        }
        else {
            for (uint16_t i = 0; i < points.size() - 1; ++i)
                drawLine(points[i], points[i + 1]);
            
            drawLine(points[points.size() - 1], points[0]);
        }
    }

private: /* drawPoly helper function */
    template<size_t S, size_t V = S - 2>
    std::array<Vector3, V> polyTriSplit(const std::array<Point2d, S>& points){
        std::vector<size_t> iList(S);

        for (size_t i = 0; i < S; ++i)
            iList[i] = i;

        std::array<Vector3, V> triangles;
        size_t triCount = 0;

        while (iList.size() > 3){
            for (size_t i = 0; i < iList.size(); ++i){
                size_t a = iList[i];
                size_t b = getItem(iList, i - 1);
                size_t c = getItem(iList, i + 1);

                Vector2 vecA = points[a];
                Vector2 vecB = points[b];
                Vector2 vecC = points[c];

                Vector2 vecAtoB = vecB - vecA;
                Vector2 vecAtoC = vecC - vecA;

                if (crossProduct(vecAtoB, vecAtoC) > 0.0f)
                    continue;
                else{
                    bool isEar = true;
                    for (size_t j = 0; j < S; ++j){
                        if (j == a || j == b || j == c)
                            continue;
                        else {
                            Vector2 point = points[j];

                            if (pointInTriangle(point, vecB, vecA, vecC)){
                                isEar = false;
                            }
                        }

                    }

                    if (isEar){
                        triangles[triCount++] = Vector3(a, b, c);
                        iList.erase(iList.begin() + i);
                        break; 
                    }
                }
            }
        }

        triangles[V - 1] = Vector3(iList[0], iList[1], iList[2]);

        return triangles;
    }

    /* should be inlined in the future */
    float crossProduct(Vector2 a, Vector2 b){
        return (float)a.a * (float)b.b - (float)a.a * (float)b.b;
    }

    /* implements a circular method for indexing items from a vector */
    template<typename T>
    T& getItem(std::vector<T>& arr, int64_t i){
        if ((int64_t)i >= (int64_t)arr.size())
            return arr[i % arr.size()]; else
        if (i < 0)
            return arr[arr.size() - std::abs(i)];
        else
            return arr[i];
    }

    /* if point P is inside tri ABC */
    bool pointInTriangle(Vector2 p, Vector2 a, Vector2 b, Vector2 c){
        auto triArea = [](const Point& a, const Point& b, const Point& c){
            return std::abs((float)a.x * ((float)b.y - (float)c.y) + (float)b.x * ((float)c.y - (float)a.y) + (float)c.x * ((float)a.y - (float)b.y)) / 0.5f;
        };

        float area = triArea(a, b, c);
        float areaPAB = triArea(p, a, b);
        float areaPBC = triArea(p, b, c);
        float areaPCA = triArea(p, c, a);

        return (float)areaPAB + (float)areaPBC + (float)areaPCA == (float)area;
    }

/* filled triangle helper functions */
private:
    void fillFlatBottom(const Point2d& a, const Point2d& b, const Point2d& c){
        const float lSlope = ((float)c.x - (float)a.x) / ((float)c.y - (float)a.y);
        const float rSlope = ((float)b.x - (float)a.x) / ((float)b.y - (float)a.y);

        const float lDistance = sqrtf((float)pow((float)c.x - (float)a.x, 2) + (float)pow((float)c.y - (float)a.y, 2));
        const float rDistance = sqrtf((float)pow((float)b.x - (float)a.x, 2) + (float)pow((float)b.y - (float)a.y, 2));
        
        const float lDiffR = (float)c.r - (float)a.r;
        const float lDiffG = (float)c.g - (float)a.g;
        const float lDiffB = (float)c.b - (float)a.b;

        const float rDiffR = (float)b.r - (float)a.r;
        const float rDiffG = (float)b.g - (float)a.g;
        const float rDiffB = (float)b.b - (float)a.b;

        for (uint16_t y = a.y; y <= c.y; ++y){

            const uint8_t lX = (float)a.x + (float)(y - a.y) * (float)lSlope;
            const uint8_t rX = (float)a.x + (float)(y - a.y) * (float)rSlope;

            const float lLerpFactor = sqrtf((float)pow((float)lX - (float)a.x, 2) + (float)pow((float)y - (float)a.y, 2)) / (float)lDistance;
            const float rLerpFactor = sqrtf((float)pow((float)rX - (float)a.x, 2) + (float)pow((float)y - (float)a.y, 2)) / (float)rDistance;
        
            drawXLine({
                (uint8_t)((float)a.r + ((float)lLerpFactor * (float)lDiffR)),
                (uint8_t)((float)a.g + ((float)lLerpFactor * (float)lDiffG)),
                (uint8_t)((float)a.b + ((float)lLerpFactor * (float)lDiffB)),
                lX, y, a.c
            }, {
                (uint8_t)((float)a.r + ((float)rLerpFactor * (float)rDiffR)),
                (uint8_t)((float)a.g + ((float)rLerpFactor * (float)rDiffG)),
                (uint8_t)((float)a.b + ((float)rLerpFactor * (float)rDiffB)),
                rX, y, a.c
            });
        }
    }

    void fillFlatTop(const Point2d& a, const Point2d& b, const Point2d& c){
        const float lSlope = ((float)a.x - (float)c.x) / ((float)a.y - (float)c.y);
        const float rSlope = ((float)a.x - (float)b.x) / ((float)a.y - (float)b.y);

        const float lDistance = sqrtf((float)pow((float)c.x - (float)a.x, 2) + (float)pow((float)a.y - (float)c.y, 2));
        const float rDistance = sqrtf((float)pow((float)b.x - (float)a.x, 2) + (float)pow((float)a.y - (float)b.y, 2));
        
        const float lDiffR = (float)c.r - (float)a.r;
        const float lDiffG = (float)c.g - (float)a.g;
        const float lDiffB = (float)c.b - (float)a.b;

        const float rDiffR = (float)b.r - (float)a.r;
        const float rDiffG = (float)b.g - (float)a.g;
        const float rDiffB = (float)b.b - (float)a.b;

        for (uint16_t y = c.y; y <= a.y; ++y){

            const uint8_t lX = (float)c.x + (float)(y - c.y) * (float)lSlope;
            const uint8_t rX = (float)b.x + (float)(y - c.y) * (float)rSlope;

            const float lLerpFactor = sqrtf((float)pow((float)lX - (float)a.x, 2) + (float)pow((float)a.y - (float)y, 2)) / (float)lDistance;
            const float rLerpFactor = sqrtf((float)pow((float)rX - (float)a.x, 2) + (float)pow((float)a.y - (float)y, 2)) / (float)rDistance;
        
            drawXLine({
                (uint8_t)((float)a.r + ((float)lLerpFactor * (float)lDiffR)),
                (uint8_t)((float)a.g + ((float)lLerpFactor * (float)lDiffG)),
                (uint8_t)((float)a.b + ((float)lLerpFactor * (float)lDiffB)),
                lX, y, a.c
            }, {
                (uint8_t)((float)a.r + ((float)rLerpFactor * (float)rDiffR)),
                (uint8_t)((float)a.g + ((float)rLerpFactor * (float)rDiffG)),
                (uint8_t)((float)a.b + ((float)rLerpFactor * (float)rDiffB)),
                rX, y, a.c
            });
        }
    }

public: /* text */
    void putText(const std::string& TEXT, uint16_t X, uint16_t Y, const Pixel& P){
        for (uint16_t i = 0; i < TEXT.length(); ++i)
            screen[X + i][Y] = Pixel(TEXT[i], P.r, P.g, P.b);
    }

private: /* resize helper functions */
    void setRawMode(bool enable){
        #if defined(_WIN32) || defined(WIN32)
            HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
            DWORD mode;

            GetConsoleMode(hStdin, &mode);
            if (enable)
                mode &= ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT);
            else
                mode |= (ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT);
            SetConsoleMode(hStdin, mode);
        #else
            static struct termios oldTermios, newTermios;

            if (enable) {
                tcgetattr(STDIN_FILENO, &oldTermios);
                newTermios = oldTermios;
                newTermios.c_lflag &= ~(ICANON | ECHO);
                tcsetattr(STDIN_FILENO, TCSANOW, &newTermios);
            } else
                tcsetattr(STDIN_FILENO, TCSANOW, &oldTermios);
        #endif
    }
    std::pair<uint16_t, uint16_t> getTerminalSize(){
        std::pair<uint16_t, uint16_t> size = {0, 0};

        #ifdef _WIN32
            HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
            CONSOLE_SCREEN_BUFFER_INFO bufferInfo;
            if (GetConsoleScreenBufferInfo(hConsole, &bufferInfo)) {
                size.first = bufferInfo.srWindow.Bottom - bufferInfo.srWindow.Top + 1;
                size.second = bufferInfo.srWindow.Right - bufferInfo.srWindow.Left + 1;
            }
        #else
            io_write(1, "\033[18t", 6);

            setRawMode(true);

            std::string response;
            char ch;
            while (read(STDIN_FILENO, &ch, 1) > 0) {
                response += ch;
                if (ch == 't') break;
            }

            setRawMode(false);

            std::size_t startPos = response.find('[');
            std::size_t semicolonPos = response.find(';', startPos);
            
            if (startPos != std::string::npos && semicolonPos != std::string::npos) {
                std::istringstream rowStream(response.substr(semicolonPos + 1));
                rowStream >> size.first;

                std::size_t secondSemicolonPos = response.find(';', semicolonPos + 1);
                if (secondSemicolonPos != std::string::npos) {
                    std::istringstream colStream(response.substr(secondSemicolonPos + 1));
                    colStream >> size.second;
                }
            }
        #endif
        return size;
    }

public:
    void resize(){
        std::tie(H, W) = getTerminalSize();
        screen.resize(W, H);
    }

    void clear(){
        screen.clear();
    }

    void refresh(){
        screen.present();
    }

private: /* struct definitions */

    struct Point {
        uint16_t x, y;

        Point(uint16_t x, uint16_t y): x(x), y(y) {}
        Point(const Vector2& other): x(other.a), y(other.b) {}
        Point() = default;
        ~Point() = default;
    };

    struct Triangle {
        uint16_t b, c;
                                
        Triangle(uint16_t b, uint16_t c):
            b(b), c(c) {}
        Triangle() = default;
        ~Triangle() = default;
    };

    /* VectorX stores indicides to a buffer, not actual coordinates */

    struct Vector2 {
        int32_t a, b;
        Vector2() = default;
        ~Vector2() = default;
        Vector2(int32_t a, int32_t b): a(a), b(b) {}
        Vector2(const Point& other): a(other.x), b(other.y) {}
        Vector2(const Point2d& other): a(other.x), b(other.y) {}

        Vector2 operator- (const Vector2& other){
            return Vector2((int32_t)a - (int32_t)other.a, (int32_t)b - (int32_t)other.b);
        }
    };

    struct Vector3d {
        Vector2 a, b, c;
        Vector3d() = default;
        ~Vector3d() = default;
        Vector3d(const Vector2& a, const Vector2& b, const Vector2& c): a(a), b(b), c(c) {}
    };

    struct Vector3 {
        size_t a, b, c;
        Vector3() = default;
        ~Vector3() = default;

        Vector3(size_t a, size_t b, size_t c): a(a), b(b), c(c) {}
    };
};

#endif