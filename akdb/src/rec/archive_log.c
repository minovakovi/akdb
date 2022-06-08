
#include "archive_log.h"

/**
 * Function that creates a binary file that stores all commands that failed to
 * execute with a number that shows the size of how many commands failed.
 * @todo this function takes static filename to store the failed commands,
 *       create certain logic that would make the function to use dynamic filename
 *       (this is partly implemented inside AK_get_timestamp, but there is no logic
 *       that uses the last file when recovering - recovery.c)                                    
 * {link} recovery.c function test
 * @brief  Function for making archive log.
 * @author Dražen Bandić, update by Tomislav Turek
 * @return No retun value
 */
void AK_archive_log(int sig) {
    //AK_PRO;

    FILE *fp;

    AK_check_folder_archivelog();//poziv funkcije za provjeru postojanja archivelog foldera

    printf("AK_archive_log: Archiving redo log commands to separate file...\n");

    //otvaramo novu datoteku sa trenutnim timestampom u binarnom modu
    char* timestamp = AK_get_timestamp();

	//formiramo niz znakova koji ce postati ime datoteke u archivelog folderu
    char *destination = malloc(strlen(ARCHIVELOG_PATH)+strlen(timestamp)+2);
    strcpy(destination, ARCHIVELOG_PATH);
    strcat(destination, "/");
    strcat(destination, timestamp);

   if(access(destination,F_OK)!=-1){
	printf("archive_log filename already exists! Getting new filename...\n");
	timestamp = AK_get_timestamp();
	destination = NULL;
	destination = malloc(strlen(ARCHIVELOG_PATH)+strlen(timestamp)+2);
        strcpy(destination, ARCHIVELOG_PATH);
        strcat(destination, "/");
	strcat(destination, timestamp);
	printf("New archivelog filename sucessfully created! Continuing...\n");
	}
   else printf("archive_log filename is ok!...\n");
 

    fp = fopen(destination, "wb");//otvaramo datoteku u binarnom modu ya pisanje

    AK_free(timestamp);//dealociramo memoriju koju smo maloprije alocirali
	 
    AK_redo_log log;

    memcpy(log.command_recovery, redo_log->command_recovery, sizeof(log.command_recovery));

    log.number = redo_log->number;

    fwrite(&log, sizeof(log), 1, fp);
 
    fclose(fp);//yatvaramo datoteku
    AK_free(destination);	
    //AK_EPI;
    if(sig == SIGINT) {
	
        exit(EXIT_SUCCESS);
    }
}

/**
 * This function returns the current timestamp that could be concatenated to a 
 * log file in future usages.
 * 
 * @author Dražen Bandić main logic, replaced by Tomislav Turek
 * @brief Function that returns the current timestamp 
 * @todo Think about this in the future when creating multiple binary recovery
 *       files. Implementation gives the timestamp, but is not used anywhere for now.
 * @return char array in format day.month.year-hour:min:sec.usecu.bin
 */
//funkcija za dobavljanje trenutnog timestampa
char* AK_get_timestamp() {

    struct timeval tv;
    struct timezone tz;
    struct tm *tm;
    FILE *fp;

    AK_PRO;
    
    // dobavljanje vremena
    gettimeofday(&tv, &tz);
    tm=localtime(&tv.tv_sec);

    // dobavljanje timestampa
    char* log_name = malloc(30);
    sprintf(log_name, "%d.%d.%d-%d:%02d:%02d.%lu.bin", tm->tm_mday, tm->tm_mon+1,
            tm->tm_year+1900, tm->tm_hour, tm->tm_min, tm->tm_sec, tv.tv_usec);

    char *destination = malloc(strlen(ARCHIVELOG_PATH)+strlen("/latest.txt")+1);
    strcpy(destination, ARCHIVELOG_PATH);
    strcat(destination, "/latest.txt");// u datoteku latest.txt upisujemo ime zadnje kreirane datoteke unutar archivelog foldera
	
    // spremanje datoteke u odvojenu txt datoteku
    fp = fopen(destination, "w+");
    fprintf(fp, log_name);	
    fclose(fp);//yatvaranje datoteke
    
    AK_free(destination);//dealociranje memorije

    AK_EPI;
    return log_name;
  
}


//funkcija za provjeru postojanja foldera archivelog
int AK_check_folder_archivelog() {
  struct stat s;
  int err = stat(ARCHIVELOG_PATH, &s);
  if(-1 == err) {
      if(ENOENT == errno) {
		//izvrsava se ako folder ne postoji
          printf("[INFO] There is no folder for archive log. Creating folder at: %s\n", ARCHIVELOG_PATH);
          	#ifdef _WIN32 // nije testirano
        		return _mkdir(ARCHIVELOG_PATH);
    		#else
    		#if _POSIX_C_SOURCE
        		return mkdir(ARCHIVELOG_PATH, 0755);
    		#else
        		return mkdir "archivelog" 
    		#endif
    		#endif
      } else {
          perror("stat");
          exit(1);
      }
  } else {
      if(S_ISDIR(s.st_mode)) {
	//ako folder vec postoji
        printf("[INFO] Folder archivelog already exists\n");
      }else{
	//ako postoji nesto(datoteka), a to nije folder
	//kreira se folder s imenom archivelog, a datoteka s
	//navedenim imenom se brise
	char *answer;
	answer=malloc(sizeof(char)*1);
        printf("[INFO] Something exists, but it is not folder!\n");
	if(access(ARCHIVELOG_PATH,F_OK)!=-1){
	printf("[INFO] archivelog is file, delete file and create folder with this name?y/n\n");
	scanf(" %c",&answer);
	if(answer=='y'){	
		remove("archivelog");
		
		#ifdef _WIN32 // nije testirano
                        return _mkdir(ARCHIVELOG_PATH);
                #else
                #if _POSIX_C_SOURCE
                        return mkdir(ARCHIVELOG_PATH, 0755);
                #else
                        return mkdir "archivelog"
                #endif
                #endif

	}
	}
	AK_free(answer);//dealokacija memorije
      }
  }
  return 1;
}
