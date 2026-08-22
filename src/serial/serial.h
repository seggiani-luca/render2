#ifndef SERIAL_H
#define SERIAL_H

// forward declaration of field
typedef struct field field;

// forward declaration of scene 
typedef struct scene scene;

// forward declaration of jsonElement
typedef struct jsonElement jsonElement;

// default scene directory
#define SCENE_DIR "dat/scene/"

// default scene extension 
#define SCENE_EXT ".json" 

// -- fields

// serializes an integer field
jsonElement* intFieldSerialize(const field* f);

// deserializes an integer field
void intFieldDeserialize(field* f, const jsonElement* elem);

// serializes a float field
jsonElement* floatFieldSerialize(const field* f);

// deserializes a float field
void floatFieldDeserialize(field* f, const jsonElement* elem);

// serializes a string field
jsonElement* stringFieldSerialize(const field* f);

// deserializes a string field
void stringFieldDeserialize(field* f, const jsonElement* elem);

// vector field serialization / deserialization
#define VEC_SERIAL_DECL(n)                                              \
	jsonElement* float##n##FieldSerialize(const field* f);              \
	void float##n##FieldDeserialize(field* f, const jsonElement* elem);

// 2D vector field serialization / deserialization
VEC_SERIAL_DECL(2)

// 3D vector field serialization / deserialization
VEC_SERIAL_DECL(3)

// 4D vector field serialization / deserialization
VEC_SERIAL_DECL(4)

// matrix field serialization / deserialization
#define MAT_SERIAL_DECL(n)                                            \
	jsonElement* mat##n##FieldSerialize(const field* f);              \
	void mat##n##FieldDeserialize(field* f, const jsonElement* elem);

// 2D matrix field serialization / deserialization
MAT_SERIAL_DECL(2)

// 3D matrix field serialization / deserialization
MAT_SERIAL_DECL(3)

// 4D matrix field serialization / deserialization
MAT_SERIAL_DECL(4)

// serializes a quaternion field
jsonElement* quatFieldSerialize(const field* f);

// deserializes a quaternion field
void quatFieldDeserialize(field* f, const jsonElement* elem);

// serializes a transform field
jsonElement* transformFieldSerialize(const field* f);

// deserializes a transform field
void transformFieldDeserialize(field* f, const jsonElement* elem);

// serializes a camera field
jsonElement* cameraFieldSerialize(const field* f);

// deserializes a camera field
void cameraFieldDeserialize(field* f, const jsonElement* elem);

// serializes an atmosphere field
jsonElement* atmosphereFieldSerialize(const field* f);

// deserializes an atmosphere field
void atmosphereFieldDeserialize(field* f, const jsonElement* elem);

// serializes a texture field
jsonElement* textureFieldSerialize(const field* f);

// deserializes a texture field
void textureFieldDeserialize(field* f, const jsonElement* elem);

// serializes a mesh field
jsonElement* meshFieldSerialize(const field* f);

// deserializes a mesh field
void meshFieldDeserialize(field* f, const jsonElement* elem);

// serializes a material field
jsonElement* materialFieldSerialize(const field* f);

// deserializes a material field
void materialFieldDeserialize(field* f, const jsonElement* elem);

// -- scenes

// serializes a scene into a file
void serializeScene(const scene* scn, const char* path);

// deserializes a scene from a file
void deserializeScene(scene* scn, const char* path);

const char* getScenePath(const char* name);

#endif
