
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

// run with ./shell.out -nocolor for no colour

#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<libgen.h>
#include<sys/poll.h>
#include<sys/wait.h>
#include<string.h>
#include<assert.h>
#include<fcntl.h>
#include<signal.h>
#include<dirent.h>
#include<readline/readline.h>
#include<deque>
#include<vector>
#include<string>
#include<fstream>
#include<glob.h>

using namespace std;

#define INPUT_REDIRECT 1
#define OUTPUT_REDIRECT 2
#define MACHINE_NAME_MAX 200
#define MAX_PROGRAM_PATH_LEN 300
#define MAX_PROMPT_LEN 1000
#define HISTORY_LIMIT 1000

// for text colors
#define BLACK "\033[30m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define PURPLE "\033[35m"
#define CYAN "\033[36m"
#define WHITE "\033[37m"
#define DEFAULT "\033[0m"
#define BOLD "\033[1m"
#define NONBOLD "\033[m"

#define ITALIC "\033[3m"
#define UNDERLINE "\033[4m"

int EXECUTING_SOMETHING;
int RUN_IN_BACKGROUND;
char prompt_str[MAX_PROMPT_LEN];
int child_process_id;
bool NOCOLOR;

deque<string> bash_history;
size_t length = 0;
size_t idx = 0;
bool is_current = true;
char* temp = NULL;
char* sb_str = NULL;

// Control-a and Control-e

void prompt(char *);

int rl_b(int count, int key) {
    rl_point = 0;
    return 0;
}
int rl_e(int count, int key) {
    rl_point = rl_end;
    return 0;
}

int rl_hu(int count, int key) {
    if(is_current) {
        temp = rl_copy_text(0, rl_end);
        is_current = false;
    }

    {
        rl_begin_undo_group ();
        rl_point = 0;
        if (rl_point < rl_end)
        {
            rl_delete_text (rl_point, rl_end);
        }
        rl_mark = 0;
        rl_end_undo_group ();
    }

    if(idx > 0) --idx;
    if(length) rl_insert_text(bash_history[idx].c_str());
    else rl_insert_text(temp);


    return 0;
}

int rl_hd(int count, int key) {
    if(is_current) {
        temp = rl_copy_text(0, rl_end);
        is_current = false;
    }

    {
        rl_begin_undo_group ();
        rl_point = 0;
        if (rl_point < rl_end)
        {
            rl_delete_text (rl_point, rl_end);
        }
        rl_mark = 0;
        rl_end_undo_group ();
    }

    if(idx < length) ++idx;
    if(idx != length) rl_insert_text(bash_history[idx].c_str());
    else rl_insert_text(temp);
    return 0;
}

glob_t glob_matching(char** argn, int & wild_cards_found_in_argn){
    glob_t globbuf;
    globbuf.gl_offs = 0;
    int i=0;
    wild_cards_found_in_argn = 0;
    while(argn[i]){
        if(strstr(argn[i],"*")==NULL && strstr(argn[i],"?")==NULL){
            globbuf.gl_offs++;
        }
        else{
            wild_cards_found_in_argn = 1;
            break;
        }
        i++;
    }

    int no_arguments_matched = 1;
    if(argn[i]){
        if(!(GLOB_NOMATCH == glob(argn[i], GLOB_DOOFFS, NULL, &globbuf))){
            no_arguments_matched = 0;
        }
        i++;
        while(argn[i]){
            if(!(GLOB_NOMATCH == glob(argn[i], GLOB_DOOFFS | GLOB_APPEND, NULL, &globbuf))){
                no_arguments_matched = 0;
            }
            i++;
        }
        for(int j=0;j<globbuf.gl_offs;j++){
            globbuf.gl_pathv[j]=strdup(argn[j]);
        }
    }
    wild_cards_found_in_argn &= (no_arguments_matched ^ 1);
    return globbuf;
}

char * dot_dot_dot_ify(char * path){
    int i, slash_count = 0;
    for(i = strlen(path); i >= 0; --i){
        if(path[i] == '/'){
            ++slash_count;
        }
        if(slash_count == 3){
            break;
        }
    }
    if(i <= 0){
        return path;
    }
    char * new_path = (char *)malloc((4 + strlen(path + i)) * sizeof(char));
    new_path[0] = new_path[1] = new_path[2] = '.';
    int j = 3;
    for(;path[i]; ++i){
        new_path[j++] = path[i];
    }
    new_path[j] = '\0';
    free(path);
    return new_path;
}

void prompt(char * present_working_directory_str){

    char curr_machine_name[MACHINE_NAME_MAX + 1];
    gethostname(curr_machine_name, MACHINE_NAME_MAX + 1);
    prompt_str[0] = '\0';
    if(NOCOLOR){
    	sprintf(prompt_str + strlen(prompt_str), "[");
	    sprintf(prompt_str + strlen(prompt_str), "%s", curr_machine_name);
	    sprintf(prompt_str + strlen(prompt_str), "][");
	    sprintf(prompt_str + strlen(prompt_str), "%s", present_working_directory_str);
	    sprintf(prompt_str + strlen(prompt_str),"] # ");
	}else{
	    sprintf(prompt_str + strlen(prompt_str), DEFAULT);
    	sprintf(prompt_str + strlen(prompt_str), BOLD);
    	sprintf(prompt_str + strlen(prompt_str), "[");
    	sprintf(prompt_str + strlen(prompt_str), NONBOLD);
    	sprintf(prompt_str + strlen(prompt_str), GREEN);
	    sprintf(prompt_str + strlen(prompt_str), "%s", curr_machine_name);
	    sprintf(prompt_str + strlen(prompt_str), DEFAULT);
	    sprintf(prompt_str + strlen(prompt_str), BOLD);
	    sprintf(prompt_str + strlen(prompt_str), "][");
	    sprintf(prompt_str + strlen(prompt_str), NONBOLD);
	    sprintf(prompt_str + strlen(prompt_str), CYAN);
	    sprintf(prompt_str + strlen(prompt_str), "%s", present_working_directory_str);
	    sprintf(prompt_str + strlen(prompt_str), DEFAULT);
	    sprintf(prompt_str + strlen(prompt_str), BOLD);
	    sprintf(prompt_str + strlen(prompt_str),"] # ");
	    sprintf(prompt_str + strlen(prompt_str), NONBOLD);
	}

}

void print_error_and_exit(const char *message) {
    perror(message);
    exit(1);
}

void get_open_process_pids(const char *filepath, int *pids, int *count) {
    char path[1024];
    struct dirent   *dirent;
    DIR             *dirp;
    int             r;
    int             pid;

    // 1) find all processes
    if (!(dirp = opendir ("/proc"))) {
        fprintf(stderr, RED);
        perror ("couldn't open /proc");
        fprintf(stderr, DEFAULT);
    }

    while (dirent = readdir (dirp)) {
        // 2) we are only interested in process IDs
        if (isdigit (*dirent -> d_name)) {
            pid = atoi (dirent -> d_name);
            snprintf(path, sizeof(path), "/proc/%d/fd", pid);

            DIR* dir = opendir(path);
            if (dir == NULL) {
                continue;
            }

            struct dirent *ent;
            while ((ent = readdir(dir)) != NULL) {
                if (ent->d_type != DT_LNK) {
                    continue;
                }

                char link[1280];
                snprintf(link, sizeof(link), "%s/%s", path, ent->d_name);
                // printf("%s\n",link);
                char target[1280];
                int len = readlink(link, target, sizeof(target) - 1);
                if (len == -1) {
                    continue;
                }
                target[len] = '\0';
                // printf("%s\n",target);
                if (strcmp(target, filepath) == 0) {
                    pids[(*count)++] = pid;
                }
            }
            closedir(dir);
        }
    }
    closedir (dirp);
}

void get_locked_process_pids(const char *filepath, int *pids, int *count) {
    int fd = open(filepath, O_RDONLY);
    if (fd == -1) {
        fprintf(stderr, RED);
        perror("open");
        fprintf(stderr, DEFAULT);
    }

    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    lock.l_pid = 0;

    if (fcntl(fd, F_GETLK, &lock) == -1) {
        fprintf(stderr, RED);
        perror("fcntl");
        fprintf(stderr, DEFAULT);
    }

    close(fd);

    if (lock.l_type == F_UNLCK) {
        return;
    }

    pids[(*count)++] = lock.l_pid;
}

void delep(const char *filepath) {
    int open_pids[1024];
    int open_count = 0;
    get_open_process_pids(filepath, open_pids, &open_count);

    int locked_pids[1024];
    int locked_count = 0;
    get_locked_process_pids(filepath, locked_pids, &locked_count);

    printf("Open processes:\n");
    for (int i = 0; i < open_count; i++) {
        printf("  %d\n", open_pids[i]);
    }
    printf("Locked processes:\n");
    for (int i = 0; i < locked_count; i++) {
        printf("  %d\n", locked_pids[i]);
    }

    printf("Do you want to kill these processes and delete %s? [y/n]\n", filepath);
    char response[1024];
    fgets(response, sizeof(response), stdin);

    if (response[0] != 'y') {
        return;
    }

    for (int i = 0; i < open_count; i++) {
        if (kill(open_pids[i], SIGKILL) == -1) {
            fprintf(stderr, RED);
            perror("kill");
            fprintf(stderr, DEFAULT);
        }
    }

    for (int i = 0; i < locked_count; i++) {
        if (kill(locked_pids[i], SIGKILL) == -1) {
            fprintf(stderr, RED);
            perror("kill");
            fprintf(stderr, DEFAULT);
        }
    }

    if (unlink(filepath) == -1) {
        fprintf(stderr, RED);
        perror("unlink");
        fprintf(stderr, DEFAULT);
    }
}


void handling_CONTROL_C(int sig){
    assert(sig == SIGINT);
    printf("\n");
    if(!EXECUTING_SOMETHING){
        char * present_working_directory_str = NULL;
        present_working_directory_str = getcwd(present_working_directory_str,0);
        rl_on_new_line();
        rl_replace_line("", 0);
        rl_redisplay();
    }
}
void handling_CONTROL_Z(int sig){
    assert(sig == SIGTSTP);
    printf("\n");
    if(EXECUTING_SOMETHING){
        RUN_IN_BACKGROUND = 1;
        kill(child_process_id, SIGCONT);
    }else{
        char * present_working_directory_str = NULL;
        present_working_directory_str = getcwd(present_working_directory_str,0);
        rl_on_new_line();
        rl_replace_line("", 0);
        rl_redisplay();
    }
}


// executes command
void execute_command(char ** argn, int redirector_found, char * present_working_directory_str, char * input_redirection_file_name, char * output_redirection_file_name){
    if(argn[0] == NULL){
        fprintf(stderr, RED "Bad Command\n" DEFAULT);
        return;
    }
    if(strcmp(argn[0], "delep") == 0){
    	if(!(child_process_id = fork())){
	        int i=1;
	        int wild_cards_found_in_argn;
	        glob_t globbuf = glob_matching(argn, wild_cards_found_in_argn); // matches;
	        if(wild_cards_found_in_argn){
		        while(globbuf.gl_pathv[i]){
		            delep(globbuf.gl_pathv[i]);
		            i++;
		        }
	        }else{
		        while(argn[i]){
		            delep(argn[i]);
		            i++;
		        }
	        }
	    }else{
            while(!RUN_IN_BACKGROUND){ // while "&" or Ctrl - z was not encountered
                int status;
                if(child_process_id == waitpid(-1, &status, WNOHANG)){
                    break;
                }
            }
        }

        return;
    }
    if(strcmp(argn[0], "sb") == 0){
        free(argn[0]);
        char cmd[2048];
        sprintf(cmd,"%s/sb.out",sb_str);
        argn[0] = strdup(cmd);
    }
    if(strcmp(argn[0],"cd") && strcmp(argn[0],"pwd")){
        if(!(child_process_id = fork())){
            int input_redirection_file_fd = -1;
            int output_redirection_file_fd = -1;
            if(redirector_found & INPUT_REDIRECT){ // input redirection exists
                input_redirection_file_fd = open(input_redirection_file_name, O_RDONLY, 0);
                dup2(input_redirection_file_fd, STDIN_FILENO);
                close(input_redirection_file_fd);
            }
            if(redirector_found & OUTPUT_REDIRECT){ // output redirection exists
                output_redirection_file_fd = creat(output_redirection_file_name, 0644);
                dup2(output_redirection_file_fd, STDOUT_FILENO);
                close(output_redirection_file_fd);
            }

            // execvp(argn[0],argn);
            int wild_cards_found_in_argn;
            glob_t globbuf = glob_matching(argn, wild_cards_found_in_argn); // matches;

            if(wild_cards_found_in_argn){
                execvp(argn[0],&globbuf.gl_pathv[0]);
            }else{
                execvp(argn[0],argn);
            }
            fprintf(stderr, RED);
            perror("execvp");
            fprintf(stderr, DEFAULT);
            exit(1);
        }
        else{
            while(!RUN_IN_BACKGROUND){ // while "&" or Ctrl - z was not encountered
                int status;
                if(child_process_id == waitpid(-1, &status, WNOHANG)){
                    break;
                }
            }
        }
    }else{
        // printf("2\n");
        if(strcmp(argn[0],"cd")){
            if(redirector_found & OUTPUT_REDIRECT){ // output redirection exists (set output to file instead of stdout)
                FILE * output_redirection_file_ptr = fopen(output_redirection_file_name, "w");
                fprintf(output_redirection_file_ptr, "%s\n", present_working_directory_str);
                fclose(output_redirection_file_ptr);
            }else{
                printf("%s\n", present_working_directory_str);
            }
        }
        else{
            int val;
            if((val = chdir(argn[1] == NULL ? getenv("HOME") : argn[1])) < 0){
                fprintf(stderr, RED "ERROR: cd\n" DEFAULT);
            }
        }
    }
}

void open_history_file(char * history_file_path){
    length = 0;
    bash_history.clear();
    ifstream history_in(history_file_path);
    if(!history_in.is_open()){
        fprintf(stderr, YELLOW ITALIC "[ No history found ]\n" NONBOLD DEFAULT);
        return;
    }

    string history_line;
    while(getline(history_in, history_line)){
        bash_history.push_back(history_line);
        ++length;
    }


    history_in.close();
    idx = length - 1;
}

void save_history(char * history_file_path){

    ofstream history_out(history_file_path);
    for(string s : bash_history){
        history_out << s << '\n';
    }
    history_out.close();

}

int main(int argc, char ** argv){
    sb_str = getcwd(sb_str,0);
    signal(SIGINT , handling_CONTROL_C);
    signal(SIGTSTP, handling_CONTROL_Z);
    char * pipe_location[2];
    pipe_location[0] = strdup("/tmp/myfifo0");
    pipe_location[1] = strdup("/tmp/myfifo1");
    char * history_file_path = strdup("./g10_history.txt");
    rl_bind_key('\005', rl_e);
    rl_bind_key('\001', rl_b);
    rl_bind_keyseq("\033[A", rl_hu);
    rl_bind_keyseq("\033[B", rl_hd);

    NOCOLOR = false;
    for(int i = 0; i < argc; ++i){
    	if(strcmp(argv[i], "-nocolor") == 0){
    		NOCOLOR = true;
    		break;
    	}
    }


    open_history_file(history_file_path);


    while(1){

        EXECUTING_SOMETHING = 0;

        char * present_working_directory_str = NULL;
        present_working_directory_str = getcwd(present_working_directory_str,0);
        present_working_directory_str = dot_dot_dot_ify(present_working_directory_str);

        prompt(present_working_directory_str);

        is_current = true;
        char* buf=readline(prompt_str);

        char* buf2=NULL;

        EXECUTING_SOMETHING = 1;

        for(buf2 = buf; *buf2 == ' '; ++buf2);
        if(*buf2 == '\0'){ // empty command
            continue; // then repeat
        }

        bash_history.push_back(buf);
        ++length;
        if(length > HISTORY_LIMIT){
            bash_history.pop_front();
            --length;
        }
        idx = length;

        buf2=strdup(buf);
        char* token=NULL;
        int args=0;
        token=strtok(buf," ");


        //////////////////////////////////////////////////////////////////////////////////////////////////////////////
        char last_seen_redirector = '?';
        int redirector_found = 0;
        int duplicate_redirection_found = 0;
        char * output_redirection_file_name = NULL;
        char * input_redirection_file_name = NULL;
        RUN_IN_BACKGROUND = 0;
        while(token){
            if(strcmp(token, ">") == 0){ // redirect output
                if(redirector_found & OUTPUT_REDIRECT){ // output already redirected before
                    fprintf(stderr, RED "Duplicate output redirection!\n" DEFAULT);
                    duplicate_redirection_found = OUTPUT_REDIRECT;
                    break;
                }
                redirector_found |= OUTPUT_REDIRECT; // remembering that output redirection was found
                last_seen_redirector = '>';
            }else if(strcmp(token, "<") == 0){ // redirect input
                if(redirector_found & INPUT_REDIRECT){ // input already redirected before
                    fprintf(stderr, RED "Duplicate input redirection!\n" DEFAULT);
                    duplicate_redirection_found = INPUT_REDIRECT;
                    break;
                }
                redirector_found |= INPUT_REDIRECT; // remembering that input redirect was found
                last_seen_redirector = '<';
            }else{
                if(strcmp(token, "&") == 0){
                    RUN_IN_BACKGROUND = 1; // remember that we need to wait
                }else if(last_seen_redirector == '?' && !(token[strlen(token) - 1] == '\\' && (strlen(token) == 1 || token[strlen(token) - 2] != '\\'))){
                    ++args;
                }
                last_seen_redirector = '?';
            }
            token=strtok(NULL," ");
        }
        if(duplicate_redirection_found){ // if faulty command found (duplicate redirection), forget this command and re execute the shell
            continue;
        }
        if(last_seen_redirector != '?'){ // same if the redirection is improper
            fprintf(stderr, "No file found after \"%c\"\n", last_seen_redirector);
            continue;
        }



        char* argn[args+1];
        token=strtok(buf2," ");
        int j = 0;
        while(token){
            if(strcmp(token, ">") == 0){ // redirect output
                last_seen_redirector = '>';
            }else if(strcmp(token, "<") == 0){ // redirect input
                last_seen_redirector = '<';
            }else{
                if(last_seen_redirector == '>'){ // if output redirection is active
                    output_redirection_file_name = token;
                }else if(last_seen_redirector == '<'){ // if input redirection is active
                    input_redirection_file_name = token;
                }else if(strcmp(token, "&")){ // if this is not the
                    if(j && argn[j - 1][strlen(argn[j - 1]) - 1] == '\\' && (strlen(argn[j - 1]) == 1 || argn[j - 1][strlen(argn[j - 1]) - 2] != '\\')){
                        int escape_backslash_loc = strlen(argn[j - 1]) - 1;
                        argn[j - 1] = (char *)realloc(argn[j - 1], strlen(argn[j - 1]) + strlen(token) + 1);
                        strcat(argn[j - 1] , token);
                        argn[j - 1][escape_backslash_loc]=' ';
                    }else{
                        argn[j++] = strdup(token);
                    }
                }
                last_seen_redirector = '?';
            }
            token=strtok(NULL," ");
        }
        if(strcmp(argn[0], "exit") == 0){ // terminate on exit
            save_history(history_file_path);
            break;
        }
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        char ** pseudo_argn = argn;
        int pipe_count_till_now = 0;
        int dont_run = 0;
        int new_redirector_found = redirector_found & INPUT_REDIRECT;
        char * pipe_input_redirection_file_name = input_redirection_file_name, * pipe_output_redirection_file_name;
        for(int i = 0; i < args; ++i){
            if(strcmp(argn[i], ">") == 0){
                new_redirector_found |= OUTPUT_REDIRECT;
            }else if(strcmp(argn[i], "<") == 0){
                new_redirector_found |= INPUT_REDIRECT;
            }else if(strcmp(argn[i], "|") == 0){ // pipe found
                free(argn[i]);
                argn[i] = NULL;

                // take input from previous instruction
                if(pipe_count_till_now){
                    if(new_redirector_found & INPUT_REDIRECT){
                        fprintf(stderr, RED "Input redirect found along with pipe\n" DEFAULT);
                        dont_run = 1;
                        break;
                    }
                    new_redirector_found |= INPUT_REDIRECT;
                    pipe_input_redirection_file_name = pipe_location[pipe_count_till_now & 1];
                }

                // increase pipe_count
                ++pipe_count_till_now;

                // redirect output
                if(new_redirector_found & OUTPUT_REDIRECT){
                    fprintf(stderr, RED "Output redirect found along with pipe\n" DEFAULT);
                    dont_run = 1;
                    break;
                }
                new_redirector_found |= OUTPUT_REDIRECT;
                pipe_output_redirection_file_name = pipe_location[pipe_count_till_now & 1];


                execute_command(pseudo_argn, new_redirector_found, present_working_directory_str, pipe_input_redirection_file_name, pipe_output_redirection_file_name);

                new_redirector_found = 0;

                pseudo_argn = &argn[i + 1];
            }
        }
        if(dont_run){
            continue;
        }


        if(pipe_count_till_now){
            if(new_redirector_found & INPUT_REDIRECT){
                fprintf(stderr, RED "Input redirect found along with pipe\n" DEFAULT);
                dont_run = 1;
                continue;
            }
            redirector_found |= INPUT_REDIRECT;
            input_redirection_file_name = pipe_location[pipe_count_till_now & 1];
        }
        argn[args]=NULL;

        execute_command(pseudo_argn, redirector_found, present_working_directory_str, input_redirection_file_name, output_redirection_file_name);
    }
}
