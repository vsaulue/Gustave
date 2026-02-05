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

#include <vector>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/core/solvers/structure/ContactIndex.hpp>
#include <gustave/core/solvers/structure/Link.hpp>
#include <gustave/core/solvers/structure/Node.hpp>
#include <gustave/utils/canNarrow.hpp>

namespace gustave::core::solvers {
    template<cfg::cLibConfig auto libCfg>
    class Structure {
    public:
        using ContactIndex = structure::ContactIndex<libCfg>;
        using Link = structure::Link<libCfg>;
        using Node = structure::Node<libCfg>;

        using LinkIndex = Link::LinkIndex;
        using NodeIndex = Node::NodeIndex;

        using Links = std::vector<Link>;
        using Nodes = std::vector<Node>;

        [[nodiscard]]
        Nodes const& nodes() const {
            return nodes_;
        }

        [[nodiscard]]
        Links const& links() const {
            return links_;
        }

        NodeIndex addNode(Node const& newNode) {
            auto const result = nodes_.size();
            if (!utils::canNarrow<NodeIndex>(result)) {
                throw std::overflow_error("Maximum number of nodes allowed by library configuration reached.");
            }
            nodes_.push_back(newNode);
            return result;
        }

        LinkIndex addLink(Link const& newLink) {
            auto const result = links_.size();
            if (!utils::canNarrow<LinkIndex>(result)) {
                throw std::overflow_error("Maximum number of links allowed by library configuration reached.");
            }
            assert(newLink.localNodeId() < nodes_.size());
            assert(newLink.otherNodeId() < nodes_.size());
            links_.push_back(newLink);
            return result;
        }
    private:
        Nodes nodes_;
        Links links_;
    };
}
