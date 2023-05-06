# Tuple with Visitation

It is a simple tuple implementation that differs from the standard tuple not
only by less amount of features (just because of my lazyness), but by the
visitation feature.

Tuple is a fixed-size collection of values, that stores the value of each type
instead of a Variant, that stores only one type's value at a time. But, variant
allows to access this value at runtime using generalized lambda or an overloaded
for each type function or method.

Standard tuple allows to access values only at compile-time using a template
`std::get` function. But imagine all the power, if you have access to tuple
elements at a runtime!

Suppose that you have multiple objects of different classes inherited from an
abstract class. They're stored in `std::vector`, and you can iterate thru them
and call their virtual method. But this requires usage of vtable, and compiler
won't optimize the code for you. It is slow and memory demanding.

C++ offers you static interfaces and templates as an another way. Create
multiple classes, that implements some method with the same signature, and store
them... Where?

In a tuple! And iterate over them by visiting it! It is a crazy mix of runtime
and compile, that is powered by Template Metaprogramming Black Magic and offers
you to do the magic.

One more: you can do this with multiple tuples. This means multiple visitation,
that's right.

Maybe I'll fight with my lazyness and write a documentation, normal tests and
add more features from `std::tuple`. If you're also got crazy of C++ template 
metaprogramming, you can try to help me with it.

Licensed under [MIT License](./LICENSE).
