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
#include <sys/stat.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>
#include <syslog.h>

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
	pidstr_len = sprintf(pidstr, "%d", getpid());
    pidstr[pidstr_len] = 0;
	write(fd, pidstr, pidstr_len);
    
	return 0;
}
struct PidfManager {
    int fd;
    bool locked;
    PidfManager(const char* file):fd{-1}, locked{false}
    {
        fd = open(file, O_WRONLY|O_CREAT, 0600);
        locked = flock(fd,LOCK_EX|LOCK_NB) >= 0;
    }
    int writePid() {
        if (!locked) 
            return -1; 
        
        char pidstr[16];
    	int pidstr_len;
	    pidstr_len = sprintf(pidstr, "%d\n", getpid());
	    write(fd, pidstr, pidstr_len);
        pidstr[pidstr_len] = 0;
	    return getpid();
    }
    ~PidfManager() {
        close(fd);
    }
};

int loadPID()
{
 	if (access(PIDF, F_OK) <0)
	{
		return -1;
	}
	
    int pid;
	FILE *pidFile = fopen (PIDF, "r");

	if (pidFile == NULL)
	{
		fclose(pidFile);
		return -1;
	}

	fscanf(pidFile, "%d", &pid);
	fclose(pidFile);
    return pid;
}
int isRunning()
{
    int pid = loadPID();
    if (pid < 0)
        return -1;

	/* Got PID , now check if this PID is running */
	struct stat s;
	char fileString[32];
	sprintf(fileString, "/proc/%d", pid);
	int err = stat(fileString, &s);
	if(-1 == err) {
	    if(ENOENT == errno) {
	        return -1;
        }
	} 
    
	return 0;
}
void daemon_signal_handler(int sig) 
{
	if (sig == SIGTERM)
	{
        //fprintf(3, "SIGTERM catched and terminating program ");
		//unlink(PIDF);
		exit(0);
	}
}

void daemonize()
{
	pid_t pid, sid;
	signal(SIGTERM, daemon_signal_handler);
	pid = fork();
	if (pid < 0)
	{
		exit(EXIT_FAILURE);
	}
	if (pid > 0)
	{
		exit(EXIT_SUCCESS);
	}

	umask(0);
	sid = setsid();
	if (sid < 0) {
		exit(EXIT_FAILURE);
	}
	if ((chdir("/")) < 0) {
		exit(EXIT_FAILURE);
	}
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
}
void shutdown() {
    char cmd[256];
    int pid = loadPID();
    sprintf(cmd, "kill %d", pid);
    system(cmd);
}
int main(int argc, char **argv)
{
    struct PidfManager pm{PIDF};
    int c;
    int option_index = 0;
    static struct option long_options[] = {
        {"link", no_argument, 0, 0},
        {"start",  no_argument, 0, 1},
        {"close",  no_argument, 0, 2},
        {0,0,0,0}
    };
    
    c = getopt_long(argc, argv, "lsc", 
                    long_options, &option_index);
        
    if (c == -1) {
        printf("LanceX Daemon \n\n");
        printf("Commands: \n\n");
        printf("-s, --start       Start the daemon \n");
        printf("-l, --link        Link this device to LanceX servers \n");
        printf("-c, --close       Shutdown the daemon if one is running \n\n");
        exit(0);
    }
    
    switch(c) {
        case 0:
        case 'l':
            if (isRunning() == 0) {
                printf("Please close the lancex daemon by typing: \n");
                printf("lancexd --close \n");
                exit(0);
            }
            lancex::init();
            if (lancex::bind() < 0)
                exit(-1);
            
            printf("*** Now start the daemon ***: \n");
            printf("lancexd --start \n");
            break;
        case 1:           
        case 's':
            if (isRunning() == 0) {
                printf("There is already an instance of LanceX. Only one instance please \n");
                exit(-1);
            }
            if (!pm.locked) 
            {
                printf("Failed to lock /tmp/lancexd.pid \n");
                exit(-1);
            }
            
            lancex::init();
            if (lancex::link() < 0)
                exit(-1);
            printf("Starting... \n");
            daemonize();
            lancex::start();

            if (pm.writePid() < 0) 
            {
                exit(-1);        
            }
            while(1) {
		      lancex::Sleep(5);
            }
            break;
        case 2:
        case 'c':
            if (isRunning() == -1) {
                printf("LanceX is not running \n");
                exit(-1);
            }
            shutdown();
            
            break;
        default:
            printf("unrecognized option\n");
            exit(-1);
            break;
    }
     
    return 0;
}
