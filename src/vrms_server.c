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
    vrms_server_t* vrms_server = malloc(sizeof(vrms_server_t));
    vrms_server->scenes = malloc(sizeof(vrms_scene_t) * 10);
    memset(vrms_server->scenes, 0, sizeof(vrms_scene_t) * 10);
    vrms_server->next_scene_id = 1;
    return vrms_server;
}

vrms_scene_t* vrms_server_get_scene(vrms_server_t* vrms_server, uint32_t scene_id) {
    return vrms_server->scenes[scene_id];
}

uint32_t vrms_create_scene(vrms_server_t* vrms_server, char* name) {
    vrms_scene_t* vrms_scene = malloc(sizeof(vrms_scene_t));
    memset(vrms_scene, 0, sizeof(vrms_scene_t));

    vrms_scene->objects = malloc(sizeof(vrms_object_t) * 10);
    memset(vrms_scene->objects, 0, sizeof(vrms_object_t) * 10);
    vrms_scene->next_object_id = 1;

    vrms_server->scenes[vrms_server->next_scene_id] = vrms_scene;
    vrms_scene->id = vrms_server->next_scene_id;
    vrms_server->next_scene_id++;

    vrms_scene->inbound_queue = malloc(sizeof(vrms_queue_item_t) * 10);
    memset(vrms_scene->inbound_queue, 0, sizeof(vrms_queue_item_t) * 10);
    vrms_scene->inbound_queue_index = 0;
    vrms_scene->inbound_queue_lock = malloc(sizeof(pthread_mutex_t));
    memset(vrms_scene->inbound_queue_lock, 0, sizeof(pthread_mutex_t));

    return vrms_scene->id;
}

vrms_object_t* vrms_object_create(vrms_scene_t* vrms_scene) {
    vrms_object_t* vrms_object = malloc(sizeof(vrms_object_t));
    vrms_object->type = VRMS_OBJECT_INVALID;
    vrms_scene->objects[vrms_scene->next_object_id] = vrms_object;
    vrms_object->id = vrms_scene->next_object_id;
    vrms_scene->next_object_id++;
    return vrms_object;
}

uint32_t vrms_create_data_object(vrms_scene_t* vrms_scene, vrms_data_type_t type, uint32_t fd, uint32_t dtype, uint32_t offset, uint32_t size, uint32_t stride) {
    void* address;
    void* buffer;
    uint32_t* int_buffer;
    float* float_buffer;
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
    vrms_object->object.object_data = object_data;

    vrms_queue_item_data_load_t* data_load = malloc(sizeof(vrms_queue_item_data_load_t));
    memset(data_load, 0, sizeof(vrms_queue_item_data_load_t));

    data_load->size = size;
    data_load->destination = &object_data->gl_id;

    if (VRMS_FLOAT == dtype) {
        object_data->nr_values = size / sizeof(float);
        float_buffer = (float*)buffer;
        data_load->float_buffer = float_buffer;
        data_load->nr_values = object_data->nr_values;
    }
    else if (VRMS_INT == dtype) {
        object_data->nr_values = size / sizeof(uint32_t);
        int_buffer = (uint32_t*)buffer;
        data_load->int_buffer = int_buffer;
        data_load->nr_values = object_data->nr_values;
    }

    vrms_queue_item_t* queue_item = malloc(sizeof(vrms_queue_item_t));
    memset(queue_item, 0, sizeof(vrms_queue_item_t));
    queue_item->type = VRMS_QUEUE_DATA_LOAD;

    queue_item->item.data_load = data_load;

    pthread_mutex_lock(vrms_scene->inbound_queue_lock);
    vrms_scene->inbound_queue[vrms_scene->inbound_queue_index] = queue_item;
    vrms_scene->inbound_queue_index++;
    pthread_mutex_unlock(vrms_scene->inbound_queue_lock);

    return vrms_object->id;
}

void vrms_queue_item_process(vrms_queue_item_t* queue_item) {
    vrms_queue_item_data_load_t* data_load;
    switch (queue_item->type) {
        case VRMS_QUEUE_DATA_LOAD:
            data_load = queue_item->item.data_load;
            if (NULL != data_load->float_buffer) {
                glGenBuffers(1, data_load->destination);
                glBindBuffer(GL_ARRAY_BUFFER, *data_load->destination);
                glBufferData(GL_ARRAY_BUFFER, data_load->size, data_load->float_buffer, GL_STATIC_DRAW);
                free(data_load->float_buffer);
            }
            else if (NULL != data_load->int_buffer) {
                glGenBuffers(1, data_load->destination);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *data_load->destination);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, data_load->size, data_load->int_buffer, GL_STATIC_DRAW);
                free(data_load->int_buffer);
            }
            else {
                fprintf(stderr, "malformed buffer\n");
            }
            free(data_load);
            free(queue_item);
        break;
        case VRMS_QUEUE_EVENT:
            fprintf(stderr, "not supposed to get a VRMS_QUEUE_EVENT from a client\n");
        break;
    }
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

vrms_object_t* vrms_server_get_object_by_id(vrms_scene_t* vrms_scene, uint32_t id) {
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

void vrms_server_draw_mesh_color(vrms_scene_t* scene, GLuint shader_id, vrms_object_mesh_color_t* mesh, GLfloat* projection_matrix, GLfloat* view_matrix, GLfloat* model_matrix) {
    GLuint b_vertex, b_normal, u_color, m_mvp, m_mv;
    GLfloat* mvp_matrix;
    GLfloat* mv_matrix;

    vrms_object_t* object;
    vrms_object_geometry_t* geometry;
    vrms_object_data_t* vertex;
    vrms_object_data_t* normal;
    vrms_object_data_t* index;

    object = vrms_server_get_object_by_id(scene, mesh->geometry_id);
    geometry = object->object.object_geometry;

    object = vrms_server_get_object_by_id(scene, geometry->vertex_id);
    vertex = object->object.object_data;

    object = vrms_server_get_object_by_id(scene, geometry->normal_id);
    normal = object->object.object_data;

    object = vrms_server_get_object_by_id(scene, geometry->index_id);
    index = object->object.object_data;

    mv_matrix = esmCreateCopy(view_matrix);
    esmMultiply(mv_matrix, model_matrix);

    mvp_matrix = esmCreateCopy(projection_matrix);
    esmMultiply(mvp_matrix, view_matrix);
    esmMultiply(mvp_matrix, model_matrix);

    glBindBuffer(GL_ARRAY_BUFFER, vertex->gl_id);
    b_vertex = glGetAttribLocation(shader_id, "b_vertex");
    glVertexAttribPointer(b_vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(b_vertex);
    printOpenGLError();

    glBindBuffer(GL_ARRAY_BUFFER, normal->gl_id);
    b_normal = glGetAttribLocation(shader_id, "b_normal");
    glVertexAttribPointer(b_normal, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(b_normal);
    printOpenGLError();

    u_color = glGetUniformLocation(shader_id, "u_color");
    glUniform4f(u_color, mesh->r, mesh->g, mesh->b, mesh->a);
    glEnableVertexAttribArray(u_color);
    printOpenGLError();

    m_mvp = glGetUniformLocation(shader_id, "m_mvp");
    glUniformMatrix4fv(m_mvp, 1, GL_FALSE, mvp_matrix);
    glEnableVertexAttribArray(m_mvp);

    m_mv = glGetUniformLocation(shader_id, "m_mv");
    glUniformMatrix4fv(m_mv, 1, GL_FALSE, mv_matrix);
    glEnableVertexAttribArray(m_mv);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index->gl_id);
    glDrawElements(GL_TRIANGLES, index->nr_values, GL_UNSIGNED_INT, NULL);
    printOpenGLError();

    esmDestroy(mvp_matrix);
    esmDestroy(mv_matrix);
}

void vrms_server_draw_mesh_texture(vrms_scene_t* vrms_scene, GLuint shader_id, vrms_object_mesh_texture_t* vrms_object_mesh_texture, GLfloat* projection_matrix, GLfloat* view_matrix, GLfloat* model_matrix) {
}

void vrms_server_draw_scene(vrms_server_t* vrms_server, GLuint shader_id, GLfloat* projection_matrix, GLfloat* view_matrix, GLfloat* model_matrix) {
    int si, oi;
    vrms_scene_t* vrms_scene;
    vrms_object_t* vrms_object;

    //esmTranslatef(model_matrix, 0.0f, 0.0f, -10.0f);
    esmRotatef(model_matrix, 2.0f, 1, 0, 0);
    esmTranslatef(model_matrix, -1.0f, -1.0f, -1.0f);

    for (si = 1; si < vrms_server->next_scene_id; si++) {
        vrms_scene = vrms_server->scenes[si];
        if (NULL != vrms_scene) {
            for (oi = 1; oi < vrms_scene->next_object_id; oi++) {
                vrms_object = vrms_scene->objects[oi];
                if (NULL == vrms_object) {
                    fprintf(stderr, "null object stored in scene at %d\n", oi);
                    break;
                }
                switch (vrms_object->type) {
                    case VRMS_OBJECT_INVALID:
                    break;
                    case VRMS_OBJECT_SCENE:
                    break;
                    case VRMS_OBJECT_DATA:
                    break;
                    case VRMS_OBJECT_GEOMETRY:
                    break;
                    case VRMS_OBJECT_TEXTURE:
                    break;
                    case VRMS_OBJECT_MESH_COLOR:
                        vrms_server_draw_mesh_color(vrms_scene, shader_id, vrms_object->object.object_mesh_color, projection_matrix, view_matrix, model_matrix);
                    break;
                    case VRMS_OBJECT_MESH_TEXTURE:
                        vrms_server_draw_mesh_texture(vrms_scene, shader_id, vrms_object->object.object_mesh_texture, projection_matrix, view_matrix, model_matrix);
                    break;
                    case VRMS_OBJECT_MATRIX:
                        //vrms_server_draw_matrix();
                    break;
                }
                if (oi >= 2000) break;
            }
        }
        else {
            fprintf(stderr, "null scene at %d\n", si);
        }
        if (si >= 2000) break;
    }
}

void vrms_server_flush_client_inbound_queues(vrms_server_t* server) {
    uint32_t idx;
    vrms_scene_t* scene;
    vrms_queue_item_t* queue_item;
    scene = server->scenes[1];
    if (NULL != scene) {
        if (!pthread_mutex_trylock(scene->inbound_queue_lock)) {
            for (idx = 0; idx < scene->inbound_queue_index; idx++) {
                queue_item = scene->inbound_queue[idx];
                if (NULL != queue_item) {
                    vrms_queue_item_process(queue_item);
                }
                else {
                    fprintf(stderr, "null queue item\n");
                }
            }
            scene->inbound_queue_index = 0;
            pthread_mutex_unlock(scene->inbound_queue_lock);
        }
        else {
            fprintf(stderr, "socket thread has lock on queue\n");
        }
    }
}

void vrms_server_process_queues(vrms_server_t* server) {
    vrms_server_flush_client_inbound_queues(server);
}
