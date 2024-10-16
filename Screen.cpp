#include <vector>
#include <sstream>
#include <cstdlib>
#include <string>
#include <thread>

#include "ANSII.cpp"

#ifndef Screen_cpp
#define Screen_cpp

/*
    comment out to use characters instead of square pixels
    note that you might have to decrease window dimensions to account for bigger pixels
*/
#define USE_SQUARE_PIXELS


#if defined(_WIN32) || defined(WIN32)
#   include <io.h>
#   define io_write _write
#else
#   include <unistd.h>
#   define io_write write
#endif

/* used in the Window class */
struct Point2d {
public:
    uint8_t r, g, b;
    char c;
    uint16_t x, y;
public:
    Point2d() = default;
    Point2d(const Point2d& other) = default;
    Point2d(uint8_t r, uint8_t g, uint8_t b): r(r), g(g), b(b), c('@'), x(0), y(0) {}
    Point2d(uint8_t r, uint8_t g, uint8_t b, uint16_t x, uint16_t y): r(r), g(g), b(b), c('@'), x(x), y(y) {}
    Point2d(uint8_t r, uint8_t g, uint8_t b, uint16_t x, uint16_t y, char c): r(r), g(g), b(b), c(c), x(x), y(y){}
};

struct Pixel {
public:
    uint8_t r;
    uint8_t g;
    uint8_t b;
    char c;
public:
    Pixel(char c, uint8_t r, uint8_t g, uint8_t b): r(r), g(g), b(b), c(c) {}
    Pixel(uint8_t r, uint8_t g, uint8_t b): r(r), g(g), b(b), c('@') {}
    Pixel(const Pixel& other): r(other.r), g(other.g), b(other.g), c(other.c) {}
    Pixel(): r(0), g(0), b(0), c('@') {}
    Pixel(char c): r(0), g(0), b(0), c(c) {}
    Pixel(const Point2d& other): r(other.r), g(other.g), b(other.b), c(other.c) {}

    ~Pixel() = default;
};

class Screen {
private:
    uint16_t W;
    uint16_t H;
    std::vector<std::vector<Pixel>> pixels; // since screen size can be dynamic
public:
    Screen(uint16_t W, uint16_t H): W(W), H(H){
        pixels.resize(W);
        for (auto& column : pixels){
            column.resize(H);
            std::fill(column.begin(), column.end(), Pixel());
        }
    }
    ~Screen() = default;
public:
    uint16_t width() const { return W; }
    uint16_t height()const { return H; }
    std::vector<std::vector<Pixel>>& data() { return pixels; }
public:
    void clear(){
        for (auto& row : pixels){
            // avoid allocating new memory
            std::fill(row.begin(), row.end(), Pixel());
        }
    }

    void reset(){
        pixels.clear();
    }

    void resize(uint16_t W, uint16_t H){
        this->W = W;
        this->H = H;

        pixels.resize(W);
        for (auto& column : pixels)
            column.resize(H);
    }

    std::vector<Pixel>& operator[] (uint16_t i){
        return pixels[i];
    }

    const std::vector<Pixel>& operator[] (uint16_t i) const {
        return pixels[i];
    }
public:
    void present() const {
        #ifdef USE_SQUARE_PIXELS
            char* str = new char[H * (W * 22 + 1) + 7 + 7 + 4 + 4 + 4]; // null - initialized
        #endif
        #ifndef USE_SQUARE_PIXELS
            char* str = new char[H * (W * 21 + 1) + 7 + 7 + 4 + 4 + 4];
        #endif

        size_t pos = 0;


        snprintf(str + pos, 4, "\033[s"); pos += 4; // save cursor pos
        snprintf(str + pos, 4, "\033[H"); pos += 4; // reset cursor pos (to top left)
        snprintf(str + pos, 7, "\033[?25l");  pos += 7; // hide cursor

        for (uint16_t i = 0; i < H; ++i){
            for (uint16_t j = 0; j < W; ++j){
                const Pixel& pixel = pixels[j][i];

                

                #ifdef USE_SQUARE_PIXELS
                    snprintf(str + pos, 20, "\033[48;2;%03u;%03u;%03um", pixel.r, pixel.g, pixel.b); // write the color string
                    pos += 20;
                    str[pos++] = ' '; // write the char
                    str[pos++] = ' ';
                #endif
                #ifndef USE_SQUARE_PIXELS
                    snprintf(str + pos, 20, "\033[38;2;%03u;%03u;%03um", pixel.r, pixel.g, pixel.b); // write the color string
                    pos += 20;
                    str[pos++] = pixel.c;
                #endif
                
            }
            if (i != H - 1) str[pos++] = '\n'; // avoid last newline
        }
        
        snprintf(str + pos, 7, "\033[?25h");  pos += 7; // show cursor
        snprintf(str + pos, 4, "\033[u"); pos += 4; // load cursor pos

        write(1, str, pos);
        delete[] str;
    }
};

#endif