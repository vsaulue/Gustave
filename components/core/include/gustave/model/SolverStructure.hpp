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

#include "gustave/cfg/cLibConfig.hpp"
#include "ContactArea.hpp"
#include "Node.hpp"

namespace Gustave::Model {
    template<Cfg::cLibConfig auto cfg>
    struct SolverStructure {
        [[nodiscard]]
        std::vector<Node<cfg>>& nodes() {
            return nodes_;
        }

        [[nodiscard]]
        std::vector<Node<cfg>> const& nodes() const {
            return nodes_;
        }

        [[nodiscard]]
        std::vector<ContactArea<cfg>> const& links() const {
            return links_;
        }

        void addLink(ContactArea<cfg> const& newLink) {
            assert(newLink.localNodeId() < nodes_.size());
            assert(newLink.otherNodeId() < nodes_.size());
            links_.push_back(newLink);
        }
    private:
        std::vector<Node<cfg>> nodes_;
        std::vector<ContactArea<cfg>> links_;
    };
}
