#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>

#ifdef RASPBERRYPI
#include <GLES/gl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#else /* not RASPBERRYPI */
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#endif /* RASPBERRYPI */

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <syscall.h>
#include <sys/un.h>
#include <linux/memfd.h>
#include <unistd.h>
#include "memfd.h"
#include "vrms_server.h"
#include "esm.h"

int VprintGlError(char *file, int line) {
    GLenum glErr;
    int retCode = 0;
    glErr = glGetError();
    switch (glErr) {
        case GL_INVALID_ENUM:
            printf("GL_INVALID_ENUM in file %s @ line %d: %d\n", file, line, glErr);
            retCode = 1;
        break;
        case GL_INVALID_VALUE:
            printf("GL_INVALID_VALUE in file %s @ line %d: %d\n", file, line, glErr);
            retCode = 1;
        break;
        case GL_INVALID_OPERATION:
            printf("GL_INVALID_OPERATION in file %s @ line %d: %d\n", file, line, glErr);
            retCode = 1;
        break;
        case GL_STACK_OVERFLOW:
            printf("GL_STACK_OVERFLOW in file %s @ line %d: %d\n", file, line, glErr);
            retCode = 1;
        break;
        case GL_STACK_UNDERFLOW:
            printf("GL_STACK_UNDERFLOW in file %s @ line %d: %d\n", file, line, glErr);
            retCode = 1;
        break;
        case GL_OUT_OF_MEMORY:
            printf("GL_OUT_OF_MEMORY in file %s @ line %d: %d\n", file, line, glErr);
            retCode = 1;
        break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            printf("GL_INVALID_FRAMEBUFFER_OPERATION in file %s @ line %d: %d\n", file, line, glErr);
            retCode = 1;
        break;
    }
    return retCode;
}

#define printOpenGLError() VprintGlError(__FILE__, __LINE__)

vrms_server_t* vrms_server_create() {
    vrms_server_t* server = malloc(sizeof(vrms_server_t));

    server->scenes = malloc(sizeof(vrms_scene_t) * 10);
    memset(server->scenes, 0, sizeof(vrms_scene_t) * 10);
    server->next_scene_id = 1;

    server->inbound_queue = malloc(sizeof(vrms_queue_item_t) * 10);
    memset(server->inbound_queue, 0, sizeof(vrms_queue_item_t) * 10);
    server->inbound_queue_index = 0;
    server->inbound_queue_lock = malloc(sizeof(pthread_mutex_t));
    memset(server->inbound_queue_lock, 0, sizeof(pthread_mutex_t));

    return server;
}

vrms_scene_t* vrms_server_get_scene(vrms_server_t* vrms_server, uint32_t scene_id) {
    vrms_scene_t* scene;
    if (scene_id >= vrms_server->next_scene_id) {
        fprintf(stderr, "invalid scene_id [%d] requested: id out of range\n", scene_id);
        return NULL;
    }
    scene = vrms_server->scenes[scene_id];
    if (NULL == scene) {
        fprintf(stderr, "invalid scene_id [%d] requested: scene does not exist\n", scene_id);
        return NULL;
    }
    return scene;
}

uint32_t vrms_create_scene(vrms_server_t* server, char* name) {
    vrms_scene_t* scene = malloc(sizeof(vrms_scene_t));
    memset(scene, 0, sizeof(vrms_scene_t));

    scene->server = server;

    scene->objects = malloc(sizeof(vrms_object_t) * 10);
    memset(scene->objects, 0, sizeof(vrms_object_t) * 10);
    scene->next_object_id = 1;

    server->scenes[server->next_scene_id] = scene;
    scene->id = server->next_scene_id;
    server->next_scene_id++;

    scene->render_buffer_nr_objects = 0;
    scene->render_buffer_lock = malloc(sizeof(pthread_mutex_t));
    memset(scene->render_buffer_lock, 0, sizeof(pthread_mutex_t));

    return scene->id;
}

vrms_object_t* vrms_scene_get_object_by_id(vrms_scene_t* vrms_scene, uint32_t id) {
    vrms_object_t* vrms_object;
    if (vrms_scene->next_object_id <= id) {
        fprintf(stderr, "id out of range\n");
        return NULL;
    }
    vrms_object = vrms_scene->objects[id];
    if (NULL == vrms_object) {
        fprintf(stderr, "undefined object ofr id: %d\n", id);
        return NULL;
    }
    return vrms_object;
}

void vrms_object_data_destroy(vrms_object_data_t* data) {
    if (NULL != data->local_storage) {
        free(data->local_storage);
    }
    if (0 < data->gl_id) {
        glDeleteBuffers(1, &data->gl_id);
    }
    free(data);
}

void vrms_object_geometry_destroy(vrms_object_geometry_t* geometry) {
    free(geometry);
}

void vrms_object_mesh_color_destroy(vrms_object_mesh_color_t* mesh_color) {
    free(mesh_color);
}

void vrms_object_mesh_texture_destroy(vrms_object_mesh_texture_t* mesh_texture) {
    free(mesh_texture);
}

void vrms_object_matrix_destroy(vrms_object_matrix_t* matrix) {
    free(matrix->data);
    free(matrix);
}

void vrms_object_destroy(vrms_object_t* object) {
    if (VRMS_OBJECT_DATA == object->type) {
        vrms_object_data_destroy(object->object.object_data);
    }
    else if (VRMS_OBJECT_GEOMETRY == object->type) {
        vrms_object_geometry_destroy(object->object.object_geometry);
    }
    else if (VRMS_OBJECT_MESH_COLOR == object->type) {
        vrms_object_mesh_color_destroy(object->object.object_mesh_color);
    }
    else if (VRMS_OBJECT_MESH_TEXTURE == object->type) {
        vrms_object_mesh_texture_destroy(object->object.object_mesh_texture);
    }
    else if (VRMS_OBJECT_MATRIX == object->type) {
        vrms_object_matrix_destroy(object->object.object_matrix);
    }
}

void vrms_scene_destroy_objects(vrms_scene_t* scene) {
    uint32_t idx;
    vrms_object_t* object;

    for (idx = 1; idx < scene->next_object_id; idx++) {
        object = vrms_scene_get_object_by_id(scene, idx);
        vrms_object_destroy(object);
    }
}

void vrms_scene_empty_outbound_queue(vrms_scene_t* scene) {
/*
    uint32_t idx;
    vrms_queue_item_t* queue_item;
    vrms_queue_item_data_load_t* data_load;

    if (!pthread_mutex_lock(scene->outbound_queue_lock)) {
        for (idx = 0; idx < scene->outbound_queue_index; idx++) {
            queue_item = scene->outbound_queue[idx];
            free(queue_item);
        }
        pthread_mutex_unlock(scene->inbound_queue_lock);
    }
*/
}

void vrms_server_destroy_scene(vrms_server_t* server, uint32_t scene_id) {
    vrms_scene_t* scene = vrms_server_get_scene(server, scene_id);
    if (NULL == scene) {
        fprintf(stderr, "request to destroy already destroyed scene\n");
        return;
    }

    server->scenes[scene_id] = NULL;

    if (!pthread_mutex_lock(scene->render_buffer_lock)) {
        free(scene->render_buffer);
        pthread_mutex_unlock(scene->render_buffer_lock);
        free(scene->render_buffer_lock);
    }

    vrms_scene_destroy_objects(scene);
    free(scene->objects);

/*
    vrms_scene_empty_outbound_queue(scene);
    free(scene->outbound_queue);
    free(scene->outbound_queue_lock);
*/

    free(scene);
}

void vrms_server_queue_destroy_scene(vrms_server_t* server, uint32_t scene_id) {
    vrms_queue_item_scene_destroy_t* scene_destroy = malloc(sizeof(vrms_queue_item_scene_destroy_t));
    memset(scene_destroy, 0, sizeof(vrms_queue_item_scene_destroy_t));

    vrms_queue_item_t* queue_item = malloc(sizeof(vrms_queue_item_t));
    memset(queue_item, 0, sizeof(vrms_queue_item_t));
    queue_item->type = VRMS_QUEUE_SCENE_DESTROY;

    queue_item->item.scene_destroy = scene_destroy;

    pthread_mutex_lock(server->inbound_queue_lock);
    server->inbound_queue[server->inbound_queue_index] = queue_item;
    server->inbound_queue_index++;
    pthread_mutex_unlock(server->inbound_queue_lock);
}

vrms_object_t* vrms_object_create(vrms_scene_t* vrms_scene) {
    vrms_object_t* vrms_object = malloc(sizeof(vrms_object_t));
    vrms_object->type = VRMS_OBJECT_INVALID;
    vrms_scene->objects[vrms_scene->next_object_id] = vrms_object;
    vrms_object->id = vrms_scene->next_object_id;
    vrms_scene->next_object_id++;
    return vrms_object;
}

void vrms_queue_add_data_load(vrms_scene_t* scene, uint32_t size, GLuint* gl_id_ref, vrms_data_type_t type, void* buffer) {
    vrms_server_t* server;

    server = scene->server;

    vrms_queue_item_data_load_t* data_load = malloc(sizeof(vrms_queue_item_data_load_t));
    memset(data_load, 0, sizeof(vrms_queue_item_data_load_t));

    data_load->size = size;
    data_load->destination = gl_id_ref;
    data_load->type = type;
    data_load->buffer = buffer;

    vrms_queue_item_t* queue_item = malloc(sizeof(vrms_queue_item_t));
    memset(queue_item, 0, sizeof(vrms_queue_item_t));
    queue_item->type = VRMS_QUEUE_DATA_LOAD;

    queue_item->item.data_load = data_load;

    pthread_mutex_lock(server->inbound_queue_lock);
    server->inbound_queue[server->inbound_queue_index] = queue_item;
    server->inbound_queue_index++;
    pthread_mutex_unlock(server->inbound_queue_lock);
}

uint32_t vrms_create_data_object(vrms_scene_t* vrms_scene, vrms_data_type_t type, uint32_t fd, uint32_t offset, uint32_t size, uint32_t nr_strides, uint32_t stride) {
    void* address;
    void* buffer;
    int32_t seals;
    vrms_object_t* vrms_object;

    buffer = malloc(size);
    seals = fcntl(fd, F_GET_SEALS);
    if (!(seals & F_SEAL_SHRINK)) {
        fprintf(stderr, "got non-sealed memfd\n");
        return 0;
    }

    address = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
    if (MAP_FAILED == address) {
        fprintf(stderr, "memory map failed\n");
        return 0;
    }

    memcpy(buffer, &((char*)address)[offset], size);

    vrms_object = vrms_object_create(vrms_scene);
    vrms_object->type = VRMS_OBJECT_DATA;

    vrms_object_data_t* object_data = malloc(sizeof(vrms_object_data_t));
    memset(object_data, 0, sizeof(vrms_object_data_t));

    object_data->type = type;
    object_data->size = size;
    object_data->nr_strides = nr_strides;
    object_data->stride = stride;
    vrms_object->object.object_data = object_data;

    if (VRMS_MATRIX == type) {
        fprintf(stderr, "allocating local space for matrix data\n");
        object_data->local_storage = buffer;
    }
    else {
        vrms_queue_add_data_load(vrms_scene, size, &object_data->gl_id, type, buffer);
    }

    return vrms_object->id;
}

uint32_t vrms_create_geometry_object(vrms_scene_t* vrms_scene, uint32_t vertex_id, uint32_t normal_id, uint32_t index_id) {
    vrms_object_t* vrms_object;
    vrms_object = vrms_object_create(vrms_scene);
    vrms_object->type = VRMS_OBJECT_GEOMETRY;

    vrms_object_geometry_t* object_geometry = malloc(sizeof(vrms_object_geometry_t));
    object_geometry->vertex_id = vertex_id;
    object_geometry->normal_id = normal_id;
    object_geometry->index_id = index_id;

    vrms_object->object.object_geometry = object_geometry;

    return vrms_object->id;
}

uint32_t vrms_create_mesh_color(vrms_scene_t* vrms_scene, uint32_t geometry_id, float r, float g, float b, float a) {
    vrms_object_t* vrms_object;
    vrms_object = vrms_object_create(vrms_scene);
    vrms_object->type = VRMS_OBJECT_MESH_COLOR;

    vrms_object_mesh_color_t* object_mesh_color = malloc(sizeof(vrms_object_mesh_color_t));
    object_mesh_color->geometry_id = geometry_id;
    object_mesh_color->r = r;
    object_mesh_color->g = g;
    object_mesh_color->b = b;
    object_mesh_color->a = a;

    vrms_object->object.object_mesh_color = object_mesh_color;

    return vrms_object->id;
}

uint32_t vrms_create_mesh_texture(vrms_scene_t* vrms_scene, uint32_t geometry_id, uint32_t uv_id, uint32_t texture_id) {
    vrms_object_t* vrms_object;
    vrms_object = vrms_object_create(vrms_scene);
    vrms_object->type = VRMS_OBJECT_MESH_TEXTURE;

    vrms_object_mesh_texture_t* object_mesh_texture = malloc(sizeof(vrms_object_mesh_texture_t));
    object_mesh_texture->geometry_id = geometry_id;
    object_mesh_texture->uv_id = uv_id;
    object_mesh_texture->texture_id = texture_id;

    vrms_object->object.object_mesh_texture = object_mesh_texture;

    return vrms_object->id;
}

uint32_t vrms_set_render_buffer(vrms_scene_t* vrms_scene, uint32_t fd, uint32_t nr_objects) {
    void* address;
    int32_t seals;
    size_t size;

    size = (sizeof(uint32_t) * 3) * nr_objects;
    seals = fcntl(fd, F_GET_SEALS);
    if (!(seals & F_SEAL_SHRINK)) {
        fprintf(stderr, "got non-sealed memfd\n");
        return 0;
    }

    address = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
    if (MAP_FAILED == address) {
        fprintf(stderr, "memory map failed\n");
        return 0;
    }

    pthread_mutex_lock(vrms_scene->render_buffer_lock);
    if (NULL != vrms_scene->render_buffer) {
        free(vrms_scene->render_buffer);
    }
    vrms_scene->render_buffer = malloc(size);
    vrms_scene->render_buffer_nr_objects = nr_objects;
    memcpy(vrms_scene->render_buffer, (char*)address, size);
    pthread_mutex_unlock(vrms_scene->render_buffer_lock);

    return 1;
}

void vrms_server_draw_mesh_color(vrms_scene_t* scene, GLuint shader_id, vrms_object_mesh_color_t* mesh, GLfloat* projection_matrix, GLfloat* view_matrix, GLfloat* model_matrix) {
    GLuint b_vertex, b_normal, u_color, m_mvp, m_mv;
    GLfloat* mvp_matrix;
    GLfloat* mv_matrix;

    vrms_object_t* object;
    vrms_object_geometry_t* geometry;
    vrms_object_data_t* vertex;
    vrms_object_data_t* normal;
    vrms_object_data_t* index;

    object = vrms_scene_get_object_by_id(scene, mesh->geometry_id);
    geometry = object->object.object_geometry;

    object = vrms_scene_get_object_by_id(scene, geometry->vertex_id);
    vertex = object->object.object_data;

    object = vrms_scene_get_object_by_id(scene, geometry->normal_id);
    normal = object->object.object_data;

    object = vrms_scene_get_object_by_id(scene, geometry->index_id);
    index = object->object.object_data;

    if ((0 == vertex->gl_id) || (0 == normal->gl_id) || (0 == index->gl_id)) {
        fprintf(stderr, "request to render unrealized geometry: V[%d] N[%d] I[%d]\n", vertex->gl_id, normal->gl_id, index->gl_id);
        return;
    }

    mv_matrix = esmCreateCopy(view_matrix);
    esmMultiply(mv_matrix, model_matrix);

    mvp_matrix = esmCreateCopy(projection_matrix);
    esmMultiply(mvp_matrix, view_matrix);
    esmMultiply(mvp_matrix, model_matrix);

esmDump(mv_matrix, "mv_matrix");
esmDump(mvp_matrix, "mvp_matrix");

    glBindBuffer(GL_ARRAY_BUFFER, vertex->gl_id);
    b_vertex = glGetAttribLocation(shader_id, "b_vertex");
    glVertexAttribPointer(b_vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(b_vertex);

    glBindBuffer(GL_ARRAY_BUFFER, normal->gl_id);
    b_normal = glGetAttribLocation(shader_id, "b_normal");
    glVertexAttribPointer(b_normal, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(b_normal);

    u_color = glGetUniformLocation(shader_id, "u_color");
    glUniform4f(u_color, mesh->r, mesh->g, mesh->b, mesh->a);
    glEnableVertexAttribArray(u_color);

    m_mvp = glGetUniformLocation(shader_id, "m_mvp");
    glUniformMatrix4fv(m_mvp, 1, GL_FALSE, mvp_matrix);

    m_mv = glGetUniformLocation(shader_id, "m_mv");
    glUniformMatrix4fv(m_mv, 1, GL_FALSE, mv_matrix);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index->gl_id);
    glDrawElements(GL_TRIANGLES, index->nr_strides, GL_UNSIGNED_SHORT, NULL);

    esmDestroy(mvp_matrix);
    esmDestroy(mv_matrix);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void vrms_server_draw_mesh_texture(vrms_scene_t* vrms_scene, GLuint shader_id, vrms_object_mesh_texture_t* vrms_object_mesh_texture, GLfloat* projection_matrix, GLfloat* view_matrix, GLfloat* model_matrix) {
}

void vrms_server_draw_scene_object(vrms_scene_t* scene, uint32_t matrix_id, uint32_t matrix_idx, uint32_t mesh_id, GLuint shader_id, float* projection_matrix, float* view_matrix, float* model_matrix) {
    vrms_object_t* matrix_object;
    vrms_object_t* mesh_object;
    vrms_object_data_t* matrix;
    float matrix_buffer[16];

    if (matrix_id >= scene->next_object_id) {
        fprintf(stderr, "matrix object: %d is out of bounds\n", matrix_id);
        return;
    }

    if (mesh_id >= scene->next_object_id) {
        fprintf(stderr, "mesh object: %d is out of bounds\n", mesh_id);
        return;
    }

    matrix_object = vrms_scene_get_object_by_id(scene, matrix_id);
    matrix = matrix_object->object.object_data;

    if (NULL != matrix->local_storage) {
        uint32_t offset = matrix_idx * 16;
        uint32_t size = sizeof(float) * 16;
        memcpy(matrix_buffer, &((char*)matrix->local_storage)[offset], size);
        esmMultiply(model_matrix, matrix_buffer);
    }

    mesh_object = vrms_scene_get_object_by_id(scene, mesh_id);

    if (VRMS_OBJECT_MESH_COLOR == mesh_object->type) {
        vrms_server_draw_mesh_color(scene, shader_id, mesh_object->object.object_mesh_color, projection_matrix, view_matrix, model_matrix);
    }
    else if (VRMS_OBJECT_MESH_TEXTURE == mesh_object->type) {
        vrms_server_draw_mesh_texture(scene, shader_id, mesh_object->object.object_mesh_texture, projection_matrix, view_matrix, model_matrix);
    }
}

void vrms_server_draw_scene_buffer(vrms_scene_t* scene, GLuint shader_id, float* projection_matrix, float* view_matrix, float* model_matrix) {
    uint32_t matrix_id, matrix_idx, mesh_id;

    if (!pthread_mutex_trylock(scene->render_buffer_lock)) {
        int i = 0;
        int idx = 0;
        for (i = 0; i < scene->render_buffer_nr_objects; i++) {
            matrix_id = scene->render_buffer[idx + 0];
            matrix_idx = scene->render_buffer[idx + 1];
            mesh_id = scene->render_buffer[idx + 2];
            vrms_server_draw_scene_object(scene, matrix_id, matrix_idx, mesh_id, shader_id, projection_matrix, view_matrix, model_matrix);
            idx += 3;
        }
        pthread_mutex_unlock(scene->render_buffer_lock);
    }
    else {
        fprintf(stderr, "lock on render bufer\n");
    }
}

void vrms_server_draw_scenes(vrms_server_t* server, GLuint shader_id, float* projection_matrix, float* view_matrix, float* model_matrix) {
    int si;//, oi;
    vrms_scene_t* scene;

    for (si = 1; si < server->next_scene_id; si++) {
        scene = server->scenes[si];
        if (NULL != scene) {
            vrms_server_draw_scene_buffer(scene, shader_id, projection_matrix, view_matrix, model_matrix);
        }
        if (si >= 2000) break;
    }
}

void vrms_queue_load_gl_element_buffer(vrms_queue_item_data_load_t* data_load) {
    if (NULL != data_load->buffer) {
        glGenBuffers(1, data_load->destination);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *data_load->destination);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, data_load->size, data_load->buffer, GL_STATIC_DRAW);
        printOpenGLError();
        if (0 == *data_load->destination) {
            fprintf(stderr, "unable to load gl element buffer");
        }
        free(data_load->buffer);
    }
}

void vrms_queue_load_gl_buffer(vrms_queue_item_data_load_t* data_load) {
    if (NULL != data_load->buffer) {
        glGenBuffers(1, data_load->destination);
        glBindBuffer(GL_ARRAY_BUFFER, *data_load->destination);
        glBufferData(GL_ARRAY_BUFFER, data_load->size, data_load->buffer, GL_STATIC_DRAW);
        printOpenGLError();
        if (0 == *data_load->destination) {
            fprintf(stderr, "unable to load gl buffer");
        }
        free(data_load->buffer);
    }
}

void vrms_server_queue_item_process(vrms_server_t* server, vrms_queue_item_t* queue_item) {
    if (VRMS_QUEUE_DATA_LOAD == queue_item->type) {
        vrms_queue_item_data_load_t* data_load = queue_item->item.data_load;
        if (VRMS_INDEX == data_load->type) {
            vrms_queue_load_gl_element_buffer(data_load);
        }
        else {
            vrms_queue_load_gl_buffer(data_load);
        }
        free(data_load);
    }
    else if (VRMS_QUEUE_SCENE_DESTROY == queue_item->type) {
        vrms_queue_item_scene_destroy_t* scene_destroy = queue_item->item.scene_destroy;
        vrms_server_destroy_scene(server, scene_destroy->scene_id);
        free(scene_destroy);
    }
    else if (VRMS_QUEUE_EVENT == queue_item->type) {
        fprintf(stderr, "not supposed to get a VRMS_QUEUE_EVENT from a client\n");
    }
    else {
        fprintf(stderr, "unknown queue type!!\n");
    }
    free(queue_item);
}

void vrms_scene_queue_item_flush(vrms_server_t* server) {
    uint32_t idx;
    vrms_queue_item_t* queue_item;
    for (idx = 0; idx < server->inbound_queue_index; idx++) {
        queue_item = server->inbound_queue[idx];
        if (NULL != queue_item) {
            vrms_server_queue_item_process(server, queue_item);
        }
        else {
            fprintf(stderr, "null queue item\n");
        }
    }
    server->inbound_queue_index = 0;
}

void vrms_server_flush_client_inbound_queue(vrms_server_t* server) {
    if (!pthread_mutex_trylock(server->inbound_queue_lock)) {
        vrms_scene_queue_item_flush(server);
        pthread_mutex_unlock(server->inbound_queue_lock);
    }
    else {
        fprintf(stderr, "socket thread has lock on queue\n");
    }
}

void vrms_server_process_queue(vrms_server_t* server) {
    vrms_server_flush_client_inbound_queue(server);
}
