#include "quat.h"

#include <math.h>

void eulerToQuat(vec3 euler, versor quat)
{
    double roll = euler[0], pitch = euler[1], yaw = euler[2];

    double cr = cos(roll * 0.5);
    double sr = sin(roll * 0.5);
    double cp = cos(pitch * 0.5);
    double sp = sin(pitch * 0.5);
    double cy = cos(yaw * 0.5);
    double sy = sin(yaw * 0.5);

    quat[0] = sr * cp * cy - cr * sp * sy;
    quat[1] = cr * sp * cy + sr * cp * sy;
    quat[2] = cr * cp * sy - sr * sp * cy;
    quat[3] = cr * cp * cy + sr * sp * sy;
}

void quatToEuler(versor quat, vec3 euler)
{
    double sinr_cosp = 2 * (quat[3] * quat[0] + quat[1] * quat[2]);
    double cosr_cosp = 1 - 2 * (quat[0] * quat[0] + quat[1] * quat[1]);
    euler[0] = glm_deg(atan2(sinr_cosp, cosr_cosp));

    double sinp = sqrt(1 + 2 * (quat[3] * quat[1] - quat[0] * quat[2]));
    double cosp = sqrt(1 - 2 * (quat[3] * quat[1] - quat[0] * quat[2]));
    euler[1] = glm_deg(2 * atan2(sinp, cosp) - M_PI / 2);

    double siny_cosp = 2 * (quat[3] * quat[2] + quat[0] * quat[1]);
    double cosy_cosp = 1 - 2 * (quat[1] * quat[1] + quat[2] * quat[2]);
    euler[2] = glm_deg(atan2(siny_cosp, cosy_cosp));
}

