/* This file is part of Gustave, a structural integrity library for video games.
 *
 * Copyright (c) 2022-2026 Vincent Saulue-Laborde <vincent_saulue@hotmail.fr>
 *
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files(the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions :
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include <type_traits>

#include <gustave/core/scenes/common/cSceneUserData.hpp>

namespace gustave::core::scenes::common {
    struct EmptyUserData {};

    template<common::cSceneUserData UserDatas_>
    struct UserDataTraits {
    private:
        [[nodiscard]]
        static consteval meta::cTypeWrapper auto blockUserDataType() {
            if constexpr (std::is_void_v<UserDatas_>) {
                return meta::TypeWrapper<void>{};
            } else {
                return meta::TypeWrapper<typename UserDatas_::Block>{};
            }
        }

        [[nodiscard]]
        static consteval meta::cTypeWrapper auto structureUserDataType() {
            if constexpr (std::is_void_v<UserDatas_>) {
                return meta::TypeWrapper<void>{};
            } else {
                return meta::TypeWrapper<typename UserDatas_::Structure>{};
            }
        }
    public:
        using Block = decltype(blockUserDataType())::Type;
        using Structure = decltype(structureUserDataType())::Type;

        [[nodiscard]]
        static constexpr bool hasBlockUserData() {
            return not std::is_void_v<Block>;
        }

        [[nodiscard]]
        static constexpr bool hasStructureUserData() {
            return not std::is_void_v<Structure>;
        }

        using BlockMember = std::conditional_t<hasBlockUserData(), Block, EmptyUserData>;
        using StructureMember = std::conditional_t<hasStructureUserData(), Structure, EmptyUserData>;
    };
}
