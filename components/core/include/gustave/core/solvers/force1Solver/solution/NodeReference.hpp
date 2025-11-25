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
#include <gustave/core/solvers/force1Solver/solution/ContactReference.hpp>
#include <gustave/core/solvers/Structure.hpp>
#include <gustave/utils/ForwardIterator.hpp>
#include <gustave/utils/NoInit.hpp>

namespace gustave::core::solvers::force1Solver::solution {
    template<cfg::cLibConfig auto libCfg>
    class ContactReference;

    template<cfg::cLibConfig auto libCfg>
    class NodeReference {
    private:
        using SolutionData = detail::SolutionData<libCfg>;
        using Structure = solvers::Structure<libCfg>;

        using F1LocalContacts = SolutionData::F1Structure::LocalContacts;
        using F1Node = SolutionData::F1Structure::F1Node;
        using LinkIndex = cfg::LinkIndex<libCfg>;
        using NodeStats = SolutionData::ForceRepartition::NodeStats;
        using StructureNode = Structure::Node;
        using StructureLink = Structure::Link;
        using StructureLinks = Structure::Links;

        template<cfg::cUnitOf<libCfg> auto unit>
        using Real = cfg::Real<libCfg, unit>;

        template<cfg::cUnitOf<libCfg> auto unit>
        using Vector3 = cfg::Vector3<libCfg, unit>;

        static constexpr auto u = cfg::units(libCfg);
    public:
        using ContactReference = solution::ContactReference<libCfg>;

        using ContactIndex = Structure::ContactIndex;
        using NodeIndex = Structure::NodeIndex;

        class Contacts {
        private:
            class Enumerator {
            private:
                using DataIterator = F1LocalContacts::iterator;
            public:
                [[nodiscard]]
                Enumerator()
                    : contacts_{ nullptr }
                    , dataIterator_{}
                    , value_{ utils::NO_INIT }
                {}

                [[nodiscard]]
                explicit Enumerator(Contacts const& contacts)
                    : contacts_{ &contacts }
                    , dataIterator_{ contacts.fLocalContacts_.begin() }
                    , value_{ utils::NO_INIT }
                {
                    updateValue();
                }

                [[nodiscard]]
                bool isEnd() const {
                    return dataIterator_ == contacts_->fLocalContacts_.end();
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
                        StructureLink const& link = (*contacts_->links_)[linkId];
                        bool isOnLocalNode = (link.localNodeId() == contacts_->node_.index_);
                        value_ = ContactReference{ *contacts_->node_.solution_, ContactIndex{ linkId, isOnLocalNode } };
                    }
                }

                Contacts const* contacts_;
                DataIterator dataIterator_;
                ContactReference value_;
            };
        public:
            using Iterator = utils::ForwardIterator<Enumerator>;

            [[nodiscard]]
            explicit Contacts(NodeReference const& node)
                : node_{ node }
                , links_{ &node_.solution_->basis().structure().links() }
                , fLocalContacts_{ node.fLocalContacts() }
            {}

            [[nodiscard]]
            ContactReference at(ContactIndex const& contactIndex) const {
                StructureLink const& link = links_->at(contactIndex.linkIndex);
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
                return Iterator{ *this };
            }

            [[nodiscard]]
            constexpr std::default_sentinel_t end() const {
                return {};
            }

            [[nodiscard]]
            std::size_t size() const {
                return fLocalContacts_.size();
            }
        private:
            NodeReference node_;
            StructureLinks const* links_;
            F1LocalContacts fLocalContacts_;
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
            return stats().force() * solution_->fStructure().normalizedG();
        }

        [[nodiscard]]
        Real<u.potential> potential() const {
            return solution_->basis().potentials()[index_];
        }

        [[nodiscard]]
        Real<u.one> relativeError() const {
            return solution_->forceRepartition().relativeErrorOf(index_);
        }

        [[nodiscard]]
        Real<u.force> weight() const {
            return fNode().weight;
        }

        [[nodiscard]]
        Vector3<u.force> weightVector() const {
            return fNode().weight * solution_->fStructure().normalizedG();
        }

        [[nodiscard]]
        bool operator==(NodeReference const&) const = default;
    private:
        [[nodiscard]]
        F1Node const& fNode() const {
            return solution_->fStructure().fNodes()[index_];
        }

        [[nodiscard]]
        F1LocalContacts fLocalContacts() const {
            return solution_->fStructure().fContactsOf(index_);
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
