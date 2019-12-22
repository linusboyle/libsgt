#ifndef SGT_H
#define SGT_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>

#define MAX_MSG_BUFFER 200

enum class sgt_metric_type {
    SGT_FLEXIBLE,
    SGT_FIXED
};

struct sgt_image {
	SDL_Texture *image;
	int w;
	int h;
};

struct sgt_font {
	TTF_Font *font;
	int fontheight;
	int spacing;
	int lineheight;
	int advance;
	int ascent;
};

class sgt_widget {
    public:
        int width; // minimum width
        sgt_metric_type width_type;
        int height; // minimum height
        sgt_metric_type height_type;
        virtual int draw(SDL_Renderer*, int x, int y, int width, int height) = 0;
        virtual ~sgt_widget() {}
        virtual void update() = 0; // update esp. width/height
        virtual void updateAll() { update(); }
        virtual void handle_event(int x, int y, int width, int height, SDL_Event e) = 0;
};

template <int subl>
class sgt_container : public sgt_widget {
    public:
        sgt_widget* subwidgets[subl];
        virtual int draw(SDL_Renderer*, int x, int y, int width, int height) override = 0;
        virtual ~sgt_container() {
            for (int i = 0; i < subl; ++i) {
                delete subwidgets[i];
            }
        }
        virtual void update() override = 0;
        virtual void updateAll() override {
            for (int i = 0; i < subl; ++i) {
                subwidgets[i]->updateAll();
                update();
            }
        }
};

class sgt_window : public sgt_container<1> {
    public:
        SDL_Color bg;
        virtual int draw(SDL_Renderer*, int x, int y, int width, int height) override;
        virtual void update() override;
        virtual void handle_event(int x, int y, int width, int height, SDL_Event e) override;
};

class sgt_button : public sgt_widget {
    public:
        bool clicked;

        bool active;
        bool highlight;
        sgt_image normalimg;
        sgt_image activeimg;
        sgt_image highlightimg;
        virtual int draw(SDL_Renderer*, int x, int y, int width, int height) override;
        virtual void update() override;
        virtual void handle_event(int x, int y, int width, int height, SDL_Event e) override;

    private:
        SDL_Rect makerect(int, int, int, int);
};

class sgt_label : public sgt_widget {
    public:
        char msg [MAX_MSG_BUFFER];
        sgt_font font;
        SDL_Color color;
        virtual int draw(SDL_Renderer*, int x, int y, int width, int height) override;
        virtual void update() override;
        virtual void handle_event(int x, int y, int width, int height, SDL_Event e) override;
};

class sgt_vstack : public sgt_container<2> {
    public:
        virtual int draw(SDL_Renderer*, int x, int y, int width, int height) override;
        virtual void update() override;
        virtual void handle_event(int x, int y, int width, int height, SDL_Event e) override;
    private:
        void compute_height(int, int*, int*);
};

class sgt_hstack : public sgt_container<2> {
    public:
        virtual int draw(SDL_Renderer*, int x, int y, int width, int height) override;
        virtual void update() override;
        virtual void handle_event(int x, int y, int width, int height, SDL_Event e) override;
    private:
        void compute_width(int, int*, int*);
};

    sgt_hstack* sgt_hstack_create(sgt_widget*, sgt_widget*);
    sgt_vstack* sgt_vstack_create(sgt_widget*, sgt_widget*);
    sgt_window* sgt_window_create(sgt_widget*, int width, int height, SDL_Color color);
    sgt_label* sgt_label_create(sgt_font font, SDL_Color color);
    sgt_button* sgt_button_create(sgt_image, sgt_image, sgt_image);
#endif
