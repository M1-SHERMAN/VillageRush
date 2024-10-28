#ifndef _HOME_MANAGER_H_
#define _HOME_MANAGER_H_

#include "manager.h"
#include "config_manager.h"
#include "resources_manager.h"

class HomeManager : public Manager<HomeManager>
{
	friend class Manager<HomeManager>;

public:
	double get_current_hp_num()
	{
		return num_hp;
	}

	void decrease_hp(double val)
	{
		static ConfigManager *config_manager = ConfigManager::instance();
		num_hp -= val;
		if (num_hp < 0)
		{
			num_hp = 0;
			config_manager->is_game_win = false;
			config_manager->is_game_over = true;
		}

		static const ResourcesManager::SoundPool &sound_pool = ResourcesManager::instance()->get_sound_pool();

		Mix_PlayChannel(-1, sound_pool.find(ResID::Sound_HomeHurt)->second, 0);
	}

protected:
	HomeManager()
	{
		num_hp = ConfigManager::instance()->num_initial_hp;
	}
	~HomeManager() = default;

private:
	double num_hp = 0;
};

#endif // !_HOME_MANAGER_H_
