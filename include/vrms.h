typedef enum vrms_object_type {
    VRMS_OBJECT_INVALID,
    VRMS_OBJECT_SCENE,
    VRMS_OBJECT_DATA,
    VRMS_OBJECT_GEOMETRY,
    VRMS_OBJECT_TEXTURE,
    VRMS_OBJECT_MESH_COLOR,
    VRMS_OBJECT_MESH_TEXTURE,
    VRMS_OBJECT_MATRIX
} vrms_object_type_t;

typedef enum vrms_type {
    VRMS_REPLY,
    VRMS_CREATESCENE,
    VRMS_DESTROYSCENE,
    VRMS_CREATEDATAOBJECT,
    VRMS_DESTROYDATAOBJECT,
    VRMS_CREATEGEOMETRYOBJECT,
    VRMS_CREATECOLORMESH,
    VRMS_CREATETEXTUREMESH,
} vrms_type_t;

typedef enum vrms_data_type {
    VRMS_UV,
    VRMS_COLOR,
    VRMS_TEXTURE,
    VRMS_VERTEX,
    VRMS_NORMAL,
    VRMS_INDEX
} vrms_data_type_t;

typedef enum vrms_error {
    VRMS_OK,
    VRMS_INVALIDREQUEST,
    VRMS_UNKNOWNID,
    VRMS_OUTOFMEMORY
} vrms_error_t;

typedef enum vrms_dtype {
    VRMS_FLOAT,
    VRMS_INT
} vrms_dtype_t;
