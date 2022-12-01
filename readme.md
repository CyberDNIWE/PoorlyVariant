# poorly :: variant & poorly :: variant_visitable

[Poorly (made) variant](https://github.com/CyberDNIWE/PoorlyVariant) and [variant_visitable](https://github.com/CyberDNIWE/PoorlyVariant) are non-allocating stl-free C++11 variant with base functionality akin to [`std::variant`](https://en.cppreference.com/w/cpp/utility/variant)  and its _visitable_ version (similar in intent to [std::visit](https://en.cppreference.com/w/cpp/utility/variant/visit)).

Made primarily for platforms for embeded devices which compiler vendors did not bother with providing `STL` with their compiler (looking at you `arduino avr-gcc`).

It is **not meant** as a drop-in replacement for [`std::variant`](https://en.cppreference.com/w/cpp/utility/variant), so use standard one wherever you are able to.

> **There is no guarantee it will work for your case/compiler and using it in production (or at all, really) is highly inadvisable (hence the namespace)**


## Features
 - STL-free
   - All required utilities come in `StlReplacer.h`
 - Guaranteed not to allocate
 - Compile-time type checking 
   - Fails compilation when trying to `set<T>(...)` of type not listed in `variant<Ts...>` Ts... list declaration
 - Preferes not to throw 
   - `T* get<T>()` returns pointer that is `null`ed if stored value is not of type `T`
 - Copy/Move-constructable
   - Uses underlying types copy/move- constructors too when possible
 - Shared makeshift `type_id`
   - Unique `type_id` numbers are generated on per-type basis

### Design priorities (desc order)
Compile time type safety -> Lowest memory footprint -> Execution speed 

## Memory footprint
  - `poorly::variant`: 
    - `sizeof(POORLY_SIZE_T)` worth of bytes for `type_id` "tag" 
    - `sizeof(POORLY_SIZE_T)` worth of bytes for unique `_type<T>::type_id()` magic static
  - `poorly::variant_visitable`: 
    - Everything from `poorly::variant` (has one as a member)
    - `sizeof(FunctionPointerType)` worth of bytes for visiting function pointer member
 
## Todo:
  - Make type ID generator compile-time (still can't figure it out)
    - Right now uses crude magic static hack which is not constexpr

## Compatibility chart

| Compiler      | Version         | Compiles |  Works  |
|:-------------:|:---------------:|:--------:|:-------:|
|     MSVC      |     14.16       |   YES    |   YES   |
|    AVR-GCC    | 7.3.0 (Arduino) |   YES    | UNKNOWN |

> Compilers not on this chart have not been tested with


## Usage
### variant
```c++
using MyVariant = poorly::variant<int, double>;
MyVariant mySimpleVariant = {};

// Always set with explicit type
mySimpleVariant.set<int>(1);

//mySimpleVariant.set<float>(1.f); // Will fail to compile since only <int, double> expected

// Get by type -> check if returned ptr not null	
auto* i_ptr = mySimpleVariant.get<int>();
auto  i = i_ptr ? *i_ptr : 0; // i will be 1

// mySimpleVariant.get<double>(); // Will return nullptr since int is stored
```

### variant_visitable
```c++
// Case for times when only one concrete visitor is needed, also modifies the value
struct MyVariantVisitsConcrete
{
	void operator()(const int& i)
	{ std::cout << "MyVariantVisitsConcrete::operator() called with const int:    " << i << std::endl; }

	void operator()(const double& d)
	{ std::cout << "MyVariantVisitsConcrete::operator() called with const double: " << d << std::endl; }
};


using MyVariantVisitable = poorly::variant_visitable<MyVariantVisitsConcrete, int, double>;
MyVariantVisitable variantVisitableConcrete = {};


// Store value into variant
variantVisitableConcrete.set<int>(2);

// Basic use of variant: query by type -> check if returned ptr not null
i_ptr = variantVisitableConcrete.get<int>();
i = i_ptr ? *i_ptr : 0;

// Make visitor and visit
MyVariantVisitsConcrete visitorConcrete = {};
variantVisitableConcrete.accept_visitor(visitorConcrete); // Will call const int& overload
// See main.cpp for polymorphic visitor example
```

> See main.cpp for more examples


 
## Contributing
Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change.

## Licence
No licence

> _Because licences are beuracratic bs and I can never spell them correctly!_

## Acknowledgment
> Originally made from a version from [this gist](https://gist.github.com/tibordp/6909880), but since became its own thing.