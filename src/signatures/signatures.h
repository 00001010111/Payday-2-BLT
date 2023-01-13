#ifndef __SIGNATURE_HEADER__
#define __SIGNATURE_HEADER__

#include <string>
#include <vector>

#define CREATE_CALLABLE_SIGNATURE(name, retn, signature, mask, offset, ...) \
	typedef retn(__fastcall *name ## ptr)(__VA_ARGS__); \
	name ## ptr name = NULL; \
	SignatureSearch name ## search(#name, &name, signature, mask, offset);

#define CREATE_NORMAL_CALLABLE_SIGNATURE(name, retn, signature, mask, offset, ...) \
	typedef retn(*name ## ptr)(__VA_ARGS__); \
	name ## ptr name = NULL; \
	SignatureSearch name ## search(#name, &name, signature, mask, offset);

#define CREATE_CALLABLE_CLASS_SIGNATURE(name, retn, signature, mask, offset, ...) \
	typedef retn(__thiscall *name ## ptr)(void*, __VA_ARGS__); \
	name ## ptr name = NULL; \
	SignatureSearch name ## search(#name, &name, signature, mask, offset);

#define CREATE_LUA_FUNCTION(lua_func, name) \
	lua_pushcclosure(L, lua_func, 0); \
	_asm add esp, 4 \
	lua_setfield(L, LUA_GLOBALSINDEX, name); \
	_asm add esp, 4

struct SignatureF {
	const char* funcname;
	const char* signature;
	const char* mask;
	int offset;
	void* address;
};

class SignatureSearch {
public:
	SignatureSearch(const char* funcname, void* address, const char* signature, const char* mask, int offset);
	static void Search();
};

class FuncDetour {
public:
	FuncDetour(void** oldF, void* newF);
	~FuncDetour();
	void Attach();
	void Detach();
protected:
	void** oldFunction;
	void* newFunction;
};


#endif // __SIGNATURE_HEADER__