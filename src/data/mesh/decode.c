#include "mesh.h"
#include "../decode.h"
#include <stdlib.h>

// size of file line
#define MESH_LINE_SIZ 256

// imports a mesh in .obj format
int meshDecode(mesh* mesh, FILE* file) {
	char line[MESH_LINE_SIZ];

	// count vertices
	while(fgets(line, MESH_LINE_SIZ, file)) 
		if(checkKey(line, "f ")) mesh->vertCount++;
	mesh->vertCount *= 3;

	// reset file cursor
	fseek(file, 0, SEEK_SET);

	// allocate vertices
	mesh->verts = malloc(sizeof(vertex) * mesh->vertCount);

	// temporary buffers
	float tVert[mesh->vertCount * 3];
	float tUv[mesh->vertCount * 2];
	float tNorm[mesh->vertCount * 3];
	int nVert = 0;
	int nUv = 0;
	int nNorm = 0;

	// go throgh each data line
	long pos = 0;
	while(fgets(line, MESH_LINE_SIZ, file)) {
		// parse lines into temporary buffers
		parseFloatBufKey(line, "vt ", 2, &nUv, tUv);
		parseFloatBufKey(line, "vn ", 3, &nNorm, tNorm);
		parseFloatBufKey(line, "v ", 3, &nVert, tVert);

		// rollback and quit on first face
		if(checkKey(line, "f ")) {
			fseek(file, pos, SEEK_SET);
			break;
		}

		// advance position in file 
		pos = ftell(file);
	}

	// face indices
	int v[3], n[3], t[3];

	// go through each face line
	int cur = 0;
	while(fgets(line, MESH_LINE_SIZ, file)) {
		// don't overflow
		if(cur >= mesh->vertCount) {
			free(mesh->verts);
			return 0;
		}

		// only if face key matches 
		char* face = checkKey(line, "f ");
		if(!face) continue;

		// parse face line
		sscanf(face, "%d/%d/%d %d/%d/%d %d/%d/%d", 
			&v[0], &t[0], &n[0],
			&v[1], &t[1], &n[1],
			&v[2], &t[2], &n[2]
		);
		for(int i = 0; i < 3; i++) {
			int vi = (v[i] - 1) * 3;
			int ni = (n[i] - 1) * 3;
			int ti = (t[i] - 1) * 2;

			// index data arrays with face
			mesh->verts[cur] = (vertex) {
				// vertex
				.x = tVert[vi],
				.y = tVert[vi + 1],
				.z = tVert[vi + 2],

				// normal
				.nx = tNorm[ni],
				.ny = tNorm[ni + 1],
				.nz = tNorm[ni + 2],

				// texture
				.u = tUv[ti],
				.v = tUv[ti + 1]
			};

			cur++;
		}
	}

	return 1;
}
