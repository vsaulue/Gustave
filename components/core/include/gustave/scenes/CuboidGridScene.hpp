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

#include <memory>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/math3d/BasicDirection.hpp>
#include <gustave/scenes/cuboidGridScene/detail/SceneData.hpp>
#include <gustave/scenes/cuboidGridScene/detail/SceneUpdater.hpp>
#include <gustave/scenes/cuboidGridScene/detail/StructureData.hpp>
#include <gustave/scenes/cuboidGridScene/Blocks.hpp>
#include <gustave/scenes/cuboidGridScene/Contacts.hpp>
#include <gustave/scenes/cuboidGridScene/Links.hpp>
#include <gustave/scenes/cuboidGridScene/Structures.hpp>
#include <gustave/scenes/cuboidGridScene/Transaction.hpp>
#include <gustave/scenes/cuboidGridScene/TransactionResult.hpp>
#include <gustave/utils/NoInit.hpp>

namespace gustave::scenes {
    template<cfg::cLibConfig auto libCfg>
    class CuboidGridScene {
    private:
        static constexpr auto u = cfg::units(libCfg);

        using SceneData = cuboidGridScene::detail::SceneData<libCfg>;
        using SceneUpdater = cuboidGridScene::detail::SceneUpdater<libCfg>;
        using StructureData = cuboidGridScene::detail::StructureData<libCfg>;

        template<cfg::cUnitOf<libCfg> auto unit>
        using Real = cfg::Real<libCfg, unit>;

        template<cfg::cUnitOf<libCfg> auto unit>
        using Vector3 = cfg::Vector3<libCfg, unit>;
    public:
        using Blocks = cuboidGridScene::Blocks<libCfg>;
        using Contacts = cuboidGridScene::Contacts<libCfg>;
        using Links = cuboidGridScene::Links<libCfg>;
        using Structures = cuboidGridScene::Structures<libCfg>;
        using Transaction = cuboidGridScene::Transaction<libCfg>;
        using TransactionResult = cuboidGridScene::TransactionResult<libCfg>;

        using BlockIndex = typename Blocks::BlockIndex;
        using BlockReference = typename Blocks::BlockReference;
        using ContactIndex = typename Contacts::ContactIndex;
        using ContactReference = typename Contacts::ContactReference;
        using Direction = typename Contacts::ContactIndex::Direction;
        using StructureReference = typename Structures::StructureReference;


        [[nodiscard]]
        explicit CuboidGridScene(Vector3<u.length> const& blockSize)
            : data_{ blockSize }
        {}

        CuboidGridScene(CuboidGridScene const&) = delete;
        CuboidGridScene& operator=(CuboidGridScene const&) = delete;

        TransactionResult modify(Transaction const& transaction) {
            auto upResult = SceneUpdater{ data_ }.runTransaction(transaction);
            return TransactionResult{ asReferences(std::move(upResult.newStructures)), asReferences(std::move(upResult.removedStructures)) };
        }

        [[nodiscard]]
        Blocks blocks() const {
            return Blocks{ data_ };
        }

        [[nodiscard]]
        Vector3<u.length> const& blockSize() const {
            return data_.blocks.blockSize();
        }

        [[nodiscard]]
        Contacts contacts() const {
            return Contacts{ data_ };
        }

        [[nodiscard]]
        Real<u.area> contactAreaAlong(Direction direction) const {
            return data_.blocks.contactAreaAlong(direction);
        }

        [[nodiscard]]
        Links links() const {
            return Links{ data_ };
        }

        [[nodiscard]]
        Structures structures() const {
            return Structures{ data_ };
        }

        [[nodiscard]]
        Real<u.length> thicknessAlong(Direction direction) const {
            return data_.blocks.thicknessAlong(direction);
        }
    private:
        [[nodiscard]]
        std::vector<StructureReference> asReferences(std::vector<std::shared_ptr<StructureData const>>&& structures) {
            std::vector<StructureReference> result;
            result.reserve(structures.size());
            for (auto&& structurePtr : structures) {
                result.emplace_back(std::move(structurePtr));
            }
            return result;
        }

        SceneData data_;
    };
}
