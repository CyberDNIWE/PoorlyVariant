#pragma once
// Taken from the gist thread from github ( https://gist.github.com/tibordp/6909880 )
// but modified somewhat to have less footprint and not depend on STL


#include <new.h>

// TypeID numeric int type, define to something else if size_t is not suitable
#ifndef POORLY_TYPEID_T
#define POORLY_SIZE_T size_t
#else
#define POORLY_SIZE_T POORLY_TYPEID_T
#endif

#include "StlReplacers.h"

namespace poorly 
{
	namespace // hidden unnamed namespace for inner workings
	{
		void _no_op(void*, void*) noexcept {};

		template<typename T, typename VARIANT_T, typename VISITOR_T>
		void _visit_variant_with_captured_types_of(void* variant, void* visitor);
	};

	template<typename... Ts>
	struct variant_helper;

	template<typename ST, typename... Ts>
	struct variant_helper<ST, Ts...>
	{
		// Checks recursively if T is same type as current variant_helper's ST in compile time
		template<typename T>
		inline static constexpr bool isExpected(T* tptr = nullptr, ST* stptr = nullptr) noexcept
		{
			return poorly::stl_replacers::tricks::is_same_type(tptr, stptr) ?
				poorly::stl_replacers::tricks::is_same_type(tptr, stptr) :
				variant_helper<Ts...>::template isExpected<T>();
		}

		inline static constexpr POORLY_SIZE_T get_id() noexcept
		{
			return poorly::stl_replacers::type_id_of<ST>();
		};

		inline static void destroy(POORLY_SIZE_T id, void* data)
		{
			if(id == get_id())
			{
				reinterpret_cast<ST*>(data)->~ST();
			}
			else
			{
				variant_helper<Ts...>::destroy(id, data);
			}
		}
	
		inline static void move(POORLY_SIZE_T old_t, void* old_v, void* new_v)
		{
			if(old_t == get_id())
			{
				new (new_v) ST(poorly::stl_replacers::move(*reinterpret_cast<ST*>(old_v)));
			}
			else
			{
				variant_helper<Ts...>::move(old_t, old_v, new_v);
			}
		}
	
		inline static void copy(POORLY_SIZE_T old_t, const void* old_v, void* new_v)
		{
			if(old_t == get_id())
			{
				new (new_v) ST(*reinterpret_cast<const ST*>(old_v));
			}
			else
			{
				variant_helper<Ts...>::copy(old_t, old_v, new_v);
			}
		}
	};

	template<>
	struct variant_helper<>
	{
		template<typename T>
		inline static constexpr bool isExpected(T* tptr = nullptr, void* stptr = nullptr) { return false; }

		inline static constexpr POORLY_SIZE_T get_id() noexcept  { return 0; }
		inline static void destroy(POORLY_SIZE_T, void*) { }
		inline static void move(POORLY_SIZE_T, void*, void*) { }
		inline static void copy(POORLY_SIZE_T, const void*, void*) { }
	};

	

	template<typename... Ts>
	struct variant
	{
	public:
		static constexpr size_t max_size = poorly::stl_replacers::tricks::static_max<sizeof(Ts)... >::value();
		static constexpr size_t max_align = poorly::stl_replacers::tricks::static_max<alignof(Ts)...>::value();
	
	protected:
		using data_t   = typename poorly::stl_replacers::aligned_storage<max_size, max_align>::type;
		using helper_t = variant_helper<Ts...>;

		static constexpr inline POORLY_SIZE_T invalid_type() { return 0; }
	

		POORLY_SIZE_T	type_id;
		data_t			data;

	public:
		constexpr variant() : type_id(invalid_type())
		{}

		variant(const variant<Ts...>& old) : type_id(old.type_id)
		{
			helper_t::copy(old.type_id, &old.data, &data);
		}
	
		variant(variant<Ts...>&& old) : type_id(old.type_id)
		{
			helper_t::move(old.type_id, &old.data, &data);
		}
	
		// Serves as both the move and the copy asignment operator.
		variant<Ts...>& operator= (variant<Ts...>& old)
		{
			type_id = old.type_id;
			data	= old.data;
	
			return *this;
		}
	
		template<typename T>
		inline bool is() const noexcept
		{
			return (type_id == poorly::stl_replacers::type_id_of<T>());
		}
	
		inline bool valid() const noexcept
		{
			return (type_id != invalid_type());
		}
		
		// Will fail compilation if T is not found in Ts...
		template<typename T, typename... Args>
		void set(Args&&... args)
		{
			constexpr bool ret = helper_t::template isExpected<T>();
			if(ret)
			{
				// First we destroy the current contents    
				helper_t::destroy(type_id, &data);
				new (&data) T(poorly::stl_replacers::forward<Args>(args)...);
				type_id = poorly::stl_replacers::type_id_of<T>();
			}
			else
			{
				static_assert(ret, "Variant does not expcept type T! Make sure it is listed in variant template parameter list declaration.");
			}
		}

		void reset() 
		{
			if(valid())
			{
				helper_t::destroy(type_id, &data);
				type_id = invalid_type();
			}
		}
	
		// No-exception variant's get, requires nullptr check after call (useful for embeded programing)
		template<typename T>
		T* get() noexcept
		{
			return is<T>() ? reinterpret_cast<T*>(&data) : nullptr;
		}

		~variant() 
		{
			helper_t::destroy(type_id, &data);
		}
	};

	template<typename VIS, typename... Ts>
	struct variant_visitable
	{
	protected:
		using underlying_variant_t = poorly::variant<Ts...>;
		typedef void(*visitFunc)(void*, void*);

		underlying_variant_t m_variant;
		visitFunc			 visit_f;

	public:
		constexpr variant_visitable() : m_variant(), visit_f(poorly::_no_op)
		{};

		constexpr variant_visitable(const underlying_variant_t& other) :  m_variant(other), visit_f(other.visit_f)
		{};

		constexpr variant_visitable(const underlying_variant_t&& other) : m_variant(other), visit_f(other.visit_f)
		{};

		variant_visitable(const variant<VIS, Ts...>& old)				: m_variant(old), visit_f(old.visit_f)
		{};

		variant_visitable(variant<VIS, Ts...>&& old)					: m_variant(old), visit_f(old.visit_f)
		{};

		template<typename T>
		inline bool is() const noexcept
		{
			return m_variant.template is<T>();
		}

		inline bool valid() const noexcept
		{
			return m_variant.valid();
		}

		template<typename T>
		inline T* get() noexcept
		{
			return m_variant.template get<T>();
		}

		template<typename T, typename... Args>
		void set(Args&&... args)
		{
			m_variant.template set<T, Args...>(poorly::stl_replacers::forward<Args>(args)...);
			visit_f = poorly::_visit_variant_with_captured_types_of<T, underlying_variant_t, VIS>;
		}

		void accept_visitor(VIS& visitor)
		{ // No check needed because visit_f is guaranteed to be at least no_op
			visit_f(&m_variant, &visitor);
		}

		inline void reset()
		{
			visit_f = poorly::_no_op;
			m_variant.reset();
		}

		~variant_visitable() = default;

	};


	namespace // hidden unnamed namespace for inner workings
	{
		template<typename T, typename VARIANT_T, typename VISITOR_T>
		void _visit_variant_with_captured_types_of(void* variant, void* visitor)
		{ // No checks are needed since visitor is gotten from ref and self is always this of variant
			auto* const ptr = (reinterpret_cast<VARIANT_T*>(variant))->template get<T>();
			if(ptr)
			{
				// If you cant compile because of this line - your visitor does not have appropriate overload of operator().
				(*reinterpret_cast<VISITOR_T*>(visitor))(*ptr);
			}
		};
	};

};

#ifdef POORLY_SIZE_T
#undef POORLY_SIZE_T
#endif