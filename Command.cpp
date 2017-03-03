// 
// 
// 

#include "Command.h"

CommandClass* CommandClass::_instance;

void CommandClass::init()
{
	_write = false;
}


CommandClass* CommandClass::Instance()
{
	if (CommandClass::_instance == NULL) {
		CommandClass::_instance = new CommandClass();
	}
	return _instance;
}
