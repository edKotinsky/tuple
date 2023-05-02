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

    template <typename Tuple, typename... Ts>
    struct SpreadValues {
      SpreadValues(Tuple&) {}
    };

    template <typename Tuple, typename T, typename... Ts>
    struct SpreadValues<Tuple, T, Ts...> {
      SpreadValues(Tuple& t, T v, Ts... vs) {
        t.value = v;
        SpreadValues<decltype(t.next), Ts...> sv(t.next, vs...);
      }
    };

    template <class Tuple, unsigned Index, unsigned Current, unsigned Size,
              typename AlwaysVoid>
    struct StaticGetter {};

    template <unsigned Index, unsigned Current, unsigned Size, typename T,
              typename... Ts>
    struct StaticGetter<TupleData<T, Ts...>, Index, Current, Size,
                        std::enable_if_t<(Current != Index)>> {
      using getter_t =
          StaticGetter<TupleData<Ts...>, Index, Current + 1, Size, void>;

      auto& operator()(TupleData<T, Ts...>& t) {
        getter_t getter;
        return getter(t.next);
      }

      auto const& operator()(TupleData<T, Ts...> const& t) const {
        getter_t getter;
        return getter(t.next);
      }
    };

    template <unsigned Index, unsigned Current, unsigned Size, typename T,
              typename... Ts>
    struct StaticGetter<TupleData<T, Ts...>, Index, Current, Size,
                        std::enable_if_t<(Current == Index)>> {
      T& operator()(TupleData<T, Ts...>& t) { return t.value; }

      T const& operator()(TupleData<T, Ts...> const& t) const {
        return t.value;
      }
    };

    struct Error_Tuple_Out_Of_Bounds {};

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

      template <class Tuple, class Visitor, std::size_t CurrentElement = 0>
      auto get(Tuple const& t, Visitor&& vis, std::size_t index)
          -> std::enable_if_t<IsTupleValid_v<std::decay_t<Tuple>>> const {
        if (index == CurrentElement) {
          std::invoke(std::forward<Visitor>(vis), t.value);
          return;
        }
        using tuple_t = std::decay_t<Tuple>;
        using next_t = typename tuple_t::next_t;
        get<next_t, Visitor, CurrentElement + 1>(
            t.next, std::forward<Visitor>(vis), index);
      }

      template <class Tuple, class Visitor, std::size_t CurrentElement = 0>
      auto get(Tuple const&, Visitor&&, std::size_t)
          -> std::enable_if_t<!IsTupleValid_v<std::decay_t<Tuple>>> const {}
    };

  } // namespace details

  template <typename... Ts>
  class Tuple {
    using data_t = details::TupleData<Ts...>;
  public:
    explicit Tuple(Ts... ts) {
      details::SpreadValues<data_t, Ts...> sv(data, ts...);
    }

    Tuple() = default;

    template <unsigned Index>
    auto& get() {
      using trueType = details::StaticGetter<data_t, Index, 0, sz, void>;
      using falseType = details::Error_Tuple_Out_Of_Bounds;
      using result = std::conditional_t<(Index < sz), trueType, falseType>;
      result getter;
      return getter(data);
    }

    template <unsigned Index>
    auto const& get() const {
      using trueType = details::StaticGetter<data_t, Index, 0, sz, void>;
      using falseType = details::Error_Tuple_Out_Of_Bounds;
      using result = std::conditional_t<(Index < sz), trueType, falseType>;
      result const getter;
      return getter(data);
    }

    template <class Visitor>
    void visit(unsigned index, Visitor&& vis) {
      details::Getter getter;
      getter.get(data, std::forward<Visitor>(vis), index);
    }

    template <class Visitor>
    void visit(unsigned index, Visitor&& vis) const {
      details::Getter getter;
      getter.get(data, std::forward<Visitor>(vis), index);
    }

    constexpr unsigned size() { return sz; }

  private:
    static constexpr unsigned sz = sizeof...(Ts);
    data_t data;
  };

} // namespace my

#endif
