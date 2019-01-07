#pragma once
#include "src/ABHeader.h"

namespace ab {
	template<typename Class>
	class Singleton {
		AB_DISALLOW_COPY_AND_MOVE(Singleton)
	protected:
		Singleton() {};
		virtual ~Singleton() {};

	private:
		inline static Class* m_Object = nullptr;

	public:
		template <typename... Args>
		inline static void Initialize(Args&&... args) noexcept(std::is_nothrow_constructible<Class>::value);
		inline static Class* GetInstance() noexcept;
		inline static void Destroy() noexcept(std::is_nothrow_destructible<Class>::value);
	};
}
#include "Singleton.inl"