#include <stdio.h>
#include <string.h>
#include "safe_malloc.h"
#include "vrms_client.h"
#include "vrms_render_vm.h"
#include "vrms_geometry.h"
#include "esm.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void std_cube_generate_verticies(float* verts, uint32_t x, uint32_t y, uint32_t z) {
    uint32_t off = 0;

    // front
    verts[off + 0] = 0.0f; // 0
    verts[off + 1] = 0.0f;
    verts[off + 2] = 0.0f;
    off += 3;
    verts[off + 0] = x; // 1
    verts[off + 1] = 0.0f;
    verts[off + 2] = 0.0f;
    off += 3;
    verts[off + 0] = 0.0f; // 2
    verts[off + 1] = y;
    verts[off + 2] = 0.0f;
    off += 3;
    verts[off + 0] = x; // 3
    verts[off + 1] = y;
    verts[off + 2] = 0.0f;
    off += 3;

    // left
    verts[off + 0] = 0.0f; // 4
    verts[off + 1] = 0.0f;
    verts[off + 2] = 0.0f;
    off += 3;
    verts[off + 0] = 0.0f; // 5
    verts[off + 1] = y;
    verts[off + 2] = z;
    off += 3;
    verts[off + 0] = 0.0f; // 6
    verts[off + 1] = 0.0f;
    verts[off + 2] = z;
    off += 3;
    verts[off + 0] = 0.0f; // 7
    verts[off + 1] = y;
    verts[off + 2] = 0.0f;
    off += 3;

    // right
    verts[off + 0] = x; // 8
    verts[off + 1] = y;
    verts[off + 2] = 0.0f;
    off += 3;
    verts[off + 0] = x; // 9
    verts[off + 1] = 0.0f;
    verts[off + 2] = z;
    off += 3;
    verts[off + 0] = x; // 10
    verts[off + 1] = y;
    verts[off + 2] = z;
    off += 3;
    verts[off + 0] = x; // 11
    verts[off + 1] = 0.0f;
    verts[off + 2] = 0.0f;
    off += 3;

    // top
    verts[off + 0] = x; // 12
    verts[off + 1] = y;
    verts[off + 2] = 0.0f;
    off += 3;
    verts[off + 0] = 0.0f; // 13
    verts[off + 1] = y;
    verts[off + 2] = z;
    off += 3;
    verts[off + 0] = x; // 14
    verts[off + 1] = y;
    verts[off + 2] = z;
    off += 3;
    verts[off + 0] = 0.0f; // 15
    verts[off + 1] = y;
    verts[off + 2] = 0.0f;
    off += 3;

    // bottom
    verts[off + 0] = 0.0f; // 16
    verts[off + 1] = 0.0f;
    verts[off + 2] = 0.0f;
    off += 3;
    verts[off + 0] = x; // 17
    verts[off + 1] = 0.0f;
    verts[off + 2] = z;
    off += 3;
    verts[off + 0] = 0.0f; // 18
    verts[off + 1] = 0.0f;
    verts[off + 2] = z;
    off += 3;
    verts[off + 0] = x; // 19
    verts[off + 1] = 0.0f;
    verts[off + 2] = 0.0f;
    off += 3;

    // back
    verts[off + 0] = 0.0f; // 20
    verts[off + 1] = 0.0f;
    verts[off + 2] = z;
    off += 3;
    verts[off + 0] = x; // 21
    verts[off + 1] = y;
    verts[off + 2] = z;
    off += 3;
    verts[off + 0] = 0.0f; // 22
    verts[off + 1] = y;
    verts[off + 2] = z;
    off += 3;
    verts[off + 0] = x; // 23
    verts[off + 1] = 0.0f;
    verts[off + 2] = z;
    off += 3;
}

void std_cube_generate_normals(float* norms) {
    uint32_t off = 0;

    // [+-1, 0, 0],   [0,+-1,0],    [0,0,+-1]
    // (Left/Right), (Top/Bottom), (Front/Back)

    // front
    norms[off + 0] = 0.0f; // 0
    norms[off + 1] = 0.0f;
    norms[off + 2] = -1.0f;
    off += 3;
    norms[off + 0] = 0.0f; // 1
    norms[off + 1] = 0.0f;
    norms[off + 2] = -1.0f;
    off += 3;
    norms[off + 0] = 0.0f; // 2
    norms[off + 1] = 0.0f;
    norms[off + 2] = -1.0f;
    off += 3;
    norms[off + 0] = 0.0f; // 3
    norms[off + 1] = 0.0f;
    norms[off + 2] = -1.0f;
    off += 3;

    // left
    norms[off + 0] = -1.0f; // 4
    norms[off + 1] = 0.0f;
    norms[off + 2] = 0.0f;
    off += 3;
    norms[off + 0] = -1.0f; // 5
    norms[off + 1] = 0.0f;
    norms[off + 2] = 0.0f;
    off += 3;
    norms[off + 0] = -1.0f; // 6
    norms[off + 1] = 0.0f;
    norms[off + 2] = 0.0f;
    off += 3;
    norms[off + 0] = -1.0f; // 7
    norms[off + 1] = 0.0f;
    norms[off + 2] = 0.0f;
    off += 3;

    // right
    norms[off + 0] = 1.0f; // 8
    norms[off + 1] = 0.0f;
    norms[off + 2] = 0.0f;
    off += 3;
    norms[off + 0] = 1.0f; // 9
    norms[off + 1] = 0.0f;
    norms[off + 2] = 0.0f;
    off += 3;
    norms[off + 0] = 1.0f; // 10
    norms[off + 1] = 0.0f;
    norms[off + 2] = 0.0f;
    off += 3;
    norms[off + 0] = 1.0f; // 11
    norms[off + 1] = 0.0f;
    norms[off + 2] = 0.0f;
    off += 3;

    // top
    norms[off + 0] = 0.0f; // 12
    norms[off + 1] = 1.0f;
    norms[off + 2] = 0.0f;
    off += 3;
    norms[off + 0] = 0.0f; // 13
    norms[off + 1] = 1.0f;
    norms[off + 2] = 0.0f;
    off += 3;
    norms[off + 0] = 0.0f; // 14
    norms[off + 1] = 1.0f;
    norms[off + 2] = 0.0f;
    off += 3;
    norms[off + 0] = 0.0f; // 15
    norms[off + 1] = 1.0f;
    norms[off + 2] = 0.0f;
    off += 3;

    // bottom
    norms[off + 0] = 0.0f; // 16
    norms[off + 1] = -1.0f;
    norms[off + 2] = 0.0f;
    off += 3;
    norms[off + 0] = 0.0f; // 17
    norms[off + 1] = -1.0f;
    norms[off + 2] = 0.0f;
    off += 3;
    norms[off + 0] = 0.0f; // 18
    norms[off + 1] = -1.0f;
    norms[off + 2] = 0.0f;
    off += 3;
    norms[off + 0] = 0.0f; // 19
    norms[off + 1] = -1.0f;
    norms[off + 2] = 0.0f;
    off += 3;

    // back
    norms[off + 0] = 0.0f; // 20
    norms[off + 1] = 0.0f;
    norms[off + 2] = 1.0f;
    off += 3;
    norms[off + 0] = 0.0f; // 21
    norms[off + 1] = 0.0f;
    norms[off + 2] = 1.0f;
    off += 3;
    norms[off + 0] = 0.0f; // 22
    norms[off + 1] = 0.0f;
    norms[off + 2] = 1.0f;
    off += 3;
    norms[off + 0] = 0.0f; // 23
    norms[off + 1] = 0.0f;
    norms[off + 2] = 1.0f;
    off += 3;
}

void std_cube_generate_indicies(uint16_t* indicies) {
    uint32_t off = 0;

    // front
    indicies[off + 0] = 0;
    indicies[off + 1] = 1;
    indicies[off + 2] = 2;
    indicies[off + 3] = 1;
    indicies[off + 4] = 2;
    indicies[off + 5] = 3;
    off += 6;

    // left
    indicies[off + 0] = 4;
    indicies[off + 1] = 5;
    indicies[off + 2] = 6;
    indicies[off + 3] = 4;
    indicies[off + 4] = 5;
    indicies[off + 5] = 7;
    off += 6;

    // right
    indicies[off + 0] = 8;
    indicies[off + 1] = 9;
    indicies[off + 2] = 10;
    indicies[off + 3] = 8;
    indicies[off + 4] = 9;
    indicies[off + 5] = 11;
    off += 6;

    // top
    indicies[off + 0] = 12;
    indicies[off + 1] = 13;
    indicies[off + 2] = 14;
    indicies[off + 3] = 12;
    indicies[off + 4] = 13;
    indicies[off + 5] = 15;
    off += 6;

    // bottom
    indicies[off + 0] = 16;
    indicies[off + 1] = 17;
    indicies[off + 2] = 18;
    indicies[off + 3] = 16;
    indicies[off + 4] = 17;
    indicies[off + 5] = 19;
    off += 6;

    // back
    indicies[off + 0] = 20;
    indicies[off + 1] = 21;
    indicies[off + 2] = 22;
    indicies[off + 3] = 20;
    indicies[off + 4] = 21;
    indicies[off + 5] = 23;
    off += 6;
}

void std_cube_generate_uvs(float* uvs) {
    uint32_t off = 0;

    // front
    uvs[off + 0] = 0.0f;
    uvs[off + 1] = 0.0f;
    off += 2;

    uvs[off + 0] = 1.0f;
    uvs[off + 1] = 0.0f;
    off += 2;

    uvs[off + 0] = 0.0f;
    uvs[off + 1] = 1.0f;
    off += 2;

    uvs[off + 0] = 1.0f;
    uvs[off + 1] = 1.0f;
    off += 2;

    // left
    uvs[off + 0] = 0.0f;
    uvs[off + 1] = 0.0f;
    off += 2;

    uvs[off + 0] = 1.0f;
    uvs[off + 1] = 0.0f;
    off += 2;

    uvs[off + 0] = 0.0f;
    uvs[off + 1] = 1.0f;
    off += 2;

    uvs[off + 0] = 1.0f;
    uvs[off + 1] = 1.0f;
    off += 2;

    // right
    uvs[off + 0] = 0.0f;
    uvs[off + 1] = 0.0f;
    off += 2;

    uvs[off + 0] = 1.0f;
    uvs[off + 1] = 0.0f;
    off += 2;

    uvs[off + 0] = 0.0f;
    uvs[off + 1] = 1.0f;
    off += 2;

    uvs[off + 0] = 1.0f;
    uvs[off + 1] = 1.0f;
    off += 2;

    // top
    uvs[off + 0] = 0.0f;
    uvs[off + 1] = 0.0f;
    off += 2;

    uvs[off + 0] = 1.0f;
    uvs[off + 1] = 0.0f;
    off += 2;

    uvs[off + 0] = 0.0f;
    uvs[off + 1] = 1.0f;
    off += 2;

    uvs[off + 0] = 1.0f;
    uvs[off + 1] = 1.0f;
    off += 2;

    // bottom
    uvs[off + 0] = 0.0f;
    uvs[off + 1] = 0.0f;
    off += 2;

    uvs[off + 0] = 1.0f;
    uvs[off + 1] = 0.0f;
    off += 2;

    uvs[off + 0] = 0.0f;
    uvs[off + 1] = 1.0f;
    off += 2;

    uvs[off + 0] = 1.0f;
    uvs[off + 1] = 1.0f;
    off += 2;

    // back
    uvs[off + 0] = 0.0f;
    uvs[off + 1] = 0.0f;
    off += 2;

    uvs[off + 0] = 1.0f;
    uvs[off + 1] = 0.0f;
    off += 2;

    uvs[off + 0] = 0.0f;
    uvs[off + 1] = 1.0f;
    off += 2;

    uvs[off + 0] = 1.0f;
    uvs[off + 1] = 1.0f;
    off += 2;
}

uint32_t vrms_geometry_cube(vrms_client_t* client, uint32_t x, uint32_t y, uint32_t z, float r, float g, float b, float a) {
    uint32_t nr_verticies, nr_indicies, nr_vert_floats;
    size_t size_of_verts, size_of_indicies;
    uint32_t memory_id;
    uint8_t* address = NULL;
    float* verts;
    float* norms;
    uint16_t* indicies;
    uint32_t buff_off;

    nr_verticies = 4 * 6;
    nr_vert_floats = 3 * nr_verticies;

    verts = SAFEMALLOC(sizeof(float) * nr_vert_floats);
    norms = SAFEMALLOC(sizeof(float) * nr_vert_floats);

    nr_indicies = 6 * 6;
    indicies = SAFEMALLOC(sizeof(uint16_t) * nr_indicies);

    std_cube_generate_verticies(verts, x, y, z);
    std_cube_generate_normals(norms);
    std_cube_generate_indicies(indicies);

    size_of_verts = sizeof(float) * nr_vert_floats;
    size_of_indicies = sizeof(uint16_t) * nr_indicies;

    memory_id = vrms_client_create_memory(client, &address, (size_of_verts * 2) + size_of_indicies);
    if (0 == memory_id) {
        return 0;
    }

    buff_off = 0;
    memcpy(&address[buff_off], verts, size_of_verts);
    uint32_t vertex_id = vrms_client_create_data_object(client, VRMS_VERTEX, memory_id, buff_off, size_of_verts, sizeof(float));

    buff_off += size_of_verts;
    memcpy(&address[buff_off], norms, size_of_verts);
    uint32_t normal_id = vrms_client_create_data_object(client, VRMS_NORMAL, memory_id, buff_off, size_of_verts, sizeof(float));

    buff_off += size_of_verts;
    memcpy(&address[buff_off], indicies, size_of_indicies);
    uint32_t index_id = vrms_client_create_data_object(client, VRMS_INDEX, memory_id, buff_off, size_of_indicies, sizeof(uint16_t));

    uint32_t geometry_id = vrms_client_create_geometry_object(client, vertex_id, normal_id, index_id);

    uint32_t mesh_id = vrms_client_create_mesh_color(client, geometry_id, r, g, b, a);

    free(verts);
    free(norms);
    free(indicies);

    return mesh_id;
}

// destination == where the square should go
// source    == source data
// square_px == nr. pixels per side of the square
// bytes_pl  == bytes per line original image
// bytes_pp  == bytes per pixel
// x_idx     == index of the square in x
// y_idx     == index of the square in y
// dest_off  == location to put the data
void vrms_geometry_image_copy_square(uint8_t* destination, uint8_t* source, uint32_t square_px, uint32_t bytes_pl, uint32_t bytes_pp, uint32_t x_idx, uint32_t y_idx, uint32_t dest_off) {
    uint32_t i, start, length;

    start = ((square_px * y_idx) * bytes_pl) + ((square_px * x_idx) * bytes_pp);
    length = (square_px * bytes_pp);

    for (i = 0; i < square_px; i++) {
        memcpy(&destination[dest_off], &source[start], length);
        start += bytes_pl;
        dest_off += length;
    }
}

uint32_t vrms_load_skybox_texture(vrms_client_t* client, const char* filename) {
    int32_t memory_id;
    int32_t width, height, bytes_pp;
    uint8_t* data = NULL;
    uint8_t* address = NULL;
    uint32_t total_byte_size;
    uint32_t square_px;
    uint32_t bytes_sq;
    uint32_t bytes_pl;
    uint32_t dest_off;

    width = 0;
    height = 0;
    bytes_pp = 0;
    data = stbi_load(filename, &width, &height, &bytes_pp, 3);
    if (NULL == data) {
        return 0;
    }

    square_px = width / 4;
    bytes_pl = width * bytes_pp;
    bytes_sq = square_px * square_px * bytes_pp;
    total_byte_size = bytes_sq * 6;

    memory_id = vrms_client_create_memory(client, &address, total_byte_size);
    if (0 == memory_id) {
        stbi_image_free(data);
        return 0;
    }

    dest_off = 0;

    /*
          [YPOS]
    [XNEG][ZPOS][XPOS][ZNEG]
          [YNEG]
    */

    // XPOS
    vrms_geometry_image_copy_square(address, data, square_px, bytes_pl, bytes_pp, 2, 1, dest_off);
    dest_off += bytes_sq;

    // XNEG
    vrms_geometry_image_copy_square(address, data, square_px, bytes_pl, bytes_pp, 0, 1, dest_off);
    dest_off += bytes_sq;

    // YPOS
    vrms_geometry_image_copy_square(address, data, square_px, bytes_pl, bytes_pp, 1, 2, dest_off);
    dest_off += bytes_sq;

    // YNEG
    vrms_geometry_image_copy_square(address, data, square_px, bytes_pl, bytes_pp, 1, 0, dest_off);
    dest_off += bytes_sq;

    // ZPOS
    vrms_geometry_image_copy_square(address, data, square_px, bytes_pl, bytes_pp, 1, 1, dest_off);
    dest_off += bytes_sq;

    // ZNEG
    vrms_geometry_image_copy_square(address, data, square_px, bytes_pl, bytes_pp, 3, 1, dest_off);
    dest_off += bytes_sq;

    uint32_t texture_id = vrms_client_create_texture_object(client, memory_id, 0, bytes_sq, square_px, square_px, VRMS_RGB8, VRMS_TEXTURE_CUBE_MAP);

    return texture_id;
}

uint32_t vrms_load_texture(vrms_client_t* client, const char* filename, vrms_texture_type_t type) {
    int32_t memory_id;
    int32_t width, height, bytes_pp;
    uint8_t* data = NULL;
    uint8_t* address = NULL;
    uint32_t total_byte_size;

    width = 0;
    height = 0;
    bytes_pp = 0;
    data = stbi_load(filename, &width, &height, &bytes_pp, 3);
    if (NULL == data) {
        stbi_image_free(data);
        return 0;
    }

    total_byte_size = width * height * bytes_pp;

    memory_id = vrms_client_create_memory(client, &address, total_byte_size);
    if (0 == memory_id) {
        return 0;
    }

    memcpy(address, data, total_byte_size);

    uint32_t texture_id = vrms_client_create_texture_object(client, memory_id, 0, total_byte_size, width, height, VRMS_RGB8, type);

    return texture_id;
}

uint32_t vrms_geometry_cube_textured(vrms_client_t* client, uint32_t x, uint32_t y, uint32_t z, const char* filename) {
    uint32_t nr_verticies, nr_indicies, nr_vert_floats, nr_uv_floats;
    size_t size_of_verts, size_of_norms, size_of_indicies, size_of_uvs, size_total;
    uint32_t buff_off;

    int32_t memory_id;
    uint8_t* address = NULL;

    float* verts;
    float* norms;
    uint16_t* indicies;
    float* uvs;

    nr_verticies = 4 * 6;
    nr_vert_floats = 3 * nr_verticies;
    nr_indicies = 6 * 6;
    nr_uv_floats = nr_verticies * 2;

    verts = SAFEMALLOC(sizeof(float) * nr_vert_floats);
    norms = SAFEMALLOC(sizeof(float) * nr_vert_floats);
    indicies = SAFEMALLOC(sizeof(uint16_t) * nr_indicies);
    uvs = SAFEMALLOC(sizeof(float) * nr_uv_floats);

    std_cube_generate_verticies(verts, x, y, z);
    std_cube_generate_normals(norms);
    std_cube_generate_indicies(indicies);
    std_cube_generate_uvs(uvs);

    size_of_verts = sizeof(float) * nr_vert_floats;
    size_of_norms = size_of_verts;
    size_of_indicies = sizeof(uint16_t) * nr_indicies;
    size_of_uvs = sizeof(float) * nr_uv_floats;
    size_total = size_of_verts + size_of_norms + size_of_indicies + size_of_uvs;

    memory_id = vrms_client_create_memory(client, &address, size_total);
    if (0 == memory_id) {
        return 0;
    }

    buff_off = 0;
    memcpy(&address[buff_off], verts, size_of_verts);
    uint32_t vertex_id = vrms_client_create_data_object(client, VRMS_VERTEX, memory_id, buff_off, size_of_verts, sizeof(float));

    buff_off += size_of_verts;
    memcpy(&address[buff_off], norms, size_of_verts);
    uint32_t normal_id = vrms_client_create_data_object(client, VRMS_NORMAL, memory_id, buff_off, size_of_verts, sizeof(float));

    buff_off += size_of_norms;
    memcpy(&address[buff_off], indicies, size_of_indicies);
    uint32_t index_id = vrms_client_create_data_object(client, VRMS_INDEX, memory_id, buff_off, size_of_indicies, sizeof(uint16_t));

    buff_off += size_of_indicies;
    memcpy(&address[buff_off], uvs, size_of_uvs);
    uint32_t uv_id = vrms_client_create_data_object(client, VRMS_UV, memory_id, buff_off, size_of_uvs, sizeof(float));

    uint32_t geometry_id = vrms_client_create_geometry_object(client, vertex_id, normal_id, index_id);
    uint32_t texture_id = vrms_load_texture(client, filename, VRMS_TEXTURE_2D);
    if (0 == texture_id) {
        fprintf(stderr, "unable to create texture\n");
    }

    uint32_t mesh_id = vrms_client_create_mesh_texture(client, geometry_id, texture_id, uv_id);

    free(verts);
    free(norms);
    free(indicies);
    free(uvs);

    return mesh_id;
}

uint32_t vrms_geometry_skybox(vrms_client_t* client, const char* filename) {
    uint32_t texture_id, skybox_id;

    texture_id = vrms_load_skybox_texture(client, filename);
    skybox_id = vrms_client_create_skybox(client, texture_id, 10);

    return skybox_id;
}

void std_plane_generate_verticies(float* verts, uint32_t x, uint32_t y) {
    uint32_t off = 0;

    verts[off + 0] = 0.0f; // 0
    verts[off + 1] = 0.0f;
    verts[off + 2] = 0.0f;
    off += 3;
    verts[off + 0] = x; // 1
    verts[off + 1] = 0.0f;
    verts[off + 2] = 0.0f;
    off += 3;
    verts[off + 0] = 0.0f; // 2
    verts[off + 1] = y;
    verts[off + 2] = 0.0f;
    off += 3;
    verts[off + 0] = x; // 3
    verts[off + 1] = y;
    verts[off + 2] = 0.0f;
    off += 3;
}

void std_plane_generate_normals(float* norms) {
    uint32_t off = 0;

    norms[off + 0] = 0.0f; // 0
    norms[off + 1] = 0.0f;
    norms[off + 2] = 1.0f;
    off += 3;
    norms[off + 0] = 0.0f; // 1
    norms[off + 1] = 0.0f;
    norms[off + 2] = 1.0f;
    off += 3;
    norms[off + 0] = 0.0f; // 2
    norms[off + 1] = 0.0f;
    norms[off + 2] = 1.0f;
    off += 3;
    norms[off + 0] = 0.0f; // 3
    norms[off + 1] = 0.0f;
    norms[off + 2] = 1.0f;
    off += 3;
}

void std_plane_generate_indicies(uint16_t* indicies) {
    uint32_t off = 0;

    indicies[off + 0] = 0;
    indicies[off + 1] = 1;
    indicies[off + 2] = 2;
    indicies[off + 3] = 1;
    indicies[off + 4] = 2;
    indicies[off + 5] = 3;
    off += 6;
}

void std_plane_generate_uvs(float* uvs) {
    uint32_t off = 0;

    uvs[off + 0] = 0.0f;
    uvs[off + 1] = 0.0f;
    off += 2;

    uvs[off + 0] = 1.0f;
    uvs[off + 1] = 0.0f;
    off += 2;

    uvs[off + 0] = 0.0f;
    uvs[off + 1] = 1.0f;
    off += 2;

    uvs[off + 0] = 1.0f;
    uvs[off + 1] = 1.0f;
    off += 2;
}

uint32_t vrms_geometry_plane(vrms_client_t* client, uint32_t x, uint32_t y, float r, float g, float b, float a) {
    uint32_t nr_verticies, nr_indicies, nr_vert_floats;
    size_t size_of_verts, size_of_indicies;

    int32_t memory_id;
    uint8_t* address = NULL;

    float* verts;
    float* norms;
    uint16_t* indicies;

    nr_verticies = 4;
    nr_vert_floats = 3 * nr_verticies;

    verts = SAFEMALLOC(sizeof(float) * nr_vert_floats);
    norms = SAFEMALLOC(sizeof(float) * nr_vert_floats);

    nr_indicies = 6;
    indicies = SAFEMALLOC(sizeof(uint16_t) * nr_indicies);

    std_plane_generate_verticies(verts, x, y);
    std_plane_generate_normals(norms);
    std_plane_generate_indicies(indicies);

    size_of_verts = sizeof(float) * nr_vert_floats;
    size_of_indicies = sizeof(uint16_t) * nr_indicies;

    memory_id = vrms_client_create_memory(client, &address, (size_of_verts * 2) + size_of_indicies);
    if (0 == memory_id) {
        return 0;
    }

    memcpy(address, verts, size_of_verts);
    memcpy(&address[size_of_verts], norms, size_of_verts);
    memcpy(&address[size_of_verts * 2], indicies, size_of_indicies);

    uint32_t vertex_id = vrms_client_create_data_object(client, VRMS_VERTEX, memory_id, 0, size_of_verts, sizeof(float));
    uint32_t normal_id = vrms_client_create_data_object(client, VRMS_NORMAL, memory_id, size_of_verts, size_of_verts, sizeof(float));
    uint32_t index_id = vrms_client_create_data_object(client, VRMS_INDEX, memory_id, size_of_verts * 2, size_of_indicies, sizeof(uint16_t));

    uint32_t geometry_id = vrms_client_create_geometry_object(client, vertex_id, normal_id, index_id);

    uint32_t mesh_id = vrms_client_create_mesh_color(client, geometry_id, r, g, b, a);

    free(verts);
    free(norms);
    free(indicies);

    return mesh_id;
}

uint32_t vrms_geometry_plane_textured(vrms_client_t* client, uint32_t x, uint32_t y, const char* filename) {
    uint32_t nr_verticies, nr_indicies, nr_vert_floats, nr_uv_floats;
    size_t size_of_verts, size_of_norms, size_of_indicies, size_of_uvs, size_total;
    uint32_t buff_off;

    int32_t memory_id;
    uint8_t* address = NULL;

    float* verts;
    float* norms;
    uint16_t* indicies;
    float* uvs;

    nr_verticies = 4;
    nr_vert_floats = 3 * nr_verticies;
    nr_indicies = 6;
    nr_uv_floats = nr_verticies * 2;

    verts = SAFEMALLOC(sizeof(float) * nr_vert_floats);
    norms = SAFEMALLOC(sizeof(float) * nr_vert_floats);
    indicies = SAFEMALLOC(sizeof(uint16_t) * nr_indicies);
    uvs = SAFEMALLOC(sizeof(float) * nr_uv_floats);

    std_plane_generate_verticies(verts, x, y);
    std_plane_generate_normals(norms);
    std_plane_generate_indicies(indicies);
    std_plane_generate_uvs(uvs);

    size_of_verts = sizeof(float) * nr_vert_floats;
    size_of_norms = size_of_verts;
    size_of_indicies = sizeof(uint16_t) * nr_indicies;
    size_of_uvs = sizeof(float) * nr_uv_floats;
    size_total = size_of_verts + size_of_norms + size_of_indicies + size_of_uvs;

    memory_id = vrms_client_create_memory(client, &address, size_total);
    if (0 == memory_id) {
        return 0;
    }

    buff_off = 0;
    memcpy(&address[buff_off], verts, size_of_verts);
    uint32_t vertex_id = vrms_client_create_data_object(client, VRMS_VERTEX, memory_id, 0, size_of_verts, sizeof(float));

    buff_off += size_of_verts;
    memcpy(&address[buff_off], norms, size_of_verts);
    uint32_t normal_id = vrms_client_create_data_object(client, VRMS_NORMAL, memory_id, buff_off, size_of_verts, sizeof(float));

    buff_off += size_of_verts;
    memcpy(&address[buff_off], indicies, size_of_indicies);
    uint32_t index_id = vrms_client_create_data_object(client, VRMS_INDEX, memory_id, buff_off, size_of_indicies, sizeof(uint16_t));

    buff_off += size_of_indicies;
    memcpy(&address[buff_off], uvs, size_of_uvs);
    uint32_t uv_id = vrms_client_create_data_object(client, VRMS_UV, memory_id, buff_off, size_of_uvs, sizeof(float));

    uint32_t geometry_id = vrms_client_create_geometry_object(client, vertex_id, normal_id, index_id);
    uint32_t texture_id = vrms_load_texture(client, filename, VRMS_TEXTURE_2D);
    if (0 == texture_id) {
        fprintf(stderr, "unable to create texture\n");
    }

    uint32_t mesh_id = vrms_client_create_mesh_texture(client, geometry_id, texture_id, uv_id);

    free(verts);
    free(norms);
    free(indicies);
    free(uvs);

    return mesh_id;
}

uint32_t vrms_geometry_load_matrix_data(vrms_client_t* client, uint32_t nr_matricies, float* matrix_data) {
    size_t size_of_matricies;
    int32_t memory_id;
    uint8_t* address = NULL;

    size_of_matricies = (sizeof(float) * 16) * nr_matricies;
    memory_id = vrms_client_create_memory(client, &address, size_of_matricies);
    if (0 == memory_id) {
        return 0;
    }

    memcpy(address, matrix_data, size_of_matricies);

    uint32_t matrix_id = vrms_client_create_data_object(client, VRMS_MATRIX, memory_id, 0, size_of_matricies, sizeof(float));

    return matrix_id;
}

uint8_t vrms_geometry_render_buffer_basic(vrms_client_t* client, uint32_t object_id, float x, float y, float z) {
    uint32_t memory_id;
    uint32_t render_ret;
    uint32_t matrix_size;
    uint32_t render_size;
    uint8_t* shared_mem;
    float* model_matrix;
    uint8_t* render_buffer;

    matrix_size = sizeof(float) * 16;
    render_size = sizeof(uint8_t) * 22;
    memory_id = vrms_client_create_memory(client, &shared_mem, matrix_size + render_size);

    if (NULL == shared_mem) {
        fprintf(stderr, "Unable to initialize shared memory\n");
        return 0;
    }

    model_matrix = (float*)shared_mem;
    render_buffer = (uint8_t*)&shared_mem[matrix_size];

    render_buffer[0] = VM_LOADI;
    render_buffer[1] = VM_REG0;
    render_buffer[2] = object_id;
    render_buffer[3] = VM_LOADI;
    render_buffer[4] = VM_REG1;
    render_buffer[5] = memory_id;
    render_buffer[6] = VM_LOADI;
    render_buffer[7] = VM_REG2;
    render_buffer[8] = 0;
    render_buffer[9] = VM_LOADI;
    render_buffer[10] = VM_REG3;
    render_buffer[11] = 16;
    render_buffer[12] = VM_LOADMM;
    render_buffer[13] = VM_REG0;
    render_buffer[14] = VM_REG1;
    render_buffer[15] = VM_REG2;
    render_buffer[16] = VM_DRAW;
    render_buffer[17] = VM_REG0;
    render_buffer[18] = VM_REG0;
    render_buffer[19] = VM_FRWAIT;
    render_buffer[20] = VM_GOTO;
    render_buffer[21] = VM_REG3;

    esmLoadIdentity(model_matrix);
    esmTranslatef(model_matrix, x, y, z);

    render_ret = vrms_client_render_buffer_set(client, memory_id, matrix_size, render_size);

    if (render_ret) {
        return 1;
    }
    return 0;
}
