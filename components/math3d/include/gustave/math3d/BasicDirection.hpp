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

#include <functional>
#include <ostream>
#include <stdexcept>

namespace gustave::math3d {
    class BasicDirection {
    public:
        enum class Id { plusX, minusX, plusY, minusY, plusZ, minusZ };

        [[nodiscard]]
        constexpr BasicDirection(Id id)
            : id_{ id }
        {
            if (static_cast<int>(id_) >= 6) {
                throw std::invalid_argument(invalidValueMsg());
            }
        }

        [[nodiscard]]
        static constexpr BasicDirection plusX() { return Id::plusX; }
        [[nodiscard]]
        static constexpr BasicDirection minusX() { return Id::minusX; }
        [[nodiscard]]
        static constexpr BasicDirection plusY() { return Id::plusY; }
        [[nodiscard]]
        static constexpr BasicDirection minusY() { return Id::minusY; }
        [[nodiscard]]
        static constexpr BasicDirection plusZ() { return Id::plusZ; }
        [[nodiscard]]
        static constexpr BasicDirection minusZ() { return Id::minusZ; }

        [[nodiscard]]
        constexpr Id id() const {
            return id_;
        }

        [[nodiscard]]
        std::string invalidValueMsg() const {
            std::string msg = "Invalid BasicDirection : ";
            msg += static_cast<int>(id_);
            msg += '.';
            return msg;
        }

        [[nodiscard]]
        constexpr BasicDirection opposite() const {
            switch (id_) {
            case Id::plusX:
                return minusX();
            case Id::minusX:
                return plusX();
            case Id::plusY:
                return minusY();
            case Id::minusY:
                return plusY();
            case Id::plusZ:
                return minusZ();
            case Id::minusZ:
                return plusZ();
            };
            throw std::invalid_argument(invalidValueMsg());
        };

        constexpr friend std::ostream& operator<<(std::ostream& stream, BasicDirection direction) {
            switch (direction.id_) {
            case Id::plusX:
                stream << "plusX";
                break;
            case Id::minusX:
                stream << "minusX";
                break;
            case Id::plusY:
                stream << "plusY";
                break;
            case Id::minusY:
                stream << "minusY";
                break;
            case Id::plusZ:
                stream << "plusZ";
                break;
            case Id::minusZ:
                stream << "minusZ";
                break;
            }
            return stream;
        }

        [[nodiscard]]
        constexpr bool operator==(BasicDirection const& other) const = default;

        struct Hasher {
        public:
            [[nodiscard]]
            std::size_t operator()(BasicDirection direction) const {
                return std::hash<Id>{}(direction.id_);
            }
        };
    private:
        Id id_;
    };
}

template<>
struct std::hash<gustave::math3d::BasicDirection> : gustave::math3d::BasicDirection::Hasher {};
