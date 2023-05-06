#ifndef MY_TUPLE_HPP
#define MY_TUPLE_HPP

#include <functional>
#include <type_traits>

namespace my {

  namespace details {

    template <typename... Ts>
    struct TupleData {};

    template <typename T, typename... Ts>
    struct TupleData<T, Ts...> {
      TupleData(T v, Ts... vs) : value(v), next(vs...) {}

      TupleData() = default;
      using value_t = T;
      using reference_type = value_t&;
      using const_reference = value_t const&;
      using next_t = TupleData<Ts...>;
      value_t value;
      next_t next;
    };

    template <class Tuple>
    struct IsTupleValid : std::false_type {};

    template <typename T, typename... Ts>
    struct IsTupleValid<TupleData<T, Ts...>> : std::true_type {};

    template <class Tuple>
    static constexpr bool IsTupleValid_v = IsTupleValid<Tuple>::value;

    template <class Tuple, std::size_t Index, std::size_t Current,
              typename AlwaysVoid>
    struct StaticGetterImpl {};

    template <std::size_t Index, std::size_t Current, typename T,
              typename... Ts>
    struct StaticGetterImpl<TupleData<T, Ts...>, Index, Current,
                            std::enable_if_t<(Current != Index)>> {
      using getter_t =
          StaticGetterImpl<TupleData<Ts...>, Index, Current + 1, void>;

      template <class Tuple>
      static auto& get(Tuple&& t) {
        static_assert(std::is_same_v<std::decay_t<Tuple>, TupleData<T, Ts...>>);
        return getter_t::get(t.next);
      }
    };

    template <std::size_t Index, std::size_t Current, typename T,
              typename... Ts>
    struct StaticGetterImpl<TupleData<T, Ts...>, Index, Current,
                            std::enable_if_t<(Current == Index)>> {
      template <class Tuple>
      static constexpr auto& get(Tuple&& t) {
        static_assert(std::is_same_v<std::decay_t<Tuple>, TupleData<T, Ts...>>);
        return t.value;
      }
    };

    template <std::size_t Index>
    struct StaticGetter {
      template <class Tuple>
      static constexpr auto& get(Tuple&& t) {
        using getter_t = StaticGetterImpl<std::decay_t<Tuple>, Index, 0, void>;
        return getter_t::get(t);
      }
    };

    template <bool Valid, typename Ret>
    struct Dispatcher {
      template <std::size_t Index, class Visitor, class... Tuples>
      static constexpr Ret case_(Visitor&&, Tuples&&...) {}

      template <std::size_t Base, std::size_t Size, class Visitor,
                class... Tuples>
      static constexpr Ret switch_(std::size_t, Visitor&&, Tuples&&...) {}
    };

    template <std::size_t Index, typename T>
    struct GetterType {
      using type = decltype(StaticGetter<Index>::get(std::declval<T>()));
    };

    template <typename Ret>
    struct Dispatcher<true, Ret> {
      template <std::size_t Index, class Visitor, class... Tuples>
      static constexpr Ret case_(Visitor&& vis, Tuples&&... t) {
        using return_t = decltype(std::invoke(
            std::declval<Visitor>(),
            std::declval<typename GetterType<Index, Tuples>::type>()...));
        using expected_t = Ret;
        static_assert(std::is_same_v<return_t, expected_t>);
        using getter_t = StaticGetter<Index>;
        return std::invoke(std::forward<Visitor>(vis), getter_t::get(t)...);
      }

      template <std::size_t Base, std::size_t Size, class Visitor,
                class... Tuples>
      static constexpr Ret switch_(std::size_t index, Visitor&& vis,
                                   Tuples&&... t) {
        switch (index) {
          case Base:
            return Dispatcher<(Base < Size), Ret>::template case_<Base>(
                std::forward<Visitor>(vis), t...);
          case Base + 1:
            return Dispatcher<(Base + 1 < Size), Ret>::template case_<Base + 1>(
                std::forward<Visitor>(vis), t...);
          case Base + 2:
            return Dispatcher<(Base + 2 < Size), Ret>::template case_<Base + 2>(
                std::forward<Visitor>(vis), t...);
          case Base + 3:
            return Dispatcher<(Base + 3 < Size), Ret>::template case_<Base + 3>(
                std::forward<Visitor>(vis), t...);
          default:
            return Dispatcher<(Base + 4 < Size), Ret>::template switch_<
                Base + 4, Size>(index, std::forward<Visitor>(vis), t...);
        }
      }
    };

    struct Error_Tuple_Out_Of_Bounds {};

  } // namespace details

  template <typename... Ts>
  class Tuple {
  public:
    using data_t = details::TupleData<Ts...>;

    explicit Tuple(Ts... ts) : data(ts...) {}

    Tuple() = default;

    template <std::size_t Index>
    constexpr auto& get() {
      using trueType = details::StaticGetter<Index>;
      using falseType = details::Error_Tuple_Out_Of_Bounds;
      using result = std::conditional_t<(Index < _size), trueType, falseType>;
      return result::get(data);
    }

    template <std::size_t Index>
    constexpr auto const& get() const {
      using trueType = details::StaticGetter<Index>;
      using falseType = details::Error_Tuple_Out_Of_Bounds;
      using result = std::conditional_t<(Index < _size), trueType, falseType>;
      return result::get(data);
    }

    constexpr std::size_t size() const { return _size; }

    template <class Visitor, class... Tuples>
    friend decltype(auto) visit(std::size_t index, Visitor&& v, Tuples&&... t);
  private:
    static constexpr std::size_t _size = sizeof...(Ts);
    data_t data;
  };

  template <class Tuple>
  struct tuple_size {};

  template <typename... Ts>
  struct tuple_size<Tuple<Ts...>>
      : std::integral_constant<std::size_t, sizeof...(Ts)> {};

  template <class Tuple>
  static constexpr bool tuple_size_v = tuple_size<Tuple>::value;

  namespace details {

    template <std::size_t Size, typename... Ts>
    struct CheckSize {
      static constexpr bool value = true;
    };

    template <std::size_t Size, typename T, typename... Ts>
    struct CheckSize<Size, T, Ts...> {
      static constexpr bool value =
          Size == tuple_size_v<T> && CheckSize<Size, Ts...>::value;
    };

    template <typename... T>
    struct GetSize {};

    template <>
    struct GetSize<> {
      static constexpr std::size_t size = 0;
    };

    template <typename T, typename... Ts>
    struct GetSize<T, Ts...> {
      static constexpr std::size_t size = tuple_size_v<T>;
    };

    template <typename T>
    struct TupleDataType {
      static constexpr bool is_const = std::is_const_v<T>;
      using tuple_t = std::decay_t<T>;
      using data_t = typename tuple_t::data_t;
      using const_reference = typename data_t::const_reference;
      using reference_type = typename data_t::reference_type;
      using type =
          std::conditional_t<is_const, const_reference, reference_type>;
    };

    template <class Visitor, typename... T>
    struct GetReturnType {};

    template <class Visitor, typename... Ts>
    struct GetReturnTypeImplValueT {
      static_assert(std::is_invocable_v<Visitor, typename Ts::type...>);
      using type = decltype(std::invoke(std::declval<Visitor>(),
                                        std::declval<typename Ts::type>()...));
    };

    template <class Visitor, typename T, typename... Ts>
    struct GetReturnType<Visitor, T, Ts...> {
      using type = typename GetReturnTypeImplValueT<Visitor, TupleDataType<T>,
                                                    TupleDataType<Ts>...>::type;
    };

    template <class Visitor>
    struct GetReturnType<Visitor> {
      static_assert(std::is_invocable_v<Visitor>);
      using type = decltype(std::invoke(std::declval<Visitor>()));
    };

  } // namespace details

  template <class Visitor, class... Tuples>
  decltype(auto) visit(std::size_t index, Visitor&& v, Tuples&&... t) {
    constexpr std::size_t size =
        details::GetSize<std::decay_t<Tuples>...>::size;
    static_assert(details::CheckSize<size, std::decay_t<Tuples>...>::value,
        "All tuples must be the same size");
    using return_t = typename details::GetReturnType<Visitor, Tuples...>::type;
    return details::Dispatcher<true, return_t>::template switch_<0, size>(
        index, std::forward<Visitor>(v), t.data...);
  }

} // namespace my

#endif
