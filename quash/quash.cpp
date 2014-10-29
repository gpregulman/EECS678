#include <cstdlib>
#include <stdlib.h>
#include <sstream>
#include <cstring>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <cerrno>
#include <iostream>
#include <fstream>
#include <sys/wait.h>
#include <string>
#include <cstring>
#include <sys/types.h>
#include <vector>
#include <fcntl.h>
using namespace std;

bool fileExists(const char* filePath)
{
	struct stat fileAttribute;
	if(stat(filePath, &fileAttribute) != 0)
		perror(filePath);
	return S_ISREG(fileAttribute.st_mode);
}

bool dirExists(const char* filePath)
{
	struct stat fileAttribute;
	if(stat(filePath, &fileAttribute) != 0)
		perror(filePath);
	return S_ISDIR(fileAttribute.st_mode);
}

struct job
{
	string name;
	int id;
	pid_t pid;
};
vector<job*> jobList;
void printJobs();
void execution(vector<string> jobs, char **);
void back_execution(vector<string> jobs, char **);
vector<string> createCMD(string arg);
string getPath(string);

int main(int argc, char* argv[], char **envp)
{
	char* env = getenv("PATH");
	char* home = getenv("HOME");
	setenv("PATH", env, 1);
	bool background=false;
	string line;
	vector<string> command;
	char* cur = getenv("PWD");
	while(getline(cin, line))
	{
		for(int i = 0;i < jobList.size();i++)
		{
			int signal;
			if(waitpid(jobList[i]->pid, &signal, WNOHANG)>0){
				cout<<"["<<jobList[i]->id<<"] "<<jobList[i]->pid<<" finished "
					<<jobList[i]->name<<'\n';
				jobList.erase(jobList.begin()+i);
			}
		}
		if(line.at(line.length()-1)=='&'){
			background=true;
			line.erase(line.find('&'));
		}
		command = createCMD(line);
		int pipeVal=0;
		for(int i = 0;i < command.size() ;i++)
		{
			if(command[i]=="|"){
				pipeVal=i;
				break;
			}
		}
		int inIndex=0;
		for(int i = 0;i < command.size() ;i++)
		{
			if(command[i]=="<"){
				inIndex=i;
				break;
			}
		}
		int outIndex=0;
		for(int i = 0;i < command.size() ;i++)
		{
			if(command[i]==">"){
				outIndex=i;
				break;
			}
		}
		if(command[0] == "quit" || command[0]=="exit"){
			exit(0);
		}
		if(command[0] =="HOME"){
			cout<<getenv("HOME")<<'\n';
		}
		if(command[0] == "PATH"){
			cout<<getenv("PATH")<<'\n';
		}
		if(command[0]=="PWD")
			cout<<getenv("PWD")<<endl;
		if(command[0] == "cd"){
			if(command.size()==1){
				setenv("PWD", getenv("HOME"), 1);
			}
			else if(command.size()==2){
				if(dirExists(command[1].c_str())){
					setenv("PWD", command[1].c_str(), 1);
				}
			}
		}
		if(command[0]=="jobs"){
			printJobs();
		}
		if(command[0]=="set"){
			if(command.size()==1){
				cout<<"Set must have HOME= or PATH=\n";
			}
			string path = command[1].substr(5);
			if(strncmp(command[1].c_str(), "HOME=", 5)==0){
				if(dirExists(path.c_str()))
					setenv("HOME", path.c_str(), 1);
			}
			else if(strncmp(command[1].c_str(), "PATH", 5)==0){
				setenv("PATH", path.c_str(), 1);
			}
		}
		else
		{
			if(background){
				if(pipeVal==0 && inIndex==0 && outIndex==0)
					back_execution(command, envp);
				else if(pipeVal==0 && inIndex!=0 && outIndex==0){
					vector<string> left, right;
					for(int i = 0; i < inIndex;i++)
					{
						left.push_back(command[i]);
					}
					for(int i = inIndex+1;i < command.size();i++)
					{
						right.push_back(command[i]);
					}
					int fd = open(right[0].c_str(), O_RDONLY);
					dup2(fd, STDIN_FILENO);
					back_execution(left, envp);
				}
				else if(pipeVal==0 && inIndex==0 && outIndex!=0){
					vector<string> left, right;
					int fds[2];
					pipe(fds);
					for(int i = 0; i < outIndex;i++)
					{
						left.push_back(command[i]);
					}
					for(int i = outIndex+1;i < command.size();i++)
					{
						right.push_back(command[i]);
					}
					int fd = open(right[0].c_str(), O_WRONLY|O_APPEND);
					dup2(fd, STDOUT_FILENO);
					back_execution(left, envp);
				}
				else if(pipeVal!=0 && inIndex==0 && outIndex==0){
					int fds[2];
					pipe(fds);
					vector<string> ex1, ex2;
					for(int i = 0; i < pipeVal;i++)
					{
						ex1.push_back(command[i]);
					}
					for(int i = pipeVal+1;i < command.size();i++)
					{
						ex2.push_back(command[i]);
					}
					pid_t pid1, pid2;
					pid1=fork();
					if(pid1==0){
						dup2(fds[1], STDOUT_FILENO);
						back_execution(ex1, envp);
						close(fds[0]);
					}
					close(fds[1]);
					pid2=fork();
					if(pid2==0){
						dup2(fds[0], STDIN_FILENO);
						back_execution(ex2, envp);
						close(fds[1]);
					}
					close(fds[0]);
					close(fds[1]);
				}
			}
			else
				if(pipeVal==0 && inIndex==0 && outIndex==0)
					execution(command, envp);
				else if(pipeVal==0 && inIndex!=0 && outIndex==0){
					vector<string> left, right;
					for(int i = 0; i < inIndex;i++)
					{
						left.push_back(command[i]);
					}
					for(int i = inIndex+1;i < command.size();i++)
					{
						right.push_back(command[i]);
					}
					int fd = open(right[0].c_str(), O_RDONLY);
					dup2(fd, STDIN_FILENO);
					execution(left, envp);
				}
				else if(pipeVal==0 && inIndex==0 && outIndex!=0){
					vector<string> left, right;
					int fds[2];
					pipe(fds);
					for(int i = 0; i < outIndex;i++)
					{
						left.push_back(command[i]);
					}
					for(int i = outIndex+1;i < command.size();i++)
					{
						right.push_back(command[i]);
					}
					int fd = open(right[0].c_str(), O_WRONLY|O_APPEND);
					dup2(fd, STDOUT_FILENO);
					execution(left, envp);
				}
				else if(pipeVal!=0 && inIndex==0 && outIndex==0){
					int fds[2];
					pipe(fds);
					vector<string> left, right;
					for(int i = 0; i < pipeVal;i++)
					{
						left.push_back(command[i]);
					}
					for(int i = pipeVal+1;i < command.size();i++)
					{
						right.push_back(command[i]);
					}
					pid_t pid1, pid2;
					pid1=fork();
					if(pid1==0){
						dup2(fds[1], STDOUT_FILENO);
						execution(left, envp);
						close(fds[0]);
					}
					close(fds[1]);
					pid2=fork();
					if(pid2==0){
						dup2(fds[0], STDIN_FILENO);
						execution(right, envp);
						close(fds[1]);
					}
					close(fds[0]);
					close(fds[1]);
				}
			background==false;
		}
	}
}

void execution(vector<string> pro, char ** envp)
{
	bool exists;
	char ** argv = new char*[pro.size() + 1];
	for(int i = 0; i < pro.size();i++)
	{
		argv[i] = new char[pro[i].length() + 1];
		strcpy(argv[i], pro[i].c_str());
	}
	argv[pro.size()]=NULL;
	if(pro[0][0]=='/')
		exists = fileExists(pro[0].c_str());
	else if(!strncmp("./", pro[0].c_str(), 2))
	{
		pro[0].erase(0, 2);
		exists=fileExists(pro[0].c_str());
	}
	else
	{
		pro[0]=getPath(pro[0]);
		if(pro[0]!=""){
			cout<<pro[0]<<'\n';
			exists=true;
		}
		else
			exists=false;
	}
	if(exists)
	{
		pid_t pid = fork();
		if(pid<0)
			cout<<"Fork failed\n";
		else if(pid == 0)
			execve(pro[0].c_str(), argv, envp);
	}
	else
		cout<<"File not found\n";
	wait(NULL);
}

void back_execution(vector<string> pro, char ** envp)
{
	bool exists;
	char ** argv = new char*[pro.size() + 1];
	for(int i = 0; i < pro.size();i++)
	{
		argv[i] = new char[pro[i].length() + 1];
		strcpy(argv[i], pro[i].c_str());
	}
	argv[pro.size()]=NULL;
	if(pro[0][0]=='/')
		exists = fileExists(pro[0].c_str());
	else if(!strncmp("./", pro[0].c_str(), 2)){
		pro[0].erase(0, 2);
		exists=fileExists(pro[0].c_str());
	}
	else{
		pro[0]=getPath(pro[0]);
		if(pro[0]!=""){
			cout<<pro[0]<<'\n';
			exists=true;
		}
		else
			exists=false;
	}
	if(exists){
		pid_t pid = fork();
		if(pid<0)
			cout<<"Fork failed\n";
		else if(pid == 0)
			execve(pro[0].c_str(), argv, envp);
		job* back = new job();
		jobList.push_back(back);
		back->name=pro[0];
		back->id=jobList.size();
		back->pid=pid;
		cout<<"["<<back->id<<"] "<<back->pid<<" running in background\n";
	}
	else
		cout<<"File not found\n";
	
}

vector<string> createCMD(string arg)
{
	string iter;
	stringstream in;
	vector<string> cmd;
	in << arg;
	while(in >> iter)
	{
		cmd.push_back(iter);
	}
	return cmd;
}

void printJobs()
{
	if(jobList.empty())
		cout<<"No jobs running.\n";
	for(int i = 0; i < jobList.size(); i++)
	{
		cout<<jobList[i]->id<<" "<<jobList[i]->pid<<" "<<jobList[i]->name<<'\n';
	}
}

string getPath(string Executable)
{
	stringstream ss(getenv("PATH"));
	string tokens;
	vector<string> paths;
	while(getline(ss, tokens, ':'))
	{
		paths.push_back(tokens);
	}
		for(int i = 0; i < paths.size(); i++)
		{
			paths[i]+='/' + Executable;
			if(fileExists(paths[i].c_str()))
				return paths[i];
		}
	return "";
}
