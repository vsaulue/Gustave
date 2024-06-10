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

#include <gustave/core/cGustave.hpp>
#include <gustave/examples/jsonGustave/svgRenderer/phases/Phase.hpp>
#include <gustave/examples/jsonGustave/Color.hpp>
#include <gustave/examples/jsonGustave/Json.hpp>

namespace gustave::examples::jsonGustave::svgRenderer::phases {
    template<core::cGustave G>
    class BlockTypePhase : public Phase<G> {
    public:
        using Float = typename G::RealRep;

        using Color = jsonGustave::Color<Float>;
        using Config = typename Phase<G>::Config;
        using JsonWorld = typename Phase<G>::JsonWorld;
        using SvgCanvasContext = typename Phase<G>::SvgCanvasContext;
        using SvgDims = typename Phase<G>::SvgDims;
        using SvgPhaseCanvas = typename Phase<G>::SvgPhaseCanvas;
        using PhaseContext = typename Phase<G>::PhaseContext;

        class BlockTypePhaseContext : public PhaseContext {
        public:
            [[nodiscard]]
            explicit BlockTypePhaseContext(SvgCanvasContext const& ctx, BlockTypePhase const& phase)
                : PhaseContext{ ctx }
                , phase_{ phase }
            {
                this->setLegendDims(generateLegendDims());
            }

            void render(SvgPhaseCanvas& canvas) const override {
                renderWorldBlocks(canvas);
                renderLegend(canvas);
            }
        private:
            [[nodiscard]]
            std::string getHatchColorCode() const {
                return phase_.foundationHatchColor_.svgCode();
            }

            [[nodiscard]]
            static constexpr std::string_view legendTitle() {
                return "Block types:";
            }

            [[nodiscard]]
            static constexpr std::string_view foundationCaption() {
                return "Foundation";
            }

            [[nodiscard]]
            Float legendItemHeight() const {
                return std::max(this->svgBlockHeight(), this->config().legendTextSize());
            }

            void renderLegend(SvgPhaseCanvas& canvas) const {
                auto const textColorCode = this->config().legendTextColor().svgCode();
                auto const blockBorderColorCode = phase_.blockBorderColor_.svgCode();
                auto const titleHeight = this->config().legendTitleSize();
                auto const space = this->config().legendSpace();
                canvas.drawLegendText(0.f, titleHeight, legendTitle(), {
                    {"font-size", titleHeight},
                    {"fill", textColorCode},
                });
                std::array<svgw::attr, 2> const textAttrs = { {
                    {"font-size", this->config().legendTextSize()},
                    {"fill", textColorCode},
                } };
                Float yBlock = titleHeight + space + 0.5f * (legendItemHeight() - this->svgBlockHeight());
                Float yText = titleHeight + space + 0.5f * (legendItemHeight() + this->config().legendTextSize());
                Float const xText = this->svgBlockWidth() + this->config().legendSpace();
                Float const lineHeight = legendItemHeight() + this->config().legendSpace();
                std::array<svgw::attr, 3> blockAttrs = { {
                    {"stroke", blockBorderColorCode},
                    {"stroke-width", phase_.blockBorderWidth_},
                    {"fill", "UNDEFINED"},
                } };
                for (auto const& blockType : this->jsonWorld().blockTypes()) {
                    auto const svgColor = blockType.color().svgCode();
                    blockAttrs[2].value = svgColor;
                    canvas.drawLegendBlock(0.f, yBlock, blockAttrs);
                    canvas.drawLegendText(xText, yText, blockType.name(), textAttrs);
                    yBlock += lineHeight;
                    yText += lineHeight;
                }
                std::array<svgw::attr, 3> const foundationAttrs = { {
                    {"stroke", blockBorderColorCode},
                    {"stroke-width", phase_.blockBorderWidth_},
                    {"fill-opacity",0.f},
                } };
                canvas.drawLegendBlock(0.f, yBlock, foundationAttrs);
                canvas.hatchLegendBlock(0.f, yBlock, { {"stroke", getHatchColorCode() },{"stroke-width", phase_.foundationHatchWidth_} });
                canvas.drawLegendText(xText, yText, foundationCaption(), textAttrs);
            }

            void renderWorldBlocks(SvgPhaseCanvas& canvas) const {
                canvas.startGroup({ {"stroke", phase_.blockBorderColor_.svgCode()}, {"stroke-width", phase_.blockBorderWidth_} });
                auto const hatchColorCode = getHatchColorCode();
                for (auto const& block : this->syncWorld().blocks()) {
                    auto const svgColor = this->jsonWorld().blockTypeOf().at(block.index())->color().svgCode();
                    canvas.drawWorldBlock(block, { {"fill", svgColor } });
                    if (block.isFoundation()) {
                        canvas.hatchWorldBlock(block, { {"stroke", hatchColorCode },{"stroke-width", phase_.foundationHatchWidth_} });
                    }
                }
                canvas.endGroup();
            }

            [[nodiscard]]
            SvgDims generateLegendDims() const {
                auto const space = this->config().legendSpace();
                auto const textSize = this->config().legendTextSize();
                auto const matSize = this->jsonWorld().blockTypes().size();
                Float height = this->config().legendTitleSize();
                height += Float(matSize + 1) * (legendItemHeight() + space);
                Float blockNameWidth = this->textWidth(foundationCaption(), textSize);
                for (auto const& blockType : this->jsonWorld().blockTypes()) {
                    blockNameWidth = std::max(blockNameWidth, this->textWidth(blockType.name(), textSize));
                }
                Float titleWidth = this->textWidth(legendTitle(), this->config().legendTitleSize());
                Float const width = 2 * space + std::max(titleWidth, blockNameWidth + this->svgBlockWidth() + space);
                return { width, height };
            }

            BlockTypePhase const& phase_;
        };

        [[nodiscard]]
        BlockTypePhase()
            : blockBorderColor_{ 0.f, 0.f, 0.f }
            , foundationHatchColor_{ 0.f, 0.f, 0.f }
            , blockBorderWidth_{ 1.f }
            , foundationHatchWidth_{ 2.f }
        {}

        [[nodiscard]]
        explicit BlockTypePhase(Color const& blockBorderColor, Float blockBorderWidth, Color const& foundationHatchColor, Float foundationHatchWidth)
            : blockBorderColor_{ blockBorderColor }
            , foundationHatchColor_{ foundationHatchColor }
            , blockBorderWidth_{ blockBorderWidth }
            , foundationHatchWidth_{ foundationHatchWidth }
        {
            if (blockBorderWidth_ < 0.f) {
                throw Phase<G>::invalidWidthError("blockBorderWidth", blockBorderWidth_);
            }
            if (foundationHatchWidth_ < 0.f) {
                throw Phase<G>::invalidWidthError("foundationHatchWidth", foundationHatchWidth_);
            }
        }

        [[nodiscard]]
        std::unique_ptr<PhaseContext> makeContext(SvgCanvasContext const& canvasCtx) const override {
            return std::make_unique<BlockTypePhaseContext>(canvasCtx, *this);
        }
    private:
        Color blockBorderColor_;
        Color foundationHatchColor_;
        Float blockBorderWidth_;
        Float foundationHatchWidth_;
    };
}

template<gustave::core::cGustave G>
struct nlohmann::adl_serializer<gustave::examples::jsonGustave::svgRenderer::phases::BlockTypePhase<G>> {
    using BlockTypePhase = gustave::examples::jsonGustave::svgRenderer::phases::BlockTypePhase<G>;

    using Color = typename BlockTypePhase::Color;
    using Float = typename BlockTypePhase::Float;

    [[nodiscard]]
    static BlockTypePhase from_json(nlohmann::json const& json) {
        Color const blockBorderColor = json.at("blockBorderColor").get<Color>();
        Float const blockBorderWidth = json.at("blockBorderWidth").get<Float>();
        Color const foundationHatchColor = json.at("foundationHatchColor").get<Color>();
        Float const foundationHatchWidth = json.at("foundationHatchWidth").get<Float>();
        return BlockTypePhase{ blockBorderColor, blockBorderWidth, foundationHatchColor, foundationHatchWidth };
    }
};
