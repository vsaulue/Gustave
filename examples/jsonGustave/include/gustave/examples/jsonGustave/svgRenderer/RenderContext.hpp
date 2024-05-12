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

#include <svgwrite/writer.hpp>

#include <gustave/core/cGustave.hpp>
#include <gustave/examples/jsonGustave/svgRenderer/Config.hpp>
#include <gustave/examples/jsonGustave/JsonWorld.hpp>

namespace gustave::examples::jsonGustave::svgRenderer {
    template<core::cGustave G>
    class RenderContext {
    public:
        using Float = typename G::RealRep;

        using Config = svgRenderer::Config<Float>;
        using JsonWorld = jsonGustave::JsonWorld<G>;
        using SyncWorld = typename G::Worlds::SyncWorld;
    private:
        using GridCoord = typename SyncWorld::BlockIndex::Coord;

        struct BlockCoordinates {
            Float xMin;
            Float yMin;
            Float width;
            Float height;

            [[nodiscard]]
            Float xMax() const {
                return xMin + width;
            }

            [[nodiscard]]
            Float xMean() const {
                return xMin + width / 2;
            }

            [[nodiscard]]
            Float yMax() const {
                return yMin + height;
            }

            [[nodiscard]]
            Float yMean() const {
                return yMin + height / 2;
            }
        };

        struct Point {
            Float x;
            Float y;
        };

        struct WorldLimits {
            using Limits = std::numeric_limits<GridCoord>;

            WorldLimits()
                : xMax{ Limits::min() }
                , xMin{ Limits::max() }
                , yMax{ Limits::min() }
                , yMin{ Limits::max() }
            {}

            explicit WorldLimits(SyncWorld const& world)
                : WorldLimits{}
            {
                if (world.blocks().size() == 0) {
                    xMax = 0;
                    xMin = 0;
                    yMax = 0;
                    yMin = 0;
                }
                else {
                    for (auto const& block : world.blocks()) {
                        auto const& id = block.index();
                        xMax = std::max(xMax, id.x);
                        xMin = std::min(xMin, id.x);
                        yMax = std::max(yMax, id.y);
                        yMin = std::min(yMin, id.y);
                    }
                }
            }

            GridCoord xMax;
            GridCoord xMin;
            GridCoord yMax;
            GridCoord yMin;
        };
    public:
        [[nodiscard]]
        explicit RenderContext(JsonWorld const& world, std::ostream& output, Config const& config)
            : jsonWorld_{ world }
            , config_{ config }
            , output_{ output }
            , limits_{ world.syncWorld() }
            , writer_{ output }
            , groupCount_{ 0 }
            , finalized_{ false }
        {
            for (auto const& block : world.syncWorld().blocks()) {
                if (block.index().z != 0) {
                    std::stringstream msg;
                    msg << "SvgRenderer doesn't support 3d scenes: all blocks must hase 'index.z == 0' (passed: " << block.index() << ").";
                    throw std::invalid_argument(msg.str());
                }
            }
            writer_.start_svg(worldFrameWidth(), worldFrameHeight());
        }

        RenderContext(RenderContext const&) = delete;
        RenderContext(RenderContext&&) = delete;

        void drawBlock(SyncWorld::BlockReference const& block, svgw::attr_list attrs = { {} }) {
            throwIfFinalized();
            auto const coords = blockCoordinates(block);
            writer_.rect(coords.xMin, coords.yMin, coords.width, coords.height, attrs);
        }

        void drawContactArrow(SyncWorld::ContactReference const& contact, Float lengthRatio, svgw::attr_list attrs = { {} }) {
            throwIfFinalized();
            using Direction = SyncWorld::ContactIndex::Direction;
            auto const blockCoords = blockCoordinates(contact.localBlock());
            auto const direction = contact.index().direction();
            Float const triangleFactor = config_.arrowTriangleFactor();
            Float const minDim = std::min(blockCoords.height, blockCoords.width);
            Float const triangleSize = minDim * triangleFactor;
            Float const lineWidth = triangleSize * config_.arrowLineFactor();
            Float const lineLength = minDim * (0.5f - triangleFactor) * lengthRatio;
            std::stringstream path;
            switch (direction.id()) {
            case Direction::Id::minusX: {
                Float const xTriangleBase = blockCoords.xMin + lineLength;
                Float const yMean = blockCoords.yMean();
                path << 'M' << xTriangleBase << ' ' << yMean - triangleSize;
                path << " L" << xTriangleBase + triangleSize << ' ' << yMean;
                path << " L" << xTriangleBase << ' ' << yMean + triangleSize;
                path << " L" << xTriangleBase << ' ' << yMean + lineWidth;
                path << " L" << blockCoords.xMin << ' ' << yMean + lineWidth;
                path << " L" << blockCoords.xMin << ' ' << yMean - lineWidth;
                path << " L" << xTriangleBase << ' ' << yMean - lineWidth;
                break;
            }
            case Direction::Id::plusX: {
                Float const xMax = blockCoords.xMax();
                Float const xTriangleBase = xMax - lineLength;
                Float const yMean = blockCoords.yMean();
                path << 'M' << xTriangleBase << ' ' << yMean - triangleSize;
                path << " L" << xTriangleBase - triangleSize << ' ' << yMean;
                path << " L" << xTriangleBase << ' ' << yMean + triangleSize;
                path << " L" << xTriangleBase << ' ' << yMean + lineWidth;
                path << " L" << xMax << ' ' << yMean + lineWidth;
                path << " L" << xMax << ' ' << yMean - lineWidth;
                path << " L" << xTriangleBase << ' ' << yMean - lineWidth;
                break;
            }
            case Direction::Id::minusY: {
                Float const xMean = blockCoords.xMean();
                Float const yMax = blockCoords.yMax();
                Float const yTriangleBase = yMax - lineLength;
                path << 'M' << xMean - triangleSize << ' ' << yTriangleBase;
                path << " L" << xMean << ' ' << yTriangleBase - triangleSize;
                path << " L" << xMean + triangleSize << ' ' << yTriangleBase;
                path << " L" << xMean + lineWidth << ' ' << yTriangleBase;
                path << " L" << xMean + lineWidth << ' ' << yMax;
                path << " L" << xMean - lineWidth << ' ' << yMax;
                path << " L" << xMean - lineWidth << ' ' << yTriangleBase;
                break;
            }
            case Direction::Id::plusY: {
                Float const xMean = blockCoords.xMean();
                Float const yTriangleBase = blockCoords.yMin + lineLength;
                path << 'M' << xMean - triangleSize << ' ' << yTriangleBase;
                path << " L" << xMean << ' ' << yTriangleBase + triangleSize;
                path << " L" << xMean + triangleSize << ' ' << yTriangleBase;
                path << " L" << xMean + lineWidth << ' ' << yTriangleBase;
                path << " L" << xMean + lineWidth << ' ' << blockCoords.yMin;
                path << " L" << xMean - lineWidth << ' ' << blockCoords.yMin;
                path << " L" << xMean - lineWidth << ' ' << yTriangleBase;
                break;
            }
            case Direction::Id::minusZ:
            case Direction::Id::plusZ: {
                std::stringstream msg;
                msg << "Unsupported direction for SvgRender:" << direction;
                throw std::invalid_argument(msg.str());
            }
            }
            path << " Z";
            writer_.path(path.str(), attrs);
        }

        void drawWorldFrame(svgw::attr_list attrs = { {} }) {
            throwIfFinalized();
            writer_.rect(0.f, 0.f, worldFrameWidth(), worldFrameHeight(), attrs);
        }

        void endGroup() {
            throwIfFinalized();
            if (groupCount_ == 0) {
                throw std::logic_error("Invalid endGroup(): no group to close.");
            }
            --groupCount_;
            writer_.end_g();
        }

        void finalize() {
            throwIfFinalized();
            if (groupCount_ > 0) {
                throw std::logic_error("Invalid finalize(): all groups aren't closed.");
            }
            finalized_ = true;
            writer_.end_svg();
            output_ << '\n';
        }

        void hatchBlock(SyncWorld::BlockReference const& block, svgw::attr_list attrs = { {} }) {
            throwIfFinalized();
            auto coords = blockCoordinates(block);
            writer_.start_g(attrs);
            writer_.line(coords.xMean(), coords.yMin, coords.xMax(), coords.yMean());
            writer_.line(coords.xMin, coords.yMin, coords.xMax(), coords.yMax());
            writer_.line(coords.xMin, coords.yMean(), coords.xMean(), coords.yMax());
            writer_.end_g();
        }

        void startGroup(svgw::attr_list attrs) {
            throwIfFinalized();
            writer_.start_g(attrs);
            ++groupCount_;
        }

        [[nodiscard]]
        JsonWorld const& world() const {
            return jsonWorld_;
        }
    private:
        [[nodiscard]]
        BlockCoordinates blockCoordinates(SyncWorld::BlockReference const& block) const {
            Float const spaceRes = config_.spaceRes();
            auto const& id = block.index();
            auto const& blockSize = block.blockSize();
            Float const x = spaceRes * blockSize.x().value() * (0.5f + Float(id.x - limits_.xMin));
            Float const y = spaceRes * blockSize.y().value() * (0.5f + Float(limits_.yMax - id.y));
            return { x, y, spaceRes * blockSize.x().value(), spaceRes * blockSize.y().value() };
        }

        void throwIfFinalized() const {
            if (finalized_) {
                throw std::logic_error("SvgRenderContext finalized twice.");
            }
        }

        [[nodiscard]]
        Float worldFrameHeight() const {
            auto const& blockSize = jsonWorld_.syncWorld().scene().blockSize();
            return config_.spaceRes() * blockSize.y().value() * Float(2 + limits_.yMax - limits_.yMin);
        }

        [[nodiscard]]
        Float worldFrameWidth() const {
            auto const& blockSize = jsonWorld_.syncWorld().scene().blockSize();
            return config_.spaceRes() * blockSize.x().value() * Float(2 + limits_.xMax - limits_.xMin);
        }

        JsonWorld const& jsonWorld_;
        Config const& config_;
        std::ostream& output_;
        WorldLimits limits_;
        svgw::writer writer_;
        unsigned groupCount_;
        bool finalized_;
    };
}
