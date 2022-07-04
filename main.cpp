#include "utils/includes_bloat.h"
#include "utils/utils.h"

using namespace std;

#define scanrange_default 12
#define noisegate_default 5.f
#define extrareactiondelay_default 0
#define maxframeslimit_default 512
#define triggerkey_default XK_V

// evil globals
bool is_enabled = false;

void listener(KeySym triggerkey) 
{
    for (;;)
    {
        is_enabled = utils::iskeydown(triggerkey);
        this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}

int main(int argc, char ** argv) {

    // **support** used for multimonitor, windowed, offcenter crosshair, all that good stuff.
    int offsetx = 0;
    int offsety = 0;

    int scanrange = scanrange_default;
    float noisegate = noisegate_default;
    int extrareactiondelay = extrareactiondelay_default;
    int maxframeslimit = maxframeslimit_default;
    KeySym triggerkey = triggerkey_default; 

    if (argc > 1) 
    {
        scanrange = atoi(argv[1]);

        if (scanrange <= 0) 
        {
            scanrange = scanrange_default;
            cout << "invalid scan range, defaulting to " << scanrange << endl;
        }
    }
    if (argc > 2)
    {
        noisegate = atof(argv[2]);

        if (noisegate <= 0.1f)
        {
            noisegate = noisegate_default;
            cout << "invalid gate, defaulting to " << noisegate << endl;
        }   
    }
    if (argc > 3)
    {
        extrareactiondelay = atoi(argv[3]);

        if (extrareactiondelay < 0)
        {
            extrareactiondelay = extrareactiondelay_default;
            cout << "invalid delay, defaulting to " << extrareactiondelay << endl;
        }   
    }
    if (argc > 4)
    {
        maxframeslimit = atoi(argv[4]);

        if (maxframeslimit <= 0)
        {
            maxframeslimit = maxframeslimit_default;
            cout << "invalid limit, defaulting to " << maxframeslimit << endl;
        }   
    }
    if (argc > 6)
    {
        offsetx = atoi(argv[5]);
        offsety = atoi(argv[6]);
        if (offsetx == 0 && offsety == 0)
        {
            cout << "invalid offset detected" << endl;
        }
        else if (offsetx != 0 && offsety != 0)
        {
            cout << "offset X " << offsetx << " Y " << offsety << endl;
        }
        else if (offsetx != 0)
        {
            cout << "offset X " << offsetx << endl;
        }
        else if (offsety != 0)
        {
            cout << "offset Y " << offsety << endl;
        }
    }
    if (argc > 7)
    {
        triggerkey = atoi(argv[7]);
        if (triggerkey == 0)
        {
            triggerkey = triggerkey_default;
            cout << "invalid key, defaulting to " << triggerkey << endl;
        }

    }

    cout << "settings: "    << endl;
    cout << "scanrange: "   << scanrange << endl;
    cout << "gate: "        << noisegate << endl;
    cout << "extra delay: " << extrareactiondelay << endl;
    cout << "maxframes: "   << maxframeslimit << endl;
    if (offsetx != 0 || offsety != 0)
    {
        cout << "offsetX: "   << offsetx << endl;
        cout << "offsetY: "   << offsety << endl;
    }
    else
    {
        cout << "no offset, auto range detection" << endl;
    }
    cout << "triggerkey: "   << triggerkey << endl;

    const int sleeper = (1/maxframeslimit)*1000;

    vec2 scanPoints[4] = 
    { 
        {0, 0}, 
        {0, scanrange-1}, 
        {scanrange-1, 0}, 
        {scanrange-1, scanrange-1}
    };

    Display * display = XOpenDisplay(NULL);

    Window root = DefaultRootWindow(display);

    XWindowAttributes gwa;
    XGetWindowAttributes(display, root, &gwa);
    int width = gwa.width, height = gwa.height;

    if (offsetx == 0)
    {
        offsetx = width/2;
        cout << "X: " << offsetx << endl;
    }
    if (offsety == 0)
    {
        offsety = height/2;
        cout << "Y: " << offsety << endl;
    }

    ofstream output("config.sh");
    output << "./glare " << scanrange 
    << " " << noisegate 
    << " " << extrareactiondelay 
    << " " << maxframeslimit 
    << " " << offsetx 
    << " " << offsety 
    << " " << triggerkey
    << endl;

    color LastCol[4] = { {0,0,0},{0,0,0},{0,0,0},{0,0,0} };
    color CurCol[4] = { {0,0,0},{0,0,0},{0,0,0},{0,0,0} };

    bool waitTick = true;

    cout << "starting listener" << endl;
    std::thread t1(listener, triggerkey);

    cout << "starting main loop" << endl;
    while (true)
    {
        XImage * image = XGetImage(
                    display,
                    root,
                    offsetx - scanrange/2,
                    offsety - scanrange/2,
                    scanrange,
                    scanrange,
                    AllPlanes,
                    ZPixmap
                );
        bool cur_differences[4] = {0,0,0,0};

        for (size_t i = 0; i < 4; i++)
        {
            unsigned long pixel = XGetPixel(image, scanPoints[i].x, scanPoints[i].y);
            unsigned char blue = pixel & image->blue_mask;
            unsigned char green = (pixel & image->green_mask) >> 8;
            unsigned char red = (pixel & image->red_mask) >> 16;
    
            CurCol[i] = {red, green, blue};
            if (utils::compare(LastCol[i], CurCol[i], noisegate) )
            {
                if (waitTick) 
                {
                    LastCol[i] = CurCol[i];
                }
                cur_differences[i] = true;
            }
        }
        waitTick = false;
        if (is_enabled) 
        {
            // check in checkerboard pattern
            if ((cur_differences[0] && cur_differences[3]) 
            || (cur_differences[1] && cur_differences[2]))
            {
                // shoot
                XTestFakeButtonEvent(display, Button1, true, extrareactiondelay);
                XFlush(display);
                this_thread::sleep_for(std::chrono::milliseconds(1));
                // release left mouse
                XTestFakeButtonEvent(display, Button1, false, 0);
                XFlush(display);
                waitTick = true;
            }
        }
        else 
        {
            waitTick = true;
        }
        XDestroyImage(image);

        this_thread::sleep_for(std::chrono::milliseconds(sleeper));
    }

    XCloseDisplay(display);
    return 0;
}
