
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////                                  //////////////////////////////////
//////////////////////////////////      OPERATING SYSTEMS LAB       //////////////////////////////////
//////////////////////////////////           ASSIGNMENT 2           //////////////////////////////////
//////////////////////////////////                                  //////////////////////////////////
//////////////////////////////////    ARITRA  MITRA  (20CS30006)    //////////////////////////////////
//////////////////////////////////    SUBHAM  GHOSH  (20CS10065)    //////////////////////////////////
//////////////////////////////////    SHILADITYA DE  (20CS30061)    //////////////////////////////////
//////////////////////////////////    ANUBHAV  DHAR  (20CS30004)    //////////////////////////////////
//////////////////////////////////                                  //////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <bits/stdc++.h>
using namespace std;

int ids[10000];
int cnt = 1;
map<int, vector<int>> mp;

int getparent(char *s)
{
    int pid = 0;
    int pos = 0;
    int i = 0;
    while(pos < 3)
    {
        if(s[i] == ' ')
        {
            pos++;
        }
        i++;
    }
    while(s[i] != ' ')
    {
        pid = pid*10 + (int)(s[i] - '0');
        i++;
    }
    return pid;
}

void getparrecursive(char *p)
{
    char pid[10];
    while(1)
    {
        for(int i = 0; i<10; i++) pid[i] = '\0';
        strcpy(pid, p);
        char stat_file[30] = "/proc/"; 
        strcat(stat_file, pid);
        strcat(stat_file, "/stat");

        FILE* fp = fopen(stat_file, "r");
        if(fp == NULL)
        {
            printf("Could not open the stat file %s\n", stat_file);
            exit(1);
        }

        char *line = (char *)malloc(1000*sizeof(char));
        size_t mx_sz = 1000;
        getline(&line, &mx_sz, fp);
        fclose(fp);
        int t = getparent(line);
        printf("Parent of %s : %d\n", p, t);
        ids[cnt++] = t; 
        if(t == 1) break;
        sprintf(p, "%d", t);
    }
}

void gen_graph() {
  DIR *pd = opendir("/proc");
  if (pd == NULL) {
    perror("/proc file can't be opened\n");
    return;
  }

  struct dirent *e;
  while ((e = readdir(pd)) != NULL) {
    if (e->d_type != DT_DIR) {
      continue;
    }

    char *endptr;
    pid_t pid = strtol(e->d_name, &endptr, 10);
    if (*endptr != '\0') {
      continue;
    }

    char sfp[64]; //status file path
    sprintf(sfp,"/proc/%d/status", pid);

    FILE *stat_file = fopen(sfp, "r");
    if (stat_file == NULL) {
      continue;
    }

    char line[256];
    while (fgets(line, sizeof(line), stat_file) != NULL) {
      if (strncmp(line, "PPid:", 5) == 0) {
        pid_t parent_pid;
        sscanf(line + 5, "%d", &parent_pid);
        mp[parent_pid].push_back(pid);
        break;
      }
    }

    fclose(stat_file);
  }

  closedir(pd);
}

void dfs(map<int, vector<int>> &mp, int id, int &cnt)
{
	cnt++;
	if(mp.find(id) != mp.end())
	{
		for(auto u: mp[id])
		{
			dfs(mp, u, cnt);
		}
	}
}


int main(int argc, char * argv[])
{
    if(argc < 2)
    {
        printf("Enter the pid of suspected process\n");
    }
    else
    {
        int x;
        sscanf(argv[1], "%d", &x);
        ids[0] = x;
        cout << "Getting the process tree: \n";
        getparrecursive(argv[1]);
        cout << "\n";
    }
   if(argc == 3 && strcmp(argv[2], "-suggest") == 0){
     int c = 0, sum = 0;
     map<int, int> mp_cnt;
     while(1)
     {
       gen_graph();
       for(int i = 0; i<cnt; i++)
       {
         if(ids[i] >= 5000)
         {
           dfs(mp, ids[i], sum);
           mp_cnt[ids[i]] = sum;
           sum = 0;
         }
       }
       mp.clear();
       c++;
       if(c >= 8) break;
       sleep(60);
     }
     int final_ans = 0, fcnt = 0;
     for(auto v: mp_cnt)
     {
       if(v.second > 150 && v.second > fcnt)
       {
         final_ans = v.first; 
         fcnt = v.second;
       }
     }
     if(final_ans) cout << "Probable bug is: " << final_ans << endl;
     else
          cout << "No bugs. Your computer is safe\n";
   }
    return 0;
}
