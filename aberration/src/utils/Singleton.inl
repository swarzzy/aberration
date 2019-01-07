namespace ab {
	template <typename Class>
	template <typename... Args>
	void Singleton<Class>::Initialize(Args&&... args) noexcept(std::is_nothrow_constructible<Class>::value) {
		if (m_Object != nullptr)
			//PRX_ERROR("Trying to create more than one copy of singleton.\n-> CLASS: ",
			//			typeid(Class).name());
			do {} while (false);
		else
			m_Object = new Class(std::forward<Args>(args)...);
	}

	template <typename Class>
	Class* Singleton<Class>::GetInstance() noexcept {
		//PRX_ASSERT((m_Object != nullptr), "Attempt to get an instance of not initialized singleton.\n-> CLASS: ",
		//				typeid(Class).name());
		return m_Object;
	}

	template <typename Class>
	void Singleton<Class>::Destroy() noexcept(std::is_nothrow_destructible<Class>::value) {
		delete m_Object;
		m_Object = nullptr;
	}
}
