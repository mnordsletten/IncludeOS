#ifndef STD_FUNCTIONAL_HPP
#define STD_FUNCTIONAL_HPP

#include "memory.hpp"
#include "type_traits.hpp"
#include "reference_wrapper.hpp"

namespace std
{
	// std::function
	// http://en.cppreference.com/w/cpp/utility/functional/function
	template <typename>
	class function;
	
	class allocator_arg_t {};
	constexpr allocator_arg_t allocator_arg = allocator_arg_t();
	
	template <typename ReturnType, typename... ArgumentTypes>
	class function <ReturnType (ArgumentTypes...)>
	{
	public:
		function() : mInvoker() {}
		
		template <typename FunctionT>
		function(FunctionT f)
			: mInvoker(new free_function_holder<FunctionT>(f)) {}
		
		template <typename FunctionType, typename ClassType>
		function(FunctionType ClassType::* f)
			: mInvoker(new member_function_holder<FunctionType, ArgumentTypes ...>(f)) {}
		
		ReturnType operator() (ArgumentTypes... args)
		{
			return mInvoker->invoke(args ...);
		}
		
		function(const function& other)
			: mInvoker(other.mInvoker->clone()) {}
		
		template <class Fn, class Alloc>
		function (allocator_arg_t aa, const Alloc& alloc, Fn fn) {}
		
		function& operator= (const function& other)
		{
			mInvoker = other.mInvoker->clone();
			return *this;
		}
		
	private:
		class function_holder_base
		{
		public:
			function_holder_base() {}
			virtual ~function_holder_base(){}
			
			virtual ReturnType invoke(ArgumentTypes ... args) = 0;
			virtual std::shared_ptr<function_holder_base> clone() = 0;
			
		private:
			function_holder_base(const function_holder_base&);
			void operator= (const function_holder_base&);
		};
		
		typedef std::shared_ptr<function_holder_base> invoker_t;
		
		template <typename FunctionT>
		class free_function_holder : public function_holder_base
		{
		public:
			typedef free_function_holder<FunctionT> self_type;
			
			free_function_holder(FunctionT func)
				: function_holder_base(), mFunction(func) {}
			
			virtual ReturnType invoke(ArgumentTypes... args)
			{
				return mFunction(args...);
			}
			
			virtual invoker_t clone()
			{
				return invoker_t(new self_type(mFunction));
			}
			
		private:
			FunctionT mFunction;
		};
		
		template <typename FunctionType, typename ClassType, typename... RestArgumentTypes>
		class member_function_holder : public function_holder_base
		{
		public:
			typedef FunctionType ClassType::* member_function_signature_t;
			
			member_function_holder(member_function_signature_t f)
				: mFunction(f) {}
			
			virtual ReturnType invoke(ClassType obj, RestArgumentTypes ... restArgs)
			{
				return (obj.*mFunction)(restArgs...);
			}
			
			virtual invoker_t clone()
			{
				return invoker_t(new member_function_holder(mFunction));
			}
			
		private:
			member_function_signature_t mFunction;
		};
		
		invoker_t mInvoker;
	};
	
	// std::hash
	// superbad implementation
	
	template <class Key>
	struct hash
	{
		hash() {}
		size_t operator() (const Key& arg) noexcept
		{
			// computes the hash of an employee using a variant 
			// of the Fowler-Noll-Vo hash function
			size_t result = 2166136261;
			
			for (size_t i = 0; i < sizeof(arg); i += 8)
			{
				result = (result * 16777619) ^ ((arg >> i) & 255);
			}
			
			return result ^ (arg << 1);
		}
	};
	
}

#endif