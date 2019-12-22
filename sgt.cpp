#include "sgt.hpp"
#include "sgt_itf.h"
#include <map>

SDL_Color kiss_white = {255, 255, 255, 255};
SDL_Color kiss_black = {0, 0, 0, 255};

int spacing = 3;

inline int max(int a, int b) {
    return a < b ? b : a;
}

int sgt_textwidth(sgt_font font, char *str) {
    int width;
    TTF_SizeUTF8(font.font, str, &width, NULL);

    return width;
}

int sgt_fillrect (SDL_Renderer* renderer, SDL_Rect* rect, SDL_Color color) {
    if (!renderer || !rect) return -1;

	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
	SDL_RenderFillRect(renderer, rect);
	return 0;
}

int sgt_makerect(SDL_Rect *rect, int x, int y, int w, int h)
{
	if (!rect) return -1;
	rect->x = x;
	rect->y = y;
	rect->w = w;
	rect->h = h;
	return 0;
}

int sgt_renderimage(SDL_Renderer *renderer, sgt_image image,
	SDL_Rect dst, SDL_Rect *clip)
{

	if (!renderer || !image.image) return -1;
    if (clip) dst.w = clip->w;
    if (clip) dst.h = clip->h;
	SDL_RenderCopy(renderer, image.image, clip, &dst);
	return 0;
}

int sgt_rendertext(SDL_Renderer* renderer, char* text, int x, int y, sgt_font* font, SDL_Color color) {
    SDL_Surface* surface;
    sgt_image img;

    if (!renderer || !text || !font) return -1;
    surface = TTF_RenderUTF8_Blended(font->font, text, color);
    img.image = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_QueryTexture(img.image, NULL, NULL, &img.w, &img.h);

    if (surface) SDL_FreeSurface(surface);
    SDL_Rect rect;
    sgt_makerect(&rect, x, y, img.w, img.h);
    sgt_renderimage(renderer, img, rect, NULL);
    SDL_DestroyTexture(img.image);
    return 0;
}

int sgt_window::draw(SDL_Renderer* renderer, int x, int y, int width, int height) {
    if (!renderer) return -1;
    SDL_Rect rect;
    sgt_makerect(&rect, x, y, width, height);
    sgt_fillrect(renderer, &rect, bg);
    return subwidgets[0]->draw(renderer, x, y, width, height);
}

SDL_Rect sgt_button::makerect(int x, int y, int width, int height) {
    SDL_Rect rec;
    sgt_makerect(&rec, x + spacing, y + spacing, width - 2 * spacing, height - 2 * spacing);
    return rec;
}

int sgt_button::draw(SDL_Renderer* renderer, int x, int y, int width, int height) {
    if (!renderer) return -1;
    SDL_Rect rec = makerect(x, y, width, height);
    if (active) {
		sgt_renderimage(renderer, activeimg, rec, NULL);
    } else if (highlight && !active) {
		sgt_renderimage(renderer, highlightimg, rec, NULL);
    } else {
		sgt_renderimage(renderer, normalimg, rec, NULL);
    }

    return 0;
}

int sgt_label::draw(SDL_Renderer* renderer, int x, int y, int , int ) {
    if (!renderer) return -1;

    sgt_rendertext(renderer, msg, x, y, &font, color);
	return 0;
}

void sgt_vstack::compute_height(int height, int *height1, int *height2) {
    if (subwidgets[0]->height_type == sgt_metric_type::SGT_FIXED) {
        *height1 = subwidgets[0]->height;
        *height2 = height - *height1;
    } else {
        if (subwidgets[1]->height_type == sgt_metric_type::SGT_FIXED) {
            *height1 = subwidgets[1]->height;
            *height2 = height - *height1;
        } else {
            *height1 = height / 2;
            *height2 = height / 2;
        }
    }
}

int sgt_vstack::draw(SDL_Renderer* renderer, int x, int y, int width, int height) {
    if (!renderer) return -1;

    int height1, height2;
    compute_height(height, &height1, &height2);

    subwidgets[0]->draw(renderer, x, y, width, height1);
    subwidgets[1]->draw(renderer, x, y + height1, width, height2);

    return 0;
}

void sgt_hstack::compute_width(int width, int *width1, int *width2) {
    if (subwidgets[0]->width_type == sgt_metric_type::SGT_FIXED) {
        *width1 = subwidgets[0]->width;
        *width2 = width - *width1;
    } else {
        if (subwidgets[1]->width_type == sgt_metric_type::SGT_FIXED) {
            *width1 = subwidgets[1]->width;
            *width2 = width - *width1;
        } else {
            *width1 = width / 2;
            *width2 = width / 2;
        }
    }
}

int sgt_hstack::draw(SDL_Renderer* renderer, int x, int y, int width, int height) {
    if (!renderer) return -1;

    int width1, width2;
    compute_width(width, &width1, &width2);

    subwidgets[0]->draw(renderer, x, y, width1, height);
    subwidgets[1]->draw(renderer, x + width1, y, width2, height);

    return 0;
}

sgt_hstack* sgt_hstack_create(sgt_widget* w1, sgt_widget* w2) {
    sgt_hstack* hstack = new sgt_hstack();
    hstack->subwidgets[0] = w1; 
    hstack->subwidgets[1] = w2; 
    if (w1->width_type == sgt_metric_type::SGT_FIXED && w2->width_type == sgt_metric_type::SGT_FIXED) {
        hstack->width_type = sgt_metric_type::SGT_FIXED;
    } else {
        hstack->width_type = sgt_metric_type::SGT_FLEXIBLE;
    }
            
    if (w1->height_type == sgt_metric_type::SGT_FIXED || w2->height_type == sgt_metric_type::SGT_FIXED) {
        hstack->height_type = sgt_metric_type::SGT_FIXED;
    } else {
        hstack->height_type = sgt_metric_type::SGT_FLEXIBLE;
    }

    hstack->update();

    return hstack;
}

void sgt_hstack::update() {
    width = subwidgets[0]->width + subwidgets[1]->width; 
    height = max(subwidgets[0]->height, subwidgets[1]->height);
}

sgt_vstack* sgt_vstack_create(sgt_widget* w1, sgt_widget* w2) {
    sgt_vstack* vstack = new sgt_vstack();
    vstack->subwidgets[0] = w1; 
    vstack->subwidgets[1] = w2; 
    if (w1->height_type == sgt_metric_type::SGT_FIXED && w2->height_type == sgt_metric_type::SGT_FIXED) {
        vstack->height_type = sgt_metric_type::SGT_FIXED;
    }

    if (w1->width_type == sgt_metric_type::SGT_FIXED || w2->width_type == sgt_metric_type::SGT_FIXED) {
        vstack->width_type = sgt_metric_type::SGT_FIXED;
    } else {
        vstack->width_type = sgt_metric_type::SGT_FLEXIBLE;
    }

    vstack->update();

    return vstack;
}

void sgt_vstack::update() {
    height = subwidgets[0]->height + subwidgets[1]->height; 
    width = max(subwidgets[0]->width, subwidgets[1]->width);
}

sgt_window* sgt_window_create(sgt_widget* w, int , int , SDL_Color color) {
    sgt_window* window = new sgt_window();
    window->bg = color;
    window->height_type = sgt_metric_type::SGT_FIXED;
    window->width_type = sgt_metric_type::SGT_FIXED;
    window->subwidgets[0] = w;
    window->update();
    return window;
}

void sgt_window::update() {
    height = subwidgets[0]->height;
    width = subwidgets[0]->width;
}

sgt_label* sgt_label_create(sgt_font font, SDL_Color color) {
    sgt_label* label = new sgt_label();
    label->font = font;
    label->height_type = sgt_metric_type::SGT_FIXED;
    label->width_type = sgt_metric_type::SGT_FLEXIBLE;
    label->color = color;
    strcpy(label->msg, "hello world");
    label->width = sgt_textwidth(font, label->msg);
    label->update();
    return label;
}

void sgt_label::update() {
    height = font.lineheight;
    width = sgt_textwidth(font, msg);
}

sgt_button* sgt_button_create(sgt_image normal , sgt_image active, sgt_image highlight) {
    sgt_button* button = new sgt_button();
    button->normalimg = normal;
    button->activeimg = active;
    button->highlightimg = highlight;
    button->active = false;
    button->highlight = false;

    button->height_type = sgt_metric_type::SGT_FLEXIBLE;
    button->width_type = sgt_metric_type::SGT_FLEXIBLE;
    button->update();

    return button;
}

void sgt_button::update() {
    height = normalimg.h + spacing * 2;
    width = normalimg.w + spacing * 2;
    clicked = false;
}

void sgt_label::handle_event(int x, int y, int width, int height, SDL_Event e) {
    // do nothing;
    return;
}

void sgt_button::handle_event(int x, int y, int width, int height, SDL_Event e) {
    SDL_Point p;
    SDL_Rect rec = makerect(x, y, width, height);
    switch (e.type) {
        case SDL_MOUSEMOTION:
            {
                p = {e.motion.x, e.motion.y};
                if (SDL_PointInRect(&p, &rec))
                    highlight = true;
                else
                    highlight = false;
            }
            break;
        case SDL_MOUSEBUTTONDOWN:
            {
                p = {e.button.x, e.button.y};
                if (SDL_PointInRect(&p, &rec)) {
                    clicked = true;
                    active = true;
                }
            }
            break;
        case SDL_MOUSEBUTTONUP:
            {
                if (active) {
                    active = false;
                }
            }
            break;
        default:
            break;
    }
}

void sgt_hstack::handle_event(int x, int y, int width, int height, SDL_Event e) {
    int width1, width2;
    compute_width(width, &width1, &width2);

    switch (e.type) {
        case SDL_MOUSEMOTION:
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
            subwidgets[0]->handle_event(x, y, width1, height, e);
            subwidgets[1]->handle_event(x + width1, y, width2, height, e);
            break;
        default:
            break;
    }
}

void sgt_vstack::handle_event(int x, int y, int width, int height, SDL_Event e) {
    int height1, height2;
    compute_height(height, &height1, &height2);

    switch (e.type) {
        case SDL_MOUSEMOTION:
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
            subwidgets[0]->handle_event(x, y, width, height1, e);
            subwidgets[1]->handle_event(x, y + height1, width, height2, e);
            break;
        default:
            break;
    }
}

void sgt_window::handle_event(int x, int y, int width, int height, SDL_Event e) {
    subwidgets[0]->handle_event(x, y, width, height, e);
}

std::map<int, sgt_widget*> collection;
bool inited = false;
SDL_Renderer* renderer;
SDL_Window* window;
sgt_font font;
sgt_image normal;
sgt_image highlight;
sgt_image active;

int window_height;
int window_width;

sgt_widget* w_toplevel;
int top_id;

void init_font(sgt_font* font, const char* name, int fontsize) {
    font->font = TTF_OpenFont(name, fontsize);
    font->fontheight = TTF_FontHeight(font->font);
    font->spacing = font->fontheight / 2;
    font->lineheight = font->fontheight + font->spacing;
    font->ascent = TTF_FontAscent(font->font);
    TTF_GlyphMetrics(font->font, 'W', NULL, NULL, NULL, NULL, &(font->advance));
}

void init_image(sgt_image *image, const char* name, SDL_Renderer* renderer) {
	image->image = IMG_LoadTexture(renderer, name);
	SDL_QueryTexture(image->image, NULL, NULL, &image->w, &image->h);
}

void init() {
    if (inited) return;
    SDL_Init(SDL_INIT_EVERYTHING);
    IMG_Init(IMG_INIT_PNG);
    TTF_Init();
    window = SDL_CreateWindow("hello", 0, 0, 640, 480, SDL_WINDOW_RESIZABLE);
    window_height = 480;
    window_width = 640;

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    init_font(&font, "kiss_font.ttf", 15);
    init_image(&normal, "kiss_normal.png", renderer);
    init_image(&active, "kiss_active.png", renderer);
    init_image(&highlight, "kiss_highlight.png", renderer);

    inited = true;
}

void update_toplevel(int id, sgt_widget* w) {
    w_toplevel = w;
    top_id = id;
}

void create_button(int id) {
    init();

    sgt_button* button = sgt_button_create(normal, active, highlight);
    if (collection.count(id) == 1)
        delete collection[id];
    collection[id] = button;

    if (!w_toplevel) {
        update_toplevel(id, button);
    }
}

void create_label(int id) {
    init();

    sgt_label* label = sgt_label_create(font, kiss_black);
    if (collection.count(id) == 1)
        delete collection[id];
    collection[id] = label;
    if (!w_toplevel) {
        update_toplevel(id, label);
    }
}

void create_vstack(int id, int w1, int w2) {
    init();

    sgt_vstack* vstack = sgt_vstack_create(collection[w1], collection[w2]);
    if (collection.count(id) == 1)
        delete collection[id];
    collection[id] = vstack;

    if (w1 == top_id || w2 == top_id) {
        update_toplevel(id, vstack);
    }
}

void create_hstack(int id, int w1, int w2) {
    init();

    sgt_hstack* hstack = sgt_hstack_create(collection[w1], collection[w2]);
    if (collection.count(id) == 1)
        delete collection[id];
    collection[id] = hstack;

    if (w1 == top_id || w2 == top_id) {
        top_id = id;
        update_toplevel(id, hstack);
    }
}

void create_window (int id, int w) {
    init();

    sgt_window* window = sgt_window_create(collection[w], 640, 480, kiss_white);
    
    if (collection.count(id) == 1)
        delete collection[id];
        
    collection[id] = window;

    if (w == top_id) {
        update_toplevel(id, window);
    }
}

void notify_update_change() {
    if (w_toplevel) {
        w_toplevel->updateAll();
        SDL_SetWindowMinimumSize(window, w_toplevel->width, w_toplevel->height);
        w_toplevel->draw(renderer, 0, 0, window_width, window_height);
        SDL_RenderPresent(renderer);
    }
}

void clean() {
    TTF_CloseFont(font.font);
    SDL_DestroyTexture(normal.image);
    SDL_DestroyTexture(active.image);
    SDL_DestroyTexture(highlight.image);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    delete w_toplevel;

    SDL_Quit();
    TTF_Quit();
    IMG_Quit();
}

void handle_windowevent(SDL_Event e) {
    switch (e.window.event) {
        case SDL_WINDOWEVENT_SIZE_CHANGED:
            window_width = e.window.data1;
            window_height = e.window.data2;
            break;
        default:
            break;
    }
}

void notify_get_event() {
    SDL_Delay(10);
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
            case SDL_QUIT:
                clean();
                exit(0);
            case SDL_WINDOWEVENT:
                handle_windowevent(e);
                break;
            default:
                w_toplevel->handle_event(0, 0, window_width, window_height, e);
                break;
        }
    }
}

int getevent_button_click(int id) {
    sgt_button* button = dynamic_cast<sgt_button*>(collection[id]);

    return button->clicked;
}

void update_label_text(int id, int text) {
    sgt_label* label = dynamic_cast<sgt_label*>(collection[id]);
    sprintf(label->msg, "%d", text);
}
