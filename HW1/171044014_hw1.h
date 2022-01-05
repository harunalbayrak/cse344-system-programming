#include<sys/stat.h>
#include<fcntl.h>
#include<dirent.h>
#include"171044014_helper_hw1.h"
#include<signal.h>
#include<stdio.h>

int count = 0;

void catchCtrlC(int sig_num){
    signal(SIGINT, catchCtrlC);
    printErrorExit("Exiting the program... (Because of CTRL+C interrupt)\n");
    fflush(stdout);
    fflush(stderr);
}

int checkFileStat(int depth,struct stat filestat, const char *filename, const char *fargs,const char *bargs, const char *targs, const char *pargs, const char *largs){
    int pflag = 1,fflag = 1,lflag = 1,bflag=1;

    if(largs != NULL){
        int lll = charArrayToInteger(largs);
        if(lll != filestat.st_nlink)
            lflag = 0;
    }

    if(bargs != NULL){
        int bbb = charArrayToInteger(bargs);
        if(bbb != filestat.st_size)
            bflag = 0;
    }

    if(fargs != NULL && compareStringRegex(filename,fargs) != 0)
        fflag = 0;

    if(pargs != NULL){
        if(((filestat.st_mode & S_IRUSR)? 'r':'-') != pargs[0])
            pflag = 0;
        if(((filestat.st_mode & S_IWUSR)? 'w':'-') != pargs[1])
            pflag = 0;
        if(((filestat.st_mode & S_IXUSR)? 'x':'-') != pargs[2])
            pflag = 0;
        if(((filestat.st_mode & S_IRGRP)? 'r':'-') != pargs[3])
            pflag = 0;
        if(((filestat.st_mode & S_IWGRP)? 'w':'-') != pargs[4])
            pflag = 0;
        if(((filestat.st_mode & S_IXGRP)? 'x':'-') != pargs[5])
            pflag = 0;
        if(((filestat.st_mode & S_IROTH)? 'r':'-') != pargs[6])
            pflag = 0;
        if(((filestat.st_mode & S_IWOTH)? 'w':'-') != pargs[7])
            pflag = 0;
        if(((filestat.st_mode & S_IXOTH)? 'x':'-') != pargs[8])
            pflag = 0;
    }
    
    /*if(pflag){
        printf("pflag:1");
    }*/

    if(targs == NULL){
        if(pflag && fflag && bflag && lflag){
            count++;
            printByDepth(depth);
            printWithNewline(filename);
        }
    }
    else{
        if( (compareString(targs,"s")==0 && S_ISSOCK(filestat.st_mode) )
            && (pflag) && (fflag) && (lflag) && (bflag) ){
            count++;
            printByDepth(depth);
            printWithNewline(filename);
        }
        else if( (compareString(targs,"b")==0 && S_ISBLK(filestat.st_mode))
            && (pflag) && (fflag) && (lflag) && (bflag) ){
            count++;
            printByDepth(depth);
            printWithNewline(filename);
        }
        else if( (compareString(targs,"c")==0 && S_ISCHR(filestat.st_mode))
            && (pflag) && (fflag) && (lflag) && (bflag) ){
            count++;
            printByDepth(depth);
            printWithNewline(filename);
        }
        else if((compareString(targs,"f")==0 && S_ISREG(filestat.st_mode)) 
            && (pflag) && (fflag) && (lflag) && (bflag) ){
            count++;
            printByDepth(depth);
            printWithNewline(filename);
        }
        else if( (compareString(targs,"p")==0 && S_ISFIFO(filestat.st_mode))
            && (pflag) && (fflag) && (lflag) && (bflag) ){
            count++;
            printByDepth(depth);
            printWithNewline(filename);
        }
        else if( (compareString(targs,"l")==0 && S_ISLNK(filestat.st_mode))
            && (pflag) && (fflag) && (lflag) && (bflag) ){
            count++;
            printByDepth(depth);
            printWithNewline(filename);
        }
    }

    return 0;
}

int checkTargs(struct stat filestat, const char *targs, const char *bargs, const char *largs, const char *pargs){
    int lflag=1,bflag=1,pflag=1;

    if(largs != NULL){
        int lll = charArrayToInteger(largs);
        if(lll != filestat.st_nlink)
            lflag = 0;
    }

    if(bargs != NULL){
        int bbb = charArrayToInteger(bargs);
        if(bbb != filestat.st_size)
            bflag = 0;
    }

    if(pargs != NULL){
        if(((filestat.st_mode & S_IRUSR)? 'r':'-') != pargs[0])
            pflag = 0;
        if(((filestat.st_mode & S_IWUSR)? 'w':'-') != pargs[1])
            pflag = 0;
        if(((filestat.st_mode & S_IXUSR)? 'x':'-') != pargs[2])
            pflag = 0;
        if(((filestat.st_mode & S_IRGRP)? 'r':'-') != pargs[3])
            pflag = 0;
        if(((filestat.st_mode & S_IWGRP)? 'w':'-') != pargs[4])
            pflag = 0;
        if(((filestat.st_mode & S_IXGRP)? 'x':'-') != pargs[5])
            pflag = 0;
        if(((filestat.st_mode & S_IROTH)? 'r':'-') != pargs[6])
            pflag = 0;
        if(((filestat.st_mode & S_IWOTH)? 'w':'-') != pargs[7])
            pflag = 0;
        if(((filestat.st_mode & S_IXOTH)? 'x':'-') != pargs[8])
            pflag = 0;
    }

    if(targs != NULL && (compareString(targs,"f")==0 && S_ISREG(filestat.st_mode)) && lflag && bflag && pflag){
        return 1;
    } else if(targs != NULL && (compareString(targs,"s")==0 && S_ISSOCK(filestat.st_mode)) && lflag && bflag && pflag){
        return 1;
    } else if(targs != NULL && (compareString(targs,"b")==0 && S_ISBLK(filestat.st_mode)) && lflag && bflag && pflag){
        return 1;
    } else if(targs != NULL && (compareString(targs,"c")==0 && S_ISCHR(filestat.st_mode)) && lflag && bflag && pflag){
        return 1;
    } else if(targs != NULL && (compareString(targs,"p")==0 && S_ISFIFO(filestat.st_mode)) && lflag && bflag && pflag){
        return 1;
    } else if(targs != NULL && (compareString(targs,"l")==0 && S_ISLNK(filestat.st_mode)) && lflag && bflag && pflag){
        return 1;
    } else if(targs != NULL && (compareString(targs,"d")==0 && S_ISDIR(filestat.st_mode)) && lflag && bflag && pflag){
        return 1;
    }

    if(targs == NULL && lflag && bflag && pflag){
        return 1;
    }

    return 0;
}

void checkUnderDirectories(int depth,const char *wargs, const char *fargs, const char *targs, const char *largs, const char *bargs, const char *pargs, const char *name){
    struct stat _file;
    DIR *_dir2;
    struct dirent *_dirent2;
    char path2[500];

    if((_dir2 = opendir(wargs)) == 0){
        closedir(_dir2);
        return;
    }

    while((_dirent2 = readdir(_dir2)) != 0){
        if(_dirent2->d_type == DT_DIR){
            if (compareString(_dirent2->d_name, ".") != 0 && compareString(_dirent2->d_name, "..") != 0){
                getNewPath(path2,wargs,_dirent2->d_name);

                if(fargs != NULL && (compareStringRegex(_dirent2->d_name,fargs) == 0)){
                    stat(path2, &_file);
                    if(checkTargs(_file,targs,bargs,largs,pargs)){
                        count++;
                        printByDepth(depth);
                        printWithNewline(name);
                    }
                } 

                checkUnderDirectories(depth,path2,fargs,targs,largs,bargs,pargs,name);
            }
        } else{
            if(fargs != NULL && compareStringRegex(_dirent2->d_name,fargs) == 0){
                if(checkTargs(_file,targs,bargs,largs,pargs)){
                    count++;
                    printByDepth(depth);
                    printWithNewline(name);
                }
                closedir(_dir2);
                return;
            }
        }
    }

    closedir(_dir2);
}

void getFiles(const char *wargs, const char *fargs, const char *bargs,
                        const char *targs, const char *pargs, const char *largs, int depth){
    struct stat _file;
    char path[500];
    DIR *_dir;
    struct dirent *_dirent;
    int checkStat=0;

    if((_dir = opendir(wargs)) == 0){
        closedir(_dir);
        return;
    }

    while((_dirent = readdir(_dir)) != 0){
        if(_dirent->d_type == DT_DIR){
            if (compareString(_dirent->d_name, ".") != 0 && compareString(_dirent->d_name, "..") != 0){
                getNewPath(path,wargs,_dirent->d_name);
                checkStat = stat(path, &_file);

                if(fargs != NULL && (compareStringRegex(_dirent->d_name,fargs) == 0)){
                    if(checkTargs(_file,targs,bargs,largs,pargs)){
                        count++;
                        printByDepth(depth);
                        printWithNewline(_dirent->d_name);
                    } 
                } else if(fargs == NULL){
                    if(checkTargs(_file,targs,bargs,largs,pargs)){
                        count++;
                        printByDepth(depth);
                        printWithNewline(_dirent->d_name);
                    } 
                }

                checkUnderDirectories(depth,path,fargs,targs,largs,bargs,pargs,_dirent->d_name);

                /* Recursive call */
                getFiles(path,fargs,bargs,targs,pargs,largs,depth+1);
            }
        } else{
            getNewPath(path,wargs,_dirent->d_name);
            checkStat = stat(path, &_file);

            if(checkStat == -1){
                closedir(_dir);
                return;
            } else{
                checkFileStat(depth,_file,_dirent->d_name,fargs,bargs,targs,pargs,largs);
            }
        }
    }

    closedir(_dir);
}

void handleCommandLine(int argc , char** argv){
    int opt=0;
    int wflag=0, fflag=0, bflag=0, tflag=0, pflag=0, lflag=0;
    const char *wargs=NULL, *fargs=NULL, *bargs=NULL, *targs=NULL, *pargs=NULL, *largs=NULL;

    while((opt = getopt(argc, argv, "w:f:b:t:p:l:")) != -1) { 
        switch(opt) { 
            case 'w': 
                wargs = optarg;
                wflag++;
                break;
            case 'f': 
                fargs = optarg;
                fflag++;
                break;
            case 'b': 
                if(isNumber(optarg) == -1){
                    printErrorExit("Wrong parameter argument: -b flag's argument has to be number.\n");
                }
                bargs = optarg;
                bflag++;
                break;
            case 't':
                if(!((compareString(optarg,"d")==0) || (compareString(optarg,"s")==0) 
                    || (compareString(optarg,"b")==0) || (compareString(optarg,"c")==0)
                    || (compareString(optarg,"f")==0) || (compareString(optarg,"p")==0)
                    || (compareString(optarg,"l")==0))){
                    printErrorExit("Wrong parameter argument: -t flag's argument has to be one of them 'd','s','b','c','f','p','l'.\n");
                }
                targs = optarg;
                tflag++;
                break;
            case 'p': 
                if(myStrlen(optarg) != 9)
                    printErrorExit("Wrong parameter argument: -p flag's argument has to have length of 9.\n");
                pargs = optarg;
                pflag++;
                break;
            case 'l': 
                if(isNumber(optarg) == -1){
                    printErrorExit("Wrong parameter argument: -l flag's argument has to be number.\n");
                }
                largs = optarg;
                lflag++;
                break;
        } 
    }

    if(wflag == 0){
        printErrorExit("error. -w yok\n");
    }

    if(wflag > 1 || fflag > 1 || bflag > 1 || tflag > 1 || pflag > 1 || lflag > 1){
        printErrorExit("Each flag can only use one time.\n");
    }

    printWithNewline(wargs);
    getFiles(wargs,fargs,bargs,targs,pargs,largs,0);
    if(count == 0){
        printWithNewline("No file found");
    }
}









