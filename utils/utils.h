struct vec2 
{
    int x, y;
};

struct color 
{
    unsigned char r, g, b;
};

namespace utils 
{
    bool compare(color A,color B, float noisegate);
    bool iskeydown(KeySym key);
};