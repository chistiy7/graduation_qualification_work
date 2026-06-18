#pragma once

#include "domain/test_route.hpp"
#include "model/problem.hpp"

#include <string>
#include <vector>

namespace lab {

[[nodiscard]] std::vector<std::string> validateSpecimenOneTestRule(
    const ProblemDefinition& problem);

[[nodiscard]] std::vector<std::string> validateRouteOneSpecimenOneStep(const TestRoute& route);

[[nodiscard]] bool specimenAppearsOnceInRoute(const TestRoute& route);

}  // namespace lab
