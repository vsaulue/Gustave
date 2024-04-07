/* This file is part of Gustave, a structural integrity library for video games.
 *
 * Copyright (c) 2022-2024 Vincent Saulue-Laborde <vincent_saulue@hotmail.fr>
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
#include <gustave/solvers/force1/detail/NodeInfo.hpp>
#include <gustave/solvers/force1/detail/NodeStats.hpp>
#include <gustave/solvers/force1/detail/SolutionData.hpp>
#include <gustave/solvers/force1/solution/ContactReference.hpp>
#include <gustave/solvers/Structure.hpp>
#include <gustave/utils/EndIterator.hpp>
#include <gustave/utils/ForwardIterator.hpp>
#include <gustave/utils/NoInit.hpp>

namespace gustave::solvers::force1::solution {
    template<cfg::cLibConfig auto libCfg>
    class ContactReference;

    template<cfg::cLibConfig auto libCfg>
    class NodeReference {
    private:
        using SolutionData = detail::SolutionData<libCfg>;
        using Structure = solvers::Structure<libCfg>;

        using LinkIndex = cfg::LinkIndex<libCfg>;
        using NodeInfo = detail::NodeInfo<libCfg>;
        using NodeStats = detail::NodeStats<libCfg>;
        using StructureNode = typename Structure::Node;
        using StructureLink = typename Structure::Link;

        template<cfg::cUnitOf<libCfg> auto unit>
        using Real = cfg::Real<libCfg, unit>;

        template<cfg::cUnitOf<libCfg> auto unit>
        using Vector3 = cfg::Vector3<libCfg, unit>;

        static constexpr auto u = cfg::units(libCfg);
    public:
        using ContactReference = solution::ContactReference<libCfg>;

        using ContactIndex = typename Structure::ContactIndex;
        using NodeIndex = typename Structure::NodeIndex;

        class Contacts {
        private:
            class Enumerator {
            private:
                using DataIterator = typename NodeInfo::Contacts::const_iterator;
            public:
                [[nodiscard]]
                Enumerator()
                    : node_{ nullptr }
                    , dataIterator_{}
                    , value_{ utils::NO_INIT }
                {}

                [[nodiscard]]
                explicit Enumerator(NodeReference const& node)
                    : node_{ node }
                    , dataIterator_{ node.info().contacts.begin() }
                    , value_{ utils::NO_INIT }
                {
                    updateValue();
                }

                [[nodiscard]]
                bool isEnd() const {
                    return dataIterator_ == node_.info().contacts.end();
                }

                void operator++() {
                    ++dataIterator_;
                    updateValue();
                }

                [[nodiscard]]
                ContactReference const& operator*() const {
                    return value_;
                }

                [[nodiscard]]
                bool operator==(Enumerator const& other) const {
                    return dataIterator_ == other.dataIterator_;
                }
            private:
                void updateValue() {
                    if (!isEnd()) {
                        LinkIndex linkId = dataIterator_->linkIndex();
                        StructureLink const& link = node_.solution_->basis().structure().links()[linkId];
                        bool isOnLocalNode = (link.localNodeId() == node_.index_);
                        value_ = ContactReference{ *node_.solution_, ContactIndex{ linkId, isOnLocalNode } };
                    }
                }

                NodeReference node_;
                DataIterator dataIterator_;
                ContactReference value_;
            };
        public:
            using Iterator = utils::ForwardIterator<Enumerator>;

            [[nodiscard]]
            explicit Contacts(NodeReference const& node)
                : node_{ node }
            {}

            [[nodiscard]]
            ContactReference at(ContactIndex const& contactIndex) const {
                StructureLink const& link = node_.solution_->basis().structure().links().at(contactIndex.linkIndex);
                if (contactIndex.isOnLocalNode) {
                    if (link.localNodeId() == node_.index_) {
                        return ContactReference{ *node_.solution_, contactIndex };
                    }
                }
                else {
                    if (link.otherNodeId() == node_.index_) {
                        return ContactReference{ *node_.solution_, contactIndex };
                    }
                }
                std::stringstream msg;
                msg << "Node " << node_.index_ << " does not contain contact " << contactIndex << '.';
                throw std::out_of_range(msg.str());
            }

            [[nodiscard]]
            Iterator begin() const {
                return Iterator{ node_ };
            }

            [[nodiscard]]
            utils::EndIterator end() const {
                return {};
            }

            [[nodiscard]]
            std::size_t size() const {
                return node_.info().contacts.size();
            }
        private:
            NodeReference node_;
        };

        [[nodiscard]]
        explicit NodeReference(SolutionData const& solution, NodeIndex const index)
            : solution_{ &solution }
            , index_{ index }
        {}

        [[nodiscard]]
        explicit NodeReference(utils::NoInit)
            : solution_{ nullptr }
            , index_{ 0 }
        {}

        [[nodiscard]]
        Contacts contacts() const {
            return Contacts{ *this };
        }

        [[nodiscard]]
        Vector3<u.force> forceVectorFrom(NodeIndex otherNodeId) const {
            return solution_->forceRepartition().forceVector(index_, otherNodeId);
        }

        [[nodiscard]]
        NodeIndex index() const {
            return index_;
        }

        [[nodiscard]]
        bool isFoundation() const {
            return structureNode().isFoundation;
        }

        [[nodiscard]]
        Real<u.mass> mass() const {
            return structureNode().mass();
        }

        [[nodiscard]]
        Real<u.force> netForceCoord() const {
            return stats().force();
        }

        [[nodiscard]]
        Vector3<u.force> netForceVector() const {
            return stats().force() * solution_->forceBalancer().normalizedG();
        }

        [[nodiscard]]
        Real<u.potential> potential() const {
            return solution_->basis().potentials()[index_];
        }

        [[nodiscard]]
        Real<u.one> relativeError() const {
            return stats().relativeError();
        }

        [[nodiscard]]
        Real<u.force> weight() const {
            return info().weight;
        }

        [[nodiscard]]
        Vector3<u.force> weightVector() const {
            return info().weight * solution_->forceBalancer().normalizedG();
        }

        [[nodiscard]]
        bool operator==(NodeReference const&) const = default;
    private:
        [[nodiscard]]
        NodeInfo const& info() const {
            return solution_->forceBalancer().nodeInfos()[index_];
        }

        [[nodiscard]]
        StructureNode const& structureNode() const {
            return solution_->basis().structure().nodes()[index_];
        }

        [[nodiscard]]
        NodeStats stats() const {
            return solution_->forceRepartition().statsOf(index_);
        }

        SolutionData const* solution_;
        NodeIndex index_;
    };
}
