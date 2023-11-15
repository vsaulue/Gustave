/* This file is part of Gustave, a structural integrity library for video games.
 *
 * Copyright (c) 2022,2023 Vincent Saulue-Laborde <vincent_saulue@hotmail.fr>
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

#include <vector>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/scenes/cuboidGrid/detail/StructureData.hpp>

namespace Gustave::Scenes::CuboidGrid {
    template<Cfg::cLibConfig auto cfg>
    class TransactionResult {
    private:
        using StructureData = detail::StructureData<cfg>;
    public:
        using DeletedSet = std::vector<StructureData const*>;
        using NewSet = std::vector<std::shared_ptr<StructureData const>>;

        [[nodiscard]]
        TransactionResult(NewSet newStructures, DeletedSet deletedStructures)
            : newStructures_{ std::move(newStructures) }
            , deletedStructures_{ std::move(deletedStructures) }
        {}

        [[nodiscard]]
        NewSet const& newStructures() const {
            return newStructures_;
        }

        [[nodiscard]]
        DeletedSet const& deletedStructures() const {
            return deletedStructures_;
        }
    private:
        NewSet newStructures_;
        DeletedSet deletedStructures_;
    };
}
