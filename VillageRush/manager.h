#ifndef _MANAGER_H
#define _MANAGER_H

template <typename T>
class Manager
{
public:
	// 新たなマネージャーインスタンスを作成するために使用
	static T *instance()
	{
		if (!manager)
			manager = new T();
		return manager;
	}

private:
	static T *manager;

protected:
	Manager() = default;
	~Manager() = default;
	Manager(const Manager &) = delete;
	Manager &operator=(const Manager &) = delete;
};
template <typename T>
T *Manager<T>::manager = nullptr;

#endif // !_MANAGER_H
