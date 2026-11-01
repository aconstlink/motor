// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT

#pragma once

namespace motor
{
namespace gfx
{
namespace volumes
{
class bounding_volume
{
};

class aabb : public bounding_volume
{
};

class obb : public bounding_volume
{
};

class sphere : public bounding_volume
{
};

} // namespace volumes

} // namespace gfx
} // namespace motor