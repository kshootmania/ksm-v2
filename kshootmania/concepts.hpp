#pragma once

template <typename T>
concept Int32 = std::is_same_v<T, int32>;

template <typename T>
concept Int32Enum = true;// std::is_enum_v<T>&& std::is_same_v<std::underlying_type<T>, int32>;
