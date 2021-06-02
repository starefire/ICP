#pragma once
#include <string>
#include "lua.hpp"

class lua_engine
{
	lua_State * L;
public:
	lua_engine();

	int loadfile(std::string filename);
	int run(void);

	~lua_engine();
};

