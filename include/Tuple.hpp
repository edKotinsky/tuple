#ifndef MY_TUPLE_HPP
#define MY_TUPLE_HPP

#include <type_traits>

namespace my {

  namespace details {

    template <typename... Ts>
    struct TupleData {};

    template <typename T, typename... Ts>
    struct TupleData<T, Ts...> {
      T value;
      TupleData<Ts...> next;
    };

    template <>
    struct TupleData<> {};

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

    template <class Tuple, class Visitor, unsigned Current>
    struct Getter {
      void operator()(Tuple&, unsigned, Visitor) {}

      void operator()(Tuple const&, unsigned, Visitor) const {}
    };

    template <unsigned Current, class Visitor, typename T, typename... Ts>
    struct Getter<TupleData<T, Ts...>, Visitor, Current> {
      using next_t = Getter<TupleData<Ts...>, Visitor, Current + 1>;

      void operator()(TupleData<T, Ts...>& t, unsigned index, Visitor vis) {
        if (index == Current) {
          vis(t.value);
          return;
        }
        next_t n;
        n(t.next, index, vis);
      }

      void operator()(TupleData<T, Ts...> const& t, unsigned index,
                      Visitor vis) const {
        if (index == Current) {
          vis(t.value);
          return;
        }
        next_t n;
        n(t.next, index, vis);
      }
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
    void visit(unsigned index, Visitor vis) {
      details::Getter<data_t, Visitor, 0> getter;
      getter(data, index, vis);
    }

    template <class Visitor>
    void visit(unsigned index, Visitor vis) const {
      details::Getter<data_t, Visitor, 0> const getter;
      getter(data, index, vis);
    }

    constexpr unsigned size() { return sz; }

  private:
    static constexpr unsigned sz = sizeof...(Ts);
    data_t data;
  };

} // namespace my

#endif
