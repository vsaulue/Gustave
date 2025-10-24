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

#include <memory>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/core/scenes/cSceneUserData.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/SceneData.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/SceneUpdater.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/StructureData.hpp>
#include <gustave/core/scenes/cuboidGridScene/Blocks.hpp>
#include <gustave/core/scenes/cuboidGridScene/Contacts.hpp>
#include <gustave/core/scenes/cuboidGridScene/Links.hpp>
#include <gustave/core/scenes/cuboidGridScene/Structures.hpp>
#include <gustave/core/scenes/cuboidGridScene/Transaction.hpp>
#include <gustave/core/scenes/cuboidGridScene/TransactionResult.hpp>
#include <gustave/utils/NoInit.hpp>

namespace gustave::core::scenes {
    template<cfg::cLibConfig auto libCfg, cSceneUserData UserData_ = void>
    class CuboidGridScene {
    private:
        static constexpr auto u = cfg::units(libCfg);

        using SceneData = cuboidGridScene::detail::SceneData<libCfg, UserData_>;
        using SceneUpdater = cuboidGridScene::detail::SceneUpdater<libCfg, UserData_>;
        using StructureData = cuboidGridScene::detail::StructureData<libCfg, UserData_>;

        template<cfg::cUnitOf<libCfg> auto unit>
        using Real = cfg::Real<libCfg, unit>;

        template<cfg::cUnitOf<libCfg> auto unit>
        using Vector3 = cfg::Vector3<libCfg, unit>;
    public:
        using Blocks = cuboidGridScene::Blocks<libCfg, UserData_>;
        using Contacts = cuboidGridScene::Contacts<libCfg, UserData_>;
        using Links = cuboidGridScene::Links<libCfg, UserData_>;
        using Structures = cuboidGridScene::Structures<libCfg, UserData_>;
        using Transaction = cuboidGridScene::Transaction<libCfg>;
        using TransactionResult = cuboidGridScene::TransactionResult<libCfg>;

        using BlockIndex = typename Blocks::BlockIndex;
        using BlockReference = typename Blocks::BlockReference;
        using ContactIndex = typename Contacts::ContactIndex;
        using ContactReference = typename Contacts::ContactReference;
        using Direction = typename Contacts::ContactIndex::Direction;
        using StructureIndex = Structures::StructureIndex;
        using StructureReference = typename Structures::StructureReference;


        [[nodiscard]]
        explicit CuboidGridScene(Vector3<u.length> const& blockSize)
            : data_{ blockSize }
        {}

        CuboidGridScene(CuboidGridScene const&) = delete;
        CuboidGridScene& operator=(CuboidGridScene const&) = delete;

        [[nodiscard]]
        CuboidGridScene(CuboidGridScene&&) = default;

        CuboidGridScene& operator=(CuboidGridScene&&) = default;

        TransactionResult modify(Transaction const& transaction) {
            return SceneUpdater{ data_ }.runTransaction(transaction);
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
        SceneData data_;
    };
}
