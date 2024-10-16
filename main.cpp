#include "Renderer.cpp"
#include <thread>

int main(){
    Window window(79, 59);

    Renderer renderer(window);


    std::array<Point3d, 8> buffer = {
        Point3d{255, 0, 0, -15, 15, 0}, // A
        Point3d{0, 255, 0, 15, 15, 0}, // B
        Point3d{255, 255, 0, 15, -15, 0}, // C
        Point3d{0, 0, 255, -15, -15, 0}, // D

        Point3d{255, 0, 255, -15, 15, 20}, // E
        Point3d{0, 255, 255, 15, 15, 20}, // F
        Point3d{255, 255, 255, 15, -15, 20}, // G
        Point3d{0, 0, 0, -15, -15, 20} // H
    };

    std::array<Point3d, 3> triBuff = {
        Point3d{255, 0, 0, 0, 10, 0},
        Point3d{0, 255, 0, 10, 0, 0},
        Point3d{0, 0, 255, -10, 0, 0}
    };

    for (float i = 0.0f; i < 20.0f; i += 0.1f){
        window.clear();

        window.drawRect({255, 255, 255, 0, 0}, {255, 255, 255, (uint16_t)(window.width() - 1), (uint16_t)(window.height() - 1)}, false);

        auto rotatedBuffer = Renderer::rotateYAxis(i, buffer);
        auto rotatedTriBuffer = Renderer::rotateYAxis(i, triBuff);

        renderer.renderRegObj<4>(
            rotatedBuffer.data(), false, 
            4, 0, 1, 5,
            7, 3, 2, 6,
            4, 0, 3, 7,
            5, 1, 2, 6,
            0, 1, 2, 3,
            4, 5, 6, 7
        );

        renderer.renderTriObj(rotatedTriBuffer.data(), true, 
            0, 1, 2
        );

        window.refresh();

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    window.clear();
    window.refresh();

    std::cin.get();
}
