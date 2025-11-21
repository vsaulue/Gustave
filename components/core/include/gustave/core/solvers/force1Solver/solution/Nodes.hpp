/* This file is part of Gustave, a structural integrity library for video games.
 *
 * Copyright (c) 2022-2025 Vincent Saulue-Laborde <vincent_saulue@hotmail.fr>
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

#include <sstream>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/cUnitOf.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/core/solvers/force1Solver/detail/SolutionData.hpp>
#include <gustave/core/solvers/force1Solver/solution/NodeReference.hpp>
#include <gustave/core/solvers/Structure.hpp>
#include <gustave/utils/ForwardIterator.hpp>

namespace gustave::core::solvers::force1Solver::solution {
    template<cfg::cLibConfig auto libCfg>
    class Nodes {
    public:
        using NodeReference = solution::NodeReference<libCfg>;
        using NodeIndex = typename NodeReference::NodeIndex;
    private:
        using SolutionData = detail::SolutionData<libCfg>;
        using Structure = solvers::Structure<libCfg>;
        using StructureNodes = typename Structure::Nodes;

        class Enumerator {
        public:
            [[nodiscard]]
            Enumerator()
                : solution_{ nullptr }
                , value_{ nullptr, 0 }
            {}

            [[nodiscard]]
            explicit Enumerator(SolutionData const& solution)
                : solution_{ &solution }
                , value_{ solution, 0 }
            {}

            [[nodiscard]]
            bool isEnd() const {
                return value_.index() >= solution_->basis().structure().nodes().size();
            }

            void operator++() {
                value_ = NodeReference{ *solution_, value_.index() + 1};
            }

            [[nodiscard]]
            NodeReference const& operator*() const {
                return value_;
            }

            [[nodiscard]]
            bool operator==(Enumerator const&) const = default;
        private:
            SolutionData const* solution_;
            NodeReference value_;
        };
    public:
        using Iterator = utils::ForwardIterator<Enumerator>;

        [[nodiscard]]
        explicit Nodes(SolutionData const& solution)
            : solution_{ &solution }
        {}

        [[nodiscard]]
        NodeReference at(NodeIndex index) const {
            std::size_t nodesCount = structureNodes().size();
            if (index >= nodesCount) {
                std::stringstream msg;
                msg << "Index " << index << " is out of range (size: " << nodesCount << ").";
                throw std::out_of_range(msg.str());
            }
            return NodeReference{ *solution_, index };
        }

        [[nodiscard]]
        Iterator begin() const {
            return Iterator{ *solution_ };
        }

        [[nodiscard]]
        constexpr std::default_sentinel_t end() const {
            return {};
        }

        [[nodiscard]]
        std::size_t size() const {
            return structureNodes().size();
        }
    private:
        [[nodiscard]]
        StructureNodes const& structureNodes() const {
            return solution_->basis().structure().nodes();
        }

        SolutionData const* solution_;
    };
}
