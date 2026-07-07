/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Core test runner — check helper and failure counter
*/

#include "Engine/Core/CoreTestRunner.hpp"

#include <cstdio>

void CoreTestRunner::check(bool cond, int line, const char *expr)
{
    if (!cond) {
        std::fprintf(stderr, "ECHEC ligne %d : %s\n", line, expr);
        ++failures_;
    }
}

int CoreTestRunner::failures() const
{
    return failures_;
}
