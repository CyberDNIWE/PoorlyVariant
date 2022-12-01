#pragma once

namespace poorly
{
	namespace stl_replacers
	{
		// unnamed namespace for sneaky stuff
		namespace 
		{
			#ifndef POORLY_SIZE_T
			#define POORLY_SIZE_T size_t 
			#endif
			
			static POORLY_SIZE_T g_type_id = 0;

			template<typename T>
			struct _type
			{
				// This is a crude hack, #todo: figure out a way of generating type IDs in constexpr
				static POORLY_SIZE_T type_id() noexcept
				{
					static POORLY_SIZE_T my_type_id = ++g_type_id;
					return my_type_id;
				};
			};

			//ref removes etc
			template<class T>
			struct remove_reference
			{
				using type = T;
			};

			template<class T>
			struct remove_reference<T&>
			{
				using type = T;
			};

			template<class T>
			struct remove_reference<T&&>
			{
				using type = T;
			};

		
		};
		

		template <typename T>
		constexpr POORLY_SIZE_T type_id_of() noexcept
		{
			return _type<T>::type_id();
		}
	
		template <typename T>
		constexpr POORLY_SIZE_T type_id_of(const T&) noexcept
		{
			return _type<T>::type_id();
		}
	
		
		template<class T>
		using remove_reference_t = typename remove_reference<T>::type;
	
		template<class T>
		constexpr T&& forward(poorly::stl_replacers::remove_reference_t<T>&& arg) noexcept
		{
		//	static_assert(!is_lvalue_reference_v<T>, "bad forward call");
			return (static_cast<T&&>(arg));
		}
	
		template<class T>
		constexpr T&& forward(poorly::stl_replacers::remove_reference_t<T>& arg) noexcept
		{
			return (static_cast<T&&>(arg));
		}
	
		template<class T>
		constexpr poorly::stl_replacers::remove_reference_t<T>&& move(T&& arg) noexcept
		{
			return (static_cast<poorly::stl_replacers::remove_reference_t<T>&&>(arg));
		}
	
		// Equivalent to std::aligned_storage (taken from https://gist.github.com/calebh/fd00632d9c616d4b0c14e7c2865f3085)
		template<unsigned int Len, unsigned int Align>
		struct aligned_storage 
		{
			struct type 
			{
				alignas(Align) unsigned char data[Len];
			};
		};
	
		template<class T, class T2 = T> 
		inline void swap(T& l, T2& r)
		{
			T tmp = poorly::stl_replacers::move(l);
			l = poorly::stl_replacers::move(r);
			r = poorly::stl_replacers::move(tmp);
		}
	
		
	
		namespace tricks
		{
	
			// static_max, made more constexpr-friendly
			// #todo: be as creative as this guy!
	
			template <size_t arg1, size_t ... others>
			struct static_max;
	
			template <size_t arg>
			struct static_max<arg>
			{
				static constexpr size_t value() noexcept { return arg; }
			};
	
			template <size_t arg1, size_t arg2, size_t ... others>
			struct static_max<arg1, arg2, others...>
			{
				static constexpr size_t value() noexcept
				{
					return arg1 >= arg2 ? static_max<arg1, others...>::value() : static_max<arg2, others...>::value();
				}
			};
	
			////////////////////////////MAYBE_UNUSED////////////////////////////////////
			template<typename _A, typename _B>
			inline constexpr bool isSameType(_A& a, _B& b) noexcept
			{
				return false;
			}
	
			template<typename _A>
			inline constexpr bool isSameType(_A& a, _A& b) noexcept
			{
				return true;
			}

			template<typename _A, typename _B>
			inline constexpr bool is_same_type(_A* a, _B* b) noexcept
			{
				return false;
			}

			template<typename _A>
			inline constexpr bool is_same_type(_A* a, _A* b) noexcept
			{
				return true;
			}
		};
	
	};

};