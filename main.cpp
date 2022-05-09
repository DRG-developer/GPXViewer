#include <iostream>
#include <SDL.h>
#include "pugixml.hpp"
#include <vector>


struct Waypoint {
    double elevation;
    double lat;
    double lon;
};
std::vector<Waypoint> points;
double min_lat = 100, min_lon = 100, max_lat = -100, max_lon = -100;
float zoom = 1.0;
float pan_y = 0, pan_x = 0;
bool should_update = false;


void load_track(pugi::xml_node& track) {

        for (auto& obj : track.children()) {
            double lat = obj.attribute("lat").as_double();
            double lon = obj.attribute("lon").as_double();
            double elevation = obj.child("ele").text().as_double();

            Waypoint point{};
            point.elevation = elevation;
            point.lat = lat;
            point.lon = lon;

            if (lat < min_lat) {
                min_lat = lat;
            } else if (lat > max_lat) {
                max_lat = lat;
            }

            if (lon < min_lon) {
                min_lon = lon;
            } else if (lon > max_lon) {
                max_lon = lon;
            }



            points.push_back(point);
        }
}



void parse_xml(const char* filename) {
    pugi::xml_document doc;

    pugi::xml_parse_result result = doc.load_file(filename);
    if (result.status != pugi::xml_parse_status::status_ok) {
        std::cout << "fuck: " << result.description() << std::endl;
        exit(1);
    }

    pugi::xml_node metadata = doc.child("gpx").child("metadata");
    std::string gpx_name = metadata.child("name").text().as_string();

    pugi::xml_node track = doc.child("gpx").child("trk").child("trkseg");

    load_track(track);
}


void draw_track(SDL_Renderer* renderer, SDL_Window* window) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 200, 255, 200, 255);
    double prev_lat = points[0].lat;
    double prev_lon = points[0].lon;

    int width, height;
    SDL_GetWindowSize(window, &width, &height);
    double lat_diff = (max_lat - min_lat);
    double lon_diff = (max_lon - min_lon);
    double scaler;

    if (width / lon_diff < height / lat_diff) {
        scaler = (width - 8) / lon_diff * zoom;
    } else {
        scaler = (height - 8) /lat_diff * zoom;
    }

    for (auto& point : points) {
        int x1 = (int)((prev_lon - min_lon) * scaler) + 4 + pan_x;
        int x2 = (int)((point.lon - min_lon) * scaler) + 4 + pan_x;
        int y1 = (int)((prev_lat - min_lat) * scaler) + 4 + pan_y;
        int y2 = (int)((point.lat - min_lat) * scaler) + 4 + pan_y;

        prev_lat = point.lat;
        prev_lon = point.lon;

        SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
    }

}
bool clicked = false;
void onMouseMove(void* data, SDL_Event* evt) {
    switch (evt->type) {
        case SDL_MOUSEMOTION:
            if (clicked) {
                std::cout << evt->motion.x << std::endl;
                pan_x += (float)evt->motion.xrel;
                pan_y += (float)evt->motion.yrel;
                should_update = true;
            }
            break;
        case SDL_MOUSEBUTTONDOWN:
            clicked = true;
            break;
        case SDL_MOUSEBUTTONUP:
            clicked = false;
            break;
        case SDL_MOUSEWHEEL:
            zoom += evt->wheel.y;
            should_update = true;
    }
}

/*void onMouseWheel(int x, int y) {

    std::cout << y << std::endl;
    zoom += 0.01 * (float)y;
    should_update = true;
}*/
int main() {

    SDL_Window* window = SDL_CreateWindow("GPX Visualizer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 500, 500, SDL_WINDOW_SHOWN); //Display window
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC); //Create renderer
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_SetWindowResizable(window, SDL_TRUE);
    void *ptr;
    SDL_AddEventWatch(reinterpret_cast<SDL_EventFilter>(onMouseMove), ptr);

    parse_xml("../course.gpx");
    draw_track(renderer, window);

    while (true) {
        SDL_Event evt;
        if (SDL_PollEvent(&evt) == 0) {
            SDL_Delay(10);
        }

        switch (evt.type) {
            case SDL_QUIT:
                SDL_DestroyRenderer(renderer);
                SDL_DestroyWindow(window);
                SDL_Quit();

                return 0;
                break;
            case SDL_WINDOWEVENT:
                should_update = true;
                break;



        }

        if (should_update) {
            should_update = false;
            draw_track(renderer, window);
            SDL_RenderPresent(renderer);

        }

    }

    return 0;
}
