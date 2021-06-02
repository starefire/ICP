#include <exception>
#include <iostream>
#include <string>

#include "lua_engine.h"

// Catch C++ exceptions and convert them to Lua error messages.
// Customize as needed for your own exception classes.
static int wrap_exceptions(lua_State *L, lua_CFunction f)
{
	try {
		return f(L);  // Call wrapped function and return result.
	}
	catch (const char *s) {  // Catch and convert exceptions.
		lua_pushstring(L, s);
	}
	catch (std::exception& e) {
		lua_pushstring(L, e.what());
	}
	catch (...) {
		lua_pushliteral(L, "caught (...)");
	}
	return lua_error(L);  // Rethrow as a Lua error.
}

lua_engine::lua_engine()
{
	/*
	* All Lua contexts are held in this structure. We work with it almost
	* all the time.
	*/
	this->L = luaL_newstate();

	luaL_openlibs(this->L); /* Load Lua libraries */

	// Define wrapper function and enable it.
	lua_pushlightuserdata(this->L, (void *)wrap_exceptions);
	luaJIT_setmode(this->L, -1, LUAJIT_MODE_WRAPCFUNC | LUAJIT_MODE_ON);
	lua_pop(this->L, 1);
}

int lua_engine::loadfile(std::string filename)
{
	int status;

	/* Load the file containing the script we are going to run */
	status = luaL_loadfile(this->L, filename.c_str());
	if (status) {
		/* If something went wrong, error message is at the top of */
		/* the stack */
		std::cerr << "Couldn't load file: " << lua_tostring(this->L, -1) << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int lua_engine::run(void)
{
	int result; 

	/* Ask Lua to run our little script */
	result = lua_pcall(this->L, 0, LUA_MULTRET, 0);
	if (result) {
		std::cerr << "FAiled to run script: " << lua_tostring(this->L, -1) << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

lua_engine::~lua_engine()
{
	lua_close(this->L);   /* Cya, Lua */
}
