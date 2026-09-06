#include "mesh.h"
#include "../decode.h"
#include <stdlib.h>

// size of file line
#define MESH_LINE_SIZ 256

// max n-gon vertices
#define MAX_NGON_VERTS 16

// imports a mesh in .obj format
int meshDecode(mesh* mesh, FILE* file) {
	char line[MESH_LINE_SIZ];

	// count unique attributes 
	int maxVert = 0, maxUv = 0, maxNorm = 0;
	while (fgets(line, MESH_LINE_SIZ, file)) {
		if      (checkKey(line, "v "))  maxVert++;
		else if (checkKey(line, "vt ")) maxUv++;
		else if (checkKey(line, "vn ")) maxNorm++;
	}

	// reset file cursor
	fseek(file, 0, SEEK_SET);

	// allocate temporary buffers
	float* tempVert = maxVert ? malloc(sizeof(float) * maxVert * 3) : NULL;
    float* tempUv   = maxUv   ? malloc(sizeof(float) * maxUv   * 2) : NULL;
    float* tempNorm = maxNorm ? malloc(sizeof(float) * maxNorm * 3) : NULL;

	if (!tempVert
	||  (maxVert > 0 && !tempVert )
	||  (maxUv    > 0 && !tempUv  )
	||  (maxNorm  > 0 && !tempNorm)) {
		// free temporary buffers
		free(tempVert);
		free(tempUv);
		free(tempNorm);
		return 0;
	}

	int nVert = 0;
	int nUv = 0;
	int nNorm = 0;

	// fetch all unique attributes 
	while (fgets(line, MESH_LINE_SIZ, file)) {
		if (tempUv)   parseFloatBufKey(line, "vt ", 2, &nUv,   tempUv  );
		if (tempNorm) parseFloatBufKey(line, "vn ", 3, &nNorm, tempNorm);
		if (tempVert) parseFloatBufKey(line, "v ",  3, &nVert, tempVert);
	}

	// allocate dynamic buffer for output vertices
	size_t capacity = 1024;
	mesh->vertCount = 0;
	mesh->verts = malloc(sizeof(vertex) * capacity);

	if (!mesh->verts) {
		// free temporary buffers
		free(tempVert);
		free(tempUv);
		free(tempNorm);
		return 0;
	}

	// reset file cursor
	fseek(file, 0, SEEK_SET);

	// fetch all faces (tris or n-gons) 
	while (fgets(line, MESH_LINE_SIZ, file)) {
		char* face = checkKey(line, "f ");
		if (!face) continue;

		// support n-gons 
		int v_idx[MAX_NGON_VERTS] = {0};
		int t_idx[MAX_NGON_VERTS] = {0};
		int n_idx[MAX_NGON_VERTS] = {0};
		int polyVerts = 0;

		char* ptr = face;
		int v, t, n;

		// extract all vertex / UV / normal combinations 
		while (polyVerts < MAX_NGON_VERTS) {
			int offset = 0;

			if (sscanf(ptr, "%d/%d/%d%n", &v, &t, &n, &offset) == 3) {
				v_idx[polyVerts] = v;
				t_idx[polyVerts] = t;
				n_idx[polyVerts] = n;
			} else if (sscanf(ptr, "%d//%d%n", &v, &n, &offset) == 2) {
				v_idx[polyVerts] = v;
				t_idx[polyVerts] = 0;
				n_idx[polyVerts] = n;
			} else if (sscanf(ptr, "%d/%d%n", &v, &t, &offset) == 2) {
				v_idx[polyVerts] = v;
				t_idx[polyVerts] = t;
				n_idx[polyVerts] = 0;
			} else if (sscanf(ptr, "%d%n", &v, &offset) == 1) {
				v_idx[polyVerts] = v;
				t_idx[polyVerts] = 0;
				n_idx[polyVerts] = 0;
			} else break; 

			if (offset == 0) break; // safeguard

			polyVerts++;
			ptr += offset;
			while (*ptr == ' ') ptr++; // skip whitespace
		}

		if (polyVerts < 3) continue; // invalid face 

		// fan triangulate if n-gon 
		for (int i = 1; i < polyVerts - 1; i++) {
			// grow vertex buffer 
			if (mesh->vertCount + 3 > capacity) {
				capacity *= 2;
				vertex* newVerts = realloc(mesh->verts, sizeof(vertex) * capacity);
				if (!newVerts) {
					free(mesh->verts);
	
					// free temporary buffers
					free(tempVert);
					free(tempUv);
					free(tempNorm);
					return 0;
				}
				mesh->verts = newVerts;
			}

			// map triangulated indices
			int tri[3] = {0, i, i + 1};

			// construct vertices
			for (int k = 0; k < 3; k++) {
				int idx = tri[k];
				int vi, ti, ni;

				int hasUv = t_idx[idx];
				int hasNorm = n_idx[idx];
				
				// index attributes relative to size 
				vi = (v_idx[idx] > 0 ? v_idx[idx] - 1 : (nVert / 3) + v_idx[idx]) * 3;
				if(hasUv) 
					ti = (t_idx[idx] > 0 ? t_idx[idx] - 1 : (nUv / 2)   + t_idx[idx]) * 2;
				if(hasNorm) 
					ni = (n_idx[idx] > 0 ? n_idx[idx] - 1 : (nNorm / 3) + n_idx[idx]) * 3;

				// initialize vertex
				vertex vert = {0};

				if (vi >= 0 && vi + 2 < nVert) {
					vert.x = tempVert[vi];
					vert.y = tempVert[vi + 1];
					vert.z = tempVert[vi + 2];
				}
				if(hasUv) {
					if (ti >= 0 && ti + 1 < nUv) {
						vert.u = tempUv[ti];
						vert.v = tempUv[ti + 1];
					}
				} else {
					vert.u = 0.0f;
					vert.v = 0.0f;
				}
				if(hasNorm) {
					if (ni >= 0 && ni + 2 < nNorm) {
						vert.nx = tempNorm[ni];
						vert.ny = tempNorm[ni + 1];
						vert.nz = tempNorm[ni + 2];
					}
				} else {
					vert.nx = 1.0f;
					vert.ny = 0.0f;
					vert.nz = 0.0f;
				}

				// write vertex to mesh
				mesh->verts[mesh->vertCount++] = vert;
			}
		}
	}

	// resize to free heap space
	if (mesh->vertCount > 0) {
		vertex* exactVerts = realloc(mesh->verts, sizeof(vertex) * mesh->vertCount);
		if (exactVerts) mesh->verts = exactVerts;
	} else {
		free(mesh->verts);
		mesh->verts = NULL;
	}

	// free temporary buffers
	free(tempVert);
	free(tempUv);
	free(tempNorm);

	return 1;
}
