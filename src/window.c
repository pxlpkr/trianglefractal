#include <time.h>
#include <SDL.h>

typedef struct {double x; double y;} PT;

//<-- SETTINGS -->
#define WIDTH       924
#define HEIGHT      800

#define VERTICES    3
PT get_vertex(int i) {
    switch (i) {
        case 1:     return (PT) {50, 50};
        case 2:     return (PT) {WIDTH-50, 50};
        case 3:     return (PT) {WIDTH/2, HEIGHT-50};
        default:    return (PT) {-1, -1};
    }
}

#define MAX_FPS     60
#define PER_TICK    25
#define TITLE       "Sierpiński triangle"












//<-- CODE -->

//Macros
#define MAX_INT     2147483647

//"Global" struct
struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;

    u_int32_t buffer[WIDTH * HEIGHT];

    int exit_requested;

    PT cur_pt;
    PT v_pt[VERTICES];

    int c;
} app;

//Allow exiting
void event_handler(void) {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                app.exit_requested = 1;
                break;
        }
    }
}

double rand_double(void) {
    return (double) rand() / MAX_INT;
}

PT get_random_point_in_triangle(void) {
    double r[2] = {rand_double(), rand_double()};
    double sqrt_r[2] = {sqrt(r[0]), sqrt(r[1])};
    return (PT) {
        (1 - sqrt_r[0]) * app.v_pt[0].x + (sqrt_r[0] * (1 - r[1])) * app.v_pt[1].x + (sqrt_r[0] * r[1]) * app.v_pt[2].x,
        (1 - sqrt_r[0]) * app.v_pt[0].y + (sqrt_r[0] * (1 - r[1])) * app.v_pt[1].y + (sqrt_r[0] * r[1]) * app.v_pt[2].y
    };
}

void paint_pt(PT pt, u_int32_t color) {
    if (pt.x < 0 || pt.y < 0 || pt.x >= WIDTH || pt.y >= HEIGHT)
        return;
    app.buffer[(int) pt.y * WIDTH + (int) pt.x] = color;
}

void solve(void) {
    for (int i = 0; i < PER_TICK; i++) {
        //Draw the current point to the screen
        paint_pt(app.cur_pt, 0xFFFFFFFF);

        //Get a vertex at random
        PT chosen_vertex = app.v_pt[rand() % VERTICES];

        //Move the current point halfway to the random chosen vertex
        app.cur_pt = (PT) {
            (app.cur_pt.x + chosen_vertex.x)/2,
            (app.cur_pt.y + chosen_vertex.y)/2
        };
    }
}

void iterate(void) {
    clock_t start = clock();

    event_handler();

    solve();

    SDL_UpdateTexture(
        app.texture,
        NULL,                       //Select entire texture
        app.buffer,
        WIDTH * 4                   //Bytes per Pixel
    );
    SDL_RenderCopy(
        app.renderer,
        app.texture,
        NULL, NULL                  //Select entire source and destination
    );
    SDL_RenderPresent(app.renderer);


    //Delay next frame to ensure MAX_FPS frames happen in one second
    //Consider execution time of previous frame to prevent unnecessary waiting
    clock_t end = clock();
    double time_elapsed = (double) (end - start) * 1000.0 / CLOCKS_PER_SEC;

    double delay_nanos = 1000000000 / MAX_FPS - time_elapsed * 1000000;
    if (delay_nanos > 0) {
        struct timespec delay = {0, delay_nanos};
        nanosleep(&delay, NULL);
    }

    // printf("%f ms \n", time_elapsed);
}

void initialize(void) {
    //Seed the random, twice for good measure
    srand(time(0));
    srand(rand());

    //Create the SDL Window Context
    app.window = SDL_CreateWindow(
        TITLE,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WIDTH,
        HEIGHT,
        SDL_WINDOW_ALLOW_HIGHDPI
    );

    app.renderer = SDL_CreateRenderer(
        app.window,
        -1,                             //Get default driver
        0
    );

    app.texture = SDL_CreateTexture(
        app.renderer,
        SDL_PIXELFORMAT_RGBA8888,       //1 Byte x 4 Channels (ARGB)
        SDL_TEXTUREACCESS_STREAMING,
        WIDTH,
        HEIGHT
    );

    //Load all vertices to memory
    for (int i = 0; i < VERTICES; i++)
        app.v_pt[i] = get_vertex(i+1);

    //Select random starting point
    app.cur_pt  = get_random_point_in_triangle();
}

int main() {
    initialize();

    while (!app.exit_requested)
        iterate();

    SDL_DestroyTexture(app.texture);
    SDL_DestroyRenderer(app.renderer);
    SDL_DestroyWindow(app.window);

    return 0;
}