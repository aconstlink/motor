// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT
#pragma once

namespace motor
{
namespace collide
{
enum class hit_test_type
{
    inside,
    outside,
    intersect,
    disjoint,
    overlap,
    num_hit_test_types
};
}

} // namespace motor