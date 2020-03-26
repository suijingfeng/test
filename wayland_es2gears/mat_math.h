#ifndef MAT_MATH_H_
#define MAT_MATH_H_

#include <GLES2/gl2.h>
#include <EGL/egl.h>


/**
 * Multiplies two 4x4 matrices.
 *
 * The result is stored in matrix m.
 *
 * @param m the first matrix to multiply
 * @param n the second matrix to multiply
 */
void multiply(GLfloat *m, const GLfloat *n);


/**
 * Rotates a 4x4 matrix.
 *
 * @param[in,out] m the matrix to rotate
 * @param angle the angle to rotate
 * @param x the x component of the direction to rotate to
 * @param y the y component of the direction to rotate to
 * @param z the z component of the direction to rotate to
 */
void rotate(GLfloat *m, GLfloat angle, GLfloat x, GLfloat y, GLfloat z);


/**
 * Translates a 4x4 matrix.
 *
 * @param[in,out] m the matrix to translate
 * @param x the x component of the direction to translate to
 * @param y the y component of the direction to translate to
 * @param z the z component of the direction to translate to
 */
void translate(GLfloat *m, GLfloat x, GLfloat y, GLfloat z);

/**
 * Creates an identity 4x4 matrix.
 *
 * @param m the matrix make an identity matrix
 */
void identity(GLfloat *m);


/**
 * Transposes a 4x4 matrix.
 *
 * @param m the matrix to transpose
 */
void transpose(GLfloat *m);


/**
 * Inverts a 4x4 matrix.
 *
 * This function can currently handle only pure translation-rotation matrices.
 * Read http://www.gamedev.net/community/forums/topic.asp?topic_id=425118
 * for an explanation.
 */
void invert(GLfloat *m);


/**
 * Calculate a perspective projection transformation.
 *
 * @param m the matrix to save the transformation in
 * @param fovy the field of view in the y direction
 * @param aspect the view aspect ratio
 * @param zNear the near clipping plane
 * @param zFar the far clipping plane
 */
void perspective(GLfloat *m, GLfloat fovy, GLfloat aspect, GLfloat zNear, GLfloat zFar);

/*
 * Compute sin(val) and cos(val) simiously
 * I dont want define the _GUN_SOURCE macro in order to use sincos function
 */
void compute_sin_cos(double val, double * const pS, double * const pC);
#endif
