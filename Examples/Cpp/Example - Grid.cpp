#include "example_setup.h"

using namespace gooey;

UIApplication* app;

int main() {
    app              = create_gooey_application();
    UIWindow* window = app->add_window();
    window->set_title("Button Grid");
    window->set_size(1400, 1000);
    window->show();
    window->add_label("<top>");
    auto* row2   = window->add_horizontal_panel();
    auto* label1 = row2->add_label("<left>");

    auto* grid = row2->add_grid(8, 4, 50);

    grid->on_click([](uint x, uint y) {
        // std::cout << "clicked: " << x << ", " << y << std::endl;
        std::string message =
            "You clicked cell x: " + std::to_string(x) + ", y: " + std::to_string(y) + "!";
        app->message_box("Clicked", message.c_str());
    });

    // get mouse movements

    // get clicks

    // add something to a single cell

    // add something to multiple cells

    auto* label2 = row2->add_label("<right>");
    window->add_label("<bottom>");
    app->run_blocking();
    return 0;
}