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

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iterator>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include <gustave/meta/Meta.hpp>

namespace gustave::utils {
    // A poor man's version of std::inplace_vector (C++26)
    template<typename Value_, std::size_t capacity_>
    class InplaceVector {
    private:
    public:
        using Value = Value_;

        using Iterator = Value*;
        using ConstIterator = Value const*;

        [[nodiscard]]
        static constexpr std::size_t capacity() {
            return capacity_;
        }

        [[nodiscard]]
        static constexpr bool isConstCopyable() {
            return requires { Value{ std::declval<Value const&>() }; };
        }

        [[nodiscard]]
        static constexpr bool isMoveCopyable() {
            return requires { Value{ std::declval<Value&&>() }; };
        }

        [[nodiscard]]
        static constexpr bool isNonConstCopyable() {
            return requires { Value{ std::declval<Value&>() }; };
        }

        [[nodiscard]]
        constexpr InplaceVector()
            : size_{ 0 }
        {}

        [[nodiscard]]
        constexpr InplaceVector(InplaceVector&& other)
            requires (isMoveCopyable())
        : InplaceVector()
        {
            *this = std::move(other);
        }

        [[nodiscard]]
        constexpr InplaceVector(InplaceVector& other)
            requires (!isConstCopyable() && isNonConstCopyable())
        : InplaceVector()
        {
            *this = other;
        }

        [[nodiscard]]
        constexpr InplaceVector(InplaceVector const& other)
            requires (isConstCopyable())
        : InplaceVector()
        {
            *this = other;
        }

        constexpr ~InplaceVector() {
            clear();
        }

        constexpr InplaceVector& operator=(InplaceVector&& other)
            requires (isMoveCopyable())
        {
            if (this != &other) {
                clear();
                for (auto& otherValue : other) {
                    emplaceBack(std::move(otherValue));
                    std::destroy_at(&otherValue);
                }
                other.size_ = 0;
            }
            return *this;
        }

        constexpr InplaceVector& operator=(InplaceVector& other)
            requires (!isConstCopyable() && isNonConstCopyable())
        {
            return doCopyAssign(other);
        }

        constexpr InplaceVector& operator=(InplaceVector const& other)
            requires (isConstCopyable())
        {
            return doCopyAssign(other);
        }

        [[nodiscard]]
        Value& at(std::size_t index) {
            return doAt(*this, index);
        }

        [[nodiscard]]
        Value const& at(std::size_t index) const {
            return doAt(*this, index);
        }

        [[nodiscard]]
        constexpr Iterator begin() {
            return addrOf(0);
        }

        [[nodiscard]]
        constexpr ConstIterator begin() const {
            return addrOf(0);
        }

        constexpr void clear() {
            for (auto& value : *this) {
                std::destroy_at(std::addressof(value));
            }
            size_ = 0;
        }

        template<typename... CtorArgs>
        constexpr Value& emplaceBack(CtorArgs&&... ctorArgs) {
            if (size_ >= capacity_) {
                throw maxCapacityReachedError();
            }
            auto result = std::construct_at(addrOf(size_), std::forward<CtorArgs>(ctorArgs)...);
            ++size_;
            return *result;
        }

        [[nodiscard]]
        constexpr ConstIterator end() const {
            return addrOf(size_);
        }

        [[nodiscard]]
        constexpr Value& operator[](std::size_t index)& {
            assert(index < size_);
            return *addrOf(index);
        }

        [[nodiscard]]
        constexpr Value const& operator[](std::size_t index) const& {
            assert(index < size_);
            return *addrOf(index);
        }

        constexpr Value& pushBack(Value const& newValue)
            requires (isConstCopyable())
        {
            return emplaceBack(newValue);
        }

        constexpr Value& pushBack(Value& newValue)
            requires (!isConstCopyable() && isNonConstCopyable())
        {
            return emplaceBack(newValue);
        }

        constexpr Value& pushBack(Value&& newValue)
            requires (isMoveCopyable())
        {
            return emplaceBack(std::move(newValue));
        }

        [[nodiscard]]
        constexpr std::size_t size() const {
            return size_;
        }
    private:
        [[nodiscard]]
        constexpr Value* addrOf(std::size_t index) {
            return doAddrOf(*this, index);
        }

        [[nodiscard]]
        constexpr Value const* addrOf(std::size_t index) const {
            return doAddrOf(*this, index);
        }

        [[nodiscard]]
        static constexpr auto doAt(meta::cCvRefOf<InplaceVector> auto&& self, std::size_t index) -> decltype(self.at(index)) {
            if (index >= self.size_) {
                throw self.invalidIndexError(index);
            }
            return *doAddrOf(self, index);
        }

        [[nodiscard]]
        static constexpr auto doAddrOf(meta::cCvRefOf<InplaceVector> auto&& self, std::size_t index) {
            using Result = decltype(self.addrOf(index));
            return reinterpret_cast<Result>(self.memory_) + index;
        }

        constexpr InplaceVector& doCopyAssign(meta::cCvRefOf<InplaceVector> auto&& rhs) {
            if (this != &rhs) {
                clear();
                for (auto&& value : rhs) {
                    emplaceBack(value);
                }
            }
            return *this;
        }

        [[nodiscard]]
        std::out_of_range invalidIndexError(std::size_t index) const {
            std::stringstream msg;
            msg << "InplaceVector: index (" << index << ") is out of range (size = " << index << ')';
            return std::out_of_range(msg.str());
        }

        [[nodiscard]]
        std::length_error maxCapacityReachedError() const {
            std::stringstream msg;
            msg << "InplaceVector: maximum capacity reached (size = " << size_ << ')';
            return std::length_error(msg.str());
        }

        alignas(Value) char memory_[capacity_ * sizeof(Value)];
        std::size_t size_;
    };
}
