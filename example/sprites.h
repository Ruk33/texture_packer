static const char *sprites_paths[] = {
    "9foo.png",
    "9bar.png",
    "19baz.png",
};
enum sprites_id {
    foo,
    bar,
    baz,
    sprite_count,
};
// x, y, w, h, frames
static int sprites_coordinates[][5] = {
    {0, 64, 576, 64, 9},
    {1216, 0, 576, 64, 9},
    {0, 0, 1216, 64, 19},
};
