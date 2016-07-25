//====================================================================================
//     The MIT License (MIT)
//
//     Copyright (c) 2011 Kapparock LLC
//
//     Permission is hereby granted, free of charge, to any person obtaining a copy
//     of this software and associated documentation files (the "Software"), to deal
//     in the Software without restriction, including without limitation the rights
//     to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//     copies of the Software, and to permit persons to whom the Software is
//     furnished to do so, subject to the following conditions:
//
//     The above copyright notice and this permission notice shall be included in
//     all copies or substantial portions of the Software.
//
//     THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//     IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//     FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//     AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//     LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//     OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//     THE SOFTWARE.
//====================================================================================

#include "lancex.hpp"
#include <stdio.h>
#include <sys/file.h>
#include <sys/types.h>
#include <unistd.h>

#define PIDF "/tmp/lancexd.pid"

int write_pid()
{
	int fd = open(PIDF, O_WRONLY|O_CREAT, 0600);
	
    if (fd < 0)
	{
		printf("write_pid open fd failed \n");
        return -1;
	}
	if (flock(fd,LOCK_EX|LOCK_NB) < 0)
	{
		printf("There is already an instance of LanceX. Only one instance please \n");
		close(fd);
		return -1;
	}
	char pidstr[16];
	int pidstr_len;
	pidstr_len = sprintf(pidstr, "%d\n", getpid());
	write(fd, pidstr, pidstr_len);
	
	return 0;
}

int main(int argc, char **argv)
{

    // 1. download server information from lancex.cc
    // 2. establish connection with lancex.cc
    lancex::init();
    
    
    if (write_pid() < 0) 
    {
        exit(-1);        
    }

    lancex::bind();

    while(1) 
    {
		lancex::Sleep(5);
	}
    
    return 0;
}
