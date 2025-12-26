/*
 * quat.h
 *
 * Extra helper methods for converting between Euler angles and quaternions
 *
 * Algorithms referenced from
 * https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
 */

#ifndef QUAT_H
#define QUAT_H

#include <cglm/cglm.h>

// converts Euler angles into quaternion
void eulerToQuat(vec3 euler, versor quat);

// converts quaternion into Euler angles
void quatToEuler(versor quat, vec3 euler);

#endif

