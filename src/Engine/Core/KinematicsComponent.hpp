/*
** EPITECH PROJECT, 2026
** racer
** File description:
** ECS kinematics component
*/

#ifndef KINEMATICS_COMPONENT_HPP_
#define KINEMATICS_COMPONENT_HPP_

namespace racer::engine {

struct KinematicsComponent {
    float speed = 0.0f;
    float velocityHeading = 0.0f;
    bool isDrifting = false;
};

} // namespace racer::engine

#endif /* !KINEMATICS_COMPONENT_HPP_ */
