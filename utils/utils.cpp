#include "includes_bloat.h"
#include "utils.h"

bool utils::compare(color A,color B, float noisegate = 15.f)
{
    unsigned int r = A.r - B.r;
    unsigned int g = A.g - B.g;
    unsigned int b = A.b - B.b;

    float d = sqrt ( r * r + g * g  + b * b);
    return d > noisegate;
};

/**
 * https://stackoverflow.com/questions/18281412/check-keypress-in-c-on-linux
 * @param ks  like XK_Shift_L, see /usr/include/X11/keysymdef.h
 * @return
 */
bool utils::iskeydown(KeySym key) {
    Display *dpy = XOpenDisplay(":0");
    char keys_return[32];
    XQueryKeymap(dpy, keys_return);
    KeyCode kc2 = XKeysymToKeycode(dpy, key);
    bool isPressed = !!(keys_return[kc2 >> 3] & (1 << (kc2 & 7)));
    XCloseDisplay(dpy);
    return isPressed;
}
