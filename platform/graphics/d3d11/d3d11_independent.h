// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT

#pragma once

#include "d3d11_pimpl_data.h"

#include <variant>

namespace d3d11_detail
{

using namespace motor::core::types;

struct d3d11_independent
{

    struct compilation_result
    {
        bool_t success = false;

        // safe reference. hold ref count
        motor::graphics::msl_object_mtr_t object = nullptr;

        motor::graphics::shader_object_t shader;
        d3d11_pimpl_data::shader_data_t data;
    };
    struct release_result
    {
    };
    using backward_message = std::variant< compilation_result, release_result >;
};

} // namespace d3d11_detail
