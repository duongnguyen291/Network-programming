#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define NAME_LEN 64
#define SUBJECT_LEN 64
#define SEMESTER_LEN 32
#define LINE_LEN 256

typedef struct Student {
    char id[16];
    char first[NAME_LEN];
    char last[NAME_LEN];
    double progress;
    double final;
    char grade;
    struct Student *next;
} Student;

typedef struct ScoreBoard {
    char subjectID[SUBJECT_LEN];
    char subjectName[SUBJECT_LEN];
    char semester[SEMESTER_LEN];
    int weightProgress;
    int weightFinal;
    int studentCount;
    Student *students;
    struct ScoreBoard *next;
} ScoreBoard;

ScoreBoard *boards = NULL;

/* ===== Helper ===== */
void trim(char *s) {
    size_t len = strlen(s);
    size_t i = 0;
    while (i < len && isspace((unsigned char)s[i])) i++;
    size_t j = len;
    while (j > i && isspace((unsigned char)s[j-1])) j--;
    if (i > 0) memmove(s, s+i, j-i);
    s[j-i] = '\0';
}

int askContinue() {
    char choice;
    printf("Do you want to continue? (y/Y for yes, any other key for no): ");
    scanf(" %c", &choice);
    getchar(); // consume newline
    return (choice == 'y' || choice == 'Y');
}

char calculateGrade(double score) {
    if (score >= 8.5) return 'A';
    if (score >= 7.0) return 'B';
    if (score >= 5.5) return 'C';
    if (score >= 4.0) return 'D';
    return 'F';
}

/* ===== Linked list ===== */
ScoreBoard* findBoard(const char *subj, const char *sem) {
    for (ScoreBoard *p = boards; p; p = p->next) {
        if (strcmp(p->subjectID, subj)==0 && strcmp(p->semester, sem)==0)
            return p;
    }
    return NULL;
}

/* ===== File I/O ===== */
ScoreBoard* loadBoardFromFile(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        perror("Cannot open file");
        return NULL;
    }
    ScoreBoard *b = calloc(1, sizeof(ScoreBoard));
    char line[LINE_LEN];
    int loadedStudents = 0;
    
    while (fgets(line, sizeof line, f)) {
        trim(line);
        if (strncmp(line, "SubjectID|", 10)==0) {
            strcpy(b->subjectID, line+10);
        } else if (strncmp(line, "Subject|", 8)==0) {
            strcpy(b->subjectName, line+8);
        } else if (strncmp(line, "F|", 2)==0) {
            sscanf(line, "F|%d|%d", &b->weightProgress, &b->weightFinal);
        } else if (strncmp(line, "Semester|", 9)==0) {
            strcpy(b->semester, line+9);
        } else if (strncmp(line, "StudentCount|", 13)==0) {
            b->studentCount = atoi(line+13);
        } else if (line[0]=='S' && line[1]=='|') {
            // Check if we've already loaded the maximum number of students
            if (loadedStudents >= b->studentCount) {
                printf("Warning: File contains more students than declared count (%d). Skipping excess students.\n", b->studentCount);
                break;
            }
            
            Student *s = calloc(1, sizeof(Student));
            char prog[16], fin[16], grd[8];
            sscanf(line, "S|%15[^|]|%63[^|]|%63[^|]|%15[^|]|%15[^|]|%7[^|]|",
                   s->id, s->first, s->last, prog, fin, grd);
            s->progress = atof(prog);
            s->final = atof(fin);
            s->grade = grd[0];
            s->next = b->students;
            b->students = s;
            loadedStudents++;
        }
    }
    fclose(f);
    b->next = boards;
    boards = b;
    return b;
}

void saveBoardToFile(ScoreBoard *b) {
    char filename[128];
    snprintf(filename, sizeof filename, "%s_%s.txt", b->subjectID, b->semester);
    FILE *f = fopen(filename, "w");
    if (!f) { perror("Cannot write file"); return; }
    fprintf(f, "SubjectID|%s\n", b->subjectID);
    fprintf(f, "Subject|%s\n", b->subjectName);
    fprintf(f, "F|%d|%d\n", b->weightProgress, b->weightFinal);
    fprintf(f, "Semester|%s\n", b->semester);
    int count=0; for (Student *s=b->students;s;s=s->next) count++;
    fprintf(f, "StudentCount|%d\n", count);
    for (Student *s=b->students; s; s=s->next) {
        fprintf(f,"S|%s|%s|%s|%.1f|%.1f|%c|\n",
                s->id,s->first,s->last,s->progress,s->final,s->grade);
    }
    fclose(f);
}

/* ===== Menu functions ===== */
void addBoard() {
    do {
        char filename[128];
        printf("Enter filename to load board (ex: IT4062_20171.txt): ");
        fgets(filename, sizeof filename, stdin); trim(filename);
        loadBoardFromFile(filename);
        puts("Board loaded into system.");
    } while (askContinue());
}

void addStudent() {
    do {
        char subj[SUBJECT_LEN], sem[SEMESTER_LEN];
        printf("Enter subject ID: "); fgets(subj,sizeof subj,stdin); trim(subj);
        printf("Enter semester: "); fgets(sem,sizeof sem,stdin); trim(sem);
        ScoreBoard *b = findBoard(subj,sem);
        if (!b) { puts("Board not found."); continue; }
        
        // Count current students
        int currentCount = 0;
        for (Student *s = b->students; s; s = s->next) {
            currentCount++;
        }
        
        // Check if adding student would exceed the limit
        if (currentCount >= b->studentCount) {
            printf("Cannot add more students. Maximum capacity (%d) reached.\n", b->studentCount);
            continue;
        }
        
        Student *s = calloc(1,sizeof(Student));
        printf("Enter student ID: "); fgets(s->id,sizeof s->id,stdin); trim(s->id);
        printf("Enter first name: "); fgets(s->first,sizeof s->first,stdin); trim(s->first);
        printf("Enter last name: "); fgets(s->last,sizeof s->last,stdin); trim(s->last);
        printf("Enter progress mark: "); scanf("%lf",&s->progress);
        printf("Enter final mark: "); scanf("%lf",&s->final); getchar();
        double score = s->progress*b->weightProgress/100.0 + s->final*b->weightFinal/100.0;
        s->grade = calculateGrade(score);
        s->next = b->students; b->students=s;
        saveBoardToFile(b);
        printf("Student added with grade %c\n", s->grade);
    } while (askContinue());
}

void removeStudent() {
    do {
        char subj[SUBJECT_LEN], sem[SEMESTER_LEN], sid[16];
        printf("Enter subject ID: "); fgets(subj,sizeof subj,stdin); trim(subj);
        printf("Enter semester: "); fgets(sem,sizeof sem,stdin); trim(sem);
        printf("Enter student ID: "); fgets(sid,sizeof sid,stdin); trim(sid);
        ScoreBoard *b = findBoard(subj,sem);
        if (!b) { puts("Board not found."); continue; }
        Student *prev=NULL,*cur=b->students;
        while(cur){
            if(strcmp(cur->id,sid)==0){
                if(prev) prev->next=cur->next; else b->students=cur->next;
                free(cur);
                saveBoardToFile(b);
                puts("Student removed."); break;
            }
            prev=cur; cur=cur->next;
        }
        if(!cur) puts("Student not found.");
    } while (askContinue());
}

void searchStudent() {
    do {
        char subj[SUBJECT_LEN], sem[SEMESTER_LEN], sid[16];
        printf("Enter subject ID: "); fgets(subj,sizeof subj,stdin); trim(subj);
        printf("Enter semester: "); fgets(sem,sizeof sem,stdin); trim(sem);
        printf("Enter student ID: "); fgets(sid,sizeof sid,stdin); trim(sid);
        ScoreBoard *b=findBoard(subj,sem);
        if(!b){puts("Board not found."); continue;}
        Student *found = NULL;
        for(Student *s=b->students;s;s=s->next){
            if(strcmp(s->id,sid)==0){
                printf("%s %s %s | Progress=%.2f | Final=%.2f | Grade=%c\n",
                       s->id,s->first,s->last,s->progress,s->final,s->grade);
                found = s;
                break;
            }
        }
        if(!found) puts("Student not found.");
    } while (askContinue());
}

void displayBoard() {
    do {
        char subj[SUBJECT_LEN], sem[SEMESTER_LEN];
        printf("Enter subject ID: "); fgets(subj,sizeof subj,stdin); trim(subj);
        printf("Enter semester: "); fgets(sem,sizeof sem,stdin); trim(sem);
        ScoreBoard *b=findBoard(subj,sem);
        if(!b){puts("Board not found."); continue;}
        printf("Subject %s (%s), Semester %s\n", b->subjectID, b->subjectName,b->semester);
        puts("ID\tFirst\tLast\tProgress\tFinal\tGrade");
        Student *s=b->students; if(!s){puts("(no students)"); continue;}
        int count=0; double sum=0;
        Student *maxS=NULL,*minS=NULL;
        int hist[5]={0};
        for(;s;s=s->next){
            printf("%s\t%s\t%s\t%.2f\t%.2f\t%c\n",
                   s->id,s->first,s->last,s->progress,s->final,s->grade);
            double score=s->progress*b->weightProgress/100.0+s->final*b->weightFinal/100.0;
            if(!maxS||score>(maxS->progress*b->weightProgress/100.0+maxS->final*b->weightFinal/100.0))
                maxS=s;
            if(!minS||score<(minS->progress*b->weightProgress/100.0+minS->final*b->weightFinal/100.0))
                minS=s;
            sum+=score; count++;
            switch(s->grade){
                case 'A': hist[0]++; break;
                case 'B': hist[1]++; break;
                case 'C': hist[2]++; break;
                case 'D': hist[3]++; break;
                case 'F': hist[4]++; break;
            }
        }
        printf("\nThe student with the highest mark is: %s %s\n", maxS->first,maxS->last);
        printf("The student with the lowest mark is: %s %s\n", minS->first,minS->last);
        printf("The average mark is: %.2f\n", sum/count);
        printf("\nHistogram:\n");
        printf("A: %.*s\n", hist[0], "********************");
        printf("B: %.*s\n", hist[1], "********************");
        printf("C: %.*s\n", hist[2], "********************");
        printf("D: %.*s\n", hist[3], "********************");
        printf("F: %.*s\n", hist[4], "********************");

        // Write report file
        char rpt[128];
        snprintf(rpt,sizeof rpt,"%s_%s_rp.txt",b->subjectID,b->semester);
        FILE *rf=fopen(rpt,"w");
        if(rf){
            fprintf(rf,"The student with the highest mark is: %s %s\n",maxS->first,maxS->last);
            fprintf(rf,"The student with the lowest mark is: %s %s\n",minS->first,minS->last);
            fprintf(rf,"The average mark is: %.2f\n\n",sum/count);
            fprintf(rf,"A histogram of the subject %s is:\n",b->subjectID);
            fprintf(rf,"A:%.*s\n",hist[0],"********************");
            fprintf(rf,"B:%.*s\n",hist[1],"********************");
            fprintf(rf,"C:%.*s\n",hist[2],"********************");
            fprintf(rf,"D:%.*s\n",hist[3],"********************");
            fprintf(rf,"F:%.*s\n",hist[4],"********************");
            fclose(rf);
            printf("\nReport written to %s\n", rpt);
        }
    } while (askContinue());
}

/* ===== Main Menu ===== */
int main(void){
    for(;;){
        puts("\nLearning Management System");
        puts("-------------------------------------");
        puts("1. Add a new score board (load from file)");
        puts("2. Add score");
        puts("3. Remove score");
        puts("4. Search score");
        puts("5. Display score board and score report");
        puts("Other to quit");
        printf("Your choice: ");
        int choice;
        if(scanf("%d",&choice)!=1) break;
        getchar();
        switch(choice){
            case 1: addBoard(); break;
            case 2: addStudent(); break;
            case 3: removeStudent(); break;
            case 4: searchStudent(); break;
            case 5: displayBoard(); break;
            default: return 0;
        }
    }
    return 0;
}
