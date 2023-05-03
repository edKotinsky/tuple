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
    struct StaticGetter {
      static constexpr bool valid = false;
    };

    template <std::size_t Index, std::size_t Current, typename T,
              typename... Ts>
    struct StaticGetter<TupleData<T, Ts...>, Index, Current,
                        std::enable_if_t<(Current != Index)>> {
      static constexpr bool valid = true;
      using getter_t = StaticGetter<TupleData<Ts...>, Index, Current + 1, void>;

      auto& operator()(TupleData<T, Ts...>& t) noexcept {
        getter_t getter;
        return getter(t.next);
      }

      auto const& operator()(TupleData<T, Ts...> const& t) const noexcept {
        getter_t getter;
        return getter(t.next);
      }
    };

    template <std::size_t Index, std::size_t Current, typename T,
              typename... Ts>
    struct StaticGetter<TupleData<T, Ts...>, Index, Current,
                        std::enable_if_t<(Current == Index)>> {
      static constexpr bool valid = true;

      T& operator()(TupleData<T, Ts...>& t) noexcept { return t.value; }

      T const& operator()(TupleData<T, Ts...> const& t) const noexcept {
        return t.value;
      }
    };

    struct Error_Tuple_Out_Of_Bounds {};

    template <class TupleData, std::size_t Size, typename Void>
    struct Case {
      template <class Tuple, class Visitor>
      Case(Tuple const&, Visitor&&) noexcept {}
    };

    template <class TupleData, std::size_t Index, std::size_t Current,
              std::size_t Size>
    struct Case<
        StaticGetter<TupleData, Index, Current, void>, Size,
        std::enable_if<(StaticGetter<TupleData, Index, Current, void>::valid &&
                        Index < Size)>> {
      using sgetter_t = StaticGetter<TupleData, Index, Current, void>;

      /*
       * Noexcept if `std::invoke(vis, val)` is noexcept
       */
      template <class Tuple, class Visitor>
      Case(Tuple& t, Visitor&& vis) noexcept(std::invoke(
          std::declval<Visitor>(),
          std::declval<
              decltype(std::declval<sgetter_t>(std::declval<Tuple>()))>())) {
        sgetter_t getter;
        auto& val = getter(t);
        std::invoke(std::forward<Visitor>(vis), val);
      }
    };

    template <std::size_t Size>
    struct Getter {
      template <class Tuple, class Visitor, std::size_t CurrentElement = 0>
      auto get(Tuple& t, Visitor&& vis, std::size_t index)
          -> std::enable_if_t<IsTupleValid_v<std::decay_t<Tuple>>> {
        if (index == CurrentElement) {
          std::invoke(std::forward<Visitor>(vis), t.value);
          return;
        }
        using tuple_t = std::decay_t<Tuple>;
        using next_t = typename tuple_t::next_t;
        get<next_t, Visitor, CurrentElement + 1>(
            t.next, std::forward<Visitor>(vis), index);
      }

      template <class Tuple, class Visitor, std::size_t Current = 0>
      std::enable_if_t<IsTupleValid_v<std::decay_t<Tuple>>>
          get(Tuple const& t, Visitor&& vis, std::size_t index) const {
        switch (index) {
          case Current:
            {
              using getter_t = StaticGetter<Tuple, Current, Current, void>;
              Case<getter_t, Size, void> c(t, std::forward<Visitor>(vis));
              break;
            }
          case Current + 1:
            {
              using getter_t = StaticGetter<Tuple, Current + 1, Current, void>;
              Case<getter_t, Size, void> c(t, std::forward<Visitor>(vis));
              break;
            }
          case Current + 2:
            {
              using getter_t = StaticGetter<Tuple, Current + 2, Current, void>;
              Case<getter_t, Size, void> c(t, std::forward<Visitor>(vis));
              break;
            }
          case Current + 3:
            {
              using getter_t = StaticGetter<Tuple, Current + 3, Current, void>;
              Case<getter_t, Size, void> c(t, std::forward<Visitor>(vis));
              break;
            }
          case Current + 4:
            {
              using getter_t = StaticGetter<Tuple, Current + 4, Current, void>;
              Case<getter_t, Size, void> c(t, std::forward<Visitor>(vis));
              break;
            }
          case Current + 5:
            {
              using getter_t = StaticGetter<Tuple, Current + 5, Current, void>;
              Case<getter_t, Size, void> c(t, std::forward<Visitor>(vis));
              break;
            }
          case Current + 6:
            {
              using getter_t = StaticGetter<Tuple, Current + 6, Current, void>;
              Case<getter_t, Size, void> c(t, std::forward<Visitor>(vis));
              break;
            }
          case Current + 7:
            {
              using getter_t = StaticGetter<Tuple, Current + 7, Current, void>;
              Case<getter_t, Size, void> c(t, std::forward<Visitor>(vis));
              break;
            }
          default:
            {
              using tuple_t = std::decay_t<Tuple>;
              using next_t = typename tuple_t::next_t;
              get<next_t, Visitor, Current + 8>(
                  t.next, std::forward<Visitor>(vis), index);
              break;
            }
        }
      }

      template <class Tuple, class Visitor, std::size_t CurrentElement = 0>
      std::enable_if_t<!IsTupleValid_v<std::decay_t<Tuple>>>
          get(Tuple const&, Visitor&&, std::size_t) const noexcept {}
    };

  } // namespace details

  template <typename... Ts>
  class Tuple {
    using data_t = details::TupleData<Ts...>;
  public:
    explicit Tuple(Ts... ts) : data(ts...) {}

    Tuple() = default;

    template <std::size_t Index>
    auto& get() {
      using trueType = details::StaticGetter<data_t, Index, 0, void>;
      using falseType = details::Error_Tuple_Out_Of_Bounds;
      using result = std::conditional_t<(Index < sz), trueType, falseType>;
      result getter;
      return getter(data);
    }

    template <std::size_t Index>
    auto const& get() const {
      using trueType = details::StaticGetter<data_t, Index, 0, void>;
      using falseType = details::Error_Tuple_Out_Of_Bounds;
      using result = std::conditional_t<(Index < sz), trueType, falseType>;
      result const getter;
      return getter(data);
    }

    template <class Visitor>
    void visit(std::size_t index, Visitor&& vis) {
      details::Getter<sz> getter;
      getter.get(data, std::forward<Visitor>(vis), index);
    }

    template <class Visitor>
    void visit(std::size_t index, Visitor&& vis) const {
      details::Getter<sz> getter;
      getter.get(data, std::forward<Visitor>(vis), index);
    }

    constexpr std::size_t size() { return sz; }

  private:
    static constexpr std::size_t sz = sizeof...(Ts);
    data_t data;
  };

} // namespace my

#endif
