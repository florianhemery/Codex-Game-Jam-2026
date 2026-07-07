/*
** EPITECH PROJECT, 2026
** racer
** File description:
** ECS lap progress component
*/

#ifndef LAP_PROGRESS_COMPONENT_HPP_
#define LAP_PROGRESS_COMPONENT_HPP_

namespace racer::engine {

struct LapProgressComponent {
    int lap = 0;
    int lastSegment = 0;
    bool passedMidpoint = false;
    bool finished = false;
    float finishTime = 0.0f;
};

} // namespace racer::engine

#endif /* !LAP_PROGRESS_COMPONENT_HPP_ */
