#include <math.h>
#include "freeglut/GL/freeglut_std.h"
#include "freeglut/fg_internal.h"
#include "freeglut/fg_gl2.h"

// code taken from freeglut
// modified to return geometry data from generated shapes

// fg_init.c
/* Perform the freeglut deinitialization... */
//void fgDeinitialize(void){} // too much trouble getting this to compile

// fg_main.c
/* Error Message functions */
void fgError( const char *fmt, ... )
{
    va_list ap;

    if (fgState.ErrorFunc) {

        va_start( ap, fmt );

        /* call user set error handler here */
        fgState.ErrorFunc(fmt, ap, fgState.ErrorFuncData);

        va_end( ap );

    } else {
#ifdef FREEGLUT_PRINT_ERRORS
        va_start( ap, fmt );

        fprintf( stderr, "freeglut ");
        if( fgState.ProgramName )
            fprintf( stderr, "(%s): ", fgState.ProgramName );
        vfprintf( stderr, fmt, ap );
        fprintf( stderr, "\n" );

        va_end( ap );
#endif
        // too much trouble getting this to compile
        //if ( fgState.Initialised ) fgDeinitialize ();
        exit( 1 );
    }
}

void fgWarning( const char *fmt, ... )
{
    va_list ap;

    if (fgState.WarningFunc) {

        va_start( ap, fmt );

        /* call user set warning handler here */
        fgState.WarningFunc(fmt, ap, fgState.WarningFuncData);

        va_end( ap );

    } else {
#ifdef FREEGLUT_PRINT_WARNINGS
        va_start( ap, fmt );

        fprintf( stderr, "freeglut ");
        if( fgState.ProgramName )
            fprintf( stderr, "(%s): ", fgState.ProgramName );
        vfprintf( stderr, fmt, ap );
        fprintf( stderr, "\n" );

        va_end( ap );
#endif
    }
}



#ifndef GL_VERSION_1_1
static void fghDrawGeometryWire10(GLfloat *varr, GLfloat *narr, GLushort *iarr,
		GLsizei nparts, GLsizei npartverts, GLenum prim, GLushort *iarr2,
		GLsizei nparts2, GLsizei npartverts2)
{
	int i, j;
	GLfloat *vptr, *nptr;
	GLushort *iptr;

	if(!iarr) {
		vptr = varr;
		nptr = narr;
		for(i=0; i<nparts; i++) {
			glBegin(prim);
			for(j=0; j<npartverts; j++) {
				glNormal3fv(nptr); nptr += 3;
				glVertex3fv(vptr); vptr += 3;
			}
			glEnd();
		}
	} else {
		iptr = iarr;
		for(i=0; i<nparts; i++) {
			glBegin(prim);
			for(j=0; j<npartverts; j++) {
				int idx = *iptr++;
				idx = idx * 2 + idx;
				glNormal3fv(narr + idx);
				glVertex3fv(varr + idx);
			}
			glEnd();
		}
	}

	if(iarr2) {
		iptr = iarr2;
		for(i=0; i<nparts2; i++) {
			glBegin(GL_LINE_LOOP);
			for(j=0; j<npartverts2; j++) {
				int idx = *iptr++;
				idx = idx * 2 + idx;
				glNormal3fv(narr + idx);
				glVertex3fv(varr + idx);
			}
			glEnd();
		}
	}
}

static void fghDrawGeometrySolid10(GLfloat *varr, GLfloat *narr, GLfloat *tarr,
		GLsizei nverts, GLushort *iarr, GLsizei nparts, GLsizei npartverts)
{
    int i, j;
	GLfloat *vptr, *nptr, *tptr;
	GLushort *iptr;

	if(!iarr) {
		vptr = varr;
		nptr = narr;
		tptr = tarr;
		glBegin(GL_TRIANGLES);
		for(i=0; i<nverts; i++) {
			if(tarr) {
				glTexCoord2fv(tptr); tptr += 2;
			}
			glNormal3fv(nptr); nptr += 3;
			glVertex3fv(vptr); vptr += 3;
		}
		glEnd();
		return;
	}

	iptr = iarr;
	if(nparts > 1) {
		for(i=0; i<nparts; i++) {
			glBegin(GL_TRIANGLE_STRIP);
			for(j=0; j<npartverts; j++) {
				int idx = *iptr++;
				if(tarr) {
					glTexCoord2fv(tarr + idx * 2);
				}
				idx = idx * 2 + idx;
				glNormal3fv(narr + idx);
				glVertex3fv(varr + idx);
			}
			glEnd();
		}
	} else {
		glBegin(GL_TRIANGLES);
		for(i=0; i<npartverts; i++) {
			int idx = *iptr++;
			if(tarr) {
				glTexCoord2fv(tarr + idx * 2);
			}
			idx = idx * 2 + idx;
			glNormal3fv(narr + idx);
			glVertex3fv(varr + idx);
		}
		glEnd();
	}
}
#endif

/* Version for OpenGL (ES) 1.1 */
static void fghDrawGeometryWire11(GLfloat *vertices, GLfloat *normals,
                                  GLushort *vertIdxs, GLsizei numParts, GLsizei numVertPerPart, GLenum vertexMode,
                                  GLushort *vertIdxs2, GLsizei numParts2, GLsizei numVertPerPart2
    )
{
#if defined(GL_VERSION_1_1) || defined(GL_VERSION_ES_CM_1_0)
    int i;

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

    glVertexPointer(3, GL_FLOAT, 0, vertices);
    glNormalPointer(GL_FLOAT, 0, normals);


    if (!vertIdxs)
        /* Draw per face (TODO: could use glMultiDrawArrays if available) */
        for (i=0; i<numParts; i++)
            glDrawArrays(vertexMode, i*numVertPerPart, numVertPerPart);
    else
        for (i=0; i<numParts; i++)
            glDrawElements(vertexMode,numVertPerPart,GL_UNSIGNED_SHORT,vertIdxs+i*numVertPerPart);

    if (vertIdxs2)
        for (i=0; i<numParts2; i++)
            glDrawElements(GL_LINE_LOOP,numVertPerPart2,GL_UNSIGNED_SHORT,vertIdxs2+i*numVertPerPart2);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
#else
	fghDrawGeometryWire10(vertices, normals, vertIdxs, numParts, numVertPerPart,
			vertexMode, vertIdxs2, numParts2, numVertPerPart2);
#endif
}


static void fghDrawGeometrySolid11(GLfloat *vertices, GLfloat *normals, GLfloat *textcs, GLsizei numVertices,
                                   GLushort *vertIdxs, GLsizei numParts, GLsizei numVertIdxsPerPart)
{
#if defined(GL_VERSION_1_1) || defined(GL_VERSION_ES_CM_1_0)
    int i;

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

    glVertexPointer(3, GL_FLOAT, 0, vertices);
    glNormalPointer(GL_FLOAT, 0, normals);

    if (textcs)
    {
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer(2, GL_FLOAT, 0, textcs);
    }

    if (!vertIdxs)
        glDrawArrays(GL_TRIANGLES, 0, numVertices);
    else
        if (numParts>1)
            for (i=0; i<numParts; i++)
                glDrawElements(GL_TRIANGLE_STRIP, numVertIdxsPerPart, GL_UNSIGNED_SHORT, vertIdxs+i*numVertIdxsPerPart);
        else
            glDrawElements(GL_TRIANGLES, numVertIdxsPerPart, GL_UNSIGNED_SHORT, vertIdxs);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    if (textcs)
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
#else
	fghDrawGeometrySolid10(vertices, normals, textcs, numVertices, vertIdxs, numParts, numVertIdxsPerPart);
#endif
}

/* Version for OpenGL (ES) >= 2.0 */
static void fghDrawGeometryWire20(GLfloat *vertices, GLfloat *normals, GLsizei numVertices,
                                  GLushort *vertIdxs, GLsizei numParts, GLsizei numVertPerPart, GLenum vertexMode,
                                  GLushort *vertIdxs2, GLsizei numParts2, GLsizei numVertPerPart2,
                                  GLint attribute_v_coord, GLint attribute_v_normal)
{
#if defined(GL_VERSION_1_1) || defined(GL_VERSION_ES_CM_1_0)
    GLuint vbo_coords = 0, vbo_normals = 0,
        ibo_elements = 0, ibo_elements2 = 0;
    GLsizei numVertIdxs = numParts * numVertPerPart;
    GLsizei numVertIdxs2 = numParts2 * numVertPerPart2;
    int i;

    if (numVertices > 0 && attribute_v_coord != -1) {
        fghGenBuffers(1, &vbo_coords);
        fghBindBuffer(FGH_ARRAY_BUFFER, vbo_coords);
        fghBufferData(FGH_ARRAY_BUFFER, numVertices * 3 * sizeof(vertices[0]),
                      vertices, FGH_STATIC_DRAW);
    }

    if (numVertices > 0 && attribute_v_normal != -1) {
        fghGenBuffers(1, &vbo_normals);
        fghBindBuffer(FGH_ARRAY_BUFFER, vbo_normals);
        fghBufferData(FGH_ARRAY_BUFFER, numVertices * 3 * sizeof(normals[0]),
                      normals, FGH_STATIC_DRAW);
    }

    if (vertIdxs != NULL) {
        fghGenBuffers(1, &ibo_elements);
        fghBindBuffer(FGH_ELEMENT_ARRAY_BUFFER, ibo_elements);
        fghBufferData(FGH_ELEMENT_ARRAY_BUFFER, numVertIdxs * sizeof(vertIdxs[0]),
                      vertIdxs, FGH_STATIC_DRAW);
        fghBindBuffer(FGH_ELEMENT_ARRAY_BUFFER, 0);
    }

    if (vertIdxs2 != NULL) {
        fghGenBuffers(1, &ibo_elements2);
        fghBindBuffer(FGH_ELEMENT_ARRAY_BUFFER, ibo_elements2);
        fghBufferData(FGH_ELEMENT_ARRAY_BUFFER, numVertIdxs2 * sizeof(vertIdxs2[0]),
                      vertIdxs2, FGH_STATIC_DRAW);
        fghBindBuffer(FGH_ELEMENT_ARRAY_BUFFER, 0);
    }

    if (vbo_coords) {
        fghEnableVertexAttribArray(attribute_v_coord);
        fghBindBuffer(FGH_ARRAY_BUFFER, vbo_coords);
        fghVertexAttribPointer(
            attribute_v_coord,  /* attribute */
            3,                  /* number of elements per vertex, here (x,y,z) */
            GL_FLOAT,           /* the type of each element */
            GL_FALSE,           /* take our values as-is */
            0,                  /* no extra data between each position */
            0                   /* offset of first element */
        );
        fghBindBuffer(FGH_ARRAY_BUFFER, 0);
    }

    if (vbo_normals) {
        fghEnableVertexAttribArray(attribute_v_normal);
        fghBindBuffer(FGH_ARRAY_BUFFER, vbo_normals);
        fghVertexAttribPointer(
            attribute_v_normal, /* attribute */
            3,                  /* number of elements per vertex, here (x,y,z) */
            GL_FLOAT,           /* the type of each element */
            GL_FALSE,           /* take our values as-is */
            0,                  /* no extra data between each position */
            0                   /* offset of first element */
        );
        fghBindBuffer(FGH_ARRAY_BUFFER, 0);
    }

    if (!vertIdxs) {
        /* Draw per face (TODO: could use glMultiDrawArrays if available) */
        for (i=0; i<numParts; i++)
            glDrawArrays(vertexMode, i*numVertPerPart, numVertPerPart);
    } else {
        fghBindBuffer(FGH_ELEMENT_ARRAY_BUFFER, ibo_elements);
        for (i=0; i<numParts; i++)
            glDrawElements(vertexMode, numVertPerPart,
                           GL_UNSIGNED_SHORT, (GLvoid*)(sizeof(vertIdxs[0])*i*numVertPerPart));
        /* Clean existing bindings before clean-up */
        /* Android showed instability otherwise */
        fghBindBuffer(FGH_ELEMENT_ARRAY_BUFFER, 0);
    }

    if (vertIdxs2) {
        fghBindBuffer(FGH_ELEMENT_ARRAY_BUFFER, ibo_elements2);
        for (i=0; i<numParts2; i++)
            glDrawElements(GL_LINE_LOOP, numVertPerPart2,
                           GL_UNSIGNED_SHORT, (GLvoid*)(sizeof(vertIdxs2[0])*i*numVertPerPart2));
        /* Clean existing bindings before clean-up */
        /* Android showed instability otherwise */
        fghBindBuffer(FGH_ELEMENT_ARRAY_BUFFER, 0);
    }

    if (vbo_coords != 0)
        fghDisableVertexAttribArray(attribute_v_coord);
    if (vbo_normals != 0)
        fghDisableVertexAttribArray(attribute_v_normal);

    if (vbo_coords != 0)
        fghDeleteBuffers(1, &vbo_coords);
    if (vbo_normals != 0)
        fghDeleteBuffers(1, &vbo_normals);
    if (ibo_elements != 0)
        fghDeleteBuffers(1, &ibo_elements);
    if (ibo_elements2 != 0)
        fghDeleteBuffers(1, &ibo_elements2);
#endif	/* GL version at least 1.1 */
}




/* Version for OpenGL (ES) >= 2.0 */
static void fghDrawGeometrySolid20(GLfloat *vertices, GLfloat *normals, GLfloat *textcs, GLsizei numVertices,
                                   GLushort *vertIdxs, GLsizei numParts, GLsizei numVertIdxsPerPart,
                                   GLint attribute_v_coord, GLint attribute_v_normal, GLint attribute_v_texture)
{
#if defined(GL_VERSION_1_1) || defined(GL_VERSION_ES_CM_1_0)
    GLuint vbo_coords = 0, vbo_normals = 0, vbo_textcs = 0, ibo_elements = 0;
    GLsizei numVertIdxs = numParts * numVertIdxsPerPart;
    int i;

    if (numVertices > 0 && attribute_v_coord != -1) {
        fghGenBuffers(1, &vbo_coords);
        fghBindBuffer(FGH_ARRAY_BUFFER, vbo_coords);
        fghBufferData(FGH_ARRAY_BUFFER, numVertices * 3 * sizeof(vertices[0]),
                      vertices, FGH_STATIC_DRAW);
        fghBindBuffer(FGH_ARRAY_BUFFER, 0);
    }

    if (numVertices > 0 && attribute_v_normal != -1) {
        fghGenBuffers(1, &vbo_normals);
        fghBindBuffer(FGH_ARRAY_BUFFER, vbo_normals);
        fghBufferData(FGH_ARRAY_BUFFER, numVertices * 3 * sizeof(normals[0]),
                      normals, FGH_STATIC_DRAW);
        fghBindBuffer(FGH_ARRAY_BUFFER, 0);
    }

    if (numVertices > 0 && attribute_v_texture != -1 && textcs) {
        fghGenBuffers(1, &vbo_textcs);
        fghBindBuffer(FGH_ARRAY_BUFFER, vbo_textcs);
        fghBufferData(FGH_ARRAY_BUFFER, numVertices * 2 * sizeof(textcs[0]),
                      textcs, FGH_STATIC_DRAW);
        fghBindBuffer(FGH_ARRAY_BUFFER, 0);
    }

    if (vertIdxs != NULL) {
        fghGenBuffers(1, &ibo_elements);
        fghBindBuffer(FGH_ELEMENT_ARRAY_BUFFER, ibo_elements);
        fghBufferData(FGH_ELEMENT_ARRAY_BUFFER, numVertIdxs * sizeof(vertIdxs[0]),
                      vertIdxs, FGH_STATIC_DRAW);
        fghBindBuffer(FGH_ELEMENT_ARRAY_BUFFER, 0);
    }

    if (vbo_coords) {
        fghEnableVertexAttribArray(attribute_v_coord);
        fghBindBuffer(FGH_ARRAY_BUFFER, vbo_coords);
        fghVertexAttribPointer(
            attribute_v_coord,  /* attribute */
            3,                  /* number of elements per vertex, here (x,y,z) */
            GL_FLOAT,           /* the type of each element */
            GL_FALSE,           /* take our values as-is */
            0,                  /* no extra data between each position */
            0                   /* offset of first element */
        );
        fghBindBuffer(FGH_ARRAY_BUFFER, 0);
    };

    if (vbo_normals) {
        fghEnableVertexAttribArray(attribute_v_normal);
        fghBindBuffer(FGH_ARRAY_BUFFER, vbo_normals);
        fghVertexAttribPointer(
            attribute_v_normal, /* attribute */
            3,                  /* number of elements per vertex, here (x,y,z) */
            GL_FLOAT,           /* the type of each element */
            GL_FALSE,           /* take our values as-is */
            0,                  /* no extra data between each position */
            0                   /* offset of first element */
        );
        fghBindBuffer(FGH_ARRAY_BUFFER, 0);
    };

    if (vbo_textcs) {
        fghEnableVertexAttribArray(attribute_v_texture);
        fghBindBuffer(FGH_ARRAY_BUFFER, vbo_textcs);
        fghVertexAttribPointer(
            attribute_v_texture,/* attribute */
            2,                  /* number of elements per vertex, here (s,t) */
            GL_FLOAT,           /* the type of each element */
            GL_FALSE,           /* take our values as-is */
            0,                  /* no extra data between each position */
            0                   /* offset of first element */
            );
        fghBindBuffer(FGH_ARRAY_BUFFER, 0);
    };

    if (vertIdxs == NULL) {
        glDrawArrays(GL_TRIANGLES, 0, numVertices);
    } else {
        fghBindBuffer(FGH_ELEMENT_ARRAY_BUFFER, ibo_elements);
        if (numParts>1) {
            for (i=0; i<numParts; i++) {
                glDrawElements(GL_TRIANGLE_STRIP, numVertIdxsPerPart, GL_UNSIGNED_SHORT, (GLvoid*)(sizeof(vertIdxs[0])*i*numVertIdxsPerPart));
            }
        } else {
            glDrawElements(GL_TRIANGLES, numVertIdxsPerPart, GL_UNSIGNED_SHORT, 0);
        }
        /* Clean existing bindings before clean-up */
        /* Android showed instability otherwise */
        fghBindBuffer(FGH_ELEMENT_ARRAY_BUFFER, 0);
    }

    if (vbo_coords != 0)
        fghDisableVertexAttribArray(attribute_v_coord);
    if (vbo_normals != 0)
        fghDisableVertexAttribArray(attribute_v_normal);
    if (vbo_textcs != 0)
        fghDisableVertexAttribArray(attribute_v_texture);

    if (vbo_coords != 0)
        fghDeleteBuffers(1, &vbo_coords);
    if (vbo_normals != 0)
        fghDeleteBuffers(1, &vbo_normals);
    if (vbo_textcs != 0)
        fghDeleteBuffers(1, &vbo_textcs);
    if (ibo_elements != 0)
        fghDeleteBuffers(1, &ibo_elements);
#endif	/* GL version at least 1.1 */
}



/**
 * Generate vertex indices for visualizing the normals.
 * vertices are written into verticesForNormalVisualization.
 * This must be freed by caller, we do the free at the
 * end of fghDrawNormalVisualization11/fghDrawNormalVisualization20
 */
static GLfloat *verticesForNormalVisualization;
static GLsizei numNormalVertices = 0;
static void fghGenerateNormalVisualization(GLfloat *vertices, GLfloat *normals, GLsizei numVertices)
{
    int i,j;
    numNormalVertices = numVertices * 2;
    verticesForNormalVisualization = (GLfloat*)malloc(numNormalVertices*3 * sizeof(GLfloat));

    for (i=0,j=0; i<numNormalVertices*3/2; i+=3, j+=6)
    {
        verticesForNormalVisualization[j+0] = vertices[i+0];
        verticesForNormalVisualization[j+1] = vertices[i+1];
        verticesForNormalVisualization[j+2] = vertices[i+2];
        verticesForNormalVisualization[j+3] = vertices[i+0] + normals[i+0]/4.f;
        verticesForNormalVisualization[j+4] = vertices[i+1] + normals[i+1]/4.f;
        verticesForNormalVisualization[j+5] = vertices[i+2] + normals[i+2]/4.f;
    }
}

#if !defined(GL_VERSION_1_1) && !defined(GL_VERSION_ES_CM_1_0)
/* Version for OpenGL 1.0 */
static void fghDrawNormalVisualization10(void)
{
	int i;
	GLfloat col[4], *vptr;

	glGetFloatv(GL_CURRENT_COLOR, col);
	glColor4f(1 - col[0], 1 - col[1], 1 - col[2], col[3]);

	vptr = verticesForNormalVisualization;
	glBegin(GL_LINES);
	for(i=0; i<numNormalVertices; i++) {
		glVertex3fv(vptr);
		vptr += 3;
	}
	glEnd();

	free(verticesForNormalVisualization);
	glColor4fv(col);
}
#endif

/* Version for OpenGL (ES) 1.1 */
static void fghDrawNormalVisualization11(void)
{
#if defined(GL_VERSION_1_1) || defined(GL_VERSION_ES_CM_1_0)
    GLfloat currentColor[4];
    /* Setup draw color: (1,1,1)-shape's color */
    glGetFloatv(GL_CURRENT_COLOR,currentColor);
    glColor4f(1-currentColor[0],1-currentColor[1],1-currentColor[2],currentColor[3]);

    glEnableClientState(GL_VERTEX_ARRAY);

    glVertexPointer(3, GL_FLOAT, 0, verticesForNormalVisualization);
    glDrawArrays(GL_LINES, 0, numNormalVertices);

    glDisableClientState(GL_VERTEX_ARRAY);

    /* Done, free memory, reset color */
    free(verticesForNormalVisualization);
    glColor4f(currentColor[0],currentColor[1],currentColor[2],currentColor[3]);
#else
	fghDrawNormalVisualization10();
#endif
}

/* Version for OpenGL (ES) >= 2.0 */
static void fghDrawNormalVisualization20(GLint attribute_v_coord)
{
#if defined(GL_VERSION_1_1) || defined(GL_VERSION_ES_CM_1_0)
    GLuint vbo_coords = 0;

    if (attribute_v_coord != -1) {
        fghGenBuffers(1, &vbo_coords);
        fghBindBuffer(FGH_ARRAY_BUFFER, vbo_coords);
        fghBufferData(FGH_ARRAY_BUFFER, numNormalVertices * 3 * sizeof(verticesForNormalVisualization[0]),
            verticesForNormalVisualization, FGH_STATIC_DRAW);
    }


    if (vbo_coords) {
        fghEnableVertexAttribArray(attribute_v_coord);
        fghBindBuffer(FGH_ARRAY_BUFFER, vbo_coords);
        fghVertexAttribPointer(
            attribute_v_coord,  /* attribute */
            3,                  /* number of elements per vertex, here (x,y,z) */
            GL_FLOAT,           /* the type of each element */
            GL_FALSE,           /* take our values as-is */
            0,                  /* no extra data between each position */
            0                   /* offset of first element */
            );
        fghBindBuffer(FGH_ARRAY_BUFFER, 0);
    }

    glDrawArrays(GL_LINES, 0, numNormalVertices);

    if (vbo_coords != 0)
        fghDisableVertexAttribArray(attribute_v_coord);

    if (vbo_coords != 0)
        fghDeleteBuffers(1, &vbo_coords);

    /* Done, free memory */
    free(verticesForNormalVisualization);
#endif	/* GL version at least 1.1 */
}

void fghDrawGeometrySolid(GLfloat *vertices, GLfloat *normals, GLfloat *textcs, GLsizei numVertices,
                          GLushort *vertIdxs, GLsizei numParts, GLsizei numVertIdxsPerPart)
{
    GLint attribute_v_coord, attribute_v_normal, attribute_v_texture;
    SFG_Window *win = fgStructure.CurrentWindow;
    if(win) {
        attribute_v_coord   = win->Window.attribute_v_coord;
        attribute_v_normal  = win->Window.attribute_v_normal;
        attribute_v_texture = win->Window.attribute_v_texture;
    } else {
        attribute_v_coord = attribute_v_normal = attribute_v_texture = -1;
    }

    if (win && win->State.VisualizeNormals)
        /* generate normals for each vertex to be drawn as well */
        fghGenerateNormalVisualization(vertices, normals, numVertices);

    if (fgState.HasOpenGL20 && (attribute_v_coord != -1 || attribute_v_normal != -1))
    {
        /* User requested a 2.0 draw */
        fghDrawGeometrySolid20(vertices, normals, textcs, numVertices,
                               vertIdxs, numParts, numVertIdxsPerPart,
                               attribute_v_coord, attribute_v_normal, attribute_v_texture);

        if (win && win->State.VisualizeNormals)
            /* draw normals for each vertex as well */
            fghDrawNormalVisualization20(attribute_v_coord);
    }
    else
    {
        fghDrawGeometrySolid11(vertices, normals, textcs, numVertices,
                               vertIdxs, numParts, numVertIdxsPerPart);

        if (win && win->State.VisualizeNormals)
            /* draw normals for each vertex as well */
            fghDrawNormalVisualization11();
    }
}

void fghDrawGeometryWire(GLfloat *vertices, GLfloat *normals, GLsizei numVertices,
  GLushort *vertIdxs, GLsizei numParts, GLsizei numVertPerPart, GLenum vertexMode,
  GLushort *vertIdxs2, GLsizei numParts2, GLsizei numVertPerPart2
)
{
    SFG_Window *win = fgStructure.CurrentWindow;

    GLint attribute_v_coord  = win ? win->Window.attribute_v_coord : -1;
    GLint attribute_v_normal = win ? win->Window.attribute_v_normal : -1;

    if (fgState.HasOpenGL20 && (attribute_v_coord != -1 || attribute_v_normal != -1))
        /* User requested a 2.0 draw */
        fghDrawGeometryWire20(vertices, normals, numVertices,
                              vertIdxs, numParts, numVertPerPart, vertexMode,
                              vertIdxs2, numParts2, numVertPerPart2,
                              attribute_v_coord, attribute_v_normal);
    else
        fghDrawGeometryWire11(vertices, normals,
                              vertIdxs, numParts, numVertPerPart, vertexMode,
                              vertIdxs2, numParts2, numVertPerPart2);
}

void fghCircleTable(GLfloat **sint, GLfloat **cost, const int n, const GLboolean halfCircle)
{
    int i;

    /* Table size, the sign of n flips the circle direction */
    const int size = abs(n);

    /* Determine the angle between samples */
    const GLfloat angle = (halfCircle?1:2)*(GLfloat)M_PI/(GLfloat)( ( n == 0 ) ? 1 : n );

    /* Allocate memory for n samples, plus duplicate of first entry at the end */
    *sint = (GLfloat*)malloc(sizeof(GLfloat) * (size+1));
    *cost = (GLfloat*)malloc(sizeof(GLfloat) * (size+1));

    /* Bail out if memory allocation fails, fgError never returns */
    if (!(*sint) || !(*cost))
    {
        free(*sint);
        free(*cost);
        fgError("Failed to allocate memory in fghCircleTable");
    }

    /* Compute cos and sin around the circle */
    (*sint)[0] = 0.0;
    (*cost)[0] = 1.0;

    for (i=1; i<size; i++)
    {
        (*sint)[i] = (GLfloat)sin(angle*i);
        (*cost)[i] = (GLfloat)cos(angle*i);
    }


    if (halfCircle)
    {
        (*sint)[size] =  0.0f;  /* sin PI */
        (*cost)[size] = -1.0f;  /* cos PI */
    }
    else
    {
        /* Last sample is duplicate of the first (sin or cos of 2 PI) */
        (*sint)[size] = (*sint)[0];
        (*cost)[size] = (*cost)[0];
    }
}

void fghGenerateCylinder(
    GLfloat radius, GLfloat height, GLint slices, GLint stacks, /*  input */
    GLfloat **vertices, GLfloat **normals, int* nVert           /* output */
    )
{
    int i,j;
    int idx = 0;    /* idx into vertex/normal buffer */

    /* Step in z as stacks are drawn. */
    GLfloat radf = (GLfloat)radius;
    GLfloat z;
    const GLfloat zStep = (GLfloat)height / ( ( stacks > 0 ) ? stacks : 1 );

    /* Pre-computed circle */
    GLfloat *sint,*cost;

    /* number of unique vertices */
    if (slices==0 || stacks<1)
    {
        /* nothing to generate */
        *nVert = 0;
        return;
    }
    *nVert = slices*(stacks+3)+2;   /* need two extra stacks for closing off top and bottom with correct normals */

    if ((*nVert) > 65535)
        /*
         * limit of glushort, that's 256*256 subdivisions, should be enough in practice. See note above
         */
        fgWarning("fghGenerateCylinder: too many slices or stacks requested, indices will wrap");

    /* Pre-computed circle */
    fghCircleTable(&sint,&cost,-slices,GL_FALSE);

    /* Allocate vertex and normal buffers, bail out if memory allocation fails */
    *vertices = (GLfloat*)malloc((*nVert)*3*sizeof(GLfloat));
    *normals  = (GLfloat*)malloc((*nVert)*3*sizeof(GLfloat));
    if (!(*vertices) || !(*normals))
    {
        free(*vertices);
        free(*normals);
        fgError("Failed to allocate memory in fghGenerateCylinder");
    }

    z=0;
    /* top on Z-axis */
    (*vertices)[0] =  0.f;
    (*vertices)[1] =  0.f;
    (*vertices)[2] =  0.f;
    (*normals )[0] =  0.f;
    (*normals )[1] =  0.f;
    (*normals )[2] = -1.f;
    idx = 3;
    /* other on top (get normals right) */
    for (j=0; j<slices; j++, idx+=3)
    {
        (*vertices)[idx  ] = cost[j]*radf;
        (*vertices)[idx+1] = sint[j]*radf;
        (*vertices)[idx+2] = z;
        (*normals )[idx  ] = 0.f;
        (*normals )[idx+1] = 0.f;
        (*normals )[idx+2] = -1.f;
    }

    /* each stack */
    for (i=0; i<stacks+1; i++ )
    {
        for (j=0; j<slices; j++, idx+=3)
        {
            (*vertices)[idx  ] = cost[j]*radf;
            (*vertices)[idx+1] = sint[j]*radf;
            (*vertices)[idx+2] = z;
            (*normals )[idx  ] = cost[j];
            (*normals )[idx+1] = sint[j];
            (*normals )[idx+2] = 0.f;
        }

        z += zStep;
    }

    /* other on bottom (get normals right) */
    z -= zStep;
    for (j=0; j<slices; j++, idx+=3)
    {
        (*vertices)[idx  ] = cost[j]*radf;
        (*vertices)[idx+1] = sint[j]*radf;
        (*vertices)[idx+2] = z;
        (*normals )[idx  ] = 0.f;
        (*normals )[idx+1] = 0.f;
        (*normals )[idx+2] = 1.f;
    }

    /* bottom */
    (*vertices)[idx  ] =  0.f;
    (*vertices)[idx+1] =  0.f;
    (*vertices)[idx+2] =  height;
    (*normals )[idx  ] =  0.f;
    (*normals )[idx+1] =  0.f;
    (*normals )[idx+2] =  1.f;

    /* Release sin and cos tables */
    free(sint);
    free(cost);
}

int fghCylinder(
    GLfloat radius, GLfloat height, GLint slices, GLint stacks, GLboolean useWireMode,
    GLfloat* vertices, GLfloat* normals, GLushort* sliceIdx, GLushort* stackIdx
){
    int i,j,idx, nVert;
    //GLfloat *vertices, *normals;

    /* Generate vertices and normals */
    /* Note, (stacks+1)*slices vertices for side of object, 2*slices+2 for top and bottom closures */
    fghGenerateCylinder(radius,height,slices,stacks,&vertices,&normals,&nVert);

    if (nVert==0)
        /* nothing to draw */
        return 0;

    if (useWireMode)
    {
        //GLushort  *sliceIdx, *stackIdx;
        /* First, generate vertex index arrays for drawing with glDrawElements
         * We have a bunch of line_loops to draw for each stack, and a
         * bunch for each slice.
         */

        stackIdx = (GLushort*)malloc(slices*(stacks+1)*sizeof(GLushort));
        sliceIdx = (GLushort*)malloc(slices*2         *sizeof(GLushort));
        if (!(stackIdx) || !(sliceIdx))
        {
            //free(stackIdx);
            //free(sliceIdx);
            fgError("Failed to allocate memory in fghCylinder");
        }

        /* generate for each stack */
        for (i=0,idx=0; i<stacks+1; i++)
        {
            GLushort offset = 1+(i+1)*slices;       /* start at 1 (0 is top vertex), and we advance one stack down as we go along */
            for (j=0; j<slices; j++, idx++)
            {
                stackIdx[idx] = offset+j;
            }
        }

        /* generate for each slice */
        for (i=0,idx=0; i<slices; i++)
        {
            GLushort offset = 1+i;                  /* start at 1 (0 is top vertex), and we advance one slice as we go along */
            sliceIdx[idx++] = offset+slices;
            sliceIdx[idx++] = offset+(stacks+1)*slices;
        }

        /* draw */
        /*fghDrawGeometryWire(vertices,normals,nVert,
            sliceIdx,1,slices*2,GL_LINES,
            stackIdx,stacks+1,slices);*/

        /* cleanup allocated memory */
        //free(sliceIdx);
        //free(stackIdx);
    }
    else
    {
        /* First, generate vertex index arrays for drawing with glDrawElements
         * All stacks, including top and bottom are covered with a triangle
         * strip.
         */
        GLushort  *stripIdx;
        /* Create index vector */
        GLushort offset;

        /* Allocate buffers for indices, bail out if memory allocation fails */
        stripIdx = (GLushort*)malloc((slices+1)*2*(stacks+2)*sizeof(GLushort)); /* stacks +2 because of closing off bottom and top */
        if (!(stripIdx))
        {
            free(stripIdx);
            fgError("Failed to allocate memory in fghCylinder");
        }

        /* top stack */
        for (j=0, idx=0;  j<slices;  j++, idx+=2)
        {
            stripIdx[idx  ] = 0;
            stripIdx[idx+1] = j+1;              /* 0 is top vertex, 1 is first for first stack */
        }
        stripIdx[idx  ] = 0;                    /* repeat first slice's idx for closing off shape */
        stripIdx[idx+1] = 1;
        idx+=2;

        /* middle stacks: */
        /* Strip indices are relative to first index belonging to strip, NOT relative to first vertex/normal pair in array */
        for (i=0; i<stacks; i++, idx+=2)
        {
            offset = 1+(i+1)*slices;                /* triangle_strip indices start at 1 (0 is top vertex), and we advance one stack down as we go along */
            for (j=0; j<slices; j++, idx+=2)
            {
                stripIdx[idx  ] = offset+j;
                stripIdx[idx+1] = offset+j+slices;
            }
            stripIdx[idx  ] = offset;               /* repeat first slice's idx for closing off shape */
            stripIdx[idx+1] = offset+slices;
        }

        /* top stack */
        offset = 1+(stacks+2)*slices;
        for (j=0; j<slices; j++, idx+=2)
        {
            stripIdx[idx  ] = offset+j;
            stripIdx[idx+1] = nVert-1;              /* zero based index, last element in array (bottom vertex)... */
        }
        stripIdx[idx  ] = offset;
        stripIdx[idx+1] = nVert-1;                  /* repeat first slice's idx for closing off shape */

        /* draw */
        fghDrawGeometrySolid(vertices,normals,NULL,nVert,stripIdx,stacks+2,(slices+1)*2);

        /* cleanup allocated memory */
        free(stripIdx);
    }

    /* cleanup allocated memory */
    //free(vertices);
    //free(normals);
    return nVert;
}

#include <iostream>
//#include <format>
void PrintCylinder()
{
    GLfloat* vertices; GLfloat* normals; GLushort* sliceIdx; GLushort* stackIdx;
    double radius{1.0}; double height{-256.0}; GLint slices{64}; GLint stacks{64};
    int i,j,idx{0}; // idx into vertex/normal buffer
    
    GLfloat radf = (GLfloat)radius; GLfloat z;
    const GLfloat zStep = (GLfloat)height / ( ( stacks > 0 ) ? stacks : 1 ); // Step in z as stacks are drawn.
    GLfloat *sint,*cost; // Pre-computed circle
    fghCircleTable(&sint,&cost,-slices,GL_FALSE);
    
    int nVert{slices*(stacks+3)+2}; // need two extra stacks for closing off top and bottom with correct normals
    // Note, (stacks+1)*slices vertices for side of object, 2*slices+2 for top and bottom closures
    //fghGenerateCylinder(radius,-height,slices,stacks,&vertices,&normals,&nVert);
    vertices = (GLfloat*)malloc(nVert*3*sizeof(GLfloat));
    normals  = (GLfloat*)malloc(nVert*3*sizeof(GLfloat));
    
    z=0; // top on Z-axis
    vertices[0] =  0.f;
    vertices[1] =  0.f;
    vertices[2] =  0.f;
    normals[0] =  0.f;
    normals[1] =  0.f;
    normals[2] = -1.f;
    idx = 3;
    // other on top (get normals right)
    for (j=0; j<slices; j++, idx+=3)
    {
        vertices[idx  ] = cost[j]*radf;
        vertices[idx+1] = sint[j]*radf;
        vertices[idx+2] = z;
        normals[idx  ] = 0.f;
        normals[idx+1] = 0.f;
        normals[idx+2] = -1.f;
    }
    
    // each stack
    for (i=0; i<stacks+1; i++ )
    {
        for (j=0; j<slices; j++, idx+=3)
        {
            vertices[idx  ] = cost[j]*radf;
            vertices[idx+1] = sint[j]*radf;
            vertices[idx+2] = z;
            normals[idx  ] = cost[j];
            normals[idx+1] = sint[j];
            normals[idx+2] = 0.f;
        }

        z += zStep;
    }
    
    z -= zStep;
    // other on bottom (get normals right)
    for (j=0; j<slices; j++, idx+=3)
    {
        vertices[idx  ] = cost[j]*radf;
        vertices[idx+1] = sint[j]*radf;
        vertices[idx+2] = z;
        normals[idx  ] = 0.f;
        normals[idx+1] = 0.f;
        normals[idx+2] = 1.f;
    }
    
    // bottom
    vertices[idx  ] =  0.f;
    vertices[idx+1] =  0.f;
    vertices[idx+2] =  height;
    normals[idx  ] =  0.f;
    normals[idx+1] =  0.f;
    normals[idx+2] =  1.f;
    
    // Release sin and cos tables
    free(sint);
    free(cost);
    
    std::cout << "nVert: " << nVert << " x3: " << (nVert*3) << '\n';
    std::cout << "last vertex/normal idx: " << idx << '\n';
    
    // _____________________________________________________________//
    // fghCylinder                                                  //
    // _____________________________________________________________//
    stackIdx = (GLushort*)malloc(slices*(stacks+1)*sizeof(GLushort));
    sliceIdx = (GLushort*)malloc(slices*2         *sizeof(GLushort));
    for (i=0,idx=0; i<stacks+1; i++) // generate for each stack
    {
        GLushort offset = 1+(i+1)*slices; // start at 1 (0 is top vertex), and we advance one stack down as we go along
        for (j=0; j<slices; j++, idx++)
        {
            stackIdx[idx] = offset+j;
        }
    }
    std::cout << "last stackIdx: " << idx << '\n';
    std::cout << "calc stackMax: " << (slices*(stacks+1)) << '\n';
    for (i=0,idx=0; i<slices; i++) // generate for each slice
    {
        GLushort offset = 1+i; // start at 1 (0 is top vertex), and we advance one slice as we go along
        sliceIdx[idx++] = offset+slices;
        sliceIdx[idx++] = offset+(stacks+1)*slices;
    }
    std::cout << "last sliceIdx: " << idx << '\n';
    std::cout << "calc sliceMax: " << (slices*2) << '\n';
    
    /*std::cout << "\nvertices:";
    for (int V{0}; V < (nVert*3); ++V) {
        if ((V%3) == 0) std::cout << "\n";
        std::cout << vertices[V] << ' ';
    }
    std::cout << "\nnormals:";
    for (int V{0}; V < (nVert*3); ++V) {
        if ((V%3) == 0) std::cout << "\n";
        std::cout << normals[V] << ' ';
    }*/
    std::cout << "\nstacks:";
    for (int V{0}; V < 128; ++V) { // (slices*(stacks+1))
        if ((V%3) == 0) std::cout << "\n";
        std::cout << stackIdx[V] << ' ';
    }
    std::cout << "\nslices:";
    for (int V{0}; V < 128; ++V) { // (slices*2)
        if ((V%3) == 0) std::cout << "\n";
        std::cout << sliceIdx[V] << ' ';
    }
    std::cout << "\n\n";
    free(vertices); free(normals); free(stackIdx); free(sliceIdx);
    return;
}
