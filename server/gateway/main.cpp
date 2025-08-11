#include <iostream>
#include "proto/message.pb.h"

int main()
{
	ChatPacket p;
	p.clear_header();

	std::cout << "Gateway main function started." << std::endl;
	return 0;
}