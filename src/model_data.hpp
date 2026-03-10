#pragma once

float ground[] =
{
    // Position           // Texture coordinates
    -0.5f, 0.0f, -0.5f,   0.0f, 0.0f,
     0.5f, 0.0f, -0.5f,   5.0f, 0.0f,
     0.5f, 0.0f,  0.5f,   5.0f, 5.0f,
    -0.5f, 0.0f,  0.5f,   0.0f, 5.0f
};

float street[] =
{
    // Position           // Texture coordinates
    -0.5f, 0.0f, -0.5f,   0.0f, 0.0f,
     0.5f, 0.0f, -0.5f,   2.0f, 0.0f,
     0.5f, 0.0f,  0.5f,   2.0f, 2.0f,
    -0.5f, 0.0f,  0.5f,   0.0f, 2.0f
};

float streetcorner[] =
{
    // Position           // Texture coordinates
    -0.5f, 0.0f, -0.5f,   0.0f, 0.0f,
     0.5f, 0.0f, -0.5f,   1.0f, 0.0f,
     0.5f, 0.0f,  0.5f,   1.0f, 1.0f,
    -0.5f, 0.0f,  0.5f,   0.0f, 1.0f
};

unsigned int planeElements[] =
{
    0, 2, 1,
    0, 3, 2
};
