#ifndef VRMS_H
#define VRMS_H
typedef enum vrms_object_type {
    VRMS_OBJECT_INVALID,
    VRMS_OBJECT_SCENE,
    VRMS_OBJECT_DATA,
    VRMS_OBJECT_GEOMETRY,
    VRMS_OBJECT_MESH_COLOR,
    VRMS_OBJECT_MESH_TEXTURE
} vrms_object_type_t;

typedef enum vrms_data_type {
    VRMS_UV,
    VRMS_COLOR,
    VRMS_TEXTURE,
    VRMS_VERTEX,
    VRMS_NORMAL,
    VRMS_INDEX,
    VRMS_MATRIX
} vrms_data_type_t;

typedef enum vrms_type {
    VRMS_REPLY,
    VRMS_CREATESCENE,
    VRMS_DESTROYSCENE,
    VRMS_CREATEDATAOBJECT,
    VRMS_DESTROYDATAOBJECT,
    VRMS_CREATEGEOMETRYOBJECT,
    VRMS_CREATEMESHCOLOR,
    VRMS_CREATEMESHTEXTURE,
    VRMS_SETRENDERBUFFER
} vrms_type_t;

typedef enum vrms_error {
    VRMS_OK,
    VRMS_INVALIDREQUEST,
    VRMS_UNKNOWNID,
    VRMS_OUTOFMEMORY
} vrms_error_t;
#endif
