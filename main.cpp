#include <iostream>
#include <SDL.h>
#include "pugixml.hpp"
#include <vector>


struct Point;
std::vector<Point> points;
double min_lat = 100, min_lon = 100, max_lat = -100, max_lon = -100;
float zoom = 1.0;
float pan_y = 0, pan_x = 0;
bool should_update = false;
int width, height;

enum PointType {
    Trackpoint,
    Waypoint
};

struct Point {
    double elevation;
    double lat;
    double lon;
    std::string name;
    PointType type;
};

void load_type(PointType type, pugi::xml_node node) {
    while(node) {
        double lat = node.attribute("lat").as_double();
        double lon = node.attribute("lon").as_double();

        Point point{};
        point.lat = lat ;
        point.lon = lon;
        point.type = type;
        point.elevation = 0;

        if (type == Waypoint) {
            point.name = node.child("name").text().as_string();
        }

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
        node = node.next_sibling();
    }
}



void parse_xml(const char *filename) {
    pugi::xml_document doc;

    pugi::xml_parse_result result = doc.load_file(filename);
    if (result.status != pugi::xml_parse_status::status_ok) {
        std::cout << "fuck: " << result.description() << std::endl;
        exit(1);
    }

    pugi::xml_node metadata = doc.child("gpx").child("metadata");
    std::string gpx_name = metadata.child("name").text().as_string();

    pugi::xml_node track = doc.child("gpx").child("trk").child("trkseg").child("trkpt");
    pugi::xml_node waypoints = doc.child("gpx").child("wpt");

    load_type(Trackpoint, track);
    load_type(Waypoint, waypoints);
}

void latitude_to_range() {

}
void draw_track(SDL_Renderer *renderer, SDL_Window *window) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 200, 255, 200, 255);

    if (points.empty()) {
        std::cout << "no items to display";
        return;
    }

    double prev_lat = points[0].lat;
    double prev_lon = points[0].lon;

    SDL_GetWindowSize(window, &width, &height);
    double lat_diff = (max_lat - min_lat);
    double lon_diff = (max_lon - min_lon);
    double scaler;

    if (width / lon_diff < height / lat_diff) {
        scaler = (width - 8) / lon_diff * zoom;
    } else {
        scaler = (height - 8) / lat_diff * zoom;
    }

    for (auto &point : points) {



        int x = (int) ((point.lon - min_lon) * scaler) + 4 + pan_x;
        int y = (int) ((point.lat - min_lat) * scaler) + 4 + pan_y;

        if (point.type == Trackpoint) {
            int old_x = (int) ((prev_lon - min_lon) * scaler) + 4 + pan_x;
            int old_y = (int) ((prev_lat - min_lat) * scaler) + 4 + pan_y;
            SDL_RenderDrawLine(renderer, old_x, old_y, x, y);
        } else {
            SDL_RenderDrawPoint(renderer, x, y);
        }

        prev_lat = point.lat;
        prev_lon = point.lon;

    }

}

bool clicked = false;

void onMouseMove(void *data, SDL_Event *evt) {
    switch (evt->type) {
        case SDL_MOUSEMOTION:
            if (clicked) {
                std::cout << evt->motion.x << std::endl;
                pan_x += (float) evt->motion.xrel;
                pan_y += (float) evt->motion.yrel;
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
            float old_center = (1 * zoom) / 2;
            zoom += evt->wheel.y * 0.2 * zoom;
            float new_center = (1 * zoom) / 2;
            pan_y += (old_center - new_center) * height;
            pan_x += (old_center - new_center) * width;

            should_update = true;
    }
}


int main() {

    SDL_Window *window = SDL_CreateWindow("GPX Visualizer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 500, 500, SDL_WINDOW_SHOWN); //Display window
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC); //Create renderer
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_SetWindowResizable(window, SDL_TRUE);
    SDL_AddEventWatch(reinterpret_cast<SDL_EventFilter>(onMouseMove), (void *) nullptr);

    parse_xml("../Drinkwaterkaart.gpx");
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
