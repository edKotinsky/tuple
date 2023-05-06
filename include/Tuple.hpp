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
      template <std::size_t Index, class Visitor, class Tuple>
      static constexpr Ret case_(Visitor&&, Tuple&&) {}

      template <std::size_t Base, std::size_t Size, class Visitor, class Tuple>
      static constexpr Ret switch_(std::size_t, Visitor&&, Tuple&) {}
    };

    template <typename Ret>
    struct Dispatcher<true, Ret> {
      template <std::size_t Index, class Visitor, class Tuple>
      static constexpr Ret case_(Visitor&& vis, Tuple&& t) {
        using return_t =
            decltype(std::invoke(std::declval<Visitor>(),
                                 std::declval<decltype(StaticGetter<Index>::get(
                                     std::declval<Tuple>()))>()));
        using expected_t = Ret;
        static_assert(std::is_same_v<return_t, expected_t>);
        using getter_t = StaticGetter<Index>;
        return std::invoke(std::forward<Visitor>(vis), getter_t::get(t));
      }

      template <std::size_t Base, std::size_t Size, class Visitor, class Tuple>
      static constexpr Ret switch_(std::size_t index, Visitor&& vis, Tuple& t) {
        switch (index) {
          case Base:
            return Dispatcher<(Base < Size), Ret>::template case_<Base>(
                std::forward<Visitor>(vis), t);
          case Base + 1:
            return Dispatcher<(Base + 1 < Size), Ret>::template case_<Base + 1>(
                std::forward<Visitor>(vis), t);
          case Base + 2:
            return Dispatcher<(Base + 2 < Size), Ret>::template case_<Base + 2>(
                std::forward<Visitor>(vis), t);
          case Base + 3:
            return Dispatcher<(Base + 3 < Size), Ret>::template case_<Base + 3>(
                std::forward<Visitor>(vis), t);
          default:
            return Dispatcher<(Base + 4 < Size), Ret>::template switch_<
                Base + 4, Size>(index, std::forward<Visitor>(vis), t);
        }
      }
    };

    struct Error_Tuple_Out_Of_Bounds {};

  } // namespace details

  template <typename... Ts>
  class Tuple {
    using data_t = details::TupleData<Ts...>;
  public:
    static constexpr std::size_t _size = sizeof...(Ts);

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

    template <class Visitor, class Tuple>
    friend decltype(auto) visit(std::size_t index, Visitor&& v, Tuple&& t);
  private:
    data_t data;
  };

  template <class Visitor, class Tuple>
  decltype(auto) visit(std::size_t index, Visitor&& v, Tuple&& t) {
    using tuple_t = std::decay_t<Tuple>;
    using return_t =
        decltype(std::invoke(std::forward<Visitor>(v), t.data.value));
    return details::Dispatcher<true, return_t>::template switch_<
        0, tuple_t::_size>(index, std::forward<Visitor>(v), t.data);
  }

} // namespace my

#endif
