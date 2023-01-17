/* the ifndef compile the script if an event happen like the classic if*/
#ifndef __CONSOLE_HEADER__
#define __CONSOLE_HEADER__
#include <stdio.h>

class CConsole {
	FILE m_OldStdin, m_OldStdout;
	bool m_OwnConsole;
public:
	CConsole();
	~CConsole();
};

#endif
