#ifndef __GALA_SHADER_H__
#define __GALA_SHADER_H__

#include <estd/str.h>

#include "common.h"

GALA_ENUM(GalaType){
    GALA_TYPE_S8 = 0x1400,
    GALA_TYPE_U8 = 0x1401,
    GALA_TYPE_S16 = 0x1402,
    GALA_TYPE_U16 = 0x1403,
    GALA_TYPE_F16 = 0x140B,
    GALA_TYPE_S32 = 0x1404,
    GALA_TYPE_U32 = 0x1405,
    GALA_TYPE_F32 = 0x1406
};

GALA_ENUM(GalaBufferType){
    GALA_BUFFER_TYPE_DATA = 0,
    GALA_BUFFER_TYPE_UNIFORM = 0x8A11,
    GALA_BUFFER_TYPE_ARRAY = 0x8892,
    GALA_BUFFER_TYPE_ELEMENT = 0x8893,
};

GALA_ENUM(GalaTextureWrap){
    GALA_TEXTURE_WRAP_CLAMP_TO_EDGE = 0x812F,
    GALA_TEXTURE_WRAP_REPEAT = 0x2901,
};

GALA_ENUM(GalaTextureMin){
    GALA_TEXTURE_MIN_NEAREST = 0x2600,
    GALA_TEXTURE_MIN_LINEAR = 0x2601,
    GALA_TEXTURE_NEAREST_MIPMAP_NEAREST = 0x2700,
    GALA_TEXTURE_LINEAR_MIPMAP_NEAREST = 0x2701,
    GALA_TEXTURE_NEAREST_MIPMAP_LINEAR = 0x2702,
    GALA_TEXTURE_LINEAR_MIPMAP_LINEAR = 0x2703
};

GALA_ENUM(GalaTextureMag){
    GALA_TEXTURE_MAG_NEAREST = 0x2600,
    GALA_TEXTURE_MAG_LINEAR = 0x2601,
};

GALA_ENUM(GalaTextureType){
    GALA_TEXTURE_TYPE_2D = 0x0DE1,
    GALA_TEXTURE_TYPE_ARRAY_2D = 0x8C1A,
    GALA_TEXTURE_TYPE_CUBEMAP = 0x8513
};

GALA_ENUM(GalaTextureFormat){GALA_TEXTURE_FORMAT_R8 = 0x8229,       GALA_TEXTURE_FORMAT_R16 = 0x822A,
                             GALA_TEXTURE_FORMAT_RG8 = 0x822B,      GALA_TEXTURE_FORMAT_RG16 = 0x822C,
                             GALA_TEXTURE_FORMAT_R16F = 0x822D,     GALA_TEXTURE_FORMAT_R32F = 0x822E,
                             GALA_TEXTURE_FORMAT_RG16F = 0x822F,    GALA_TEXTURE_FORMAT_RG32F = 0x8230,
                             GALA_TEXTURE_FORMAT_R8I = 0x8231,      GALA_TEXTURE_FORMAT_R8UI = 0x8232,
                             GALA_TEXTURE_FORMAT_R16I = 0x8233,     GALA_TEXTURE_FORMAT_R16UI = 0x8234,
                             GALA_TEXTURE_FORMAT_R32I = 0x8235,     GALA_TEXTURE_FORMAT_R32UI = 0x8236,
                             GALA_TEXTURE_FORMAT_RG8I = 0x8237,     GALA_TEXTURE_FORMAT_RG8UI = 0x8238,
                             GALA_TEXTURE_FORMAT_RG16I = 0x8239,    GALA_TEXTURE_FORMAT_RG16UI = 0x823A,
                             GALA_TEXTURE_FORMAT_RG32I = 0x823B,    GALA_TEXTURE_FORMAT_RG32UI = 0x823C,
                             GALA_TEXTURE_FORMAT_RGBA32UI = 0x8D70, GALA_TEXTURE_FORMAT_RGB32UI = 0x8D71,
                             GALA_TEXTURE_FORMAT_RGBA16UI = 0x8D76, GALA_TEXTURE_FORMAT_RGB16UI = 0x8D77,
                             GALA_TEXTURE_FORMAT_RGBA8UI = 0x8D7C,  GALA_TEXTURE_FORMAT_RGB8UI = 0x8D7D,
                             GALA_TEXTURE_FORMAT_RGBA32I = 0x8D82,  GALA_TEXTURE_FORMAT_RGB32I = 0x8D83,
                             GALA_TEXTURE_FORMAT_RGBA16I = 0x8D88,  GALA_TEXTURE_FORMAT_RGB16I = 0x8D89,
                             GALA_TEXTURE_FORMAT_RGBA8I = 0x8D8E,   GALA_TEXTURE_FORMAT_RGB8I = 0x8D8F,
                             GALA_TEXTURE_FORMAT_R3_G3_B2 = 0x2A10, GALA_TEXTURE_FORMAT_RGB4 = 0x804F,
                             GALA_TEXTURE_FORMAT_RGB5 = 0x8050,     GALA_TEXTURE_FORMAT_RGB8 = 0x8051,
                             GALA_TEXTURE_FORMAT_RGB10 = 0x8052,    GALA_TEXTURE_FORMAT_RGB12 = 0x8053,
                             GALA_TEXTURE_FORMAT_RGB16 = 0x8054,    GALA_TEXTURE_FORMAT_RGBA2 = 0x8055,
                             GALA_TEXTURE_FORMAT_RGBA4 = 0x8056,    GALA_TEXTURE_FORMAT_RGB5_A1 = 0x8057,
                             GALA_TEXTURE_FORMAT_RGBA8 = 0x8058,    GALA_TEXTURE_FORMAT_RGB10_A2 = 0x8059,
                             GALA_TEXTURE_FORMAT_RGBA12 = 0x805A,   GALA_TEXTURE_FORMAT_RGBA16 = 0x805B,
                             GALA_TEXTURE_FORMAT_SRGB8 = 0x8C41,    GALA_TEXTURE_FORMAT_SRGBA8 = 0x8C43,
                             GALA_TEXTURE_FORMAT_RGB9_E5 = 0x8C3D,  GALA_TEXTURE_FORMAT_R11F_G11F_B10F = 0x8C3A};

GALA_ENUM(GalaAttributeType){
    GALA_ATTRIBUTE_TYPE_INTEGER,
    GALA_ATTRIBUTE_TYPE_FLOAT,
    GALA_ATTRIBUTE_TYPE_NORMALIZED,
};

GALA_STRUCT(GalaAttribute) {
    EstdString name;
    uint8_t count;
    GalaAttributeType type;
    GalaType element_type;
};

GALA_STRUCT(GalaTextureUnit) {
    EstdString name;
    GalaTextureType type;
    GalaTextureFormat format;
};

GALA_STRUCT(GalaVertexArray) {
    EstdString name;
    size_t attribute_count;
    GalaAttribute const* attributes;
    uint32_t gl;
};

GALA_STRUCT(GalaProgram) {
    EstdString name;
    EstdString source;

    size_t attribute_count;
    GalaAttribute const* attributes;

    size_t texture_unit_count;
    GalaTextureUnit const* texture_units;

    uint32_t gl;
};

GALA_STRUCT(GalaPipeline) {
    GalaProgram* program;
};

GALA_STRUCT(GalaBuffer) {
    GalaBufferType type;
    size_t size;
    uint32_t gl;
};

GALA_STRUCT(GalaTexture) {
    EstdString name;
    GalaTextureType type;
    GalaTextureFormat format;
    size_t mipmap_count;
    size_t width;
    size_t height;
    size_t depth;
    GalaTextureWrap wrap_s;
    GalaTextureWrap wrap_t;
    GalaTextureWrap wrap_r;
    GalaTextureMin minification;
    GalaTextureMag magnification;
    uint32_t gl;
};

#endif
