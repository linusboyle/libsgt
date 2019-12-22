#ifndef SGT_ITF_H
#define SGT_ITF_H

#ifdef __cplusplus
extern "C" {
#endif

    void create_button(int);
    void create_label(int);
    void create_vstack(int, int, int);
    void create_hstack(int, int, int);
    void create_window(int, int);

    void notify_update_change();
    void notify_get_event();

    int getevent_button_click(int);

    void update_label_text(int, int);
#ifdef __cplusplus
}
#endif

#endif
